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

#pragma once

#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "client/fontengine.h"
#include "irrlichttypes_extrabloated.h"

//! A dynamic map of string keys to dynamic values of multiple types
class DynamicSpec
{
public:
	//! All possible property types
	enum Type
	{
		BOOL,
		INT,
		FLOAT,
		COLOR,
		STRING,
		WIDE_STRING,
		STRING_VECTOR,
		VECTOR2DI,
		VECTOR2DF,
		RECTI,
		RECTF,
		COLOR_ARRAY,
		NESTED,
		NONE
	};

protected:
	//! A raw value with all possible types in a union
	struct Value
	{
		//! Destructor destroys any allocated members
		~Value() { destroy(); };
		//! If the current member has been allocated, delete it. Otherwise, do nothing.
		void destroy();

		//! What type the active member is
		Type type;
		//! Actual value described by `type`
		union {
			bool bool_;
			s32 int_;
			f32 float_;
			u32 color; // Cheaper than pointer to Irrlicht color
			std::string *str;
			std::wstring *wstr;
			std::vector<std::string> *str_vec;
			v2s32 *vec2di;
			v2f32 *vec2df;
			core::recti *recti;
			core::rectf *rectf;
			std::array<video::SColor, 4> *color_array;
			DynamicSpec *nested;
		};
	};

	//! Map of all set properties and their raw values
	std::unordered_map<std::string, Value> m_props;

public:
	//! Contructs an empty DynamicSpec
	DynamicSpec() = default;

	//! Constructs a DynamicSpec from another DynamicSpec
	DynamicSpec(const DynamicSpec &other) { addFrom(other); }

	//! Copies properties from another DynamicSpec into this one. This overwrites any
	//! existing properties in this DynamicSpec with the other's.
	void addFrom(const DynamicSpec &other);

	//! Checks whether a property exists.
	bool has(const std::string &prop) const { return m_props.count(prop); };

	//! Gets the type of the specified property. Returns NONE if nonexistent
	Type getType(const std::string &prop) const;

	//! Removes a property if it exists.
	void erase(const std::string &prop) { m_props.erase(prop); }

	// All of the following getters and setters are written explicitly without
	// macros for the benefit of Doxygen documenting them.

	//! Gets a boolean value. Returns `def` if nonexistent or not a boolean.
	bool getBool(const std::string &prop, bool def = false) const;
	//! Sets a property to a boolean value.
	void setBool(const std::string &prop, bool value);

	s32 getInt(const std::string &prop, s32 def = 0) const;
	void setInt(const std::string &prop, s32 value);

	f32 getFloat(const std::string &prop, f32 def = 0.0f) const;
	void setFloat(const std::string &prop, f32 value);

	video::SColor getColor(const std::string &prop, video::SColor def = 0x0) const;
	void setColor(const std::string &prop, video::SColor value);

	std::string getString(const std::string &prop, std::string def = "") const;
	void setString(const std::string &prop, std::string value);

	std::wstring getWideString(const std::string &prop, std::wstring def = L"") const;
	void setWideString(const std::string &prop, std::wstring value);

	std::vector<std::string> getStringVector(const std::string &prop,
			std::vector<std::string> def = {}) const;
	void setStringVector(const std::string &prop, std::vector<std::string> value);

	v2s32 getVector2di(const std::string &prop, v2s32 def = v2s32(0)) const;
	void setVector2di(const std::string &prop, v2s32 value);

	v2f32 getVector2df(const std::string &prop, v2f32 def = v2f32(0.0f)) const;
	void setVector2df(const std::string &prop, v2f32 value);

	core::recti getRecti(const std::string &prop,
			core::recti def = core::recti(0, 0, 0, 0)) const;
	void setRecti(const std::string &prop, core::recti value);

	core::rectf getRectf(const std::string &prop,
			core::rectf def = core::rectf(0.0f, 0.0f, 0.0f, 0.0f)) const;
	void setRectf(const std::string &prop, core::rectf value);

	std::array<video::SColor, 4> getColorArray(const std::string &prop,
			std::array<video::SColor, 4> def = {0x0, 0x0, 0x0, 0x0}) const;
	void setColorArray(const std::string &prop, std::array<video::SColor, 4> value);

	//! Gets a copy of a nested DynamicSpec object. Returns an empty DynamicSpec if
	//! nonexistent or not a DynamicSpec.
	DynamicSpec getNested(const std::string &prop) const;
	//! Sets a property to a copy of a DynamicSpec object.
	void setNested(const std::string &prop, const DynamicSpec &value);

	//! Creates a font from two properties: a string vector defining the font family and
	//! modifiers and an int defining the size. If one or the other font is not found,
	//! the defaults are used. `def` is returned if a font could not be constructed.
	gui::IGUIFont *createFont(const std::string &family_prop,
			const std::string &size_prop, gui::IGUIFont *def = nullptr) const;

protected:
	//! Gets a raw property value. Returns nullptr if nonexistent.
	Value *get(const std::string &prop);
	//! Gets a raw property value. If the property is nonexistent or the types
	//! are mismatched, returns nullptr.
	Value *get(const std::string &prop, Type type);

	//! Gets a constant raw property value. Returns nullptr if nonexistent.
	const Value *constGet(const std::string &prop) const;
	//! Get a constant raw property value. If the property is nonexistent or
	//! the types are mismatched, returns nullptr.
	const Value *constGet(const std::string &prop, Type type) const;
};
