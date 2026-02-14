+ Current
    * Documentation
        - Mark unimplemented stuff with quote blocks.
        - Add documentation for text, labels, icons, and objects.
        - Remove references to icons and fix `box_*`/`size`/`span` instances.
        - Make documentation for layout schemes and minimum sizes.
        - Add documentation about transitioning from formspecs with `scale =
          ui.get_coord_size()` and `truncate = "both"`.
        - Add more code samples and test existing code.
        - Add more cross-referencing to sections. Make sure we reference as few
          unimplemented features as possible. Use proper links for references?
    * Code quality
        - Use smaller serialization mask than u32 when appropriate.
        - Fix errors in `core.encode_network()` giving wrong stack traces.
        - Figure out why CI is failing on devtest unit test.
        - Add `testui` test suite in devtest.
    * API
    * Events
    * Elements
        - Make `ui.Label` and `ui.Icon` true elements with objects, with
          convenience `ui.label()` and `ui.icon()` functions.
    * Theming
        - Make `label` a specific rather than universal property
        - Add `obj_fit = fill|contain|cover|fixed`, `obj_align`, and
          `obj_scale` properties using `IObject` interface.
          `Box::set/getObject()` methods.
        - Make sure all appropriate properties are set in prelude theme.
        - Add `?*()`, `?+()`, `?++()`, `?~()`, and `?~~()` predicates and
          remove `?<>()` predicate.
        - Allow state and box selectors in predicates. Support `&` predicate to
          match other boxes in element, e.g. `scrollbar@track&(@thumb$pressed)`
        - Add combinator versions of all parent/child/sibling/etc. selectors.
        - Add media queries for screen size and mobile vs. desktop. Also add
          box child/parent/etc. selectors that operate in similar ways to
          element selectors?
    * Layout
        - Figure out layout bug in PR.
+ Complete
    * Documentation
    * Code quality
        - Moved `ui.Context` to its own file.
        - Added type assertions to all public functions to catch modder errors.
    * API
        - Added missing `core.is_subclass(class, super)` function.
        - Nested styles can now be inlined directly within the element
          definition, which is more convenient than using the `style` property,
          which would override style property inlining. This also makes style
          inlining much less irregular for elements. For consistency, styles
          can be inlined into `ui.Window` as well (but not the theme).
    * Events
        - Removed double-click to close windows.
        - Renamed `uncloseable` to `allow_close` for consistency with #15971.
    * Elements
        - Added `label` property to certain elements, which causes a text
          object to be included in one of their boxes. Also added are basic
          `text_*` style properties to configure the appearance of the text.
          Note that the `text_*` properties are basic and incomplete, and
          changes are definitely expected in the future.
    * Theming
        - Replaced `@all` with `@*` for consistency with `*`. Also replaced
          `.group` with `%group` to make groups with colons more readable.
        - Added a few selectors like `?first_match(sel)`, `?nth_match(sel;
          index)`, etc. for parity with CSS `:first-of-type(type)`, etc.
        - Removed all `icon_*` properties, renamed `box_fill` to `fill`,
          renamed `box_image` to `img_pane`, renamed `img_source` to
          `img_slice` (to avoid connotations of "image file source"), renamed
          `box_middle` to `img_border`, added `img_overlay` and `img_align`
          properties, and renamed all other `box_*` properties to `img_*`.
        - Renamed `display` style property to `clip`, and renamed what was
          formerly called `clip` to `truncate`. Also replaced `display =
          "hidden"` with `hidden = true|false` to allow boxes to be hidden when
          `clip = "overflow"`.
    * Layout
        - Renamed `span` to `size` and renamed `size` to `min`. The
          justification for naming it `min` rather than `min_size` is that I
          don't want people to assume that `size` and `min_size` have
          corresponding behaviors as those names would suggest, since the
          former uses scaled (possibly normalized) coordinates and only applies
          to `place` sizers, whereas the latter uses pixels and works across
          all sizers. I'd solicit better naming for this.
+ Future
    * Documentation
    * Code quality
    * API
    * Events
        - Support touch events.
        - Send keyboard/mouse/touch events to server.
        - Add `flush()` method to request data (such as edit box `on_change`)
          with `on_flush` callback on completion. Also flush data in `close()`.
    * Elements
        - Caption, accordion
        - Splitters, progress bars
        - Scrollbars, scrollers, sliders
            - extent/expand properties for scrollbar thumb.
            - extent/expand properties on scroller autohides scrollbars and
              gives width to scrollbar box.
            - Create pseudo-boxes for custom layout schemes.
    * Theming
        - Add style mixin functions with `def_* = function(param)` in style
          definition and `mix_* = param` at element usage site.
        - Add box predicates, e.g. `@?vertical@?thumb` for `@vthumb` or
          `@?nth_box(5)` for the fifth box.
        - Give recommended element sizes in core theme.
        - Replace server-side theming with client-side?
    * Layout
        - Add `before`, `after`, and `content` styles with `include` property?
        - Support bidi in UI, e.g. elements positioned from the right in rtl.
          When should icon images be flipped?

-------------------------------------------------------------------------------

Layout idea for sizers: First, call resize() on children from within relayout()
to get the minimum size horizontally (assuming a horizontal wrapping sizer),
then layout horizontally and apply weights, and then call resize() again with
the new horizontal space to get an accurate representation of the total space
required. The situation would be reversed for vertical sizers. However,
relayout() may need to be involved deeper in to get totally accurate
information. This sounds like a O(N^2) operation, but that might be OK due to
the limited depth of most element trees.

-------------------------------------------------------------------------------

The area given by the box's parent for the box to be layouted in is the
_bounding rectangle_. In general, the bounding rectangle is the same as the
content rectangle of the box's parent. However, certain boxes may have special
bounding rectangles, such as a moving bounding rectangle for a scrollbar thumb.

### Normalized layout

However, since positioning everything using pixels is inflexible and tedious
for UIs of any substantial size, the UI API also provides _normalized
coordinates_. These specify positions and sizes that are relative to the size
of the bounding rectangle of the box. For instance, a normalized position of
(0, 0) is the top left of a bounding rectangle, (1, 0) is the top right, and
(1/2, 1/2) is the center.

Boxes can be given a normalized position and size via the style properties
`rel_pos` and `rel_size`. They can also can have an anchor with `rel_anchor`,
which decides where the box is positioned from. Anchors are normalized based on
the box itself, so (0, 1) positions the box from its bottom left. This example
shows an box with a size of (1/2, 1/3) anchored at (1/2, 1/2) and positioned at
(1/2, 2/3) within its bounding rectangle:

```
+-----------------------------------------+
|  Bounding rect     ^                    |
|                    ^                    |
|                    ^ 2/3                |
|                    ^                    |
|                    ^                    |
|                    ^                    |
|                    ^                    |
|          +-------------------+ <        |
|          |  Layout .         | <        |
|          |   rect  . 1/2     | <        |
|< < < < < | . . . . *         | < 1/3    |
|   1/2    |   1/2             | <        |
|          |                   | <        |
|          +-------------------+ <        |
|          ^^^^^^^^^^^^^^^^^^^^^          |
|                   1/2                   |
+-----------------------------------------+
```

By default, a box has a position of (0, 0), a size of (1, 1), and an anchor of
(0, 0), meaning the box takes up the entire bounding rectangle in which it is
placed by default.

### Automatic layout

While normalized coordinates are sufficient for arranging boxes in a simple
manner, such as for positioning the buttons and track of a scrollbar, they are
too primitive to position boxes and elements in complex UIs.

Unfortunately, the current version of the UI API has no support for automatic
positioning or sizing of elements. Notably, there is a working prototype for
sizers, but it is still in progress and has not been merged, so normalized
coordinates are the only means of layouting elements for now. Additionally,
boxes do not compute their minimum size based on the size of their content,
icon, and paddings at this point, so boxes can easily overflow their parents if
the parents are not large enough to fit their children.

However, boxes do support a `size` property at the present time, which sets the
minimum size of the display rectangle in pixels. If the size computed by
`rel_size` is smaller than `size` after subtracting the margins, then the size
of the display rectangle will be expanded to `size` instead.
