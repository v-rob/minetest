if not rawget(_G, "ui") then
	return
end

local hex = "0123456789ABCDEF"
local special = {
	[string.byte("\\")] = "\\\\",
	[string.byte("\"")] = "\\\"",
	[string.byte("\a")] = "\\a",
	[string.byte("\b")] = "\\b",
	[string.byte("\f")] = "\\f",
	[string.byte("\n")] = "\\n",
	[string.byte("\r")] = "\\r",
	[string.byte("\t")] = "\\t",
	[string.byte("\v")] = "\\v",
}

function quote(str)
	local out = {}

	for i = 1, #str do
		local byte = str:byte(i)
		if special[byte] then
			out[#out + 1] = special[byte]
		elseif byte >= 32 and byte < 127 then
			out[#out + 1] = str:sub(i, i)
		else
			local lo = bit.band(byte, 0x0F) + 1
			local hi = bit.rshift(bit.band(byte, 0xF0), 4) + 1
			out[#out + 1] = "\\x" .. hex:sub(hi, hi) .. hex:sub(lo, lo)
		end
	end

	return '"' .. table.concat(out) .. '"'
end

--ui.set_default_theme(ui.get_prelude_theme())

local function gui_builder(context, player, state, param)
	local c = ui.get_coord_size()

	return ui.Window "gui" {
		ui.Style "#a$hovered" {
			prepend = "Howdy\n",
			text_italic = true,
		},
		ui.Style "?>(button)" {
			text_valign = "top",
			append = "ASDASD",
		},

		root = ui.Root {
			scale = c,

			min = {12 * c, 7 * c},
			--size = {1/2, 1/2},

			ui.Button "a" {
				pos = {0, 0},
				size = {0, 5},
				margin = {2, 2, 2, 2},

				min = {20, 20},

				fill = "brown#8C",

--				label = minetest.colorize("yellow", ""),
--				prepend = "Hello\n",
--				append = "\n\nGoodbye\nexample text",
				label = "Hello\n" .. minetest.colorize("yellow", "hi") .. "\n\nGoodbye\nexample text",

				text_size = 18,
				text_bold = true,

				text_color = "green",
				text_mark = "blue#8C",

				text_align = "center",

				on_press = function(ev)
					core.chat_send_player(player, "Pressed button")
				end,
			},

			ui.Check "b" {
				pos = {4, 0.125},
				size = {0, 0},

				label = "Attach thing",

				on_change = function(ev)
					core.chat_send_player(player,
							"Set switch to " .. tostring(ev.selected))
				end,
			},

			ui.Radio "c" {
				pos = {4, 0.75},
				size = {0, 0},

				label = "Host server",

				on_change = function(ev)
					core.chat_send_player(player,
							"Set checkbox to " .. tostring(ev.selected))
				end,
			},

			ui.Switch "d" {
				pos = {4, 1.375},
				size = {0, 0},

				label = "Host server",

				on_change = function(ev)
					core.chat_send_player(player,
							"Set checkbox to " .. tostring(ev.selected))
				end,
			},
		},

		on_close = function(ev)
			core.chat_send_player(player, "Closed window")
		end,
	}
end

local function popup_builder(context, player, state, param)
	return ui.Window "gui" {
		root = ui.Root {
			min = {100, 100},
		},
	}
end

local function show(player)
	local context = ui.Context(gui_builder, player, {x = true})
	context:open({init = true})
	core.chat_send_player(player, "Opened window")

--	core.after(2, function() ui.Context(gui_builder, player):open() end)
	core.after(2, function() context:update(); core.chat_send_player(player, "Updated window")  end)
--	core.after(4, function() context:reopen(); core.chat_send_player(player, "Reopened window") end)
--	core.after(6, function() context:close();  core.chat_send_player(player, "Closed window")   end)

--	core.after(3, function() ui.Context(popup_builder, player):open(); core.chat_send_player(player, "Opened second window")  end)

--	core.show_formspec(player, "testui:test",
--			"formspec_version[3]size[4,4]button[1,1;2,2;btn;Button]")
end

core.register_on_joinplayer(function(player)
	show(player:get_player_name())
end)

core.register_chatcommand("ui", {
	func = function(name, param)
		show(name)
	end,
})
