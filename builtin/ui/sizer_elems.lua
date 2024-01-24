-- Luanti
-- SPDX-License-Identifier: LGPL-2.1-or-later
-- Copyright (C) 2024 v-rob, Vincent Robinson <robinsonvincent89@gmail.com>

ui.Place = ui._new_type(ui.Elem, "place", 0x05, false)

function ui.Place:_init(props)
	ui.Elem._init(self, props)

	self._scale = props.scale
end

function ui.Place:_encode_fields()
	local fl = ui._make_flags()

	if ui._shift_flag(fl, self._scale) then
		ui._encode_flag(fl, "f", self._scale)
	end

	return ui._encode("SZ", ui.Elem._encode_fields(self), ui._encode_flags(fl))
end

ui.Flex = ui._new_type(ui.Elem, "flex", 0x06, false)

function ui.Flex:_init(props)
	ui.Elem._init(self, props)

	self._dir = props.dir
	self._wrap = props.wrap
end

local dir_map = {left = 0, up = 1, right = 2, down = 3};
local wrap_map = {none = 0, forward = 1, backward = 2}

function ui.Flex:_encode_fields()
	local fl = ui._make_flags()

	if ui._shift_flag(fl, self._dir) then
		ui._encode_flag(fl, "B", dir_map[self._dir])
	end
	if ui._shift_flag(fl, self._wrap) then
		ui._encode_flag(fl, "B", wrap_map[self._wrap])
	end

	return ui._encode("SZ", ui.Elem._encode_fields(self), ui._encode_flags(fl))
end

ui.Grid = ui._new_type(ui.Elem, "grid", 0x07, false)

function ui.Grid:_init(props)
	ui.Elem._init(self, props)

	self._hsizes = table.merge(props.hsizes or {})
	self._vsizes = table.merge(props.vsizes or {})

	self._hweights = table.merge(props.hweights or {})
	self._vweights = table.merge(props.vweights or {})
end

function ui.Grid:_encode_fields()
	local fl = ui._make_flags()

	if ui._shift_flag(fl, #self._hsizes > 0) then
		ui._encode_flag(fl, "Z", ui._encode_array("f", self._hsizes))
	end
	if ui._shift_flag(fl, #self._vsizes > 0) then
		ui._encode_flag(fl, "Z", ui._encode_array("f", self._vsizes))
	end

	if ui._shift_flag(fl, #self._hweights > 0) then
		ui._encode_flag(fl, "Z", ui._encode_array("f", self._hweights))
	end
	if ui._shift_flag(fl, #self._vweights > 0) then
		ui._encode_flag(fl, "Z", ui._encode_array("f", self._vweights))
	end

	return ui._encode("SZ", ui.Elem._encode_fields(self), ui._encode_flags(fl))
end
