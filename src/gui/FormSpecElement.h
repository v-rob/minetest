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

#include <initializer_list>
#include <array>
#include <string>
#include <unordered_map>
#include <vector>
#include "client/tile.h"
#include "irrlichttypes_extrabloated.h"
#include "util/string.h"

//! A single argument in a formspec element which can be interpreted as different types
class FormSpecArgument
{
protected:
	//! The full element string, stored for better error messages
	std::string m_element;

	//! The content of the argument
	std::string m_content;

	//! Whether the argument is a positional argument or a property
	bool m_is_prop;
	//! Which argument/property in the element this is
	std::string m_arg;

	//! Whether the argument was valid for the used interpretation or not
	bool m_invalid = false;

public:
	//! Makes the argument from the raw string content and sets as a numeric argument
	FormSpecArgument(const std::string &element, const std::string &content,
			size_t arg_no) :
		m_element(element), m_content(content), m_is_prop(false),
		m_arg(std::to_string(arg_no)) {}

	//! Makes the argument from the raw string content and sets as a property
	FormSpecArgument(const std::string &element, const std::string &content,
			const std::string &prop_name) :
		m_element(element), m_content(content), m_is_prop(true), m_arg(prop_name) {}

	//! Checks if the argument had an invalid format for the last interpretation.
	//! If so, the interpretation's return value will have been junk.
	bool isInvalid() const { return m_invalid; }

	//! Interprets the argument as a boolean
	bool asBool() const { return is_yes(m_content); }

	//! Interprets the argument as an integer
	s32 asInt() const { return stoi(m_content); }

	//! Interprets the argument as a float
	f32 asFloat() const { return stof(m_content); }

	//! Interprets the argument as a plain string
	std::string asString() const { return unescape_string(m_content); }

	//! Interprets the argument as an escaped wide string
	std::wstring asWideString() const
			{ return utf8_to_wide(unescape_string(m_content)); }

	//! Interprets the argument as a vector of strings
	std::vector<std::string> asStringVector() const { return split(m_content, ','); }

	//! Interprets the argument as a color. Can have an invalid format
	video::SColor asColor(u8 default_alpha = 0xff);

	//! Interprets the argument as a texture
	video::ITexture *asTexture(ISimpleTextureSource *tsrc) const
			{ return tsrc->getTexture(m_content); }

	//! Interprets the argument as a 2d vector of integers. Can have an invalid format
	v2s32 asVector2di();

	//! Interprets the argument as a 2d vector of floats. Can have an invalid format
	v2f32 asVector2df();

	//! Interprets the argument as a rectangle. Can have an invalid format.
	//! The output can be of different rectangle formats depending on `offset`:
	/*!
	  false: Acts like a 9-slice rect; has an x, y, width, and height. If the width/height
	         are negative, acts as an offset from the other side of the parent rect.
	  true: Acts like a box styling rect; all sides are offset from the parent rectangle.
	*/
	core::recti asRecti(bool offset = false);

	//! Interprets the argument as an array (meant like a rectangle, but core::rect
	//! only supports numbers) of colors. Can have an invalid format
	std::array<video::SColor, 4> asColorArray();

protected:
	//! Displays an error message if an argument could not be parsed properly
	//! and sets the argument to its invalid state
	void parsingError(const std::string &message);
};

//! A raw formspec element split into useable arguments
class FormSpecElement
{
protected:
	//! The full element string, stored for better error messages
	std::string m_raw;

	//! The type of the element
	std::string m_type;

	//! The raw string parts of the element
	std::vector<FormSpecArgument> m_args;

public:
	//! Makes the element from a raw element string. Returns true on failure.
	bool create(const std::string &raw);

	//! Gets the type of the element
	const std::string &getType() const { return m_type; }

	//! Gets the number of arguments in the element
	size_t size() const { return m_args.size(); }

	//! Gets the nth argument of the element
	FormSpecArgument &arg(size_t n) { return m_args[n]; }

	//! Checks the length of the element to ensure it is one of the lengths specified
	//! unless the formspec's version exceeds the client's formspec version. Returns
	//! true if the length is incorrect.
	bool checkLength(u16 formspec_version, std::initializer_list<size_t> lengths) const;

	//! Checks the length of the element to ensure it is at least the length specified,
	//! usually for configuration elements. Returns true if the length is incorrect.
	bool checkLength(size_t length) const;

	//! Returns true if any arguments are invalid.
	bool hasInvalidArgument() const;
};
