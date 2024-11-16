// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>

#include "serialize.h"
#include "porting.h"
#include "util/string.h"
#include "util/hex.h"
#include "exceptions.h"
#include "irrlichttypes.h"

#include <iostream>
#include <cassert>

FloatType g_serialize_f32_type = FLOATTYPE_UNKNOWN;


////
//// String
////

std::string serializeString16(std::string_view plain, bool truncate)
{
	std::string s;
	size_t size = plain.size();

	if (size > STRING_MAX_LEN) {
		if (truncate) {
			size = STRING_MAX_LEN;
		} else {
			throw SerializationError("String too long for serializeString16");
		}
	}

	char size_buf[2];
	writeU16((u8 *)size_buf, size);

	s.reserve(2 + size);
	s.append(size_buf, 2);
	s.append(plain.substr(0, size));

	return s;
}

std::string deSerializeString16(std::istream &is, bool truncate)
{
	std::string s;
	char size_buf[2];

	is.read(size_buf, 2);
	if (is.gcount() != 2) {
		if (truncate) {
			return s;
		}
		throw SerializationError("deSerializeString16: size not read");
	}

	u16 size = readU16((u8 *)size_buf);
	if (size == 0) {
		return s;
	}

	s.resize(size);
	is.read(&s[0], size);
	if (truncate) {
		s.resize(is.gcount());
	} else if (is.gcount() != size) {
		throw SerializationError("deSerializeString16: couldn't read all chars");
	}

	return s;
}


////
//// Long String
////

std::string serializeString32(std::string_view plain, bool truncate)
{
	std::string s;
	size_t size = plain.size();

	if (size > LONG_STRING_MAX_LEN) {
		if (truncate) {
			size = LONG_STRING_MAX_LEN;
		} else {
			throw SerializationError("String too long for serializeString32");
		}
	}

	char size_buf[4];
	writeU32((u8 *)size_buf, size);

	s.reserve(4 + size);
	s.append(size_buf, 4);
	s.append(plain.substr(0, size));

	return s;
}

std::string deSerializeString32(std::istream &is, bool truncate)
{
	std::string s;
	char size_buf[4];

	is.read(size_buf, 4);
	if (is.gcount() != 4) {
		if (truncate) {
			return s;
		}
		throw SerializationError("deSerializeString32: size not read");
	}

	u32 size = readU32((u8 *)size_buf);
	u32 ignore = 0;
	if (size == 0) {
		return s;
	}

	if (size > LONG_STRING_MAX_LEN) {
		if (truncate) {
			ignore = size - LONG_STRING_MAX_LEN;
			size = LONG_STRING_MAX_LEN;
		} else {
			// We don't really want a remote attacker to force us to allocate 4GB...
			throw SerializationError("deSerializeString32: "
				"string too long: " + itos(size) + " bytes");
		}
	}

	s.resize(size);
	is.read(&s[0], size);
	if (truncate) {
		s.resize(is.gcount());
	} else if (is.gcount() != size) {
		throw SerializationError("deSerializeString32: couldn't read all chars");
	}

	// If the string was truncated due to exceeding the string max length, we
	// need to ignore the rest of the characters.
	if (truncate) {
		is.seekg(ignore, std::ios_base::cur);
	}

	return s;
}

////
//// JSON-like strings
////

std::string serializeJsonString(std::string_view plain)
{
	std::string tmp;

	tmp.reserve(plain.size() + 2);
	tmp.push_back('"');

	for (char c : plain) {
		switch (c) {
			case '"':
				tmp.append("\\\"");
				break;
			case '\\':
				tmp.append("\\\\");
				break;
			case '\b':
				tmp.append("\\b");
				break;
			case '\f':
				tmp.append("\\f");
				break;
			case '\n':
				tmp.append("\\n");
				break;
			case '\r':
				tmp.append("\\r");
				break;
			case '\t':
				tmp.append("\\t");
				break;
			default: {
				if (c >= 32 && c <= 126) {
					tmp.push_back(c);
				} else {
					// We pretend that Unicode codepoints map to bytes (they don't)
					u8 cnum = static_cast<u8>(c);
					tmp.append("\\u00");
					tmp.push_back(hex_chars[cnum >> 4]);
					tmp.push_back(hex_chars[cnum & 0xf]);
				}
				break;
			}
		}
	}

	tmp.push_back('"');
	return tmp;
}

static void deSerializeJsonString(std::string &s)
{
	assert(s.size() >= 2);
	assert(s.front() == '"' && s.back() == '"');

	size_t w = 0; // write index
	size_t i = 1; // read index
	const size_t len = s.size() - 1; // string length with trailing quote removed

	while (i < len) {
		char c = s[i++];
		assert(c != '"');

		if (c != '\\') {
			s[w++] = c;
			continue;
		}

		if (i >= len)
			throw SerializationError("JSON string ended prematurely");
		char c2 = s[i++];
		switch (c2) {
			case 'b':
				s[w++] = '\b';
				break;
			case 'f':
				s[w++] = '\f';
				break;
			case 'n':
				s[w++] = '\n';
				break;
			case 'r':
				s[w++] = '\r';
				break;
			case 't':
				s[w++] = '\t';
				break;
			case 'u': {
				if (i + 3 >= len)
					throw SerializationError("JSON string ended prematurely");
				unsigned char v[4] = {};
				for (int j = 0; j < 4; j++)
					hex_digit_decode(s[i+j], v[j]);
				i += 4;
				u32 hexnumber = (v[0] << 12) | (v[1] << 8) | (v[2] << 4) | v[3];
				// Note that this does not work for anything other than ASCII
				// but these functions do not actually interact with real JSON input.
				s[w++] = (int) hexnumber;
				break;
			}
			default:
				s[w++] = c2;
				break;
		}
	}

	assert(w <= i && i <= len);
	// Truncate string to current write index
	s.resize(w);
}

std::string deSerializeJsonString(std::istream &is)
{
	std::string tmp;
	char c;
	bool was_backslash = false;

	// Parse initial doublequote
	c = is.get();
	if (c != '"')
		throw SerializationError("JSON string must start with doublequote");
	tmp.push_back(c);

	// Grab the entire json string
	for (;;) {
		c = is.get();
		if (is.eof())
			throw SerializationError("JSON string ended prematurely");

		tmp.push_back(c);
		if (was_backslash)
			was_backslash = false;
		else if (c == '\\')
			was_backslash = true;
		else if (c == '"')
			break; // found end of string
	}

	deSerializeJsonString(tmp);
	return tmp;
}

std::string serializeJsonStringIfNeeded(std::string_view s)
{
	for (size_t i = 0; i < s.size(); ++i) {
		if (s[i] <= 0x1f || s[i] >= 0x7f || s[i] == ' ' || s[i] == '\"')
			return serializeJsonString(s);
	}
	return std::string(s);
}

std::string deSerializeJsonStringIfNeeded(std::istream &is)
{
	// Check for initial quote
	char c = is.peek();
	if (is.eof())
		return "";

	if (c == '"') {
		// json string: defer to the right implementation
		return deSerializeJsonString(is);
	}

	// not a json string:
	std::string tmp;
	std::getline(is, tmp, ' ');
	if (!is.eof())
		is.unget(); // we hit a space, put it back
	return tmp;
}

