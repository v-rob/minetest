-- Luanti
-- SPDX-License-Identifier: LGPL-2.1-or-later
-- Copyright (C) 2023 v-rob, Vincent Robinson <robinsonvincent89@gmail.com>

local prelude_theme = ui.Style {
	ui.Style "root" {
		pos = {1/2},
		size = {0},
		anchor = {1/2},

		clip = "overflow",

		ui.Style "@backdrop" {
			truncate = "both",
			hidden = true,
		},
		ui.Style "@backdrop$focused" {
			hidden = false,
		},
	},
}

function ui.get_prelude_theme()
	return prelude_theme
end

local core_theme = ui.Style {
	-- Like any good theme should, we include the prelude theme first.
	prelude_theme,

	ui.Style "root" {
		ui.Style "/gui/!%ui:hide, %ui:show" {
			img_pane = "ui_root.png",
			img_border = {2/16},
			img_scale = 2,
		},
		ui.Style "/gui/!%ui:hide@backdrop" {
			fill = "black#5",
		},
	},

	ui.Style "button, toggle, option" {
		padding = {2},

		img_pane = "ui_button.png",
		img_slice = {0/6, 0, 1/6, 1},
		img_border = {1/16, 1/16, 1/16, 2/16},
		img_scale = 2,

		ui.Style "$hovered, $focused" {
			img_slice = {1/6, 0, 2/6, 1},
		},
		ui.Style "$disabled" {
			img_slice = {2/6, 0, 3/6, 1},
		},
		ui.Style "$pressed, $selected" {
			img_slice = {3/6, 0, 4/6, 1},
			img_border = {1/16, 2/16, 1/16, 1/16},
		},
		ui.Style "($hovered, $focused)$selected" {
			img_slice = {4/6, 0, 5/6, 1},
		},
		ui.Style "$disabled$selected" {
			img_slice = {5/6, 0, 6/6, 1},
		},

		ui.Style "%ui:left" {
			img_pane = "ui_button_left.png",
		},
		ui.Style "%ui:center" {
			img_pane = "ui_button_center.png",
		},
		ui.Style "%ui:right" {
			img_pane = "ui_button_right.png",
		},
	},

	ui.Style "check, switch, radio" {
		img_slice = {0/4, 0, 1/4, 1},
		img_align = {0, 1/2},
		img_scale = 2,

		ui.Style "$hovered, $focused" {
			img_slice = {1/4, 0, 2/4, 1},
		},
		ui.Style "$pressed" {
			img_slice = {2/4, 0, 3/4, 1},
		},
		ui.Style "$disabled" {
			img_slice = {3/4, 0, 4/4, 1},
		},

		ui.Style "check, radio" {
			padding = {32, 0, 0, 0},
		},
		ui.Style "switch" {
			padding = {38, 0, 0, 0},
		},

		ui.Style "check" {
			img_overlay = "ui_check.png",
		},
		ui.Style "check$selected" {
			img_overlay = "ui_check_selected.png",
		},

		ui.Style "switch" {
			img_overlay = "ui_switch.png",
		},
		ui.Style "switch$selected" {
			img_overlay = "ui_switch_selected.png",
		},

		ui.Style "radio" {
			img_overlay = "ui_radio.png",
		},
		ui.Style "radio$selected" {
			img_overlay = "ui_radio_selected.png",
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
	default_theme = ui._req(theme, ui.Style)
end
