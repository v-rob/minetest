/*
Minetest
Copyright (C) 2020 Vincent Robinson (v-rob) <robinsonvincent89@gmail.com>

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
*/

#pragma once

#include "irrlichttypes_extrabloated.h"
#include "guiInventoryList.h"
#include "guiScrollBar.h"
#include "guiTable.h"

//! Current state of various aspects of the formspec parser
struct ParserState
{
	u16 formspec_version = 1;
	bool explicit_size;
	bool real_coordinates;
	std::string focused_element;
	u8 simple_field_count;
	v2f invsize;
	v2s32 size;
	v2f32 offset;
	v2f32 anchor;
	core::rect<s32> rect;
	v2s32 basepos;
	v2u32 screensize;
	GUITable::TableOptions table_options;
	GUITable::TableColumns table_columns;
	gui::IGUIElement *current_parent = nullptr;

	GUIInventoryList::Options inventorylist_options;

	struct {
		s32 max = 1000;
		s32 min = 0;
		s32 small_step = 10;
		s32 large_step = 100;
		s32 thumb_size = 1;
		GUIScrollBar::ArrowVisibility arrow_visiblity = GUIScrollBar::DEFAULT;
	} scrollbar_options;

	// used to restore table selection/scroll/treeview state
	std::unordered_map<std::string, GUITable::DynamicData> table_dyndata;
};
