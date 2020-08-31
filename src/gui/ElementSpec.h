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

#include <unordered_map>
#include "DynamicSpec.h"

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
	ELEMENT_NONE
};

//! Specification for a formspec element
class ElementSpec : public DynamicSpec
{
protected:
	//! The type of the element
	ElementType m_element_type = ELEMENT_NONE;

public:
	//! Gets the type of this element
	ElementType getElementType() const { return m_element_type; }
	//! Sets what element type this is
	void setElementType(ElementType type) { m_element_type = type; }
};

//! Properties for styling elements
typedef DynamicSpec StyleSpec;

//! Bitmapped container of StyleSpecs for statewise styling
class StyleStateSpec
{
public:
	enum State
	{
		DEFAULT = 0,
		HOVERED = 1 << 0,
		PRESSED = 1 << 1
	};

protected:
	std::unordered_map<State, StyleSpec> m_styles;

public:
	//! Get a reference to a style for a specific state
	StyleSpec &at(State state)
	{
		return m_styles[state];
	}

	//! Copies properties from another StyleStateSpec into this one. This overwrites
	//! any existing properties in this StyleStateSpec with the other's.
	void addFrom(const StyleStateSpec &other)
	{
		for (const auto &it : other.m_styles)
			m_styles[it.first].addFrom(it.second);
	}

	//! Get a style from a specific element state by combining states that have
	//! lower precedence.
	StyleSpec getPropagatedStyle(State state)
	{
		StyleSpec ret;
		for (int i = 0; i <= state; i++) {
			if ((state & i) != 0 && m_styles.count((State)i))
				ret.addFrom(m_styles[(State)i]);
		}

		return ret;
	}
};
