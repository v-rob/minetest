// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2023 v-rob, Vincent Robinson <robinsonvincent89@gmail.com>

#include "ui/manager.h"

#include "debug.h"
#include "log.h"
#include "settings.h"
#include "client/client.h"
#include "client/renderingengine.h"
#include "client/texturesource.h"
#include "client/tile.h"
#include "util/serialize.h"

namespace ui
{
	video::ITexture *Manager::getTexture(const std::string &name) const
	{
		return m_client->tsrc()->getTexture(name);
	}

	float Manager::getScale(WindowType type) const
	{
		if (type == WindowType::GUI || type == WindowType::CHAT) {
			return m_gui_scale;
		}
		return m_hud_scale;
	}

	void Manager::reset()
	{
		m_client = nullptr;

		m_windows.clear();
	}

	void Manager::removeWindow(u64 id)
	{
		auto it = m_windows.find(id);
		if (it == m_windows.end()) {
			errorstream << "Window " << id << " is already closed" << std::endl;
			return;
		}

		m_windows.erase(it);
	}

	void Manager::receiveMessage(const std::string &data)
	{
		auto is = newIs(data);

		u32 action = readU8(is);
		u64 id = readU64(is);

		switch (action) {
		case REOPEN_WINDOW: {
			u64 close_id = readU64(is);
			removeWindow(close_id);

			[[fallthrough]];
		}

		case OPEN_WINDOW: {
			auto it = m_windows.find(id);
			if (it != m_windows.end()) {
				errorstream << "Window " << id << " is already open" << std::endl;
				break;
			}

			it = m_windows.emplace(id, id).first;
			if (!it->second.read(is, true)) {
				errorstream << "Fatal error when opening window " << id <<
					"; closing window" << std::endl;
				removeWindow(id);
				break;
			}
			break;
		}

		case UPDATE_WINDOW: {
			auto it = m_windows.find(id);
			if (it == m_windows.end()) {
				errorstream << "Window " << id << " does not exist" << std::endl;
			}

			if (!it->second.read(is, false)) {
				errorstream << "Fatal error when updating window " << id <<
					"; closing window" << std::endl;
				removeWindow(id);
				break;
			}
			break;
		}

		case CLOSE_WINDOW:
			removeWindow(id);
			break;

		default:
			errorstream << "Invalid manager action: " << action << std::endl;
			break;
		}
	}

	void Manager::preDraw()
	{
		float base_scale = RenderingEngine::getDisplayDensity();
		m_gui_scale = base_scale * g_settings->getFloat("gui_scaling");
		m_hud_scale = base_scale * g_settings->getFloat("hud_scaling");
	}

	void Manager::drawType(WindowType type)
	{
		for (auto &it : m_windows) {
			if (it.second.getType() == type) {
				it.second.drawAll();
			}
		}
	}

	Manager g_manager;
}
