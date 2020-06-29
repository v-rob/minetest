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

#include "enriched_string.h"
#include "util/string.h"
#include "debug.h"
#include "log.h"

#define COPY_BITS(to, from, bits) to.flags = (to.flags & ~bits) | (from.flags & bits)

using namespace irr::video;

EnrichedString::EnrichedString()
{
	clear();
}

EnrichedString::EnrichedString(const std::wstring &s, const SColor &color)
{
	clear();
	addAtEnd(translate_string(s), color);
}

EnrichedString::EnrichedString(const wchar_t *str, const SColor &color)
{
	clear();
	addAtEnd(translate_string(std::wstring(str)), color);
}

EnrichedString::EnrichedString(const std::wstring &s, const EnrichedString::Format &format)
{
	clear();
	addAtEnd(translate_string(s), format);
}

EnrichedString::EnrichedString(const wchar_t *str, const EnrichedString::Format &format)
{
	clear();
	addAtEnd(translate_string(std::wstring(str)), format);
}

void EnrichedString::clear()
{
	m_string.clear();
	m_format.clear();
	m_default_format = {
		irr::video::SColor(255, 255, 255, 255),
		irr::video::SColor(0, 0, 0, 0),
		FORMAT_ALL_DEFAULTS
	};
	m_last_format = m_default_format;
	m_has_background = false;
	m_background = irr::video::SColor(0, 0, 0, 0);
}

void EnrichedString::operator=(const wchar_t *str)
{
	clear();
	addAtEnd(translate_string(std::wstring(str)), m_default_format);
}

bool EnrichedString::operator==(const EnrichedString &other) const
{
	bool same = m_string == other.m_string;
	for (size_t i = 0; i < m_format.size(); i++) {
		Format f = m_format[i];
		Format o = other.m_format[i];
		same = f.color == o.color && f.highlight == o.highlight && f.flags == o.flags;
		if (!same)
			return false;
	}
	return same;
}

void EnrichedString::addAtEnd(const std::wstring &s, const SColor &initial_color)
{
	Format f = m_default_format;
	f.color = initial_color;
	addAtEnd(s, f);
}

void EnrichedString::addAtEnd(const std::wstring &s,
	const EnrichedString::Format &initial_format)
{
	Format format = initial_format;

	size_t i = 0;
	while (i < s.length()) {
		if (s[i] != L'\x1b') {
			m_string += s[i];
			m_format.push_back(format);
			++i;
			continue;
		}

		++i;
		size_t start_index = i;
		size_t length;
		if (i == s.length())
			break;

		if (s[i] == L'(') {
			++i;
			++start_index;
			while (i < s.length() && s[i] != L')') {
				if (s[i] == L'\\') {
					++i;
				}
				++i;
			}
			length = i - start_index;
			++i;
		} else {
			++i;
			length = 1;
		}

		std::wstring escape_sequence(s, start_index, length);
		std::vector<std::wstring> parts = split(escape_sequence, L'@');

		if (parts[0] == L"c") {
			if (parts.size() < 2)
				continue;

			if (parts[1] == L"default") {
				format.color = initial_format.color;
				format.flags |= FORMAT_COLOR_DEFAULT;
			} else if (parts[1] == L"last") {
				format.color = m_last_format.color;
				COPY_BITS(format, m_last_format, FORMAT_COLOR_DEFAULT);
			} else {
				parseColorString(wide_to_utf8(parts[1]), format.color, true);
				format.flags &= ~FORMAT_COLOR_DEFAULT;
			}
		} else if (parts[0] == L"b") {
			if (parts.size() < 2)
				continue;

			if (parts[1] == L"default") {
				m_has_background = false;
			} else {
				parseColorString(wide_to_utf8(parts[1]), m_background, true);
				m_has_background = true;
			}
		} else if (parts[0] == L"h") {
			if (parts.size() < 2)
				continue;

			if (parts[1] == L"default") {
				format.highlight = initial_format.highlight;
				format.flags |= FORMAT_HIGHLIGHT_DEFAULT;
			} else if (parts[1] == L"last") {
				format.highlight = m_last_format.highlight;
				COPY_BITS(format, m_last_format, FORMAT_HIGHLIGHT_DEFAULT);
			} else {
				parseColorString(wide_to_utf8(parts[1]), format.highlight, true);
				format.flags &= ~FORMAT_HIGHLIGHT_DEFAULT;
			}

#define LINE_OPTIONS(type)															\
	if (parts[1] == L"default")														\
		COPY_BITS(format, m_default_format, FORMAT_##type##S_ALL);					\
	else if (parts[1] == L"last")													\
		COPY_BITS(format, m_last_format, FORMAT_##type##S_ALL);						\
	else if (parts[1] == L"on")														\
		format.flags = (format.flags | FORMAT_##type) & ~FORMAT_##type##_DEFAULT;	\
	else if (parts[1] == L"off")													\
		format.flags &= ~FORMAT_##type##S_ALL;

		} else if (parts[0] == L"u") {
			if (parts.size() < 2)
				continue;

			LINE_OPTIONS(UNDERLINE);
		} else if (parts[0] == L"s") {
			if (parts.size() < 2)
				continue;

			LINE_OPTIONS(STRIKETHROUGH);
		} else if (parts[0] == L"o") {
			if (parts.size() < 2)
				continue;

			LINE_OPTIONS(OVERLINE);
		}
		m_last_format = format;

#undef LINE_OPTIONS
	}
}

void EnrichedString::addChar(const EnrichedString &source, size_t i)
{
	m_string += source.m_string[i];
	m_format.push_back(source.m_format[i]);
}

void EnrichedString::addCharNoColor(wchar_t c)
{
	m_string += c;
	if (m_format.empty()) {
		m_format.emplace_back(m_default_format);
	} else {
		m_format.push_back(m_format[m_format.size() - 1]);
	}
}

EnrichedString EnrichedString::operator+(const EnrichedString &other) const
{
	EnrichedString result = *this;
	result += other;
	return result;
}

void EnrichedString::operator+=(const EnrichedString &other)
{
	m_string += other.m_string;
	m_format.insert(m_format.end(), other.m_format.begin(), other.m_format.end());

	updateDefaultFormat();
}

EnrichedString EnrichedString::substr(size_t pos, size_t len) const
{
	if (pos >= m_string.length())
		return EnrichedString();

	if (len == std::string::npos || pos + len > m_string.length())
		len = m_string.length() - pos;

	EnrichedString str(
		m_string.substr(pos, len),
		std::vector<Format>(m_format.begin() + pos, m_format.begin() + pos + len)
	);

	str.m_has_background = m_has_background;
	str.m_background = m_background;

	str.setDefaultFormat(m_default_format);
	return str;
}

const wchar_t *EnrichedString::c_str() const
{
	return m_string.c_str();
}

std::vector<SColor> EnrichedString::getColors() const
{
	std::vector<irr::video::SColor> colors;
	colors.reserve(m_format.size());
	for (size_t i = 0; i < m_format.size(); i++)
		colors.push_back(m_format[i].color);
	return colors;
}

const std::wstring &EnrichedString::getString() const
{
	return m_string;
}

void EnrichedString::setDefaultColor(const irr::video::SColor &color)
{
	m_default_format.color = color;
	updateDefaultFormat();
}

void EnrichedString::setDefaultFormat(const EnrichedString::Format &format)
{
	m_default_format = format;
	m_default_format.flags |= FORMAT_ALL_DEFAULTS;
	updateDefaultFormat();
}

void EnrichedString::updateDefaultFormat()
{
	for (size_t i = 0; i < m_format.size(); i++) {
		if (m_format[i].flags & FORMAT_COLOR_DEFAULT)
			m_format[i].color = m_default_format.color;
		if (m_format[i].flags & FORMAT_HIGHLIGHT_DEFAULT)
			m_format[i].highlight = m_default_format.highlight;

		if (m_format[i].flags & FORMAT_UNDERLINE_DEFAULT)
			COPY_BITS(m_format[i].flags, m_default_format, FORMAT_UNDERLINE);
		if (m_format[i].flags & FORMAT_STRIKETHROUGH_DEFAULT)
			COPY_BITS(m_format[i].flags, m_default_format, FORMAT_STRIKETHROUGH);
		if (m_format[i].flags & FORMAT_OVERLINE_DEFAULT)
			COPY_BITS(m_format[i].flags, m_default_format, FORMAT_OVERLINE);
	}
}

#undef COPY_BITS
