// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 v-rob, Vincent Robinson <robinsonvincent89@gmail.com>

#pragma once

#include "ui/helpers.h"

#include <iostream>
#include <string>

namespace ui
{
	// Serialized enum; do not change order of entries.
	enum class LayoutType : u8
	{
		PLACE,

		MAX = PLACE,
	};

	// Serialized enum; do not change order of entries.
	enum class DirFlags : u8
	{
		NONE,
		X,
		Y,
		BOTH,

		MAX = BOTH,
	};

	// Serialized enum; do not change order of entries.
	enum class ClipMode : u8
	{
		NORMAL,
		OVERFLOW,
		COMPLETE,

		MAX = COMPLETE,
	};

	struct LayoutProps
	{
		LayoutType type;
		DirFlags truncate;

		float scale;

		LayoutProps() { reset(); }

		void reset();
		void read(std::istream &is);
	};

	struct SizingProps
	{
		SizeF min;

		DispF margin;
		DispF padding;

		PosF pos;
		SizeF size;
		PosF anchor;

		SizingProps() { reset(); }

		void reset();
		void read(std::istream &is);
	};

	struct VisualProps
	{
		ClipMode clip;
		bool hidden;

		video::SColor fill;

		VisualProps() { reset(); }

		void reset();
		void read(std::istream &is);
	};

	struct ImageProps
	{
		video::ITexture *pane;
		video::ITexture *overlay;

		video::SColor tint;
		RectF slice;

		u32 frames;
		u32 frame_time;

		DispF border;
		DirFlags tile;

		PosF align;
		float scale;

		ImageProps() { reset(); }

		void reset();
		void read(std::istream &is);
	};

	struct StyleProps
	{
		LayoutProps layout;
		SizingProps sizing;
		VisualProps visual;
		ImageProps img;

		StyleProps() { reset(); }

		void reset();
		void read(std::istream &is);
	};
}
