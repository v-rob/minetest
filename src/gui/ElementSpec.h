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

#include <string>
#include <unordered_map>
#include <vector>
#include "client/fontengine.h"
#include "irrlichttypes_extrabloated.h"

//! A formspec element type
enum ElementType
{
	ELEMENT_BACKGROUND,
	ELEMENT_BASE,
	ELEMENT_BOX,
	ELEMENT_BUTTON,
	ELEMENT_CHECKBOX,
	ELEMENT_CONTAINER,
	ELEMENT_DROPDOWN,
	ELEMENT_HYPERTEXT,
	ELEMENT_IMAGE,
	ELEMENT_INPUT,
	ELEMENT_LABEL,
	ELEMENT_LIST,
	ELEMENT_SCROLLBAR,
	ELEMENT_TABLE,
	ELEMENT_TABS,
};

//! Specification for a formspec element
class ElementSpec
{
public:
	//! All possible property types
	enum Type
	{
		BOOL,
		INT,
		FLOAT,
		STRING,
		WIDE_STRING,
		STRING_VECTOR,
		COLOR,
		TEXTURE,
		VECTOR2DI,
		VECTOR2DF,
		RECTI,
		RECTF,
		COLOR_ARRAY,
		NONE
	};

protected:
	//! The type of the element
	ElementType m_element_type;

	//! A raw value with all possible types in a union
	struct Value
	{
		//! Destructor destroys any allocated members
		~Value() { destroy(); };
		//! If the current member has been allocated, delete it. Otherwise, do nothing.
		void destroy();

		//! Whether this member has been modified since the last get
		bool modified;
		//! What type the active member is
		Type type;
		//! Actual value described by `type`
		union {
			bool bool_;
			s32 int_;
			f32 float_;
			std::string *str;
			std::wstring *wstr;
			std::vector<std::string> *str_vec;
			video::SColor *color;
			video::ITexture *texture;
			v2s32 *vec2di;
			v2f32 *vec2df;
			core::recti *recti;
			core::rectf *rectf;
			std::array<video::SColor, 4> *color_array;
		};
	};

	//! Map of all set properties and their raw values
	std::unordered_map<std::string, Value> m_props;

public:
	//! Gets the type of this element
	ElementType getElementType() const { return m_element_type; }
	//! Sets what element type this is
	void setElementType(ElementType type) { m_element_type = type; }

	//! Checks whether a property exists.
	bool hasProperty(const std::string &prop) { return get(prop) != nullptr; };

	//! Checks whether a property has been modified since the last get. Returns false
	//! if the property is nonexistent.
	//! Calling any `set*` function marks that property as modified whereas calling
	//! any `get*` function (not including `getType`) marks it as unmodified.
	bool isModified(const std::string &prop);

	//! Gets the type of the specified property. Returns NONE if nonexistent
	Type getType(const std::string &prop);

	//! Removes a property if it exists.
	void remove(const std::string &prop) { m_props.erase(prop); }

	// All of the following getters and setters are written explicitly without
	// macros for the benefit of Doxygen documenting them.

	//! Gets a boolean value. Returns `def` if nonexistent or not a boolean.
	bool getBool(const std::string &prop, bool def = false);
	//! Sets a property to a boolean value.
	void setBool(const std::string &prop, bool value);

	s32 getInt(const std::string &prop, s32 def = 0);
	void setInt(const std::string &prop, s32 value);

	f32 getFloat(const std::string &prop, f32 def = 0.0f);
	void setFloat(const std::string &prop, f32 value);

	std::string getString(const std::string &prop, std::string def = "");
	void setString(const std::string &prop, std::string value);

	std::wstring getWideString(const std::string &prop, std::wstring def = L"");
	void setWideString(const std::string &prop, std::wstring value);

	std::vector<std::string> getStringVector(const std::string &prop,
			std::vector<std::string> def = {});
	void setStringVector(const std::string &prop, std::vector<std::string> value);

	video::SColor getColor(const std::string &prop, video::SColor def = 0x0);
	void setColor(const std::string &prop, video::SColor value);

	//! Gets a texture. Returns `def` if nonexistent or not a texture.
	//! Warning: It is possible for this to return nullptr if set to that or the default
	//! value provided is used. Check the return value if necessary.
	video::ITexture *getTexture(const std::string &prop, video::ITexture *def = nullptr);
	//! Sets a property to a texture value. This value can be set to nullptr, but
	//! that is not recommended.
	void setTexture(const std::string &prop, video::ITexture *value);

	v2s32 getVector2di(const std::string &prop, v2s32 def = v2s32(0));
	void setVector2di(const std::string &prop, v2s32 value);

	v2f32 getVector2df(const std::string &prop, v2f32 def = v2f32(0.0f));
	void setVector2df(const std::string &prop, v2f32 value);

	core::recti getRecti(const std::string &prop,
			core::recti def = core::recti(0, 0, 0, 0));
	void setRecti(const std::string &prop, core::recti value);

	core::rectf getRectf(const std::string &prop,
			core::rectf def = core::rectf(0.0f, 0.0f, 0.0f, 0.0f));
	void setRectf(const std::string &prop, core::rectf value);

	std::array<video::SColor, 4> getColorArray(const std::string &prop,
			std::array<video::SColor, 4> def = {0x0, 0x0, 0x0, 0x0});
	void setColorArray(const std::string &prop, std::array<video::SColor, 4> value);

	//! Gets a font from two properties: a string vector defining the font family and
	//! modifiers and an int defining the size. If one or the other font is not found,
	//! the defaults are used. `def` is returned if a font could not be constructed.
	gui::IGUIFont *getFont(const std::string &family_prop, const std::string &size_prop,
			gui::IGUIFont *def = nullptr);

protected:
	//! Get a raw property value. Returns nullptr if nonexistent.
	Value *get(const std::string &prop);
	//! Get a raw property value. If the property is nonexistent or the types
	//! are mismatched, returns nullptr.
	Value *get(const std::string &prop, Type type);
};
