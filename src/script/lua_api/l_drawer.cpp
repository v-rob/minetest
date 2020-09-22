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

// rect(self, rect, color[, clip_rect])
int LuaScreenDrawer::l_rect(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;

	LuaScreenDrawer *drawer = checkobject(L, 1);
	if (drawer == nullptr)
		return 0;

	core::recti rect = read_recti(L, 2);
	video::SColor color(0);
	read_color(L, 3, &color);

	if (lua_istable(L, 4)) {
		core::recti clip_rect = read_recti(L, 4);
		drawer->driver->draw2DRectangle(color, rect, &clip_rect);
	} else {
		drawer->driver->draw2DRectangle(color, rect);
	}

	return 1;
}

// image(self, rect, image[, clip_rect][, middle_rect][, from_rect][, recolor])
int LuaScreenDrawer::l_image(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;

	LuaScreenDrawer *drawer = checkobject(L, 1);
	if (drawer == nullptr)
		return 0;

	core::recti rect = read_recti(L, 2);
	video::ITexture *texture = drawer->tsrc->getTexture(luaL_checkstring(L, 3));

	core::recti *clip_rect = nullptr;
	if (lua_istable(L, 4))
		clip_rect = new core::recti(read_recti(L, 4));

	core::recti from_rect;
	if (lua_istable(L, 6))
		from_rect = read_recti(L, 6);
	else
		from_rect = core::rect<s32>(core::position2d<s32>(0, 0),
			core::dimension2di(texture->getOriginalSize()));

	video::SColor color(255, 255, 255, 255);
	if (!lua_isnil(L, 7))
		read_color(L, 7, &color);
	const video::SColor colors[] = {color, color, color, color};

	if (lua_istable(L, 5)) {
		core::recti middle_rect = read_recti(L, 5);
		draw2DImage9Slice(
			drawer->driver, texture, rect, from_rect, middle_rect, clip_rect, colors);
	} else {
		draw2DImageFilterScaled(
			drawer->driver, texture, rect, from_rect, clip_rect, colors, true);
	}

	delete clip_rect;

	return 1;
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
	void *ud = luaL_checkudata(L, narg, className);
	if (!ud)
		luaL_typerror(L, narg, className);
	return *(LuaScreenDrawer **)ud;
}

int LuaScreenDrawer::gc_object(lua_State *L)
{
	LuaScreenDrawer *o = *(LuaScreenDrawer **)(lua_touserdata(L, 1));
	delete o;
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
	luamethod(LuaScreenDrawer, rect),
	luamethod(LuaScreenDrawer, image),
	{ 0, 0 }
};
