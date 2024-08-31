-- Luanti
-- SPDX-License-Identifier: LGPL-2.1-or-later
-- Copyright (C) 2023 v-rob, Vincent Robinson <robinsonvincent89@gmail.com>

local prelude_theme = ui.Style {
	ui.Style "root" {
		-- The root window should be centered on the screen, but have no size
		-- by default, since the size should be user-set.
		rel_pos = {1/2, 1/2},
		rel_anchor = {1/2, 1/2},
		rel_size = {0, 0},

		-- Normally, we don't show the backdrop unless we have user focus.
		ui.Style "@backdrop" {
			visible = false,
		},
		ui.Style "@backdrop$focused" {
			visible = true,
		}
	},
	ui.Style "image" {
		icon_scale = 0,
	},
	ui.Style "check, switch, radio" {
		icon_place = "left",
	},
}

function ui.get_prelude_theme()
	return prelude_theme
end

local core_theme = ui.Style {
	-- Like any good theme should, we include the prelude theme first.
	prelude_theme,

	ui.Style "root" {
		ui.Style "!.ui:blank" {
			box_image = "ui_root.png",
			box_middle = {2/16, 2/16, 2/16, 2/16},
			box_scale = 2,
		},
		ui.Style "@backdrop" {
			box_fill = "black#5",
		},
	},

	ui.Style "button, toggle, option" {
		box_image = "ui_button.png",
		box_source = {0/6, 0, 1/6, 1},
		box_middle = {1/16, 1/16, 1/16, 2/16},
		box_scale = 2,

		ui.Style "$hovered, $focused" {
			box_source = {1/6, 0, 2/6, 1},
		},
		ui.Style "$disabled" {
			box_source = {2/6, 0, 3/6, 1},
		},
		ui.Style "$pressed, $selected" {
			box_source = {3/6, 0, 4/6, 1},
			box_middle = {1/16, 2/16, 1/16, 1/16},
		},
		ui.Style "($hovered, $focused)$selected" {
			box_source = {4/6, 0, 5/6, 1},
		},
		ui.Style "$disabled$selected" {
			box_source = {5/6, 0, 6/6, 1},
		},

		ui.Style ".ui:left" {
			box_image = "ui_button_left.png",
		},
		ui.Style ".ui:center" {
			box_image = "ui_button_center.png",
		},
		ui.Style ".ui:right" {
			box_image = "ui_button_right.png",
		},
	},

	ui.Style "check, switch, radio" {
		icon_source = {0/4, 0, 1/4, 1},
		icon_scale = 2,

		ui.Style "$hovered, $focused" {
			icon_source = {1/4, 0, 2/4, 1},
		},
		ui.Style "$pressed" {
			icon_source = {2/4, 0, 3/4, 1},
		},
		ui.Style "$disabled" {
			icon_source = {3/4, 0, 4/4, 1},
		},

		ui.Style "check" {
			icon_image = "ui_check.png",
		},
		ui.Style "check$selected" {
			icon_image = "ui_check_selected.png",
		},

		ui.Style "switch" {
			icon_image = "ui_switch.png",
		},
		ui.Style "switch$selected" {
			icon_image = "ui_switch_selected.png",
		},

		ui.Style "radio" {
			icon_image = "ui_radio.png",
		},
		ui.Style "radio$selected" {
			icon_image = "ui_radio_selected.png",
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
