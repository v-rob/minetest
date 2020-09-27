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

#include "client/guiscalingfilter.h"
#include "l_drawer.h"
#include "l_internal.h"
#include "script/common/c_converter.h"

// get_window_size(self)
int LuaScreenDrawer::l_get_window_size(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;

	LuaScreenDrawer *drawer = checkobject(L, 1);
	if (drawer == nullptr)
		return 0;

	core::dimension2d<u32> size = drawer->driver->getScreenSize();
	push_v2f(L, v2f(size.Width, size.Height));

	return 1;
}

#include <iostream>
#define DEBUG(x) std::cout << x << std::endl

// {to_draw, rect, color[, clip_rect]}
static void draw_rect(lua_State *L, LuaScreenDrawer *drawer)
{
	DEBUG("draw_rect");
	DEBUG("2");
	lua_rawgeti(L, -1, 2);
	core::recti rect = read_recti(L, -1);
	lua_pop(L, 1);

	DEBUG("3");
	lua_rawgeti(L, -1, 3);
	video::SColor color(0x0);
	read_color(L, -1, &color);
	lua_pop(L, 1);

	DEBUG("4");
	lua_rawgeti(L, -1, 4);
	if (lua_istable(L, -1)) {
		core::recti clip_rect = read_recti(L, -1);
		drawer->driver->draw2DRectangle(color, rect, &clip_rect);
	} else {
		drawer->driver->draw2DRectangle(color, rect);
	}
	lua_pop(L, 1);
}

// {to_draw, rect, image[, clip_rect][, middle_rect][, from_rect][, recolor]}
static void draw_image(lua_State *L, LuaScreenDrawer *drawer)
{
	DEBUG("draw_image");
	DEBUG("2");
	lua_rawgeti(L, -1, 2);
	core::recti rect = read_recti(L, -1);
	lua_pop(L, 1);

	DEBUG("3");
	lua_rawgeti(L, -1, 3);
	video::ITexture *texture = drawer->tsrc->getTexture(lua_tostring(L, -1));
	lua_pop(L, 1);

	DEBUG("4");
	lua_rawgeti(L, -1, 4);
	core::recti clip_rect;
	bool use_clip = false;
	if (lua_istable(L, -1)) {
		clip_rect = read_recti(L, -1);
		use_clip = true;
	}
	lua_pop(L, 1);

	DEBUG("6");
	lua_rawgeti(L, -1, 6);
	core::recti from_rect;
	if (lua_istable(L, -1))
		from_rect = read_recti(L, -1);
	else
		from_rect = core::rect<s32>(core::position2d<s32>(0, 0),
			core::dimension2di(texture->getOriginalSize()));
	lua_pop(L, 1);

	DEBUG("7");
	lua_rawgeti(L, -1, 7);
	video::SColor color(0xFFFFFFFF);
	if (!lua_isnil(L, -1))
		read_color(L, -1, &color);
	const video::SColor colors[] = {color, color, color, color};
	lua_pop(L, 1);

	DEBUG("5");
	lua_rawgeti(L, -1, 5);
	if (lua_istable(L, -1)) {
		core::recti middle_rect = read_recti(L, -1);
		draw2DImage9Slice(drawer->driver, texture, rect, from_rect, middle_rect,
			use_clip ? &clip_rect : nullptr, colors);
	} else {
		draw2DImageFilterScaled(drawer->driver, texture, rect, from_rect,
			use_clip ? &clip_rect : nullptr, colors, true);
	}
	lua_pop(L, 1);
}

// draw(self, definitions)
int LuaScreenDrawer::l_draw(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;

	LuaScreenDrawer *drawer = checkobject(L, 1);
	if (drawer == nullptr)
		return 0;

	size_t size = lua_objlen(L, 2);
	for (size_t i = 1; i <= size; i++) {
		// Get current array element
		lua_rawgeti(L, 2, i);

		// Get what to draw from first index
		lua_rawgeti(L, -1, 1);
		s32 to_draw = lua_tointeger(L, -1);
		lua_pop(L, 1);

		// Draw using the respective function; these numbers are the same as the
		// gui.DRAW_* constants
		switch (to_draw) {
		case 0:
			draw_rect(L, drawer);
			break;
		case 1:
			draw_image(L, drawer);
			break;
		default:
			break;
		}

		// Pop the current array element
		lua_pop(L, 1);
	}

	return 0;
}

int LuaScreenDrawer::create_object(lua_State *L, video::IVideoDriver *driver,
	ISimpleTextureSource *tsrc)
{
	LuaScreenDrawer *drawer = new LuaScreenDrawer;

	drawer->driver = driver;
	drawer->tsrc = tsrc;

	*(void **)(lua_newuserdata(L, sizeof(void *))) = drawer;
	luaL_getmetatable(L, className);
	lua_setmetatable(L, -2);
	return 1;
}

LuaScreenDrawer *LuaScreenDrawer::checkobject(lua_State *L, int narg)
{
	NO_MAP_LOCK_REQUIRED;

	luaL_checktype(L, narg, LUA_TUSERDATA);
	void *drawer = luaL_checkudata(L, narg, className);
	if (!drawer)
		luaL_typerror(L, narg, className);
	return *(LuaScreenDrawer **)drawer;
}

int LuaScreenDrawer::gc_object(lua_State *L)
{
	LuaScreenDrawer *drawer = *(LuaScreenDrawer **)(lua_touserdata(L, 1));
	delete drawer;
	return 0;
}

void LuaScreenDrawer::Register(lua_State *L)
{
	lua_newtable(L);
	int methodtable = lua_gettop(L);
	luaL_newmetatable(L, className);
	int metatable = lua_gettop(L);

	lua_pushliteral(L, "__metatable");
	lua_pushvalue(L, methodtable);
	lua_settable(L, metatable);

	lua_pushliteral(L, "__index");
	lua_pushvalue(L, methodtable);
	lua_settable(L, metatable);

	lua_pushliteral(L, "__gc");
	lua_pushcfunction(L, gc_object);
	lua_settable(L, metatable);

	lua_pop(L, 1);

	luaL_openlib(L, 0, methods, 0);
	lua_pop(L, 1);
}

const char LuaScreenDrawer::className[] = "ScreenDrawer";
const luaL_Reg LuaScreenDrawer::methods[] =
{
	luamethod(LuaScreenDrawer, get_window_size),
	luamethod(LuaScreenDrawer, draw),
	{ 0, 0 }
};
