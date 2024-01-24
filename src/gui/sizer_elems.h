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

#include <array>
#include <iostream>
#include <string>
#include <vector>

namespace ui
{
	class Place : public Elem
	{
	private:
		float m_scale;

	public:
		Place(Window &window, std::string id) :
			Elem(window, std::move(id))
		{}

		virtual Type getType() const override { return PLACE; }

		virtual void reset() override;
		virtual void read(std::istream &is) override;

	protected:
		virtual void layoutChildren() override;
	};

	// Serialized enum; do not change order of entries.
	enum class Dir
	{
		LEFT,
		UP,
		RIGHT,
		DOWN,

		MAX_DIR,
	};

	Dir toDir(u8 dir);

	// Serialized enum; do not change order of entries.
	enum class Wrap
	{
		NONE,
		FORWARD,
		BACKWARD,

		MAX_WRAP,
	};

	Wrap toWrap(u8 wrap);

	class Flex : public Elem
	{
	private:
		Dir m_dir;
		Wrap m_wrap;

	public:
		Flex(Window &window, std::string id) :
			Elem(window, std::move(id))
		{}

		virtual Type getType() const override { return FLEX; }

		virtual void reset() override;
		virtual void read(std::istream &is) override;

	protected:
		virtual void layoutChildren() override;
	};

	class Grid : public Elem
	{
	private:
		std::array<std::vector<float>, 2> m_sizes;
		std::array<std::vector<float>, 2> m_weights;

	public:
		Grid(Window &window, std::string id) :
			Elem(window, std::move(id))
		{}

		virtual Type getType() const override { return GRID; }

		virtual void reset() override;
		virtual void read(std::istream &is) override;

	protected:
		virtual void layoutChildren() override;
	};
}
