--[[
Minetest
Copyright (C) 2023 v-rob, Vincent Robinson <robinsonvincent89@gmail.com>

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

local prelude_theme = ui.Style{
	ui.Style{
		sel = "root",

		-- The root window should be centered on the screen, but have no size
		-- by default, since the size should be user-set.
		rel_pos = {1/2, 1/2},
		rel_anchor = {1/2, 1/2},
		rel_size = {0, 0},

		-- Normally, we don't show the backdrop unless we have user focus.
		ui.Style{
			sel = "@backdrop",
			visible = false,
		},
		ui.Style{
			sel = "@backdrop$focused",
			visible = true,
		}
	},
	ui.Style{
		sel = "image",

		-- We generally want images to take up as much space as possible.
		fg_scale = 0,
	},
	ui.Style{
		sel = "button, toggle, check, switch, option, radio",

		-- By default, buttons, toggle buttons, option buttons, and variants
		-- thereof have their image placed on the left side by default.
		fg_halign = "left",
	},
}

function ui.get_prelude_theme()
	return prelude_theme
end

local core_theme = ui.Style{
	-- Like any good theme should, we include the prelude theme first.
	prelude_theme,

	-- TODO
	ui.Style{
		sel = "root",

		bg_fill = "black#8C",

		ui.Style{
			sel = "@backdrop",

			bg_fill = "black#50",
		},
	},
}

function ui.get_core_theme()
	return core_theme
end

local default_theme = core_theme

function ui.get_default_theme()
	return default_theme
end

function ui.set_default_theme(theme)
	default_theme = theme
end
