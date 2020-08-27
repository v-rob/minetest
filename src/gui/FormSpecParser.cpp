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

#include <unordered_map>
#include <vector>
#include "FormSpecParser.h"
#include "FormSpecElement.h"
#include "log.h"
#include "network/networkprotocol.h"

namespace {
	void low_parse_element(ParserState *state, ElementSpec *spec,
			const std::string &name)
	{
		if (!state->real_coordinates)
			spec->setBool("old_coord", true);

		if (state->focused_element == name)
			spec->setBool("focused", true);
	}

	void low_parse_button(ParserState *state, FormSpecElement *element,
			ElementSpec *spec, bool is_normal)
	{
		spec->setElementType(ELEMENT_BUTTON);

		spec->setVector2df("pos",  element->arg(0)->asVector2df());
		spec->setVector2df("size", element->arg(1)->asVector2df());
		spec->setWideString("label", element->arg(4 - is_normal)->asWideString());

		std::string name = element->arg(3 - is_normal)->asString();
		spec->setString("name", name);

		low_parse_element(state, spec, name);
	}

	bool parse_button(ParserState *state, FormSpecElement *element, ElementSpec *spec)
	{
		if (element->splitArguments(state->formspec_version, {4}))
			return true;

		low_parse_button(state, element, spec, true);

		if (element->getType() == "button_exit")
			spec->setBool("exit", true);

		return element->hasInvalidArgument();
	}

	bool parse_image_button(ParserState *state, FormSpecElement *element,
			ElementSpec *spec)
	{
		if (element->splitArguments(state->formspec_version, {5, 7, 8}))
			return true;

		low_parse_button(state, element, spec, false);

		spec->setString("image", element->arg(2)->asString());

		if (element->getType() == "image_button_exit")
			spec->setBool("exit", true);

		// Temporary until style integration
		if (element->size() >= 7) {
			spec->setBool("noclip", element->arg(5)->asBool());
			spec->setBool("border", element->arg(6)->asBool());
		}
		if (element->size() >= 8)
			spec->setString("pressed_image", element->arg(7)->asString());

		return element->hasInvalidArgument();
	}

	bool parse_item_image_button(ParserState *state, FormSpecElement *element,
			ElementSpec *spec)
	{
		if (element->splitArguments(state->formspec_version, {5}))
			return true;

		low_parse_button(state, element, spec, false);

		spec->setString("item", element->arg(2)->asString());

		return element->hasInvalidArgument();
	}

#define PARSER(e) {#e, parse_##e}
#define EXTRAP(e, f) {#e, parse_##f}

	//! Map of element types to parsing function. Functions return true on failure
	// TODO: explicit_size
	const std::unordered_map<std::string,
			bool (*)(ParserState *, FormSpecElement *, ElementSpec *)> parsers = {
		PARSER(button),
		EXTRAP(button_exit, button),
		PARSER(image_button),
		EXTRAP(image_button_exit, image_button),
		PARSER(item_image_button)
	};

#undef EXTRAP
#undef PARSER
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
		/* TODO: Uncomment when element parsing is done
		errorstream << "Unknown formspec element type \"" << element.getType() <<
				"\"\n" << "Note: Full element: \"" << raw << "]\"" << std::endl; */
		return nullptr;
	}

	std::unique_ptr<ElementSpec> spec(new ElementSpec);
	if (parser->second(state, &element, spec.get()) ||
			spec->getElementType() == ELEMENT_NONE)
		return nullptr;

	return spec;
}
