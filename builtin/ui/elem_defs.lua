--[[
Minetest
Copyright (C) 2024 v-rob, Vincent Robinson <robinsonvincent89@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
--]]

ui.Root = ui._new_type(ui.Elem, "root", 0x01, false)

function ui.Root:new(props)
	ui.Elem.new(self, props)

	self._boxes.backdrop = true
end

function ui.Root:_encode_fields()
	local fl = ui._make_flags()

	self:_encode_box(fl, self._boxes.backdrop)

	return ui._encode("SZ", ui.Elem._encode_fields(self), ui._encode_flags(fl))
end

ui.Place = ui._new_type(ui.Elem, "place", 0x02, false)

function ui.Place:new(props)
	ui.Elem.new(self, props)

	self._scale = props.scale
end

function ui.Place:_encode_fields()
	local fl = ui._make_flags()

	if ui._shift_flag(fl, self._scale) then
		ui._encode_flag(fl, "f", self._scale)
	end

	return ui._encode("SZ", ui.Elem._encode_fields(self), ui._encode_flags(fl))
end

ui.Flex = ui._new_type(ui.Elem, "flex", 0x03, false)

function ui.Flex:new(props)
	ui.Elem.new(self, props)

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

ui.Grid = ui._new_type(ui.Elem, "grid", 0x04, false)

function ui.Grid:new(props)
	ui.Elem.new(self, props)

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

ui.Button = ui._new_type(ui.Elem, "button", 0x05, true)

function ui.Button:new(props)
	ui.Elem.new(self, props)

	self._disabled = props.disabled
	self._on_press = props.on_press
end

function ui.Button:_encode_fields()
	local fl = ui._make_flags()

	ui._shift_flag(fl, self._disabled)
	ui._shift_flag(fl, self._on_press)

	return ui._encode("SZ", ui.Elem._encode_fields(self), ui._encode_flags(fl))
end

ui.Button._handlers[0x00] = function(self, ev, data)
	return self._on_press
end

ui.Toggle = ui._new_type(ui.Elem, "toggle", 0x06, true)

ui.Check  = ui.derive_elem(ui.Toggle, "check")
ui.Switch = ui.derive_elem(ui.Toggle, "switch")

function ui.Toggle:new(props)
	ui.Elem.new(self, props)

	self._disabled = props.disabled
	self._selected = props.selected

	self._on_press = props.on_press
	self._on_change = props.on_change
end

function ui.Toggle:_encode_fields()
	local fl = ui._make_flags()

	ui._shift_flag(fl, self._disabled)
	if ui._shift_flag(fl, self._selected ~= nil) then
		ui._shift_flag(fl, self._selected)
	end

	ui._shift_flag(fl, self._on_press)
	ui._shift_flag(fl, self._on_change)

	return ui._encode("SZ", ui.Elem._encode_fields(self), ui._encode_flags(fl))
end

ui.Toggle._handlers[0x00] = function(self, ev, data)
	return self._on_press
end

ui.Toggle._handlers[0x01] = function(self, ev, data)
	local selected = ui._decode("B", data)
	ev.selected = selected ~= 0

	return self._on_change
end

ui.Option = ui._new_type(ui.Elem, "option", 0x07, true)

ui.Radio = ui.derive_elem(ui.Option, "radio")

function ui.Option:new(props)
	ui.Elem.new(self, props)

	self._disabled = props.disabled
	self._family = props.family

	self._selected = props.selected

	self._on_press = props.on_press
	self._on_change = props.on_change

	if self._family then
		assert(ui.is_id(self._family), "Element family must be an ID string")
	end
end

function ui.Option:_encode_fields()
	local fl = ui._make_flags()

	ui._shift_flag(fl, self._disabled)
	if ui._shift_flag(fl, self._family) then
		ui._encode_flag(fl, "z", self._family)
	end

	if ui._shift_flag(fl, self._selected ~= nil) then
		ui._shift_flag(fl, self._selected)
	end

	ui._shift_flag(fl, self._on_press)
	ui._shift_flag(fl, self._on_change)

	return ui._encode("SZ", ui.Elem._encode_fields(self), ui._encode_flags(fl))
end

ui.Option._handlers[0x00] = function(self, ev, data)
	return self._on_press
end

ui.Option._handlers[0x01] = function(self, ev, data)
	local selected = ui._decode("B", data)
	ev.selected = selected ~= 0

	return self._on_change
end
