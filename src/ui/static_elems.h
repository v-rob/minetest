// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 v-rob, Vincent Robinson <robinsonvincent89@gmail.com>

#pragma once

#include "ui/box.h"
#include "ui/elem.h"
#include "ui/helpers.h"

#include <iostream>
#include <string>

namespace ui
{
	class Root : public Elem
	{
	private:
		Box m_backdrop_box;

	public:
		Root(Window &window, std::string id) :
			Elem(window, std::move(id)),
			m_backdrop_box(*this)
		{}

		virtual Type getType() const override { return ROOT; }

		virtual void reset() override;
		virtual void read(std::istream &is) override;

	protected:
		virtual Box &getLayoutBox() override { return m_backdrop_box; }
	};
}
