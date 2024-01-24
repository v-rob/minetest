/*
Minetest
Copyright (C) 2024 v-rob, Vincent Robinson <robinsonvincent89@gmail.com>

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

#include "gui/sizer_elems.h"

#include "debug.h"
#include "log.h"
#include "gui/manager.h"
#include "gui/texture.h"
#include "util/serialize.h"

namespace ui
{
	void Place::reset()
	{
		Elem::reset();

		m_scale = 0.0f;
	}

	void Place::read(std::istream &is)
	{
		auto super = newIs(readStr32(is));
		Elem::read(super);

		u32 set_mask = readU32(is);

		if (testShift(set_mask))
			m_scale = readF32(is);
	}

	void Place::layoutChildren()
	{
		const v2f32 &origin = getMainBox().getChildRect().UpperLeftCorner;

		for (Elem *elem : getChildren()) {
			const Style &child = elem->getMainBox().getStyle();

			// All we need to do is position the child from the top-left corner
			// of the sizer with absolute coordinates.
			rf32 elem_rect(origin + (child.pos * m_scale), child.span * m_scale);
			elem->layout(elem_rect, getMainBox().getChildClip());
		}
	}

	Dir toDir(u8 dir)
	{
		if (dir >= (u8)Dir::MAX_DIR) {
			return Dir::RIGHT;
		}
		return (Dir)dir;
	}

	Wrap toWrap(u8 wrap)
	{
		if (wrap >= (u8)Wrap::MAX_WRAP) {
			return Wrap::NONE;
		}
		return (Wrap)wrap;
	}

	void Flex::reset()
	{
		Elem::reset();

		m_dir = Dir::RIGHT;
		m_wrap = Wrap::NONE;
	}

	void Flex::read(std::istream &is)
	{
		auto super = newIs(readStr32(is));
		Elem::read(super);

		u32 set_mask = readU32(is);

		if (testShift(set_mask))
			m_dir = toDir(readU8(is));
		if (testShift(set_mask))
			m_wrap = toWrap(readU8(is));
	}

	struct FlexElem
	{
		float width = 0.0f;
		float weight = 0.0f;
		Elem *elem = nullptr;
	};

	struct FlexRun
	{
		float height = 0.0f;
		float total_width = 0.0f;
		float total_weight = 0.0f;
		std::vector<FlexElem> elems;
	};

	struct FlexLayout
	{
		float total_height = 0.0f;
		std::vector<FlexRun> runs;
	};

	void Flex::layoutChildren()
	{
		/* For simplicity of conceptualization, we pretend that the sizer is
		 * always horizontal, so we call the flex direction "X" and the wrap
		 * direction "Y". However, we still need to get the right coordinates
		 * when looking at the widths and heights of child elements and other
		 * such properties.
		 *
		 * So, the "x_dir" variable is 0 (Width) when the sizer really is
		 * horizontal, or 1 (Height) when the sizer is vertical and the
		 * opposite coordinate needs to be accessed. "y_dir" is the opposite.
		 * These can be passed to dim_at() to get the right coordinate.
		 */
		size_t x_dir = m_dir == Dir::UP || m_dir == Dir::DOWN;
		size_t y_dir = !x_dir;

		const Style &style = getMainBox().getStyle();
		const rf32 &rect = getMainBox().getChildRect();

		// We need to know the maximum amount of space we have so we can decide
		// when to wrap and how to allocate extra space.
		d2f32 max_size = rect.getSize();
		float max_width  = dim_at(max_size, x_dir);
		float max_height = dim_at(max_size, y_dir);

		// Create the base layout. Element gaps should only go between
		// elements, so we initially set the height to the negative gap because
		// we will add a gap with the first run, cancelling it out.
		FlexLayout layout;
		layout.total_height = -style.gap[y_dir];

		size_t i = 0;
		const std::vector<Elem *> &children = getChildren();

		// If there are no children in the sizer, preemptively quit so we don't
		// have to deal with the edge case of zero everywhere. As long as
		// there's a single element in the sizer, there will be at least one
		// run and at least one element in each run.
		if (children.size() == 0) {
			return;
		}

		// Loop until there are no more children in the container, packing each
		// row of children into a flex run.
		while (i < children.size()) {
			// Each run has gap between elements, so we do the same trick.
			FlexRun run;
			run.total_width = -style.gap[x_dir];

			// Starting where we left off after the last run, start adding
			// children into the run until we run out of space (for a wrapping
			// flex sizer) or until there are no more children.
			for (; i < children.size(); i++) {
				const Style &child = children[i]->getMainBox().getStyle();

				// We can determine the width and weight of each child element
				// directly.
				FlexElem elem;
				elem.width = dim_at(child.size, x_dir);
				elem.weight = child.weight;
				elem.elem = children[i];

				/* If this is a wrapping container, check if this element would
				 * cause us to run out of space. If so, we leave this element
				 * and break out of the inner element loop to finalize the run.
				 *
				 * However, if there are no elements in the run yet, then this
				 * element is too large to fit in any run. So, we leave it in
				 * the current run and let it overflow the width.
				 */
				float new_total_width = run.total_width + style.gap[x_dir] + elem.width;
				if (m_wrap != Wrap::NONE &&
						new_total_width >= max_width && run.elems.size() > 0) {
					break;
				}

				// The height of the entire run is the maximum height of all
				// the elements in the run.
				run.height = std::max(run.height, dim_at(child.size, y_dir));

				// Add the height and weight of this element to the totals and
				// push it to the run.
				run.total_width = new_total_width;
				run.total_weight += elem.weight;
				run.elems.push_back(std::move(elem));
			}

			// Do the same to the run, now that we've completed it.
			layout.total_height += style.gap[y_dir] + run.height;
			layout.runs.push_back(std::move(run));
		}

		// Now that the elements have been partitioned into runs, we can
		// allocate extra space to weighted elements and position everything.

		// Since flex sizers can position elements in any direction, we need to
		// know where the first position is and whether to position the next
		// element/run forwards or backwards.
		bool x_reverse = m_dir == Dir::LEFT || m_dir == Dir::UP;
		bool y_reverse = m_wrap == Wrap::BACKWARD;

		float x_flipper = x_reverse ? -1.0f : 1.0f;
		float y_flipper = y_reverse ? -1.0f : 1.0f;

		float vpos = y_reverse ?
			rect.LowerRightCorner[y_dir] : rect.UpperLeftCorner[y_dir];

		// For the vertical direction, we unconditionally consult the spacing
		// rules to determine how to allocate extra space if we have any.
		float extra_height = max_height - layout.total_height;
		float full_vgap = style.gap[y_dir];

		Spacing hspacing = x_dir ? style.vspacing : style.hspacing;
		Spacing vspacing = y_dir ? style.vspacing : style.hspacing;

		if (extra_height > 0.0f) {
			switch (vspacing) {
			case Spacing::BEFORE:
				vpos += extra_height * y_flipper;
				break;

			case Spacing::OUTSIDE:
				vpos += (extra_height / 2.0f) * y_flipper;
				break;

			case Spacing::AROUND: {
				float space = extra_height / layout.runs.size();
				full_vgap += space;
				vpos += (space / 2.0f) * y_flipper;
				break;
			}

			case Spacing::BETWEEN:
				full_vgap += extra_height / (layout.runs.size() - 1);
				break;

			case Spacing::EVENLY: {
				float space = extra_height / (layout.runs.size() + 1);
				full_vgap += space;
				vpos += space * y_flipper;
				break;
			}

			case Spacing::REMOVE:
				for (FlexRun &run : layout.runs) {
					run.height += extra_height / layout.runs.size();
				}
				break;

			default:
				break;
			}
		}

		for (FlexRun &run : layout.runs) {
			// If we're moving in reverse, we have to move up by the run's
			// height since rectangles are positioned by their top right.
			if (y_reverse) {
				vpos -= run.height;
			}

			float hpos = x_reverse ?
				rect.LowerRightCorner[x_dir] : rect.UpperLeftCorner[x_dir];

			// Within a run, we only apply the spacing rules for extra space if
			// every element in the run has a weight of zero.
			float extra_width = max_width - run.total_width;
			float full_hgap = style.gap[x_dir];

			if (run.total_weight == 0.0f && extra_width > 0.0f) {
				switch (hspacing) {
				case Spacing::BEFORE:
					hpos += extra_width * x_flipper;
					break;

				case Spacing::OUTSIDE:
					hpos += (extra_width / 2.0f) * x_flipper;
					break;

				case Spacing::AROUND: {
					float space = extra_width / run.elems.size();
					full_hgap += space;
					hpos += (space / 2.0f) * x_flipper;
					break;
				}

				case Spacing::BETWEEN:
					full_hgap += extra_width / (run.elems.size() - 1);
					break;

				case Spacing::EVENLY: {
					float space = extra_width / (run.elems.size() + 1);
					full_hgap += space;
					hpos += space * x_flipper;
					break;
				}

				case Spacing::REMOVE:
					for (FlexElem &elem : run.elems) {
						elem.width += extra_width / run.elems.size();
					}
					break;

				default:
					break;
				}
			}

			for (FlexElem &elem : run.elems) {
				// If we have extra vertical space and this run is weighted,
				// then give it extra space proportional to its weight.
				if (extra_width > 0.0f && elem.weight != 0.0f) {
					elem.width += (elem.weight / run.total_weight) * extra_width;
				}

				// The same logic for moving backwards vs. forwards on runs
				// applies to elements identically.
				if (x_reverse) {
					hpos -= elem.width;
				}

				// Now we can position the element straightforwardly in the
				// position and dimensions we've calculated, although switching
				// from our logical coordinates back to actual coordinates.
				rf32 elem_rect(
					v2f32(
						x_dir ? vpos : hpos,
						y_dir ? vpos : hpos
					),
					d2f32(
						x_dir ? run.height : elem.width,
						y_dir ? run.height : elem.width
					)
				);

				elem.elem->layout(elem_rect, getMainBox().getChildClip());

				if (x_reverse) {
					hpos -= full_hgap;
				} else {
					hpos += elem.width + full_hgap;
				}
			}

			// If we're moving in reverse, we need to move back again to adjust
			// for the gap after the run. If forwards, we move down by the run
			// height and the gap in one step.
			if (y_reverse) {
				vpos -= full_vgap;
			} else {
				vpos += run.height + full_vgap;
			}
		}
	}

	void Grid::reset()
	{
		Elem::reset();

		for (size_t x_dir = 0; x_dir <= 1; x_dir++) {
			m_sizes[x_dir].clear();
			m_weights[x_dir].clear();
		}
	}

	void Grid::read(std::istream &is)
	{
		auto super = newIs(readStr32(is));
		Elem::read(super);

		u32 set_mask = readU32(is);

		for (size_t x_dir = 0; x_dir <= 1; x_dir++) {
			if (testShift(set_mask)) {
				u32 num_sizes = readU32(is);

				for (size_t i = 0; i < num_sizes; i++) {
					m_sizes[x_dir].push_back(readF32(is));
				}
			}
		}

		for (size_t x_dir = 0; x_dir <= 1; x_dir++) {
			if (testShift(set_mask)) {
				u32 num_weights = readU32(is);

				for (size_t i = 0; i < num_weights; i++) {
					m_weights[x_dir].push_back(readF32(is));
				}
			}
		}
	}

	struct GridColumn
	{
		float left = 0.0f;
		float right = 0.0f;
		float width = 0.0f;

		float orig_width = 0.0f;
		float weight = 0.0f;
	};

	struct GridLayout
	{
		float total_width = 0.0f;
		float total_weight = 0.0f;
		std::vector<GridColumn> cols;
	};

	void Grid::layoutChildren()
	{
		/* We pull a naming trick similar to what we did in the flex sizer: we
		 * pretend that we're only dealing in the X direction with columns,
		 * whereas we actually use a loop that loops twice, once for columns
		 * and once for rows. We use the loop index to decide which dimension
		 * to use when accessing a two-dimensional field.
		 */
		std::array<GridLayout, 2> layouts;

		const Style &style = getMainBox().getStyle();
		const rf32 &rect = getMainBox().getChildRect();

		for (size_t x_dir = 0; x_dir <= 1; x_dir++) {
			GridLayout &layout = layouts[x_dir];

			// First, allocate enough columns to account for all the minimum
			// sizes and weights the user provided us, fill them in, and total
			// the weights.
			layout.cols.resize(
				std::max(m_weights[x_dir].size(), m_sizes[x_dir].size()));

			for (size_t i = 0; i < m_sizes[x_dir].size(); i++) {
				layout.cols[i].orig_width = m_sizes[x_dir][i];
				layout.cols[i].width = m_sizes[x_dir][i];
			}
			for (size_t i = 0; i < m_weights[x_dir].size(); i++) {
				layout.cols[i].weight = m_weights[x_dir][i];
				layout.total_weight += m_weights[x_dir][i];
			}

			for (Elem *elem : getChildren()) {
				const Style &child = elem->getMainBox().getStyle();

				// Get the range of cells [left, right) that this child spans.
				// Since the "pos" style property is a float, we need to
				// convert it to a non-negative integer before using it.
				s32 left = std::max((s32)child.pos[x_dir], 0);
				s32 span = std::max((s32)dim_at(child.span, x_dir), 0);
				s32 right = left + span;

				// If this element spans into columns that don't yet exist, add
				// new columns in.
				if (right > (s32)layout.cols.size()) {
					layout.cols.resize(right);
				}

				/* Before we can adjust the minimum sizes for each column, we
				 * need to subtract off the original widths of the columns and
				 * the gap between the columns and find the total weight of the
				 * spanned columns.
				 *
				 * We subtract the original minimum width rather than the
				 * current minimum width because the current minimum width may
				 * change depending on the order of the elements. We don't want
				 * the layout to change unpredictably when the element order is
				 * changed, so we use the original width, which is independent
				 * of element order.
				 */
				float width = dim_at(child.size, x_dir) + style.gap[x_dir];
				float span_weight = 0.0f;

				for (s32 i = left; i < right; i++) {
					width -= layout.cols[i].orig_width + style.gap[x_dir];
					span_weight += layout.cols[i].weight;
				}

				// Now, using our minimum width, adjust each column
				// proportionally to its weight, or equally to each column if
				// they all have weights of zero.
				for (s32 i = left; i < right; i++) {
					GridColumn &col = layout.cols[i];

					if (span_weight == 0.0f) {
						col.width = std::max(col.width, width / span);
					} else {
						col.width =
							std::max(col.width, width * (col.weight / span_weight));
					}
				}
			}

			// Now that all the necessary columns have been added and the
			// minimum sizes calculated, total the width of the sizer. Like the
			// flex sizer, we use the negative gap trick.
			layout.total_width = -style.gap[x_dir];

			for (GridColumn &col : layout.cols) {
				layout.total_width += col.width + style.gap[x_dir];
			}

			// Get the starting position and horizontal gap for the sizer and
			// calculate how much extra width we have.
			float hpos = rect.UpperLeftCorner[x_dir];
			float full_hgap = style.gap[x_dir];

			float max_width = dim_at(rect.getSize(), x_dir);
			float extra_width = max_width - layout.total_width;

			if (extra_width <= 0.0f) {
				// If there's no extra width, then there's no need to
				// distribute widths any further.
			} else if (layout.total_weight == 0.0f) {
				// If none of the columns have weights, then allocate the extra
				// space using the horizontal spacing rule.
				Spacing spacing = x_dir ? style.vspacing : style.hspacing;

				switch (spacing) {
				case Spacing::BEFORE:
					hpos += extra_width;
					break;

				case Spacing::OUTSIDE:
					hpos += extra_width / 2.0f;
					break;

				case Spacing::AROUND: {
					float space = extra_width / layout.cols.size();
					full_hgap += space;
					hpos += space / 2.0f;
					break;
				}

				case Spacing::BETWEEN:
					full_hgap += extra_width / (layout.cols.size() - 1);
					break;

				case Spacing::EVENLY: {
					float space = extra_width / (layout.cols.size() + 1);
					full_hgap += space;
					hpos += space;
					break;
				}

				case Spacing::REMOVE:
					for (GridColumn &col : layout.cols) {
						col.width += extra_width / layout.cols.size();
					}
					break;

				default:
					break;
				}
			} else {
				// Otherwise, we need to allocate the extra space according to
				// the weight of each column.
				for (GridColumn &col : layout.cols) {
					if (col.weight != 0.0f) {
						col.width += extra_width * (col.weight / layout.total_weight);
					}
				}
			}

			// Now that we have the widths of each column and the spacing
			// between them, we can calculate their positions.
			for (GridColumn &col : layout.cols) {
				col.left = hpos;
				col.right = hpos + col.width;
				hpos += col.width + full_hgap;
			}
		}

		// Now that the position and size for each row and column has been
		// calculated, we can position the elements accordingly.
		for (Elem *elem : getChildren()) {
			const Style &child = elem->getMainBox().getStyle();

			// Get the positions of the top left and bottom right cells for
			// this element.
			v2s32 top_left(
				std::max((s32)child.pos.X, 0),
				std::max((s32)child.pos.Y, 0)
			);
			v2s32 bot_right(
				top_left.X + std::max((s32)child.span.Width,  1) - 1,
				top_left.Y + std::max((s32)child.span.Height, 1) - 1
			);

			// Find the width and height of the element's bounding box
			// according to the rows and columns, and that's all the
			// information that's necessary to position the element.
			rf32 elem_rect(
				layouts[0].cols[top_left.X].left,
				layouts[1].cols[top_left.Y].left,
				layouts[0].cols[bot_right.X].right,
				layouts[1].cols[bot_right.Y].right
			);

			elem->layout(elem_rect, getMainBox().getChildClip());
		}
	}
}
