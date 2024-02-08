/*
Minetest
Copyright (C) 2023 v-rob, Vincent Robinson <robinsonvincent89@gmail.com>

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

#include "irrlichttypes_extrabloated.h"
#include "gui/texture.h"
#include "gui/window.h"
#include "util/basic_macros.h"

#include <iostream>
#include <map>
#include <sstream>
#include <string>

class Client;

union SDL_Event;

namespace ui
{
	// Define a few functions that are particularly useful for UI serialization
	// and deserialization.
	bool testShift(u32 &bits);

	// The UI purposefully avoids dealing with SerializationError, so it uses
	// always uses truncating or null-terminated string functions. Hence, we
	// make convenience wrappers around the string functions in "serialize.h".
	std::string readStr16(std::istream &is);
	std::string readStr32(std::istream &is);
	std::string readNullStr(std::istream &is);

	void writeStr16(std::ostream &os, const std::string &str);
	void writeStr32(std::ostream &os, const std::string &str);
	void writeNullStr(std::ostream &os, const std::string &str);

	// Convenience functions to create new binary string streams.
	std::istringstream newIs(std::string str);
	std::ostringstream newOs();

	/* Custom UI-specific event types for events of type SDL_USEREVENT. Create
	 * the event structure with createUiEvent().
	 *
	 * Some events should always return false to give parent elements and other
	 * boxes a chance to see the event. Other events return true to indicate
	 * that the element may become focused or hovered.
	 */
	enum UiEvent
	{
		UI_FOCUS_REQUEST, // Return true to accept request.
		UI_FOCUS_CHANGED, // Never return true.
		UI_FOCUS_SUBVERTED, // Not sent to parent elements. Never return true.

		UI_HOVER_REQUEST, // Return true to accept request.
		UI_HOVER_CHANGED,
	};

#define UI_USER(event) (UI_##event + SDL_USEREVENT)

	SDL_Event createUiEvent(UiEvent type, void *data1 = nullptr, void *data2 = nullptr);

	class Manager
	{
	public:
		// Serialized enum; do not change values of entries.
		enum ReceiveAction
		{
			OPEN_WINDOW   = 0x00,
			REOPEN_WINDOW = 0x01,
			UPDATE_WINDOW = 0x02,
			CLOSE_WINDOW  = 0x03,
		};

		// Serialized enum; do not change values of entries.
		enum SendAction
		{
			WINDOW_EVENT = 0x00,
			ELEM_EVENT   = 0x01,
		};

	private:
		Client *m_client;

		float m_gui_pixel_size = 0.0f;
		float m_hud_pixel_size = 0.0f;

		// Use map rather than unordered_map so that windows are always sorted
		// by window ID to make sure that they are drawn in order of creation.
		std::map<u64, Window> m_windows;

		// Keep track of which GUI windows are currently open. We also use a
		// map so we can easily find the topmost window.
		std::map<u64, Window *> m_gui_windows;

	public:
		Manager()
		{
			reset();
		}

		DISABLE_CLASS_COPY(Manager)

		Client *getClient() const { return m_client; }
		void setClient(Client *client) { m_client = client; }

		Texture getTexture(const std::string &name) const;

		float getPixelSize(WindowType type) const;
		d2f32 getScreenSize(WindowType type) const;

		v2f32 getPointerPos(WindowType type) const;
		bool isPointerPressed() const;

		void reset();
		void removeWindow(u64 id);

		void receiveMessage(const std::string &data);
		void sendMessage(const std::string &data);

		void preDraw();
		void drawType(WindowType type);

		Window *getFocused();

		bool isFocused() const;
		bool processInput(const SDL_Event &event);
	};

	extern Manager g_manager;

	// Inconveniently, we need a way to draw the "gui" window types after the
	// chat console but before other GUIs like the key change menu, formspecs,
	// etc. So, we inject our own mini Irrlicht element in between.
	class GUIManagerElem : public gui::IGUIElement
	{
	public:
		GUIManagerElem(gui::IGUIEnvironment* env, gui::IGUIElement* parent, s32 id) :
			gui::IGUIElement(gui::EGUIET_ELEMENT, env, parent, id, rs32())
		{}

		virtual void draw() override
		{
			g_manager.drawType(ui::WindowType::GUI);
			gui::IGUIElement::draw();
		}
	};
}
