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

#include "FormSpecParser.h"
#include "FormSpecElement.h"
#include "log.h"
#include "network/networkprotocol.h"

namespace {
	bool parse_button(ParserState *state, FormSpecElement *element, ElementSpec *spec)
	{
		if (element->checkLength(state->formspec_version, {4}))
			return true;

		spec->setElementType(ELEMENT_BUTTON);

		spec->setVector2df ("pos",   element->arg(0).asVector2df());
		spec->setVector2df ("size",  element->arg(1).asVector2df());
		spec->setWideString("label", element->arg(3).asWideString());

		std::string name = element->arg(2).asString();
		spec->setString("name", name);

		if (state->focused_element == name)
			spec->setBool("focused", true);

		if (element->getType() == "button_exit")
			spec->setBool("exit", true);

		return element->hasInvalidArgument();
	}

	//! Map of element types to parsing function. Functions return true on failure
	const std::unordered_map<std::string,
			bool (*)(ParserState *, FormSpecElement *, ElementSpec *)> parsers = {
		{"button", parse_button},
		{"button_exit", parse_button},
	};
}

void FormSpecParser::parse(const std::string &formspec_string)
{
	ParserState state;
}

std::unique_ptr<ElementSpec> FormSpecParser::parseElement(ParserState *state,
		const std::string &raw)
{
	FormSpecElement element;
	if (element.create(raw))
		return nullptr;

	auto parser = parsers.find(element.getType());
	if (parser == parsers.end() && state->formspec_version <= FORMSPEC_API_VERSION) {
		errorstream << "Unknown FormSpec element type \"" << element.getType() <<
				"\"\n" << "Full element: \"" << raw << "]\"" << std::endl;
		return nullptr;
	}

	std::unique_ptr<ElementSpec> spec(new ElementSpec);
	if (parser->second(state, &element, spec.get()))
		return nullptr;

	return spec;
}
