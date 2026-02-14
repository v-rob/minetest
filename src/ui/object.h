// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2025 v-rob, Vincent Robinson <robinsonvincent89@gmail.com>

#pragma once

#include "ui/helpers.h"
#include "util/basic_macros.h"

#include <iostream>
#include <string>

namespace ui
{
	class Box;
	class Elem;
	class Window;

	class IObject
	{
	private:
		Box &m_box;

	public:
		IObject(Box &box) :
			m_box(box)
		{}

		DISABLE_CLASS_COPY(Box)

		Box &getBox() { return m_box; }
		const Box &getBox() const { return m_box; }

		Elem &getElem();
		const Elem &getElem() const;

		Window &getWindow();
		const Window &getWindow() const;

		virtual void reset() = 0;
		virtual void read(std::istream &is) = 0;

		virtual void restyle() = 0;
		virtual SizeF resize() = 0;
		virtual void relayout(RectF layout_rect, RectF layout_clip) = 0;

		virtual void draw() = 0;
	};

	class LabelObject : public IObject
	{
	private:
		std::wstring m_label;
		gui::IGUIFont *m_font;

		RectF m_display_rect;
		RectF m_clip_rect;

	public:
		LabelObject(Box &box) :
			IObject(box)
		{
			reset();
		}

		virtual void reset() override;
		virtual void read(std::istream &is) override;

		virtual void restyle() override;
		virtual SizeF resize() override;
		virtual void relayout(RectF layout_rect, RectF layout_clip) override;

		virtual void draw() override;
	};

	class IconObject : public IObject
	{
	private:
		video::ITexture *m_icon;

		video::SColor m_tint;
		RectF m_slice;

		u32 m_frames;
		u32 m_frame_time;

		RectF m_icon_src;

		RectF m_display_rect;
		RectF m_clip_rect;

	public:
		IconObject(Box &box) :
			IObject(box)
		{
			reset();
		}

		virtual void reset() override;
		virtual void read(std::istream &is) override;

		virtual void restyle() override;
		virtual SizeF resize() override;
		virtual void relayout(RectF layout_rect, RectF layout_clip) override;

		virtual void draw() override;
	};
}
