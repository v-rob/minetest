// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 v-rob, Vincent Robinson <robinsonvincent89@gmail.com>

#pragma once

#include "ui/box.h"
#include "ui/elem.h"
#include "ui/helpers.h"

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

	private:
		virtual void relayout(RectF parent_rect, RectF parent_clip) override;
	};

	// Serialized enum; do not change order of entries.
	enum class FlexDir : u8
	{
		LEFT,
		UP,
		RIGHT,
		DOWN,

		MAX_DIR = DOWN,
	};

	FlexDir toFlexDir(u8 dir);

	// Serialized enum; do not change order of entries.
	enum class FlexWrap : u8
	{
		NONE,
		FORWARD,
		BACKWARD,

		MAX_WRAP = BACKWARD,
	};

	FlexWrap toFlexWrap(u8 wrap);

	class Flex : public Elem
	{
	private:
		FlexDir m_dir;
		FlexWrap m_wrap;

	public:
		Flex(Window &window, std::string id) :
			Elem(window, std::move(id))
		{}

		virtual Type getType() const override { return FLEX; }

		virtual void reset() override;
		virtual void read(std::istream &is) override;

	private:
		virtual void relayout(RectF parent_rect, RectF parent_clip) override;
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

	private:
		virtual void relayout(RectF parent_rect, RectF parent_clip) override;
	};
}
