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

bool FormSpecElement::create(const std::string &raw, size_t props)
{
	size_t pos = raw.find('[');
	if (pos == std::string::npos) {
		errorstream << "Invalid formspec element syntax: expected '['"
				"\nNote: Full element: \"" << raw << "]\"" << std::endl;
		return true;
	}

	m_raw = raw;
	m_type = trim(raw.substr(0, pos));

	m_values.clear();

	return false;
}

bool FormSpecElement::splitArguments(u16 formspec_version,
		std::initializer_list<size_t> lengths)
{
	std::vector<std::string> parts = split(raw.substr(raw.find('[') + 1), ';');

	size_t greatest = 0;
	bool invalid_size = true;
	for (size_t length : lengths) {
		if (parts.size() == length)
			invalid_size = false;

		if (length > greatest)
			greatest = length;
	}

	if (invalid_size && !(parts.size() > greatest &&
			formspec_version > FORMSPEC_API_VERSION)) {
		std::string arg_amounts = "";
		size_t i = 0;
		for (size_t length : lengths) {
			arg_amounts += std::to_string(length);

			if (lengths.size() > 1 && i != lengths.size() - 1)
				arg_amounts += ", ";

			i++;
		}

		errorstream << "Invalid formspec element number of arguments: expected one of " <<
				arg_amounts << " arguments, got " << parts.size() <<
				"\nNote: Full element: \"" << m_raw << "]\"" << std::endl;
		return true;
	}

	for (size_t i = 0; i < parts.size(); i++) {
		m_values.push_back(std::unique_ptr<FormSpecArgument>(
				new FormSpecArgument(raw, parts[i], i)));
	}

	return false;
}

bool FormSpecElement::splitProperties(size_t length)
{
	std::vector<std::string> parts = split(raw.substr(raw.find('[') + 1), ';');

	if (parts.size() < length) {
		errorstream << "Invalid formspec element number of arguments: expected at least " <<
				length << " arguments, got " << size() << "\nNote: Full element: \"" <<
				m_raw << "]\"" << std::endl;
		return true;
	}

	for (size_t i = 0; i < parts.size(); i++) {
		if (i >= props) {
			m_values.push_back(std::unique_ptr<FormSpecArgument>(
					new FormSpecArgument(raw, parts[i], i)));
		} else {
			m_values.push_back(std::unique_ptr<FormSpecProperty>(
					new FormSpecProperty(raw, parts[i])));
		}
	}

	return false;
}

bool FormSpecElement::hasInvalidArgument() const
{
	for (size_t i = 0; i < m_values.size(); i++) {
		if (m_values[i]->isInvalid())
			return true;
	}
	return false;
}

video::SColor IFormSpecValue::asColor(u8 default_alpha)
{
	video::SColor color = 0x0;
	if (!parseColorString(m_content, color, true, default_alpha))
		parsingError("Invalid color");
	return color;
}

v2s32 IFormSpecValue::asVector2di()
{
	v2s32 vec;
	std::vector<std::string> parts = split(m_content, ',');

	if (parts.size() == 1)
		vec = v2s32(stoi(parts[0]));
	else if (parts.size() == 2)
		vec = v2s32(stoi(parts[0]), stoi(parts[1]));
	else
		parsingError("Invalid 2d vector format");

	return vec;
}

v2f32 IFormSpecValue::asVector2df()
{
	v2f32 vec;
	std::vector<std::string> parts = split(m_content, ',');

	if (parts.size() == 1)
		vec = v2f32(stof(parts[0]));
	else if (parts.size() == 2)
		vec = v2f32(stof(parts[0]), stof(parts[1]));
	else
		parsingError("Invalid 2d vector format");

	return vec;
}

core::recti IFormSpecValue::asRecti(bool offset)
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
		parsingError("Invalid rectangle format");
	}

	return rect;
}

std::array<video::SColor, 4> IFormSpecValue::asColorArray()
{
	std::array<video::SColor, 4> array = {0x0, 0x0, 0x0, 0x0};
	std::vector<std::string> parts = split(m_content, ',');

	if (parts.size() == 1) {
		parts = {parts[0], parts[0], parts[0], parts[0]};
	} else if (parts.size() == 2) {
		parts = {parts[0], parts[1], parts[0], parts[1]};
	} else if (parts.size() != 4) {
		parsingError("Invalid rectangle format");
		return array;
	}

	for (size_t i = 0; i < 4; i++) {
		video::SColor color;
		if (parseColorString(parts[i], color, true, 0xff)) {
			array[i] = color;
		} else {
			parsingError("Invalid color");
			return array;
		}
	}

	return array;
}

void FormSpecArgument::parsingError(const std::string &message)
{
	errorstream << "Invalid formspec element argument " << m_arg << ": " <<
			message << " (" << m_content << ")\nNote: Full element: \"" <<
			m_element << "]\"" << std::endl;
	m_invalid = true;
}

FormSpecProperty::FormSpecProperty(const std::string &element,
		const std::string &content) : m_element(element)
{
	size_t equal_pos = content.find('=');
	if (equal_pos == std::string::npos) {
		parsingError("Property \"" + m_name + "\" missing value");
		m_name = content;
		m_content = "";
	} else {
		m_name = trim(content.substr(0, equal_pos));
		m_content = content.substr(equal_pos + 1);
	}
}

void FormSpecProperty::parsingError(const std::string &message)
{
	errorstream << "Invalid formspec element property \"" << m_name << "\": " <<
			message << " (" << m_content << ")\nNote: Full element: \"" <<
			m_element << "]\"" << std::endl;
	m_invalid = true;
}
