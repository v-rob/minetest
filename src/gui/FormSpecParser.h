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

#include <list>
#include <memory>
#include "irrlichttypes_extrabloated.h"
#include "ElementSpec.h"
#include "ParserState.h"

//! Parses a formspec string into individual elements
class FormSpecParser
{
public:
	//! Parses the formspec string
	void parse(const std::string &formspec_string);

	//! Parses a single formspec element
	std::unique_ptr<ElementSpec> parseElement(ParserState *state, const std::string &element);
};
