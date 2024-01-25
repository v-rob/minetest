// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2023 v-rob, Vincent Robinson <robinsonvincent89@gmail.com>

#pragma once

#include "ui/box.h"
#include "ui/helpers.h"
#include "util/basic_macros.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace ui
{
	class Window;

	// The private inheritance is intentional; see below.
	class Elem : private ILayout
	{
	public:
		// Serialized enum; do not change values of entries.
		enum Type : u8
		{
			ELEM = 0x00,
			ROOT = 0x01,
		};

	private:
		// The window and ID are intrinsic to the element's identity, so they
		// are set by the constructor and aren't cleared in reset() or changed
		// in read().
		Window &m_window;
		std::string m_id;

		size_t m_order;

		Elem *m_parent;
		std::vector<Elem *> m_children;

		Box m_main_box;

	public:
		static std::unique_ptr<Elem> create(Type type, Window &window, std::string id);

		Elem(Window &window, std::string id);

		DISABLE_CLASS_COPY(Elem)

		virtual ~Elem() = default;

		Window &getWindow() { return m_window; }
		const Window &getWindow() const { return m_window; }

		const std::string &getId() const { return m_id; }
		virtual Type getType() const { return ELEM; }

		size_t getOrder() const { return m_order; }
		void setOrder(size_t order) { m_order = order; }

		Elem *getParent() { return m_parent; }
		const std::vector<Elem *> &getChildren() { return m_children; }

		Box &getMainBox() { return m_main_box; }

		virtual void reset();
		virtual void read(std::istream &is);

		void restyleAll() { getLayoutBox().restyle(); }
		void relayoutAll(RectF parent_rect, RectF parent_clip)
			{ getLayoutBox().relayout(parent_rect, parent_clip); }

		void drawAll() { getLayoutBox().draw(); }

	protected:
		virtual Box &getLayoutBox() { return m_main_box; }

	private:
		void readChildren(std::istream &is);

		/* Users of Elem shouldn't use these methods from ILayout directly,
		 * since they layout the element's children directly. Instead, the
		 * methods relayoutAll(), drawAll(), etc. should be used, which will
		 * layout all the boxes before laying out the element's children at the
		 * appropriate time. Hence, we use private inheritance (useful, for
		 * once!). The element itself decides which box is able to call these
		 * ILayout methods by using `box.setContent(this);`
		 */
		virtual void restyle() override;
		virtual void relayout(RectF parent_rect, RectF parent_clip) override;

		virtual void draw() override;
	};
}
