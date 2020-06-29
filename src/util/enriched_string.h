/*
Copyright (C) 2013 xyz, Ilya Zhuravlev <whatever@xyz.is>
Copyright (C) 2016 Nore, Nathanaël Courant <nore@mesecons.net>

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

#include <string>
#include <vector>
#include "irrlichttypes_extrabloated.h"

class EnrichedString {
public:
	enum FormatFlags {
		FORMAT_NONE = 0,

		FORMAT_COLOR_DEFAULT = 0b1,
		FORMAT_HIGHLIGHT_DEFAULT = 0b10,

		FORMAT_UNDERLINE = 0b100,
		FORMAT_UNDERLINE_DEFAULT = 0b1000,
		FORMAT_UNDERLINES_ALL = 0b1100,

		FORMAT_STRIKETHROUGH = 0b10000,
		FORMAT_STRIKETHROUGH_DEFAULT = 0b100000,
		FORMAT_STRIKETHROUGHS_ALL = 0b110000,

		FORMAT_OVERLINE = 0b1000000,
		FORMAT_OVERLINE_DEFAULT = 0b10000000,
		FORMAT_OVERLINES_ALL = 0b11000000,

		FORMAT_LINES = 0b01010100,
		FORMAT_LINE_DEFAULTS = 0b10101000,
		FORMAT_LINES_ALL = 0b11111100,

		FORMAT_ALL_DEFAULTS = 0b10101011
	};

	struct Format {
		irr::video::SColor color;
		irr::video::SColor highlight;
		u8 flags;
	};

	EnrichedString();
	EnrichedString(const std::wstring &s,
		const irr::video::SColor &color = irr::video::SColor(255, 255, 255, 255));
	EnrichedString(const wchar_t *str,
		const irr::video::SColor &color = irr::video::SColor(255, 255, 255, 255));
	EnrichedString(const std::wstring &s, const Format &format);
	EnrichedString(const wchar_t *str, const Format &format);

	void clear();
	void operator=(const wchar_t *str);
	bool operator==(const EnrichedString &other) const;
	void addAtEnd(const std::wstring &s, const irr::video::SColor &initial_color);
	void addAtEnd(const std::wstring &s, const Format &initial_format);

	// Adds the character source[i] at the end.
	// An EnrichedString should always be able to be copied
	// to the end of an existing EnrichedString that way.
	void addChar(const EnrichedString &source, size_t i);

	// Adds a single character at the end, without specifying its
	// color. The color used will be the one from the last character.
	void addCharNoColor(wchar_t c);

	EnrichedString substr(size_t pos = 0, size_t len = std::string::npos) const;
	EnrichedString operator+(const EnrichedString &other) const;
	void operator+=(const EnrichedString &other);
	const wchar_t *c_str() const;
	std::vector<irr::video::SColor> getColors() const;
	const std::wstring &getString() const;

	void setDefaultColor(const irr::video::SColor &color);
	inline const irr::video::SColor &getDefaultColor() const
	{
		return m_default_format.color;
	}

	void setDefaultFormat(const Format &color);
	inline const Format &getDefaultFormat() const
	{
		return m_default_format;
	}

	inline bool operator!=(const EnrichedString &other) const
	{
		return !(*this == other);
	}
	inline bool empty() const
	{
		return m_string.empty();
	}
	inline size_t size() const
	{
		return m_string.size();
	}

	inline bool hasBackground() const
	{
		return m_has_background;
	}
	inline irr::video::SColor getBackground() const
	{
		return m_background;
	}
	inline void setBackground(const irr::video::SColor &color)
	{
		m_background = color;
		m_has_background = true;
	}

private:
	void updateDefaultFormat();

	std::wstring m_string;
	std::vector<Format> m_format;
	Format m_default_format;
	Format m_last_format;
	bool m_has_background;
	irr::video::SColor m_background;
};
