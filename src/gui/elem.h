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
#include "gui/box.h"
#include "util/basic_macros.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

union SDL_Event;

namespace ui
{
	class Window;

#define UI_CALLBACK(method)                          \
	[](Elem &elem) {                                 \
		static_cast<decltype(*this)>(elem).method(); \
	}

	class Elem
	{
	public:
		// Serialized enum; do not change values of entries.
		enum Type
		{
			ELEM = 0x00,
			ROOT = 0x01,
			PLACE = 0x02,
			FLEX = 0x03,
			GRID = 0x04,
			BUTTON = 0x05,
			TOGGLE = 0x06,
			OPTION = 0x07,
		};

		// The main box is always the zeroth item in the Box::NO_GROUP group.
		static constexpr u32 MAIN_BOX = 0;

	private:
		// The window and ID are intrinsic to the element's identity, so they
		// are set by the constructor and aren't cleared in reset() or changed
		// in read().
		Window &m_window;
		std::string m_id;

		size_t m_order;

		Elem *m_parent;
		std::vector<Elem *> m_children;

		Box m_main_box;
		u64 m_hovered_box = Box::NO_ID; // Persistent
		u64 m_pressed_box = Box::NO_ID; // Persistent

		u32 m_events;

	public:
		static std::unique_ptr<Elem> create(Type type, Window &window, std::string id);

		Elem(Window &window, std::string id);

		DISABLE_CLASS_COPY(Elem)
		ALLOW_CLASS_MOVE(Elem)

		virtual ~Elem();

		Window &getWindow() { return m_window; }
		const Window &getWindow() const { return m_window; }

		const std::string &getId() const { return m_id; }
		virtual Type getType() const { return ELEM; }

		size_t getOrder() { return m_order; }
		void setOrder(size_t order) { m_order = order; }

		Elem *getParent() { return m_parent; }
		const std::vector<Elem *> &getChildren() { return m_children; }

		Box &getMainBox() { return m_main_box; }

		u64 getHoveredBox() const { return m_hovered_box; }
		u64 getPressedBox() const { return m_pressed_box; }

		void setHoveredBox(u64 id) { m_hovered_box = id; }
		void setPressedBox(u64 id) { m_pressed_box = id; }

		virtual void reset();
		virtual void read(std::istream &is);

		void layout(const rf32 &parent_rect, const rf32 &parent_clip);
		void drawAll(Canvas &canvas);

		bool isFocused() const;

		virtual bool isBoxFocused (const Box &box) const { return isFocused(); }
		virtual bool isBoxSelected(const Box &box) const { return false; }
		virtual bool isBoxHovered (const Box &box) const { return box.getId() == m_hovered_box; }
		virtual bool isBoxPressed (const Box &box) const { return box.getId() == m_pressed_box; }
		virtual bool isBoxDisabled(const Box &box) const { return false; }

		virtual bool isPointerInside() const { return m_main_box.isPointerInside(); }
		virtual bool processInput(const SDL_Event &event) { return false; }

	protected:
		void enableEvent(u32 event);
		bool testEvent(u32 event) const;

		std::ostringstream createEvent(u32 event) const;

		virtual void layoutBoxes(const rf32 &parent_rect, const rf32 &parent_clip);
		virtual void layoutChildren();

		virtual void draw(Canvas &canvas);

	private:
		void readChildren(std::istream &is);
	};
}
