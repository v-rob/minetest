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

#include <array>
#include <initializer_list>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "client/tile.h"
#include "irrlichttypes_extrabloated.h"
#include "util/string.h"

//! A single value in a formspec element which can be interpreted as different types
class IFormSpecValue
{
protected:
	//! The full element string, stored for better error messages
	const std::string m_element;

	//! The actual string value
	std::string m_content;

	//! Whether the value was valid for the used interpretation or not
	bool m_invalid = false;

public:
	//! Checks if the value had an invalid format for the last interpretation.
	//! If so, the interpretation's return value will have zeroed default values.
	bool isInvalid() const { return m_invalid; }

	//! Returns true if the value is empty. Empty values are still valid for
	//! most interpretations.
	bool isEmpty() const { return m_content.empty(); }

	//! Interprets the value as a boolean
	bool asBool() const { return is_yes(m_content); }

	//! Interprets the value as an integer
	s32 asInt() const { return stoi(m_content); }

	//! Interprets the value as a float
	f32 asFloat() const { return stof(m_content); }

	//! Interprets the value as a color. Can have an invalid format
	video::SColor asColor(u8 default_alpha = 0xff);

	//! Interprets the value as a plain string
	std::string asString() const { return unescape_string(m_content); }

	//! Interprets the value as an escaped wide string
	std::wstring asWideString() const
			{ return utf8_to_wide(unescape_string(m_content)); }

	//! Interprets the value as a vector of strings
	std::vector<std::string> asStringVector() const { return split(m_content, ','); }

	//! Interprets the value as a 2d vector of integers. Can have an invalid format
	v2s32 asVector2di();

	//! Interprets the value as a 2d vector of floats. Can have an invalid format
	v2f32 asVector2df();

	//! Interprets the value as a rectangle. Can have an invalid format.
	//! The output can be of different rectangle formats depending on `offset`:
	/*!
	  false: Acts like a 9-slice rect; has an x, y, width, and height. If the width/height
	         are negative, acts as an offset from the other side of the parent rect.
	  true: Acts like a box styling rect; all sides are offset from the parent rectangle.
	*/
	core::recti asRecti(bool offset = false);

	//! Interprets the value as an array (meant like a rectangle, but core::rect
	//! only supports numbers) of colors. Can have an invalid format
	std::array<video::SColor, 4> asColorArray();

protected:
	//! Displays an error message if the value could not be parsed properly and
	//! sets the value to its invalid state
	void parsingError(const std::string &message) = 0;
};

//! A single argument in a formspec element which can be interpreted as different types
class FormSpecArgument : public IFormSpecValue
{
protected:
	//! The argument's position in the element; used for error messages
	size_t m_arg;

public:
	//! Creates the argument from the raw string content
	FormSpecArgument(const std::string &element, const std::string &content,
			size_t arg) : m_element(element), m_content(content), m_arg(arg) {}

protected:
	//! Displays an error message if the argument could not be parsed properly and
	//! sets the argument to its invalid state
	void parsingError(const std::string &message) override;
}

//! A single property in a formspec element which can be interpreted as different types
class FormSpecProperty : public IFormSpecValue
{
protected:
	//! The property's name
	std::string m_name;

public:
	//! Creates the property from the raw string content
	FormSpecProperty(const std::string &element, const std::string &content);

	//! Gets the name of the property
	const std::string &name() const { return m_name; }

protected:
	//! Displays an error message if the property could not be parsed properly and
	//! sets the property to its invalid state
	void parsingError(const std::string &message) override;
}

//! A raw formspec element split into useable arguments
class FormSpecElement
{
protected:
	//! The full element string, stored for better error messages
	std::string m_raw;

	//! The type of the element
	std::string m_type;

	//! The arguments/properties of the element
	std::vector<std::unique_ptr<IFormSpecValue>> m_values;

public:
	//! Creates the element from a raw element string, finding the type information,
	//! and removing all existing arguments/properties. Returns true on failure.
	bool create(const std::string &raw);

	//! Splits the element into arguments where the number of arguments must be any
	//! of `lengths` (or greater if the formspec version is high enough). Returns true
	//! if the number of arguments is incorrect.
	bool splitArguments(u16 formspec_version, std::initializer_list<size_t> lengths);

	//! Splits the element into arguments and properties where the element must be at
	//! least `length` arguments with everything after `length` being properties.
	//! Returns true if the number of arguments is incorrect.
	bool splitProperties(size_t length);

	//! Gets the type of the element
	const std::string &getType() const { return m_type; }

	//! Gets the number of arguments/properties in the element
	size_t size() const { return m_values.size(); }

	//! Gets a pointer to the nth argument of the element. This must not be called for
	//! positions that contain a property.
	FormSpecArgument *arg(size_t n)
			{ return static_cast<FormSpecArgument *>(m_values[n].get()); }

	//! Gets a pointer to the property from the nth position in the element. This must
	//! not be called for positions that contain an argument.
	FormSpecArgument *prop(size_t n)
			{ return static_cast<FormSpecProperty *>(m_values[n].get()); }

	//! Returns true if any arguments or properties are invalid.
	bool hasInvalidArgument() const;
};
