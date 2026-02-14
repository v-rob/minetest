### Event handlers

Event handlers can come in various granularities. For instance, scrollbars have
two events: `on_change` and `on_input`. The `on_input` event fires any time the
user moves the scrollbar at all, whereas the `on_change` event only fires when
the user has committed the scrollbar's value, e.g. by releasing the mouse
button after moving the thumb.

Clearly, the `on_input` event is more network-intensive, since it may send many
events for a single `on_change` event. Other events, such as the window's
`on_mouse_input` event, are even more intensive. However, the client only sends
the events that were requested by the server. Events will only be sent if the
corresponding event handler property is set, so omitting the event handler for
unused events will keep network traffic low.

### Core theme

Since the prelude theme is unusable on its own, the UI API also provides a
built-in theme called the _core theme_. The core theme styles every element
type with a reasonable default look and feel. It is designed to be sufficiently
generic and neutral to be usable in a variety of different games, but without
looking bland and unattractive like formspecs.

The core theme is subject to tweaks and changes, but a certain level of
stability is guaranteed inasmuch as the overall look and feel of the core theme
will not change drastically to the point of breaking old windows.

Certain elements in the core theme support _core groups_ with the prefix `ui:`
that apply different styles to that element. For instance, the `ui:blank`
element group may be used on the root element to suppress the default window
background image. Each element documents whether it has any core groups and
what their function is.

Games are under no obligation to use the core theme at all, and are encouraged
to make their own default themes when desired. Such themes may provide support
for the core groups provided by the core theme, but may not invent new ones
with the `ui:` prefix.

### Functions

* `ui.get_core_theme()`: Returns the style defining the core theme.
* `ui.get_default_theme()`: Returns the style used as the default theme for
  windows without an explicit theme. Defaults to the core theme.

### Sizers

_Sizers_ provide the most powerful and convenient layout capabilities. When an
element is positioned within a sizer, such as `ui.Flex`, the sizer uses the
`size` property along with its own layout rules and special style properties to
determine the bounding rectangle for each one of its child elements. Refer to
each sizer's documentation for more detailed information on how they layout
their children.

Elements are still able to use normalized coordinates within a sizer. The sizer
only determines the bounding rectangle of the element, not the element position
or size within that bounding rectangle. Elements are free to take up the entire
space, as they do by default, or they may choose different normalized
positions, sizes, and anchors within that bounding rectangle. For example:

```
   Bounding rectangles given by sizer
+--------------------+  +--------------------+  +--------------------+
|                    |  |                    |  |                    |
|                    |  |                    |  |                    |
|                    |  |                    |  |                    |
|                    |  |                    |  |                    |
+--------------------+  +--------------------+  +--------------------+

   Elements inside bounding rectangles
+--------------------+  +--------------------+  +-------------+------+
| * * * * * * * * * *|  |  +----------+      |  |             | * * *|
|* * * * * * * * * * |  |  | * * * * *|      |  |             |* * * |
| * * * * * * * * * *|  |  |* * * * * |      |  |             | * * *|
|* * * * * * * * * * |  |  | * * * * *|      |  |             |* * * |
+--------------------+  +--|* * * * * |------+  +-------------*------+
                           +----------+
```

Each sizer has its own extra properties, such as `span` or `weight`. Each sizer
describes which properties can be used and what their functions are. The full
list of styling properties can be found in the `StyleSpec` section.

### Emulating formspec coordinates

For transitioning from formspecs, the `ui.get_coord_size()` function can be
used to get the size of a single formspec coordinate. This can be used with the
`ui.Place` sizer to position things as they would be in a formspec. The
following template can be used for transitioning from formspecs:

```lua
local function builder(handle, player, cx, param)
    local coord = ui.get_coord_size()

    return ui.Window{
        type = "gui",

        root = ui.Root{
            -- Replace these numbers with the size of the formspec.
            size = {8 * coord, 5 * coord},

            ui.Place{
                scale = coord,

                -- Place all elements here, using the `pos` and `span`
                -- properties to position them.
                ui.Button{
                    id = "example",

                    pos = {1, 1},
                    span = {3, 1},
                },
            },
        },
    }
end
```

### Derived elements

* `ui.Stack`
    * Type name: `stack`
    * A sizer that stacks each child element on top of each other in order,
      with each element being given the entire content rectangle of the parent.
      Although this is the default layout behavior for all non-sizer elements,
      `ui.Elem`s that are used solely for layout should be replaced with
      `ui.Stack` instead for semantic value.

If the `ui:blank` core group is set, the core theme will not give the `main`
box a default background image, which is useful for windows with free-standing
elements or multiple separate window backgrounds.

The `ui:left`, `ui:center`, and `ui:right` core groups modify the appearance of
buttons for arrangement in a row with no spacing between each button. The
`ui:left` group, for instance, should be used on the leftmost button, causing
its right border to be merged with the button to the right of it.

, but they support the same core groups as normal buttons.
, but they support the same core groups as normal buttons.

#### Sizer fields

* `pos` (2D vector): Controls the position of the element in a sizer-dependent
  way. Has no effect for the children of non-sizer elements. Default `{0, 0}`.
* `span` (2D vector): Controls the size of the element in a sizer-dependent
  way. Has no effect for the children of non-sizer elements. May not be
  negative. Default `{0, 0}`.
* `gap` (2D vector): Gap in pixels between child elements and flex runs of a
  flex sizer or between rows and columns of a grid sizer. It is valid for gaps
  to be negative. Default `{0, 0}`.
* `weight` (number): The weight of this element within a flex sizer, i.e. what
  proportion of extra space will be assigned to this element's area. May not be
  negative. Default `0`.
* `hspacing` (string): How the sizer should horizontally distribute extra space
  around unweighted child elements. One of `before`, `after`, `outside`,
  `around`, `between`, `evenly`, or `remove`. Default `after`.
* `vspacing` (string): Same as `hspacing`, but vertically.

`ui.Place`
----------

The place sizer is the most basic sizer, used for positioning elements at
absolute locations within the sizer. The `pos` style property controls the
position of the element's bounding rectangle, measured from the top left of the
sizer. The `span` style property controls the size of the element's bounding
rectangle. The `size` property has no effect on the space the sizer allocates
for the element.

By default, the `pos` and `span` properties are measured in scaled pixels.
However, this can be changed by using the `scale` property, which scales the
`pos` and `span` properties by a fixed amount.

For instance, `scale` may be set to `ui.get_coord_size()` for transitioning
from formspec coordinates. Alternatively, the `real_gui_scaling` or
`real_hud_scaling` values in `minetest.get_player_window_information()` may be
used to un-scale the values to get physical pixels instead of scaled pixels.

### Type info

* Type name: `place`
* ID required: No
* Boxes:
    * `main` (static): See `ui.Elem`.

### Theming

There is no prelude theming for the place sizer.

### Fields

In additional to all fields in `ui.Elem`, the following fields can be provided
to the `ui.Place` constructor:

* `scale` (number): Scales the `pos` and `span` style properties by a fixed
  amount. May not be negative. Default 1.

`ui.Flex`
---------

The flex sizer is used for positioning elements in a linear (either vertical or
horizontal) or wrapped formation. It is similar to CSS flexbox in purpose and
functionality, but does not contain all the same features due to the UI API's
different layout model.

The `size` property of each element in the sizer represents its minimum size in
pixels. The sizer will attempt to partition its content box such that each
element has its minimum required space, although elements may be partially or
fully cut off if the sizer element isn't big enough. The `pos` and `span`
properties are ignored entirely.

The flex sizer has a _primary direction_ in which elements are laid out, which
lies in one of the four cardinal directions. By default, flex sizers have a
primary direction of right, which means that the first element is at the
leftmost position, the second is to the right of that one, and so on. Visually,
the primary direction types look like the following:

```
  Right ------->        Down         Up
+---+ +---+ +---+      +---+ |     +---+ ^
| 1 | | 2 | | 3 |      | 1 | |     | 3 | |
+---+ +---+ +---+      +---+ |     +---+ |
                       +---+ |     +---+ |
                       | 2 | |     | 2 | |
  Left  <-------       +---+ V     +---+ |
+---+ +---+ +---+      +---+       +---+
| 3 | | 2 | | 1 |      | 3 |       | 1 |
+---+ +---+ +---+      +---+       +---+
```

If a flex sizer has too many elements to fit in a single line, it can either
let the elements overflow outside of the element boundaries, or it can wrap
them onto a new row or column of elements, called a _run_. The _wrapping
direction_ can either be forward or backward: for horizontal sizers, forward is
down and backward is up, whereas for vertical sizers, forward is to the right
and backward is to the left. For a right-directional sizer, the different
wrapping directions look like the following:

```
     Forward                 Backward                None
+---------+ +---+ |     +---------+       ^     +---------+ +---+ +-\
|    1    | | 2 | |     |    5    |       |     |    1    | | 2 | | /
+---------+ +---+ |     +---------+       |     +---------+ +---+ +-\
+---+ +---------+ |     +---+ +---------+ |
| 3 | |    4    | |     | 3 | |    4    | |
+---+ +---------+ V     +---+ +---------+ |
+---------+             +---------+ +---+
|    5    |             |    1    | | 2 |
+---------+             +---------+ +---+
```

Flex sizers have a `gap` style property which governs the horizontal and
vertical gaps between elements and runs. For horizontal sizers, that makes X
the gap between elements and Y the gap between runs, and vice-versa for
vertical sizers. If the sizer doesn't wrap, only the value in the primary
direction is used.

Each element in a flex sizer is allocated a rectangle that is the size of its
`size` style property in pixels. Child elements in a flex sizer also have a
`weight` style property. If the sizer has extra space after giving each element
its requested size, it will distribute the rest of the space to elements with
nonzero weights. For instance, if a horizontal sizer has four elements with
the weights 0, 2, 1, 0 in the X direction, then the second gets 2/3 of the
extra space, the third gets 1/3, and the others get no extra space, like so:

```
   No weights
| +---+ +---+ +---+ +---+             |
| | 0 | | 0 | | 0 | | 0 |             |
| +---+ +---+ +---+ +---+             |

   With weights
| +---+ +-----------+ +-------+ +---+ |
| | 0 | |     2     | |   1   | | 0 | |
| +---+ +-----------+ +-------+ +---+ |
```

If every child element in a run has a weight of zero, then the sizer can
distribute the empty space in the run using the `hspacing` and `vspacing`
properties. For a horizontal sizer, `hspacing` distributes the space around
elements and `vspacing` distributes the space between runs, and vice-versa for
a vertical sizer. The spacing options are as follows:

* `after`: The space is placed after all the elements. E.g. for a
  right-directional sizer, the elements are left aligned within the space.
* `before`: The space is placed before all the elements.
* `outside`: Half of the space is before the elements, and half is after,
  making the elements centered in the sizer.
* `remove`: All extra space in the run is removed, stretching each element to
  compensate. For spacing between elements, this is equivalent to giving each
  element equal weight within that run.
* `around`: Equal portions of empty space are placed directly before and after
  each element. Hence, the space at the start and end of the run is half that
  of the space between elements.
* `between`: Equal portions of empty space are placed between each element,
  leaving no space at the start or end of the run.
* `evenly`: Equal portions of empty space are placed between each element and
  also at the start and end of the run.

For a right-directional sizer, this is how each property looks:

```
   After (default)                         Before
| +---+ +---+ +---+               |     |               +---+ +---+ +---+ |
| | 1 | | 2 | | 3 |               |     |               | 1 | | 2 | | 3 | |
| +---+ +---+ +---+               |     |               +---+ +---+ +---+ |

   Outside                                 Remove
|        +---+ +---+ +---+        |     | +--------+ +-------+ +--------+ |
|        | 1 | | 2 | | 3 |        |     | |    1   | |   2   | |   3    | |
|        +---+ +---+ +---+        |     | +--------+ +-------+ +--------+ |

   Around                                  Between
|   +---+      +---+      +---+   |     | +---+        +---+        +---+ |
|   | 1 |      | 2 |      | 3 |   |     | | 1 |        | 2 |        | 3 | |
|   +---+      +---+      +---+   |     | +---+        +---+        +---+ |

   Evenly
|     +---+    +---+    +---+     |
|     | 1 |    | 2 |    | 3 |     |
|     +---+    +---+    +---+     |
```

These properties perform the exact same layout when used with runs. For
instance, in a vertical sizer (regardless of whether it wraps), an `hspacing`
property of `remove` would cause the width of the column(s) to expand to fill
the entire sizer.

### Type info

* Type name: `flex`
* ID required: No
* Boxes:
    * `main` (static): See `ui.Elem`.

### Theming

There is no prelude theming for the flex sizer.

### Fields

In additional to all fields in `ui.Elem`, the following fields can be provided
to the `ui.Flex` constructor:

* `dir` (string): The primary direction of the flex sizer. One of `left`,
  `up`, `right`, or `down`.
* `wrap` (string): The wrapping direction of the flex sizer. One of `forward`,
  `backward`, or `none`.

`ui.Grid`
---------

The grid sizer is used for positioning elements within a two-dimensional grid.
It shares many of the same basic properties of a non-wrapping `ui.Flex`, but
extended to two dimensions.

Like the flex sizer, the `size` property is the minimum size of the element,
and the grid sizer will give each element at least that much space for its
bounding rectangle. Also similar to the flex sizer, rows and columns have
weights assigned to them to determine how much they can expand if the sizer has
extra space left over.

The grid sizer has a grid of indefinite size. The leftmost column has index 0,
the one to the right has index 1, and so on, increasing as necessary to fit any
additional elements. Similarly, the topmost row has index 0.

The grid sizer itself doesn't choose where to place elements. Instead, the
`pos` property on each element selects the column and row that the element
should be placed in. By default, each element takes up a single cell, but this
can be changed with the `span` style property, which indicates how many columns
wide and rows tall the element should be. For instance, a grid with two
elements, one at (0, 3) and the other at (2, 1) with a span of (3, 2), would
have the elements in this formation:

```
    0   1   2   3   4
  +   +   +   +   +   +
0
  +   +   +-----------+
1         |           |
  +   +   |     2     |
2         |           |
  +---+   +-----------+
3 | 1 |
  +---+   +   +   +   +
```

It is perfectly valid for elements to have spans that overlap each other, or
even to have elements occupy the same cell.

The minimum size of each row and column is influenced by a number of factors.
First, the `hsizes` and `vsizes` element properties can assign an initial
minimum width to each row and column. If no size is set via these properties,
the minimum size is zero initially. Not only does this allow empty rows or
columns to have a size, but it also helps the sizer allocate space more
efficiently when there are large elements that span many rows or columns.

If any element in the row or column has a larger minimum size than the row or
column, the minimum size is increased to fit the element. For elements that
span multiple rows or columns, multiple of rows or columns may need to have
their minimum sizes adjusted. First, the element subtracts off the relevant
initial minimum sizes set by `hsizes` and `vsizes` from its own minimum size,
and, if it fits within that space, doesn't adjust anything. Otherwise, it takes
the remaining size and distributes it among its rows or columns. If any of them
have nonzero weights, it expands the rows or columns proportional to those
weights, not expanding the ones with a weight of zero at all. Otherwise, if all
of them have weights of zero, then equal fractions of the remaining size are
used to expand the minimum sizes of the rows or columns.

When the grid has extra space left over after giving each element its minimum
size, it distributes the rest of the space the same way as runs do in
`ui.Flex`, but in both dimensions. If any columns or rows have nonzero weights
according to the `hweight` or `vweight` element properties, the sizer gives
them extra space proportional to their weight. However, if all weights are
zero, then the `hspacing` and `vspacing` style properties are consulted to
decide how to distribute the extra space between the rows or columns. All the
spacing options available for `ui.Flex` are also available for `ui.Grid`.

### Type info

* Type name: `grid`
* ID required: No
* Boxes:
    * `main` (static): See `ui.Elem`.

### Theming

There is no prelude theming for the grid sizer.

### Fields

In additional to all fields in `ui.Elem`, the following fields can be provided
to the `ui.Grid` constructor:

* `hsizes` (table of numbers): A table of initial minimum sizes for the grid
  columns, where the first element of the list corresponds to the leftmost
  column. Default `{}`.
* `vsizes` (table of numbers): A table of initial minimum sizes for the grid
  rows, where the first element of the list corresponds to the topmost row.
  Default `{}`.
* `hweights` (table of numbers): A table of column weights, where the first
  element of the list corresponds to the leftmost column. Default `{}`.
* `vweights` (table of numbers): A table of row weights, where the first
  element of the list corresponds to the topmost row. Default `{}`.
