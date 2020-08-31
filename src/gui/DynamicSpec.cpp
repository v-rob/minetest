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

#include "DynamicSpec.h"
#include "util/string.h"

void DynamicSpec::Value::destroy()
{
#define DELETE_VALUE(type, member)	\
	case type:						\
		delete member;				\
		break;

	switch (type) {
		DELETE_VALUE(STRING, str)
		DELETE_VALUE(WIDE_STRING, wstr)
		DELETE_VALUE(STRING_VECTOR, str_vec)
		DELETE_VALUE(VECTOR2DI, vec2di)
		DELETE_VALUE(VECTOR2DF, vec2df)
		DELETE_VALUE(RECTI, recti)
		DELETE_VALUE(RECTF, rectf)
		DELETE_VALUE(COLOR_ARRAY, color_array)
		DELETE_VALUE(NESTED, nested)
	default:
		break;
	}

#undef DELETE_VALUE
}

void DynamicSpec::addFrom(const DynamicSpec &other)
{
	for (auto &it : other.m_props) {
		const std::string &prop = it.first;
		const Value &other_value = it.second;

#define COPY_VALUE(type, func)					\
	case type:									\
		set##func(prop, other.get##func(prop));	\
		break;

		switch (other_value.type) {
			COPY_VALUE(BOOL, Bool)
			COPY_VALUE(INT, Int)
			COPY_VALUE(FLOAT, Float)
			COPY_VALUE(COLOR, Color)
			COPY_VALUE(STRING, String)
			COPY_VALUE(WIDE_STRING, WideString)
			COPY_VALUE(STRING_VECTOR, StringVector)
			COPY_VALUE(VECTOR2DI, Vector2di)
			COPY_VALUE(VECTOR2DF, Vector2df)
			COPY_VALUE(RECTI, Recti)
			COPY_VALUE(RECTF, Rectf)
			COPY_VALUE(COLOR_ARRAY, ColorArray)
			COPY_VALUE(NESTED, Nested)
		default:
			break;
		}

#undef COPY_VALUE
	}
}

DynamicSpec::Type DynamicSpec::getType(const std::string &prop) const
{
	const Value *val = constGet(prop);
	if (val == nullptr)
		return NONE;
	return val->type;
}

#define GETTER_SETTER(typename, func, type_id, member, assign, ret)				\
	typename DynamicSpec::get##func(const std::string &prop, typename def) const\
	{																			\
		const Value *val = constGet(prop, type_id);								\
		if (val == nullptr)														\
			return def;															\
																				\
		return ret;																\
	}																			\
																				\
	void DynamicSpec::set##func(const std::string &prop, typename value)		\
	{																			\
		Value *val = get(prop);													\
		if (val == nullptr) {													\
			m_props[prop] = Value();											\
			val = &m_props.find(prop)->second;									\
		}																		\
		val->destroy();															\
		val->type = type_id;													\
		val->member = assign;													\
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

GETTER_SETTER(video::SColor, Color, COLOR, color, value.color,
		video::SColor(val->color))

C_GETTER_SETTER(v2s32, Vector2di, VECTOR2DI, vec2di)
C_GETTER_SETTER(v2f32, Vector2df, VECTOR2DF, vec2df)

C_GETTER_SETTER(core::recti, Recti, RECTI, recti)
C_GETTER_SETTER(core::rectf, Rectf, RECTF, rectf)

// Commas can't be in macro arguments and types can't be parenthesized, so this works
// around that limitation. The internal macro is used to prevent the comma from expanding
#define A std::array<video::SColor, 4>
GETTER_SETTER(A, ColorArray, COLOR_ARRAY, color_array, new A(value),
		*val->color_array)
#undef A

#undef C_GETTER_SETTER
#undef S_GETTER_SETTER
#undef GETTER_SETTER

DynamicSpec DynamicSpec::getNested(const std::string &prop) const
{
	const Value *val = constGet(prop, NESTED);
	if (val == nullptr)
		return DynamicSpec();

	return *val->nested;
}

void DynamicSpec::setNested(const std::string &prop, const DynamicSpec &value)
{
	Value *val = get(prop);
	if (val == nullptr) {
		m_props[prop] = Value();
		val = &m_props.find(prop)->second;
	}
	val->destroy();
	val->type = NESTED;
	val->nested = new DynamicSpec(value);
}

gui::IGUIFont *DynamicSpec::createFont(const std::string &font_prop,
		const std::string &size_prop, gui::IGUIFont *def) const
{
	FontSpec spec(FONT_SIZE_UNSPECIFIED, FM_Standard, false, false);

	const Value *font = constGet(font_prop, STRING_VECTOR);
	const Value *size = constGet(size_prop);

	if (font == nullptr && size == nullptr)
		return def;

	if (font != nullptr) {
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
			calc_size = size->int_;
		} else if (size->type == STRING) {
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

DynamicSpec::Value *DynamicSpec::get(const std::string &prop)
{
	auto it = m_props.find(prop);
	if (it == m_props.end())
		return nullptr;

	return &it->second;
}

DynamicSpec::Value *DynamicSpec::get(const std::string &prop, Type type)
{
	auto it = m_props.find(prop);
	if (it == m_props.end() || it->second.type != type)
		return nullptr;

	return &it->second;
}

const DynamicSpec::Value *DynamicSpec::constGet(const std::string &prop) const
{
	const auto it = m_props.find(prop);
	if (it == m_props.end())
		return nullptr;

	return &it->second;
}

const DynamicSpec::Value *DynamicSpec::constGet(const std::string &prop,
		Type type) const
{
	const auto it = m_props.find(prop);
	if (it == m_props.end() || it->second.type != type)
		return nullptr;

	return &it->second;
}
