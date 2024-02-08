/*
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
*/

#pragma once

#include "irrlichttypes_extrabloated.h"
#include "gui/texture.h"
#include "util/basic_macros.h"

#include <array>
#include <iostream>
#include <string>

namespace ui
{
	// Serialized enum; do not change order of entries.
	enum class Align
	{
		START,
		CENTER,
		END,

		MAX_ALIGN,
	};

	Align toAlign(u8 align);

	// Serialized enum; do not change order of entries.
	enum class Spacing
	{
		BEFORE,
		AFTER,
		OUTSIDE,
		AROUND,
		BETWEEN,
		EVENLY,
		REMOVE,

		MAX_SPACING,
	};

	Spacing toSpacing(u8 spacing);

	struct Layer
	{
		Texture image;
		video::SColor fill;
		video::SColor tint;

		rf32 source;
		rf32 middle;
		float middle_scale;

		u32 num_frames;
		u32 frame_time;

		Layer()
		{
			reset();
		}

		void reset();
		void read(std::istream &is);
	};

	struct Style
	{
		d2f32 size;

		v2f32 rel_pos;
		v2f32 rel_anchor;
		d2f32 rel_size;

		rf32 margin;
		rf32 padding;

		v2f32 pos;
		d2f32 span;

		v2f32 gap;
		float weight;

		Spacing hspacing;
		Spacing vspacing;

		Layer bg;
		Layer fg;

		float fg_scale;
		Align fg_halign;
		Align fg_valign;

		bool visible;
		bool noclip;

		Style()
		{
			reset();
		}

		void reset();
		void read(std::istream &is);
	};

	class Elem;
	class Window;

	class Box
	{
	public:
		using State = u32;

		// These states are organized in order of precedence. States with a
		// larger value will override the styles of states with a lower value.
		static constexpr State STATE_NONE = 0;

		static constexpr State STATE_FOCUSED  = 1 << 0;
		static constexpr State STATE_SELECTED = 1 << 1;
		static constexpr State STATE_HOVERED  = 1 << 2;
		static constexpr State STATE_PRESSED  = 1 << 3;
		static constexpr State STATE_DISABLED = 1 << 4;

		static constexpr State NUM_STATES = 1 << 5;

		// For groups that are standalone or not part of any particular group,
		// this box group can be used.
		static constexpr u32 NO_GROUP = -1;

		// Represents a non-existent box, i.e. a box with a group of NO_GROUP
		// and an item of -1, which no box should use.
		static constexpr u64 NO_ID = -1;

	private:
		// Indicates that there is no style string for this state combination.
		static constexpr u32 NO_STYLE = -1;

		// The element, group, and item are intrinsic to the box's identity, so
		// they are set by the constructor and aren't cleared in reset() or
		// changed in read().
		Elem &m_elem;

		u32 m_group;
		u32 m_item;

		Style m_style;
		std::array<u32, NUM_STATES> m_style_refs;

		rf32 m_draw_rect;
		rf32 m_child_rect;
		rf32 m_clip_rect;

	public:
		Box(Elem &elem, u32 group, u32 item) :
			m_elem(elem),
			m_group(group),
			m_item(item)
		{
			reset();
		}

		DISABLE_CLASS_COPY(Box)
		ALLOW_CLASS_MOVE(Box)

		Elem &getElem() { return m_elem; }
		const Elem &getElem() const { return m_elem; }

		Window &getWindow();
		const Window &getWindow() const;

		u32 getGroup() const { return m_group; }
		u32 getItem() const { return m_item; }
		u64 getId() const { return ((u64)m_group << 32) | (u64)m_item; }

		const Style &getStyle() const { return m_style; }

		const rf32 &getDrawRect() const { return m_draw_rect; }
		const rf32 &getChildRect() const { return m_child_rect; }
		const rf32 &getChildClip() const { return m_clip_rect; }

		void reset();
		void read(std::istream &is);

		void layout(const rf32 &parent_rect, const rf32 &parent_clip);
		void draw(Canvas &parent);

		bool isPointerInside() const;
		bool processFullPress(const SDL_Event &event, void (*on_press)(Elem &));

	private:
		void drawForeground(Canvas &canvas);
		void drawLayer(Canvas &canvas, const Layer &layer, const rf32 &dst);

		bool isHovered() const;
		bool isPressed() const;

		void setPressed(bool pressed);
		void setHovered(bool hovered);

		void computeStyle();
	};
}
