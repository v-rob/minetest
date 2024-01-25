// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2023 v-rob, Vincent Robinson <robinsonvincent89@gmail.com>

#pragma once

#include "ui/helpers.h"
#include "util/basic_macros.h"

#include <array>
#include <iostream>
#include <string>

namespace ui
{
	class Elem;
	class Window;

	// Serialized enum; do not change order of entries.
	enum class BoxTile : u8
	{
		NONE,
		X,
		Y,
		BOTH,

		MAX_TILE = BOTH,
	};

	BoxTile toBoxTile(u8 tile);

	// Serialized enum; do not change order of entries.
	enum class IconPlace : u8
	{
		CENTER,
		LEFT,
		TOP,
		RIGHT,
		BOTTOM,

		MAX_PLACE = BOTTOM,
	};

	IconPlace toIconPlace(u8 place);

	struct Layer
	{
		video::ITexture *image;
		video::SColor fill;
		video::SColor tint;

		float scale;
		RectF source;

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
		SizeF size;

		PosF rel_pos;
		PosF rel_anchor;
		SizeF rel_size;

		DispF margin;
		DispF padding;

		Layer box;
		Layer icon;

		DispF box_middle;
		BoxTile box_tile;

		IconPlace icon_place;
		float icon_gutter;
		bool icon_overlap;

		bool visible;
		bool noclip;

		Style()
		{
			reset();
		}

		void reset();
		void read(std::istream &is);
	};

	class ILayout
	{
	public:
		virtual ~ILayout() = default;

		virtual void restyle() = 0;
		virtual void relayout(RectF parent_rect, RectF child_rect) = 0;

		virtual void draw() = 0;
	};

	class Box : public ILayout
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

	private:
		// Indicates that there is no style string for this state combination.
		static constexpr u32 NO_STYLE = -1;

		Elem &m_elem;

		ILayout *m_content;

		Style m_style;
		std::array<u32, NUM_STATES> m_style_refs;

		RectF m_display_rect;
		RectF m_icon_rect;
		RectF m_content_rect;
		RectF m_clip_rect;

	public:
		Box(Elem &elem) :
			m_elem(elem)
		{
			reset();
		}

		DISABLE_CLASS_COPY(Box)

		Elem &getElem() { return m_elem; }
		const Elem &getElem() const { return m_elem; }

		Window &getWindow();
		const Window &getWindow() const;

		ILayout *getContent() { return m_content; }
		void setContent(ILayout *content) { m_content = content; }

		const Style &getStyle() const { return m_style; }

		void reset();
		void read(std::istream &is);

		virtual void restyle() override;
		virtual void relayout(RectF parent_rect, RectF parent_clip) override;

		virtual void draw() override;

	private:
		static RectF getLayerSource(const Layer &layer);
		static SizeF getLayerSize(const Layer &layer);

		DispF getMiddleEdges();

		void drawBox();
		void drawIcon();
	};
}
