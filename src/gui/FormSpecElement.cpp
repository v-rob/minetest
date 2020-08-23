/*
Minetest
Copyright (C) 2020 Vincent Robinson (v-rob) <robinsonvincent89@gmail.com>

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

#include "FormSpecElement.h"
#include "log.h"
#include "network/networkprotocol.h"

bool FormSpecElement::create(const std::string &raw)
{
	size_t pos = raw.find('[');
	if (pos == std::string::npos) {
		errorstream << "Invalid formspec element syntax: expected '[' after element "
				"type\nFull element: \"" << raw << "]\"" << std::endl;
		return true;
	}

	m_raw = raw;
	m_type = trim(raw.substr(0, pos));
	std::vector<std::string> parts = split(raw.substr(pos + 1), ';');

	m_args.clear();
	for (size_t i = 0; i < parts.size(); i++)
		m_args.emplace_back(raw, parts[i], i);

	return false;
}

bool FormSpecElement::checkLength(u16 formspec_version,
		std::initializer_list<size_t> lengths) const
{
	size_t greatest = 0;
	for (size_t length : lengths) {
		if (size() == length)
			return false;

		if (length > greatest)
			greatest = length;
	}

	if (size() > greatest && formspec_version > FORMSPEC_API_VERSION)
		return false;

	std::string arg_amounts = "";

	size_t i = 0;
	for (size_t length : lengths) {
		arg_amounts += std::to_string(length);

		if (lengths.size() > 1 && i != lengths.size() - 1)
			arg_amounts += ", ";

		i++;
	}

	errorstream << "Invalid formspec element number of arguments: expected one of " <<
			arg_amounts << " arguments, got " << size() << "\nFull element: \"" <<
			m_raw << "]\"" << std::endl;
	return true;
}

bool FormSpecElement::checkLength(size_t length) const
{
	if (size() >= length)
		return false;

	errorstream << "Invalid formspec element number of arguments: expected at least " <<
			length << " arguments, got " << size() << "\nFull element: \"" << m_raw <<
			"]\"" << std::endl;
	return true;
}

bool FormSpecElement::hasInvalidArgument() const
{
	for (size_t i = 0; i < m_args.size(); i++) {
		if (m_args[i].isInvalid())
			return true;
	}
	return false;
}

video::SColor FormSpecArgument::asColor(u8 default_alpha)
{
	video::SColor color = 0x0;
	if (!parseColorString(m_content, color, true, default_alpha))
		error("Invalid color");
	return color;
}

v2s32 FormSpecArgument::asVector2di()
{
	v2s32 vec;
	std::vector<std::string> parts = split(m_content, ',');

	if (parts.size() == 1)
		vec = v2s32(stoi(parts[0]));
	else if (parts.size() == 2)
		vec = v2s32(stoi(parts[0]), stoi(parts[1]));
	else
		error("Invalid 2d vector format");

	return vec;
}

v2f32 FormSpecArgument::asVector2df()
{
	v2f32 vec;
	std::vector<std::string> parts = split(m_content, ',');

	if (parts.size() == 1)
		vec = v2f32(stof(parts[0]));
	else if (parts.size() == 2)
		vec = v2f32(stof(parts[0]), stof(parts[1]));
	else
		error("Invalid 2d vector format");

	return vec;
}

core::recti FormSpecArgument::asRecti(bool offset)
{
	core::rect<s32> rect;
	std::vector<std::string> parts = split(m_content, ',');

	if (parts.size() == 1) {
		s32 x = stoi(parts[0]);
		rect.UpperLeftCorner = v2s32(x, x);
		if (offset)
			rect.LowerRightCorner = v2s32(x, x);
		else
			rect.LowerRightCorner = v2s32(-x, -x);
	} else if (parts.size() == 2) {
		s32 x = stoi(parts[0]);
		s32 y =	stoi(parts[1]);
		rect.UpperLeftCorner = v2s32(x, y);
		if (offset)
			rect.LowerRightCorner = v2s32(x, y);
		else
			rect.LowerRightCorner = v2s32(-x, -y);
	} else if (parts.size() == 4) {
		rect = core::rect<s32>(stoi(parts[0]), stoi(parts[1]), stoi(parts[2]),
				stoi(parts[3]));
	} else {
		error("Invalid rectangle format");
	}

	return rect;
}

std::array<video::SColor, 4> FormSpecArgument::asColorArray()
{
	std::array<video::SColor, 4> array = {0x0, 0x0, 0x0, 0x0};
	std::vector<std::string> parts = split(m_content, ',');

	if (parts.size() == 1) {
		parts = {parts[0], parts[0], parts[0], parts[0]};
	} else if (parts.size() == 2) {
		parts = {parts[0], parts[1], parts[0], parts[1]};
	} else if (parts.size() != 4) {
		error("Invalid rectangle format");
		return array;
	}

	for (size_t i = 0; i < 4; i++) {
		video::SColor color;
		if (parseColorString(parts[i], color, true, 0xff)) {
			array[i] = color;
		} else {
			error("Invalid color");
			return array;
		}
	}

	return array;
}

void FormSpecArgument::error(const std::string &message)
{
	std::string mid;
	if (m_is_prop)
		mid = "property \"" + m_arg + "\"";
	else
		mid = "argument " + m_arg;

	errorstream << "Invalid formspec element " << mid << ": " << message << "(" <<
			m_content << ")" << "\nFull element: \"" << m_element << "]\"" << std::endl;

	m_invalid = true;
}
