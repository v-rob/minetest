-- Luanti
-- SPDX-License-Identifier: LGPL-2.1-or-later
-- Copyright (C) 2023 v-rob, Vincent Robinson <robinsonvincent89@gmail.com>

ui.Window = core.class()

ui._window_types = {
	filter = 0,
	mask = 1,
	hud = 2,
	chat = 3,
	gui = 4,
}

function ui.Window:new(param)
	local function make_window(props)
		self:_init(props)
		return self
	end

	assert(type(param) == "string", "Window type is required")

	self._type = param
	assert(ui._window_types[self._type], "Invalid window type: '" .. param .. "'")

	return make_window
end

function ui.Window:_init(props)
	self._theme = props.theme or ui.get_default_theme()
	self._style = props.style or ui.Style {}

	self._root = props.root

	self._context = nil -- Set by ui.Context

	assert(core.is_instance(self._root, ui.Root),
			"Expected root of window to be ui.Root")

	self._elems = self._root:_get_flat()
	self._elems_by_id = {}

	for _, elem in ipairs(self._elems) do
		local id = elem._id

		assert(not self._elems_by_id[id], "Element has duplicate ID: '" .. id .. "'")
		self._elems_by_id[id] = elem

		assert(elem._window == nil, "Element '" .. elem._id .. "' already has a window")
		elem._window = self
	end
end

function ui.Window:_encode(player, opening)
	local enc_styles = self:_encode_styles()
	local enc_elems = self:_encode_elems()

	local data = ui._encode("ZzZ", enc_elems, self._root._id, enc_styles)
	if opening then
		data = ui._encode("ZB", data, ui._window_types[self._type])
	end

	return data
end

function ui.Window:_encode_styles()
	-- Clear out all the boxes in every element.
	for _, elem in ipairs(self._elems) do
		for box in pairs(elem._boxes) do
			elem._boxes[box] = {n = 0}
		end
	end

	-- Get a cascaded and flattened list of all the styles for this window.
	local styles = self:_get_full_style():_get_flat()

	-- Take each style and apply its properties to every box and state matched
	-- by its selector.
	self:_apply_styles(styles)

	-- Take the styled boxes and encode their styles into a single table,
	-- replacing the boxes' style property tables with indices into this table.
	local enc_styles = self:_index_styles()

	return ui._encode_array("Z", enc_styles)
end

function ui.Window:_get_full_style()
	-- The full style contains the theme, global style, and inline element
	-- styles as sub-styles, in that order, to ensure the correct precedence.
	local styles = {self._theme, self._style}

	for _, elem in ipairs(self._elems) do
		-- Cascade the inline style with the element's ID, ensuring that the
		-- inline style globally refers to this element only.
		table.insert(styles, ui.Style("#" .. elem._id) {elem._style})
	end

	-- Return all these styles wrapped up into a single style.
	return ui.Style {nested = styles}
end

local function apply_style(elem, boxes, style)
	-- Loop through each box, applying the styles accordingly. The table of
	-- boxes may be empty, in which case nothing happens.
	for _, box in pairs(boxes) do
		local name = box.name or "main"

		-- If this style resets all properties, find all states that are a
		-- subset of the state being styled and clear their property tables.
		if style._reset then
			for i = ui._STATE_NONE, ui._NUM_STATES - 1 do
				if bit.band(box.states, i) == box.states then
					elem._boxes[name][i] = nil
				end
			end
		end

		-- Get the existing style property table for this box if it exists.
		local props = elem._boxes[name][box.states] or {}

		-- Cascade the properties from this style onto the box.
		elem._boxes[name][box.states] = ui._cascade_props(style._props, props)
	end
end

function ui.Window:_apply_styles(styles)
	-- Loop through each style and element and see if the style properties can
	-- be applied to any boxes.
	for _, style in ipairs(styles) do
		for _, elem in ipairs(self._elems) do
			-- Check if the selector for this style. If it matches, apply the
			-- style to each of the applicable boxes.
			local matches, boxes = style._sel(elem)
			if matches then
				apply_style(elem, boxes, style)
			end
		end
	end
end

local function index_style(box, i, style_indices, enc_styles)
	-- If we have a style for this state, serialize it to a string. Identical
	-- styles have identical strings, so we use this to our advantage.
	local enc = ui._encode_props(box[i])

	-- If we haven't serialized a style identical to this one before, store
	-- this as the latest index in the list of style strings.
	if not style_indices[enc] then
		style_indices[enc] = #enc_styles
		table.insert(enc_styles, enc)
	end

	-- Set the index of our state to the index of its style string, and keep
	-- count of how many states with valid indices we have for this box so far.
	box[i] = style_indices[enc]
	box.n = box.n + 1
end

function ui.Window:_index_styles()
	local style_indices = {}
	local enc_styles = {}

	for _, elem in ipairs(self._elems) do
		for _, box in pairs(elem._boxes) do
			for i = ui._STATE_NONE, ui._NUM_STATES - 1 do
				if box[i] then
					-- If this box has a style, encode and index it.
					index_style(box, i, style_indices, enc_styles)
				else
					-- Otherwise, this state has no style, so set it as such.
					box[i] = ui._NO_STYLE
				end
			end
		end
	end

	return enc_styles
end

function ui.Window:_encode_elems()
	local enc_elems = {}

	for _, elem in ipairs(self._elems) do
		table.insert(enc_elems, elem:_encode())
	end

	return ui._encode_array("Z", enc_elems)
end

ui.Context = core.class()

local open_contexts = {}

local OPEN_WINDOW   = 0x00
local REOPEN_WINDOW = 0x01
local UPDATE_WINDOW = 0x02
local CLOSE_WINDOW  = 0x03

function ui.Context:new(builder, player, state)
	self._builder = builder
	self._player = player
	self._state = state

	self._id = nil
	self._window = nil
end

function ui.Context:open(param)
	if self:is_open() then
		return self
	end

	self:_open_window()
	self:_build_window(param)

	local data = ui._encode("BL Z", OPEN_WINDOW, self._id,
			self._window:_encode(self._player, true))

	core.send_ui_message(self._player, data)
	return self
end

function ui.Context:reopen(param)
	if not self:is_open() then
		return self
	end

	local close_id = self:_close_window()
	self:_open_window()
	self:_build_window(param)

	local data = ui._encode("BLL Z", REOPEN_WINDOW, self._id, close_id,
			self._window:_encode(self._player, true))

	core.send_ui_message(self._player, data)
	return self
end

function ui.Context:update(param)
	if not self:is_open() then
		return self
	end

	self:_build_window(param)

	local data = ui._encode("BL Z", UPDATE_WINDOW, self._id,
			self._window:_encode(self._player, false))

	core.send_ui_message(self._player, data)
	return self
end

function ui.Context:close()
	if not self:is_open() then
		return self
	end

	local close_id = self:_close_window()
	local data = ui._encode("BL", CLOSE_WINDOW, close_id)

	core.send_ui_message(self._player, data)
	return self
end

function ui.Context:is_open()
	return self._id ~= nil
end

function ui.Context:get_builder()
	return self._builder
end

function ui.Context:get_player()
	return self._player
end

function ui.Context:get_state()
	return self._state
end

function ui.Context:set_state(state)
	self._state = state
end

local last_id = 0

function ui.Context:_open_window()
	self._id = last_id
	last_id = last_id + 1

	open_contexts[self._id] = self
end

function ui.Context:_build_window(param)
	self._window = self._builder(self, self._player, self._state, param)

	assert(core.is_instance(self._window, ui.Window),
			"Expected ui.Window to be returned from builder function")
	assert(not self._window._context, "Window object has already been used")

	self._window._context = self
end

function ui.Context:_close_window()
	local close_id = self._id

	self._id = nil
	self._window = nil

	open_contexts[close_id] = nil
	return close_id
end

function ui.get_open_contexts()
	local contexts = {}
	for _, context in pairs(open_contexts) do
		table.insert(contexts, context)
	end
	return contexts
end

core.register_on_leaveplayer(function(player)
	for _, context in pairs(open_contexts) do
		if context:get_player() == player:get_player_name() then
			context:_close_window()
		end
	end
end)
