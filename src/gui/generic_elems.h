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

#pragma once

#include "irrlichttypes_extrabloated.h"
#include "gui/box.h"
#include "gui/elem.h"

#include <iostream>
#include <string>

namespace ui
{
	class Root : public Elem
	{
	private:
		Box m_backdrop_box;

		static constexpr u32 BACKDROP_BOX = 1;

	public:
		Root(Window &window, std::string id) :
			Elem(window, std::move(id)),
			m_backdrop_box(*this, Box::NO_GROUP, BACKDROP_BOX)
		{}

		virtual Type getType() const override { return ROOT; }

		virtual void reset() override;
		virtual void read(std::istream &is) override;

		virtual bool isBoxFocused(const Box &box) const override;

		// Note: We don't override isPointerInside() because we don't want to
		// count the backdrop in isPointerOutside() in the window.

	protected:
		virtual void layoutBoxes(const rf32 &parent_rect, const rf32 &parent_clip);

		virtual void draw(Canvas &canvas);
	};

	class Button : public Elem
	{
	private:
		// Serialized constants; do not change values of entries.
		static constexpr u32 ON_PRESS = 0x00;

		bool m_disabled;

	public:
		Button(Window &window, std::string id) :
			Elem(window, std::move(id))
		{}

		virtual Type getType() const override { return BUTTON; }

		virtual void reset() override;
		virtual void read(std::istream &is) override;

		virtual bool isBoxDisabled(const Box &box) const override { return m_disabled; }

		virtual bool processInput(const SDL_Event &event) override;

	private:
		void onPress();
	};

	class Toggle : public Elem
	{
	private:
		// Serialized constants; do not change values of entries.
		static constexpr u32 ON_PRESS = 0x00;
		static constexpr u32 ON_CHANGE = 0x01;

		bool m_disabled;
		bool m_selected = false; // Persistent

	public:
		Toggle(Window &window, std::string id) :
			Elem(window, std::move(id))
		{}

		virtual Type getType() const override { return TOGGLE; }

		virtual void reset() override;
		virtual void read(std::istream &is) override;

		virtual bool isBoxSelected(const Box &box) const override { return m_selected; }
		virtual bool isBoxDisabled(const Box &box) const override { return m_disabled; }

		virtual bool processInput(const SDL_Event &event) override;

	private:
		void onPress();
	};

	class Option : public Elem
	{
	private:
		// Serialized constants; do not change values of entries.
		static constexpr u32 ON_PRESS = 0x00;
		static constexpr u32 ON_CHANGE = 0x01;

		bool m_disabled;
		std::string m_family;

		bool m_selected = false; // Persistent

	public:
		Option(Window &window, std::string id) :
			Elem(window, std::move(id))
		{}

		virtual Type getType() const override { return OPTION; }

		virtual void reset() override;
		virtual void read(std::istream &is) override;

		virtual bool isBoxSelected(const Box &box) const override { return m_selected; }
		virtual bool isBoxDisabled(const Box &box) const override { return m_disabled; }

		virtual bool processInput(const SDL_Event &event) override;

	private:
		void onPress();
		void onChange(bool selected);
	};
}
