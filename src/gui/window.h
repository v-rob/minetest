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
#include "gui/elem.h"
#include "util/basic_macros.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

union SDL_Event;

namespace ui
{
	// Serialized enum; do not change order of entries.
	enum class WindowType
	{
		BG,
		MASK,
		HUD,
		MESSAGE,
		GUI,
		FG,

		MAX_TYPE,
	};

	WindowType toWindowType(u8 type);

	class Window
	{
	private:
		// Serialized constants; do not change values of entries.
		static constexpr u32 ON_CLOSE = 0x00;
		static constexpr u32 ON_SUBMIT = 0x01;
		static constexpr u32 ON_FOCUS_CHANGE = 0x02;

		static constexpr size_t MAX_TREE_DEPTH = 64;

		u64 m_id;
		WindowType m_type = WindowType::GUI;

		std::unordered_map<std::string, std::unique_ptr<Elem>> m_elems;
		std::vector<Elem *> m_ordered_elems;

		Elem *m_root_elem;

		std::vector<std::string> m_style_strs;

		Elem *m_focused_elem;
		Elem *m_hovered_elem;

		bool m_uncloseable;
		u32 m_events;

	public:
		Window(u64 id) :
			m_id(id)
		{
			reset();
		}

		DISABLE_CLASS_COPY(Window)
		ALLOW_CLASS_MOVE(Window)

		u64 getId() const { return m_id; }

		WindowType getType() const { return m_type; }

		const std::vector<Elem *> &getElems() { return m_ordered_elems; }

		Elem *getElem(const std::string &id, bool required);
		Elem *getNextElem(Elem *elem, bool reverse);

		Elem *getRoot() { return m_root_elem; }
		Elem *getFocused() { return m_focused_elem; }
		Elem *getHovered() { return m_hovered_elem; }

		void clearElem(Elem *elem);

		const std::string *getStyleStr(u32 index) const;

		void reset();
		void read(std::istream &is, bool opening);

		float getPixelSize() const;
		d2f32 getScreenSize() const;
		v2f32 getPointerPos() const;

		void drawAll();

		bool isFocused() const;
		bool processInput(const SDL_Event &event);

	private:
		void enableEvent(u32 event);
		bool testEvent(u32 event) const;

		std::ostringstream createEvent(u32 event) const;

		// Warning: This method causes the window object to be destroyed.
		// Return immediately after use, and don't use the window object again.
		void close();

		Elem *sendTreeInput(Elem *elem, const SDL_Event &event, bool direct);
		Elem *sendHoveredInput(const SDL_Event &event);

		Elem *sendFocusedInput(const SDL_Event &event);

		void changeFocusedElem(Elem *new_focused, bool send_event);
		bool requestFocusedElem(Elem *new_focused, bool send_event);

		void focusNextElem(bool reverse);

		void updateFocusedElem();
		void updateHoveredElem();

		bool isPointerOutside() const;

		void readElems(std::istream &is,
				std::unordered_map<Elem *, std::string> &elem_contents);
		void readRootElem(std::istream &is);
		void readStyles(std::istream &is);

		void updateElems(std::unordered_map<Elem *, std::string> &elem_contents,
				bool set_focus, Elem *new_focused);
		bool checkTree(Elem *elem, size_t depth) const;
		size_t updateElemOrdering(Elem *elem, size_t order);
	};
}
