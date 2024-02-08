/*
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
*/

#include "gui/generic_elems.h"

#include "debug.h"
#include "log.h"
#include "gui/manager.h"
#include "util/serialize.h"

namespace ui
{
	void Root::reset()
	{
		Elem::reset();

		m_backdrop_box.reset();
	}

	void Root::read(std::istream &is)
	{
		auto super = newIs(readStr32(is));
		Elem::read(super);

		u32 set_mask = readU32(is);

		if (testShift(set_mask))
			m_backdrop_box.read(is);
	}

	bool Root::isBoxFocused(const Box &box) const
	{
		return box.getItem() == BACKDROP_BOX ? getWindow().isFocused() : isFocused();
	}

	void Root::layoutBoxes(const rf32 &parent_rect, const rf32 &parent_clip)
	{
		m_backdrop_box.layout(parent_rect, parent_clip);
		Elem::layoutBoxes(m_backdrop_box.getChildRect(), m_backdrop_box.getChildClip());
	}

	void Root::draw(Canvas &canvas)
	{
		m_backdrop_box.draw(canvas);
		Elem::draw(canvas);
	}

	void Button::reset()
	{
		Elem::reset();

		m_disabled = false;
	}

	void Button::read(std::istream &is)
	{
		auto super = newIs(readStr32(is));
		Elem::read(super);

		u32 set_mask = readU32(is);

		m_disabled = testShift(set_mask);

		if (testShift(set_mask))
			enableEvent(ON_PRESS);
	}

	bool Button::processInput(const SDL_Event &event)
	{
		return getMainBox().processFullPress(event, UI_CALLBACK(onPress));
	}

	void Button::onPress()
	{
		if (!m_disabled && testEvent(ON_PRESS)) {
			g_manager.sendMessage(createEvent(ON_PRESS).str());
		}
	}

	void Toggle::reset()
	{
		Elem::reset();

		m_disabled = false;
	}

	void Toggle::read(std::istream &is)
	{
		auto super = newIs(readStr32(is));
		Elem::read(super);

		u32 set_mask = readU32(is);

		m_disabled = testShift(set_mask);
		if (testShift(set_mask))
			m_selected = testShift(set_mask);

		if (testShift(set_mask))
			enableEvent(ON_PRESS);
		if (testShift(set_mask))
			enableEvent(ON_CHANGE);
	}

	bool Toggle::processInput(const SDL_Event &event)
	{
		return getMainBox().processFullPress(event, UI_CALLBACK(onPress));
	}

	void Toggle::onPress()
	{
		if (m_disabled) {
			return;
		}

		m_selected = !m_selected;

		// Send both a press and a change event since both occurred.
		if (testEvent(ON_PRESS)) {
			g_manager.sendMessage(createEvent(ON_PRESS).str());
		}
		if (testEvent(ON_CHANGE)) {
			auto os = createEvent(ON_CHANGE);
			writeU8(os, m_selected);

			g_manager.sendMessage(os.str());
		}
	}

	void Option::reset()
	{
		Elem::reset();

		m_disabled = false;
		m_family.clear();
	}

	void Option::read(std::istream &is)
	{
		auto super = newIs(readStr32(is));
		Elem::read(super);

		u32 set_mask = readU32(is);

		m_disabled = testShift(set_mask);
		if (testShift(set_mask))
			m_family = readNullStr(is);

		if (testShift(set_mask))
			m_selected = testShift(set_mask);

		if (testShift(set_mask))
			enableEvent(ON_PRESS);
		if (testShift(set_mask))
			enableEvent(ON_CHANGE);
	}

	bool Option::processInput(const SDL_Event &event)
	{
		return getMainBox().processFullPress(event, UI_CALLBACK(onPress));
	}

	void Option::onPress()
	{
		if (m_disabled) {
			return;
		}

		// Send a press event for this pressed option button.
		if (testEvent(ON_PRESS)) {
			g_manager.sendMessage(createEvent(ON_PRESS).str());
		}

		// Select this option button unconditionally.
		onChange(true);

		for (Elem *elem : getWindow().getElems()) {
			// Ignore all elements that aren't option buttons.
			if (elem->getType() != getType()) {
				continue;
			}

			// If the option button that was pressed has a family and this
			// option button is in that family, change its value accordingly.
			Option *option = (Option *)elem;
			if (option->m_family == m_family && !m_family.empty() && option != this) {
				option->onChange(false);
			}
		}
	}

	void Option::onChange(bool selected)
	{
		bool was_selected = m_selected;
		m_selected = selected;

		// If the state of the option button changed, send a change event.
		if (was_selected != m_selected && testEvent(ON_CHANGE)) {
			auto os = createEvent(ON_CHANGE);
			writeU8(os, m_selected);

			g_manager.sendMessage(os.str());
		}
	}
}
