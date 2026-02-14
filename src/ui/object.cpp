// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2025 v-rob, Vincent Robinson <robinsonvincent89@gmail.com>

#include "ui/object.h"

#include "debug.h"
#include "log.h"
#include "porting.h"
#include "client/fontengine.h"
#include "ui/box.h"
#include "ui/elem.h"
#include "ui/manager.h"
#include "ui/window.h"
#include "util/serialize.h"

namespace ui
{
	Elem &IObject::getElem()
	{
		return m_box.getElem();
	}

	const Elem &IObject::getElem() const
	{
		return m_box.getElem();
	}

	Window &IObject::getWindow()
	{
		return getElem().getWindow();
	}

	const Window &IObject::getWindow() const
	{
		return getElem().getWindow();
	}

	void LabelObject::reset()
	{
		m_label = L"";
		m_font = nullptr;
	}

	void LabelObject::read(std::istream &full_is)
	{
		auto is = newIs(readStr16(full_is));
		u32 set_mask = readU32(is);

		if (testShift(set_mask))
			m_label = utf8_to_wide(readStr16(is));
	}

	void LabelObject::restyle()
	{
		const StyleProps &style = getBox().getStyle();

		FontSpec spec(style.text.size, style.text.mono ? FM_Mono : FM_Standard,
			style.text.bold, style.text.italic);
		m_font = g_fontengine->getFont(spec);

		m_display_rect = RectF();
		m_clip_rect = RectF();
	}

	SizeF LabelObject::resize()
	{
		return getWindow().getTextSize(m_font, m_label);
	}

	void LabelObject::relayout(RectF layout_rect, RectF layout_clip)
	{
		m_display_rect = layout_rect;
		m_clip_rect = layout_clip;
	}

	void LabelObject::draw()
	{
		const StyleProps &style = getBox().getStyle();

		getWindow().drawText(m_display_rect, m_clip_rect, m_font, m_label,
			style.text.color, style.text.mark, style.text.align, style.text.valign);
	}

	void IconObject::reset()
	{
		m_icon = nullptr;

		m_tint = WHITE;
		m_slice = RectF(0.0f, 0.0f, 1.0f, 1.0f);

		m_frames = 1;
		m_frame_time = 1000;
	}

	void IconObject::read(std::istream &full_is)
	{
		auto is = newIs(readStr16(full_is));
		u32 set_mask = readU32(is);

		if (testShift(set_mask))
			m_icon = g_manager.getTexture(readNullStr(is));

		if (testShift(set_mask))
			m_tint = readARGB8(is);
		if (testShift(set_mask))
			m_slice = readRectF(is);

		if (testShift(set_mask))
			m_frames = std::max(readU32(is), 1U);
		if (testShift(set_mask))
			m_frame_time = std::max(readU32(is), 1U);
	}

	void IconObject::restyle()
	{
		m_display_rect = RectF();
		m_clip_rect = RectF();
	}

	SizeF IconObject::resize()
	{
		const StyleProps &style = getBox().getStyle();

		m_icon_src = m_slice;

		if (m_frames > 1) {
			float frame_height = m_icon_src.H() / m_frames;
			m_icon_src.B = m_icon_src.T + frame_height;

			s32 frames_elapsed = (porting::getTimeMs() / m_frame_time);
			float frame_offset = frame_height * (frames_elapsed % m_frames);

			m_icon_src.T += frame_offset;
			m_icon_src.B += frame_offset;
		}

		if (style.obj.fit == ObjectFit::FIXED) {
			return m_icon_src.size() * getTextureSize(m_icon) * style.obj.scale;
		}

		return SizeF();
	}

	void IconObject::relayout(RectF layout_rect, RectF layout_clip)
	{
		const StyleProps &style = getBox().getStyle();

		SizeF base_size = m_icon_src.size() * getTextureSize(m_icon);
		SizeF size;

		switch (style.obj.fit) {
		case ObjectFit::FIXED:
			size = base_size * style.obj.scale;
			break;

		case ObjectFit::FILL:
			size = layout_rect.size();
			break;

		case ObjectFit::CONTAIN:
			size = base_size *
				std::min(layout_rect.W() / base_size.W, layout_rect.H() / base_size.H);
			break;

		case ObjectFit::COVER:
			size = base_size *
				std::max(layout_rect.W() / base_size.W, layout_rect.H() / base_size.H);
			break;
		}

		PosF pos = style.obj.align * (layout_rect.size() - size);

		m_display_rect = RectF(layout_rect.TopLeft + SizeF(pos), size);
		m_clip_rect = layout_clip;
	}

	void IconObject::draw()
	{
		getWindow().drawTexture(m_display_rect, m_clip_rect, m_icon, m_icon_src, m_tint);
	}
}
