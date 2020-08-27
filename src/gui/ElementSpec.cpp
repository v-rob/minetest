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

#include "ElementSpec.h"
#include "util/string.h"

void ElementSpec::Value::destroy()
{
	switch (type) {
	case STRING:
		delete str;
		break;
	case STRING_VECTOR:
		delete str_vec;
		break;
	default:
		break;
	}
}

bool ElementSpec::isModified(const std::string &prop)
{
	Value *val = get(prop);
	return val != nullptr && val->modified;
}

void ElementSpec::setModified(const std::string &prop, bool modified)
{
	Value *val = get(prop);
	if (val != nullptr)
		val->modified = modified;
}

ElementSpec::Type ElementSpec::getType(const std::string &prop)
{
	Value *val = get(prop);
	if (val == nullptr)
		return NONE;
	return val->type;
}

#define GETTER_SETTER(typename, func, type_id, member, assign, ret)			\
	typename ElementSpec::get##func(const std::string &prop, typename def)	\
	{																		\
		Value *val = get(prop, type_id);									\
		if (val == nullptr)													\
			return def;														\
																			\
		val->modified = false;												\
		return ret;															\
	}																		\
																			\
	void ElementSpec::set##func(const std::string &prop, typename value)	\
	{																		\
		Value *val = get(prop);												\
		if (val == nullptr) {												\
			m_props[prop] = Value();										\
			val = &m_props.find(prop)->second;								\
		}																	\
		val->destroy();														\
		val->modified = true;												\
		val->type = type_id;												\
		val->member = assign;												\
	}

// Getter and setter for a trivially constructed object
#define S_GETTER_SETTER(type, func, type_id, member) \
		GETTER_SETTER(type, func, type_id, member, value, val->member)
// Getter and setter for a non-trivially constructed object; must be a pointer internally
#define C_GETTER_SETTER(type, func, type_id, member) \
		GETTER_SETTER(type, func, type_id, member, new type(value), *val->member)

S_GETTER_SETTER(bool, Bool,  BOOL,  bool_ )
S_GETTER_SETTER(s32,  Int,   INT,   int_  )
S_GETTER_SETTER(f32,  Float, FLOAT, float_)

C_GETTER_SETTER(std::string, String, STRING, str)
GETTER_SETTER(std::wstring, WideString, WIDE_STRING, wstr,
		new std::wstring(translate_string(value)), *val->wstr)
C_GETTER_SETTER(std::vector<std::string>, StringVector, STRING_VECTOR, str_vec)

GETTER_SETTER(video::SColor, Color, COLOR, color, value.color, video::SColor(val->color))

C_GETTER_SETTER(v2s32, Vector2di, VECTOR2DI, vec2di)
C_GETTER_SETTER(v2f32, Vector2df, VECTOR2DF, vec2df)

C_GETTER_SETTER(core::recti, Recti, RECTI, recti)
C_GETTER_SETTER(core::rectf, Rectf, RECTF, rectf)

// Commas can't be in macro arguments and types can't be parenthesized, so this works
// around that limitation. The internal macro is used to prevent the comma from expanding
#define A std::array<video::SColor, 4>
GETTER_SETTER(A, ColorArray, COLOR_ARRAY, color_array, new A(value), *val->color_array)
#undef A

#undef C_GETTER_SETTER
#undef S_GETTER_SETTER
#undef GETTER_SETTER

gui::IGUIFont *ElementSpec::getFont(const std::string &font_prop,
		const std::string &size_prop, gui::IGUIFont *def)
{
	FontSpec spec(FONT_SIZE_UNSPECIFIED, FM_Standard, false, false);

	Value *font = get(font_prop, STRING_VECTOR);
	Value *size = get(size_prop);

	if (font == nullptr && size == nullptr)
		return def;

	if (font != nullptr) {
		font->modified = false;

		const std::vector<std::string> &vec = *font->str_vec;
		for (size_t i = 0; i < vec.size(); i++) {
			if (vec[i] == "normal")
				spec.mode = FM_Standard;
			else if (vec[i] == "mono")
				spec.mode = FM_Mono;
			else if (vec[i] == "bold")
				spec.bold = true;
			else if (vec[i] == "italic")
				spec.italic = true;
		}
	}

	if (size != nullptr) {
		s32 calc_size = 1;

		if (size->type == INT) {
			size->modified = false;
			calc_size = size->int_;
		} else if (size->type == STRING) {
			size->modified = false;

			const std::string &str = *size->str;
			if (str.size() != 0) {
				if (str[0] == '*') {
					std::string new_size = str.substr(1); // Remove '*' (invalid for stof)
					calc_size = stof(new_size) * g_fontengine->getFontSize(spec.mode);
				} else if (str[0] == '+' || str[0] == '-') {
					calc_size = stoi(str) + g_fontengine->getFontSize(spec.mode);
				} else {
					calc_size = stoi(str);
				}
			}
		}

		spec.size = (u32)std::min(std::max(calc_size, 1), 999);
	}

	gui::IGUIFont *ret = g_fontengine->getFont(spec);
	return ret != nullptr ? ret : def;
}

ElementSpec::Value *ElementSpec::get(const std::string &prop)
{
	auto val = m_props.find(prop);
	if (val == m_props.end())
		return nullptr;

	return &val->second;
}

ElementSpec::Value *ElementSpec::get(const std::string &prop, Type type)
{
	auto val = m_props.find(prop);
	if (val == m_props.end() || val->second.type != type)
		return nullptr;

	return &val->second;
}
