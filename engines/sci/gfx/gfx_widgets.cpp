/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "sci/sci_memory.h"
#include "sci/gfx/gfx_gui.h"		// for kWindowAutoRestore
#include "sci/gfx/gfx_widgets.h"
#include "sci/gfx/gfx_state_internal.h"

namespace Sci {

#undef GFXW_DEBUG_DIRTY // Enable to debug dirty rectangle propagation (writes to stderr)

#ifdef GFXW_DEBUG_DIRTY
#  define DDIRTY fprintf(stderr, "%s:%5d| ", __FILE__, __LINE__); fprintf
#else
#  define DDIRTY if (0) fprintf
#endif

Common::Point gfxw_point_zero(0, 0);

#define MAX_SERIAL_NUMBER 0x7fffffff
static int widget_serial_number_counter = 0x10000; // Avoid confusion with IDs

#ifdef GFXW_DEBUG_WIDGETS

GfxWidget *debug_widgets[GFXW_DEBUG_WIDGETS];
int debug_widget_pos = 0;

static void _gfxw_debug_add_widget(GfxWidget *widget) {
	if (debug_widget_pos == GFXW_DEBUG_WIDGETS) {
		GFXERROR("WIDGET DEBUG: Allocated the maximum number of %d widgets- Aborting!\n", GFXW_DEBUG_WIDGETS);
		BREAKPOINT();
	}
	debug_widgets[debug_widget_pos++] = widget;
}

static void _gfxw_debug_remove_widget(GfxWidget *widget) {
	int i;
	int found = 0;
	for (i = 0; i < debug_widget_pos; i++) {
		if (debug_widgets[i] == widget) {
			memmove(debug_widgets + i, debug_widgets + i + 1, (sizeof(GfxWidget *)) * (debug_widget_pos - i - 1));
			debug_widgets[debug_widget_pos--] = NULL;
			found++;
		}
	}

	if (found > 1) {
		GFXERROR("While removing widget: Found it %d times!\n", found);
		BREAKPOINT();
	}

	if (found == 0) {
		GFXERROR("Attempted removal of unregistered widget!\n");
		BREAKPOINT();
	}
}
#else // !GFXW_DEBUG_WIDGETS
#define _gfxw_debug_add_widget(a)
#define _gfxw_debug_remove_widget(a)
#endif


static void indent(int indentation) {
	for (int i = 0; i < indentation; i++)
		sciprintf("    ");
}

static void _gfxw_print_widget(GfxWidget *widget, int indentation) {
	unsigned int i;
	char flags_list[] = "VOCDTMI";

	indent(indentation);

	if (widget->_magic == GFXW_MAGIC_VALID) {
		if (widget->_visual)
			sciprintf("v ");
		else
			sciprintf("NoVis ");
	} else if (widget->_magic == GFXW_MAGIC_INVALID)
		sciprintf("INVALID ");

	sciprintf("S%08x", widget->_serial);

	if (widget->_ID != GFXW_NO_ID) {
		sciprintf("#%x", widget->_ID);

		if (widget->_subID != GFXW_NO_ID)
			sciprintf(":%x ", widget->_subID);
		else
			sciprintf(" ");
	}

	sciprintf("[(%d,%d)(%dx%d)]", widget->_bounds.x, widget->_bounds.y, widget->_bounds.width, widget->_bounds.height);

	for (i = 0; i < strlen(flags_list); i++)
		if (widget->_flags & (1 << i))
			sciprintf("%c", flags_list[i]);

	sciprintf(" ");
}

static int _gfxwop_print_empty(GfxWidget *widget, int indentation) {
	_gfxw_print_widget(widget, indentation);
	sciprintf("<untyped #%d>", widget->_type);

	return 0;
}

GfxWidget::GfxWidget(gfxw_widget_type_t type_) {
	_magic = GFXW_MAGIC_VALID;

	_serial = widget_serial_number_counter++;
	widget_serial_number_counter &= MAX_SERIAL_NUMBER;

	_flags = GFXW_FLAG_DIRTY;
	_type = type_;
	_bounds = gfx_rect(0, 0, 0, 0);
	_next = NULL;
	_ID = GFXW_NO_ID;
	_subID = GFXW_NO_ID;
	_parent = NULL;
	_visual = NULL;
	_widgetPriority = -1;

	draw = NULL;
	widfree = NULL;
	tag = NULL;
	print = _gfxwop_print_empty;
	compare_to = NULL;
	equals = NULL;
	should_replace = NULL;
	superarea_of = NULL;
	set_visual = NULL;

	_gfxw_debug_add_widget(this);
}

static int verify_widget(GfxWidget *widget) {
	if (!widget) {
		GFXERROR("Attempt to use NULL widget\n");
#ifdef GFXW_DEBUG_WIDGETS
		BREAKPOINT();
#endif
		return 1;
	} else if (widget->_magic != GFXW_MAGIC_VALID) {
		if (widget->_magic == GFXW_MAGIC_INVALID) {
			GFXERROR("Attempt to use invalidated widget\n");
		} else {
			GFXERROR("Attempt to use non-widget\n");
		}
#ifdef GFXW_DEBUG_WIDGETS
		BREAKPOINT();
#endif
		return 1;
	}
	return 0;
}

GfxWidget::~GfxWidget() {
	_magic = GFXW_MAGIC_INVALID;
	_gfxw_debug_remove_widget(this);
}

#define VERIFY_WIDGET(w) \
	if (verify_widget((GfxWidget *)(w))) { GFXERROR("Error occured while validating widget\n"); }

static void _gfxw_unallocate_widget(gfx_state_t *state, GfxWidget *widget) {
	if (GFXW_IS_TEXT(widget)) {
		GfxText *text = (GfxText *) widget;

		if (text->text_handle) {
			if (!state) {
				GFXERROR("Attempt to free text without supplying mode to free it from!\n");
				BREAKPOINT();
			} else {
				gfxop_free_text(state, text->text_handle);
				text->text_handle = NULL;
			}
		}
	}

	delete widget;
}

#define GFX_ASSERT(_x) \
{ \
	int retval = (_x); \
	if (retval == GFX_ERROR) { \
		GFXERROR("Error occured while drawing widget!\n"); \
		return 1; \
	} else if (retval == GFX_FATAL) { \
		error("Fatal error occured while drawing widget!\nGraphics state invalid; aborting program..."); \
	} \
}

//********** Widgets *************

// Base class operations and common stuff

// Assertion for drawing
#define DRAW_ASSERT(widget, exp_type) \
	if (!(widget)) { \
		sciprintf("L%d: NULL widget", __LINE__); \
		return 1; \
	} \
	if (!(widget)->print) { \
		sciprintf("L%d: Widget of type %d does not have print function", __LINE__, (widget)->_type); \
	} \
	if ((widget)->_type != (exp_type)) { \
		sciprintf("L%d: Error in widget: Expected type " # exp_type "(%d) but got %d\n", __LINE__, exp_type, (widget)->_type); \
		sciprintf("Erroneous widget: "); \
		widget->print(widget, 4); \
		sciprintf("\n"); \
		return 1; \
	} \
	if (!(widget->_flags & GFXW_FLAG_VISIBLE)) \
		return 0; \
	if (!(widget->_type == GFXW_VISUAL || widget->_visual)) { \
		sciprintf("L%d: Error while drawing widget: Widget has no visual\n", __LINE__); \
		sciprintf("Erroneous widget: "); \
		widget->print(widget, 1); \
		sciprintf("\n"); \
		return 1; \
	}


// TODO: Turn this into an operator==
static int _color_equals(gfx_color_t a, gfx_color_t b) {
	if (a.mask != b.mask)
		return 0;

	if (a.mask & GFX_MASK_VISUAL) {
		if (a.visual.r != b.visual.r || a.visual.g != b.visual.g || a.visual.b != b.visual.b || a.alpha != b.alpha)
			return 0;
	}

	if (a.mask & GFX_MASK_PRIORITY)
		if (a.priority != b.priority)
			return 0;

	if (a.mask & GFX_MASK_CONTROL)
		if (a.control != b.control)
			return 0;

	return 1;
}

static int _gfxwop_basic_set_visual(GfxWidget *widget, GfxVisual *visual) {
	widget->_visual = visual;

	if (widget->_parent) {
		DDIRTY(stderr, "basic_set_visual: DOWNWARDS rel(%d,%d,%d,%d, 1)\n", GFX_PRINT_RECT(widget->_bounds));
		widget->_parent->add_dirty_rel(widget->_parent, widget->_bounds, 1);
	}

	return 0;
}

static int _gfxwop_basic_should_replace(GfxWidget *widget, GfxWidget *other) {
	return 0;
}

static void _gfxw_set_ops(GfxWidget *widget, gfxw_point_op *draw, gfxw_op *free, gfxw_op *tag, gfxw_op_int *print,
	gfxw_bin_op *compare_to, gfxw_bin_op *equals, gfxw_bin_op *superarea_of) {
	widget->draw = draw;
	widget->widfree = free;
	widget->tag = tag;
	widget->print = print;
	widget->compare_to = compare_to;
	widget->equals = equals;
	widget->superarea_of = superarea_of;

	widget->should_replace = _gfxwop_basic_should_replace;
	widget->set_visual = _gfxwop_basic_set_visual;
}

void gfxw_remove_widget_from_container(GfxContainer *container, GfxWidget *widget) {
	GfxWidget **seekerp;

	if (!container) {
		GFXERROR("Attempt to remove widget from NULL container!\n");
		BREAKPOINT();
	}

	seekerp = &(container->contents);

	if (GFXW_IS_LIST(widget) && GFXW_IS_PORT(container)) {
		GfxPort *port = (GfxPort *) container;
		if (port->decorations == (GfxList *) widget) {
			port->decorations = NULL;
			return;
		}
	}

	while (*seekerp && *seekerp != widget)
		seekerp = &((*seekerp)->_next);

	if (!*seekerp) {
		GFXERROR("Internal error: Attempt to remove widget from container it was not contained in!\n");
		sciprintf("Widget:");
		widget->print(GFXW(widget), 1);
		sciprintf("Container:");
		widget->print(GFXW(container), 1);
		BREAKPOINT();
		return;
	}

	if (container->nextpp == &(widget->_next))
		container->nextpp = seekerp;

	*seekerp = widget->_next; // Remove it
	widget->_parent = NULL;
	widget->_next = NULL;
}

static int _gfxwop_basic_free(GfxWidget *widget) {
	GfxVisual *visual = widget->_visual;
	gfx_state_t *state = (visual) ? visual->gfx_state : NULL;

	DDIRTY(stderr, "BASIC-FREE: SomeAddDirty\n");

	if (widget->_parent) {
		if (GFXW_IS_CONTAINER(widget))
			widget->_parent->add_dirty_abs(widget->_parent, widget->_bounds, 1);
		else
			widget->_parent->add_dirty_rel(widget->_parent, widget->_bounds, 1);

		gfxw_remove_widget_from_container(widget->_parent, widget);
	}

	_gfxw_unallocate_widget(state, widget);

	return 0;
}

static int _gfxwop_basic_tag(GfxWidget *widget) {
	widget->_flags |= GFXW_FLAG_TAGGED;

	return 0;
}

static int _gfxwop_basic_compare_to(GfxWidget *widget, GfxWidget *other) {
	return 1;
}

static int _gfxwop_basic_equals(GfxWidget *widget, GfxWidget *other) {
	return 0;
}

static int _gfxwop_basic_superarea_of(GfxWidget *widget, GfxWidget *other) {
	return (widget == other);
}

//*** Boxes ***

static rect_t _move_rect(rect_t rect, Common::Point point) {
	return gfx_rect(rect.x + point.x, rect.y + point.y, rect.width, rect.height);
}

static void _split_rect(rect_t rect, Common::Point *p1, Common::Point *p2) {
	p1->x = rect.x;
	p1->y = rect.y;
	p2->x = rect.x + rect.width;
	p2->y = rect.y + rect.height;
}

static Common::Point _move_point(rect_t rect, Common::Point point) {
	return Common::Point(rect.x + point.x, rect.y + point.y);
}

static int _gfxwop_box_draw(GfxWidget *widget, Common::Point pos) {
	GfxBox *box = (GfxBox *) widget;
	DRAW_ASSERT(widget, GFXW_BOX);
	GFX_ASSERT(gfxop_draw_box(box->_visual->gfx_state, _move_rect(box->_bounds, pos), box->_color1, box->_color2, box->_shadeType));

	return 0;
}

static int _gfxwop_box_print(GfxWidget *widget, int indentation) {
	_gfxw_print_widget(widget, indentation);
	sciprintf("BOX");
	return 0;
}

static int _gfxwop_box_superarea_of(GfxWidget *widget, GfxWidget *other) {
	GfxBox *box = (GfxBox *) widget;

	if (box->_color1.alpha)
		return 0;

	if (box->_shadeType != GFX_BOX_SHADE_FLAT && box->_color2.alpha)
		return 0;

	// Note: the check for box->_bounds and other->_bounds is NOT the same as contains()
	// in Common::Rect (this one includes equality too)
	if (!(box->_bounds.x <= other->_bounds.x && box->_bounds.y <= other->_bounds.y &&
		  box->_bounds.x + box->_bounds.width >= other->_bounds.x + other->_bounds.width &&
		  box->_bounds.y + box->_bounds.height >= other->_bounds.y + other->_bounds.height))
		return 0;

	return 1;
}

static int _gfxwop_box_equals(GfxWidget *widget, GfxWidget *other) {
	GfxBox *wbox = (GfxBox *)widget, *obox;
	if (other->_type != GFXW_BOX)
		return 0;

	obox = (GfxBox *) other;

	if (!toCommonRect(wbox->_bounds).equals(toCommonRect(obox->_bounds)))
		return 0;

	if (!_color_equals(wbox->_color1, obox->_color1))
		return 0;

	if (wbox->_shadeType != obox->_shadeType)
		return 0;

	if (wbox->_shadeType != GFX_BOX_SHADE_FLAT
	        && _color_equals(wbox->_color2, obox->_color2))
		return 0;

	return 1;
}

void _gfxw_set_ops_BOX(GfxWidget *widget) {
	_gfxw_set_ops(GFXW(widget), _gfxwop_box_draw, _gfxwop_basic_free, _gfxwop_basic_tag, _gfxwop_box_print,
	              _gfxwop_basic_compare_to, _gfxwop_box_equals, _gfxwop_box_superarea_of);
}

static int _gfxw_color_get_priority(gfx_color_t color) {
	return (color.mask & GFX_MASK_PRIORITY) ? color.priority : -1;
}

GfxBox *gfxw_new_box(gfx_state_t *state, rect_t area, gfx_color_t color1, gfx_color_t color2, gfx_box_shade_t shade_type) {
	return new GfxBox(state, area, color1, color2, shade_type);
}

GfxBox::GfxBox(gfx_state_t *state, rect_t area, gfx_color_t color1, gfx_color_t color2, gfx_box_shade_t shade_type)
	: GfxWidget(GFXW_BOX) {

	_widgetPriority = _gfxw_color_get_priority(color1);
	_bounds = area;
	_color1 = color1;
	_color2 = color2;
	_shadeType = shade_type;

	_flags |= GFXW_FLAG_VISIBLE;

	if ((_color1.mask & GFX_MASK_VISUAL) && ((state && (state->driver->mode->palette)) || (!_color1.alpha && !_color2.alpha)))
		_flags |= GFXW_FLAG_OPAQUE;

	_gfxw_set_ops_BOX(this);
}

GfxPrimitive::GfxPrimitive(rect_t area, gfx_color_t color_, gfx_line_mode_t mode,
						gfx_line_style_t style, gfxw_widget_type_t type_)
	: GfxWidget(type_) {

	_widgetPriority = _gfxw_color_get_priority(color_);
	_bounds = area;
	_color = color_;
	_lineMode = mode;
	_lineStyle = style;

	_flags |= GFXW_FLAG_VISIBLE;
}

//*** Rectangles ***

struct gfxw_rect_t : public GfxPrimitive {
	gfxw_rect_t(rect_t rect, gfx_color_t color, gfx_line_mode_t line_mode, gfx_line_style_t line_style);
};

static int _gfxwop_primitive_equals(GfxWidget *widget, GfxWidget *other) {
	GfxPrimitive *wprim = (GfxPrimitive *) widget, *oprim;
	if (widget->_type != other->_type)
		return 0;

	oprim = (GfxPrimitive *) other;

	// Check if the two primitives are equal (note: the primitives aren't always rectangles, so
	// the "width" and the "height" here could be negative)
	if (wprim->_bounds.x != oprim->_bounds.x || wprim->_bounds.y != oprim->_bounds.y ||
		wprim->_bounds.width != oprim->_bounds.width || wprim->_bounds.height != oprim->_bounds.height)
		return 0;

	if (!_color_equals(wprim->_color, oprim->_color))
		return 0;

	if (wprim->_lineMode != oprim->_lineMode)
		return 0;

	if (wprim->_lineStyle != oprim->_lineStyle)
		return 0;

	return 1;
}

static int _gfxwop_rect_draw(GfxWidget *widget, Common::Point pos) {
	GfxPrimitive *rect = (GfxPrimitive *) widget;
	DRAW_ASSERT(widget, GFXW_RECT);

	GFX_ASSERT(gfxop_draw_rectangle(rect->_visual->gfx_state, gfx_rect(rect->_bounds.x + pos.x, rect->_bounds.y + pos.y,
	                                         rect->_bounds.width - 1, rect->_bounds.height - 1), rect->_color, rect->_lineMode, rect->_lineStyle));
	return 0;
}

static int _gfxwop_rect_print(GfxWidget *rect, int indentation) {
	_gfxw_print_widget(GFXW(rect), indentation);
	sciprintf("RECT");

	return 0;
}

void _gfxw_set_ops_RECT(GfxWidget *prim) {
	_gfxw_set_ops(GFXW(prim), _gfxwop_rect_draw, _gfxwop_basic_free, _gfxwop_basic_tag, _gfxwop_rect_print,
	              _gfxwop_basic_compare_to, _gfxwop_primitive_equals, _gfxwop_basic_superarea_of);
}

GfxPrimitive *gfxw_new_rect(rect_t rect, gfx_color_t color, gfx_line_mode_t line_mode, gfx_line_style_t line_style) {
	return new gfxw_rect_t(rect, color, line_mode, line_style);
}

gfxw_rect_t::gfxw_rect_t(rect_t rect, gfx_color_t color_, gfx_line_mode_t line_mode_, gfx_line_style_t line_style_)
	: GfxPrimitive(rect, color_, line_mode_, line_style_, GFXW_RECT) {

	_bounds.width++;
	_bounds.height++; // Since it is actually one pixel bigger in each direction

	_gfxw_set_ops_RECT(this);
}

//*** Lines ***

struct gfxw_line_t : public GfxPrimitive {
	gfxw_line_t(Common::Point start, Common::Point end, gfx_color_t color, gfx_line_mode_t line_mode, gfx_line_style_t line_style);
};

static int _gfxwop_line_draw(GfxWidget *widget, Common::Point pos) {
	GfxPrimitive *line = (GfxPrimitive *)widget;
	rect_t linepos = widget->_bounds;
	Common::Point p1, p2;

	linepos.width--;
	linepos.height--;

	DRAW_ASSERT(widget, GFXW_LINE);

	_split_rect(_move_rect(linepos, pos), &p1, &p2);
	GFX_ASSERT(gfxop_draw_line(line->_visual->gfx_state, p1, p2, line->_color, line->_lineMode, line->_lineStyle));
	return 0;
}

static int _gfxwop_line_print(GfxWidget *widget, int indentation) {
	_gfxw_print_widget(widget, indentation);

	return 0;
}

void _gfxw_set_ops_LINE(GfxWidget *prim) {
	_gfxw_set_ops(GFXW(prim), _gfxwop_line_draw, _gfxwop_basic_free, _gfxwop_basic_tag, _gfxwop_line_print,
	              _gfxwop_basic_compare_to, _gfxwop_primitive_equals, _gfxwop_basic_superarea_of);
}

GfxPrimitive *gfxw_new_line(Common::Point start, Common::Point end, gfx_color_t color, gfx_line_mode_t line_mode, gfx_line_style_t line_style) {
	return new gfxw_line_t(start, end, color, line_mode, line_style);
}

gfxw_line_t::gfxw_line_t(Common::Point start, Common::Point end, gfx_color_t color_, gfx_line_mode_t line_mode_, gfx_line_style_t line_style_)
	: GfxPrimitive(gfx_rect(start.x, start.y, end.x - start.x + 1, end.y - start.y + 1), color_, line_mode_, line_style_, GFXW_LINE) {
	_gfxw_set_ops_LINE(this);
}

//*** Views and static views ***


GfxView::GfxView(gfx_state_t *state, Common::Point pos_, int view_, int loop_, int cel_, int palette_, int priority, int control,
	gfx_alignment_t halign, gfx_alignment_t valign, int flags_)
	: GfxWidget((flags_ & GFXW_VIEW_FLAG_STATIC) ? GFXW_STATIC_VIEW : GFXW_VIEW) {

	int width, height;
	Common::Point offset;

	if (!state) {
		error("Attempt to create view widget with NULL state");
	}

	if (gfxop_get_cel_parameters(state, view_, loop_, cel_, &width, &height, &offset)) {
		error("Attempt to retrieve cel parameters for (%d/%d/%d) failed (Maybe the values weren't checked beforehand?)",
		         view_, cel_, loop_);
	}

	_pos = pos_;
	_color.mask = ((priority < 0) ? 0 : GFX_MASK_PRIORITY) | ((control < 0) ? 0 : GFX_MASK_CONTROL);
	_widgetPriority = priority;
	_color.priority = priority;
	_color.control = control;
	_view = view_;
	_loop = loop_;
	_cel = cel_;
	_palette = palette_;

	if (halign == ALIGN_CENTER)
		_pos.x -= width >> 1;
	else if (halign == ALIGN_RIGHT)
		_pos.x -= width;

	if (valign == ALIGN_CENTER)
		_pos.y -= height >> 1;
	else if (valign == ALIGN_BOTTOM)
		_pos.y -= height;

	_bounds = gfx_rect(_pos.x - offset.x, _pos.y - offset.y, width, height);

	_flags |= GFXW_FLAG_VISIBLE;
}

int _gfxwop_view_draw(GfxWidget *widget, Common::Point pos) {
	GfxView *view = (GfxView *)widget;
	DRAW_ASSERT(widget, GFXW_VIEW);

	GFX_ASSERT(gfxop_draw_cel(view->_visual->gfx_state, view->_view, view->_loop, view->_cel,
				Common::Point(view->_pos.x + pos.x, view->_pos.y + pos.y), view->_color, view->_palette));

	return 0;
}

static int _gfxwop_static_view_draw(GfxWidget *widget, Common::Point pos) {
	GfxView *view = (GfxView *)widget;
	DRAW_ASSERT(widget, GFXW_VIEW);

	GFX_ASSERT(gfxop_draw_cel_static(view->_visual->gfx_state, view->_view, view->_loop,
	                                 view->_cel, _move_point(view->_bounds, pos), view->_color, view->_palette));

	return 0;
}

static int _w_gfxwop_view_print(GfxWidget *widget, const char *name, int indentation) {
	GfxView *view = (GfxView *)widget;
	_gfxw_print_widget(widget, indentation);

	sciprintf("%s", name);
	sciprintf("(%d/%d/%d)@(%d,%d)[p:%d,c:%d]", view->_view, view->_loop, view->_cel, view->_pos.x, view->_pos.y,
	          (view->_color.mask & GFX_MASK_PRIORITY) ? view->_color.priority : -1,
	          (view->_color.mask & GFX_MASK_CONTROL) ? view->_color.control : -1);

	return 0;
}

static int _gfxwop_view_print(GfxWidget *widget, int indentation) {
	return _w_gfxwop_view_print(widget, "VIEW", indentation);
}

static int _gfxwop_static_view_print(GfxWidget *widget, int indentation) {
	return _w_gfxwop_view_print(widget, "PICVIEW", indentation);
}

void _gfxw_set_ops_VIEW(GfxWidget *view, char stat) {
	_gfxw_set_ops(GFXW(view), (stat) ? _gfxwop_static_view_draw : _gfxwop_view_draw, _gfxwop_basic_free,
	              _gfxwop_basic_tag, (stat) ? _gfxwop_static_view_print : _gfxwop_view_print,
	              _gfxwop_basic_compare_to, _gfxwop_basic_equals, _gfxwop_basic_superarea_of);
}

GfxView *gfxw_new_view(gfx_state_t *state, Common::Point pos, int view_nr, int loop, int cel, int palette, int priority, int control,
	gfx_alignment_t halign, gfx_alignment_t valign, int flags) {
	GfxView *view;

	if (flags & GFXW_VIEW_FLAG_DONT_MODIFY_OFFSET) {
		int foo;
		Common::Point offset;
		gfxop_get_cel_parameters(state, view_nr, loop, cel, &foo, &foo, &offset);
		pos.x += offset.x;
		pos.y += offset.y;
	}

	view = new GfxView(state, pos, view_nr, loop, cel, palette, priority, control, halign, valign,
	                             (flags & GFXW_VIEW_FLAG_STATIC) ? GFXW_STATIC_VIEW : GFXW_VIEW);

	_gfxw_set_ops_VIEW(view, (char)(flags & GFXW_VIEW_FLAG_STATIC));

	return view;
}

//*** Dynamic Views ***

static int _gfxwop_dyn_view_draw(GfxWidget *widget, Common::Point pos) {
	GfxDynView *view = (GfxDynView *) widget;
	DRAW_ASSERT(widget, GFXW_DYN_VIEW);

	GFX_ASSERT(gfxop_draw_cel(view->_visual->gfx_state, view->_view, view->_loop,
	                          view->_cel, _move_point(view->draw_bounds, pos), view->_color, view->_palette));

	/*
	  gfx_color_t red;
	  red.visual.r = 0xff;
	  red.visual.g = red.visual.b = 0;
	  red.mask = GFX_MASK_VISUAL;
	  GFX_ASSERT(gfxop_draw_rectangle(view->visual->gfx_state,
	  gfx_rect(view->_bounds.x + pos.x, view->_bounds.y + pos.y, view->_bounds.width - 1, view->_bounds.height - 1), red, 0, 0));
	*/

	return 0;

}

static int _gfxwop_draw_nop(GfxWidget *widget, Common::Point pos) {
	return 0;
}

static int _gfxwop_pic_view_draw(GfxWidget *widget, Common::Point pos) {
	GfxDynView *view = (GfxDynView *) widget;
	DRAW_ASSERT(widget, GFXW_PIC_VIEW);

	GFX_ASSERT(gfxop_set_clip_zone(view->_visual->gfx_state, view->_parent->zone));
	GFX_ASSERT(gfxop_draw_cel_static_clipped(view->_visual->gfx_state, view->_view, view->_loop,
	           view->_cel, _move_point(view->draw_bounds, pos), view->_color, view->_palette));

	// Draw again on the back buffer
	GFX_ASSERT(gfxop_draw_cel(view->_visual->gfx_state, view->_view, view->_loop, view->_cel,
	                          _move_point(view->draw_bounds, pos), view->_color, view->_palette));


	widget->draw = _gfxwop_draw_nop; // No more drawing needs to be done

	return 0;
}

static int _gfxwop_some_view_print(GfxWidget *widget, int indentation, const char *type_string) {
	GfxDynView *view = (GfxDynView *)widget;

	_gfxw_print_widget(widget, indentation);

	sciprintf("%s", type_string);
	sciprintf(" SORT=%d z=%d seq=%d (%d/%d/%d)@(%d,%d)[p:%d,c:%d]; sig[%04x@%p]", view->force_precedence, view->_z,
	          view->sequence, view->_view, view->_loop, view->_cel, view->_pos.x, view->_pos.y,
	          (view->_color.mask & GFX_MASK_PRIORITY) ? view->_color.priority : -1,
	          (view->_color.mask & GFX_MASK_CONTROL) ? view->_color.control : -1, view->signal, view->signalp);

	return 0;
}

static int _gfxwop_dyn_view_print(GfxWidget *widget, int indentation) {
	return _gfxwop_some_view_print(widget, indentation, "DYNVIEW");
}

static int _gfxwop_pic_view_print(GfxWidget *widget, int indentation) {
	return _gfxwop_some_view_print(widget, indentation, "PICVIEW");
}

static int _gfxwop_dyn_view_equals(GfxWidget *widget, GfxWidget *other) {
	GfxDynView *wview = (GfxDynView *)widget, *oview;
	if (!GFXW_IS_DYN_VIEW(other))
		return 0;

	oview = (GfxDynView *)other;

	if (wview->_pos.x != oview->_pos.x || wview->_pos.y != oview->_pos.y || wview->_z != oview->_z)
		return 0;

	if (wview->_view != oview->_view || wview->_loop != oview->_loop || wview->_cel != oview->_cel)
		return 0;

	if (!_color_equals(wview->_color, oview->_color))
		return 0;

	if (wview->_flags != oview->_flags)
		return 0;

	return 1;
}

static int _gfxwop_dyn_view_compare_to(GfxWidget *widget, GfxWidget *other) {
	int retval;
	GfxDynView *wview = (GfxDynView *) widget, *oview;
	if (!GFXW_IS_DYN_VIEW(other))
		return 1;

	oview = (GfxDynView *) other;

	retval = wview->force_precedence - oview->force_precedence;
	if (retval)
		return retval;

	retval = wview->_pos.y - oview->_pos.y;
	if (retval)
		return retval;

	retval = (wview->_z - oview->_z);
	if (retval)
		return retval;

	return -(wview->sequence - oview->sequence);
}

void _gfxw_set_ops_DYNVIEW(GfxWidget *widget) {
	_gfxw_set_ops(GFXW(widget), _gfxwop_dyn_view_draw, _gfxwop_basic_free, _gfxwop_basic_tag,
	              _gfxwop_dyn_view_print, _gfxwop_dyn_view_compare_to, _gfxwop_dyn_view_equals, _gfxwop_basic_superarea_of);
}

void _gfxw_set_ops_PICVIEW(GfxWidget *widget) {
	_gfxw_set_ops_DYNVIEW(widget);
	widget->draw = _gfxwop_pic_view_draw;
	widget->print = _gfxwop_pic_view_print;
}

GfxDynView *gfxw_new_dyn_view(gfx_state_t *state, Common::Point pos, int z, int view, int loop, int cel, int palette, int priority, int control,
	gfx_alignment_t halign, gfx_alignment_t valign, int sequence) {
	
	return new GfxDynView(state, pos, z, view, loop, cel, palette, priority, control, halign, valign, sequence);
}

GfxDynView::GfxDynView(gfx_state_t *state, Common::Point pos_, int z_, int view_, int loop_, int cel_, int palette_, int priority, int control,
	gfx_alignment_t halign, gfx_alignment_t valign, int sequence_)
	: GfxView(state, pos_, view_, loop_, cel_, palette_, priority, control, halign, valign, 0) {
	int width, height;
	int xalignmod, yalignmod;
	Common::Point offset;

	_type = GFXW_DYN_VIEW;

	if (!state) {
		error("Attempt to create view widget with NULL state!\n");
	}

	if (gfxop_get_cel_parameters(state, view_, loop_, cel_, &width, &height, &offset)) {
		error("Attempt to retrieve cel parameters for (%d/%d/%d) failed (Maybe the values weren't checked beforehand?)",
		         view_, cel_, loop_);
	}

	_pos = pos_;
	_color.mask = ((priority < 0) ? 0 : GFX_MASK_PRIORITY) | ((control < 0) ? 0 : GFX_MASK_CONTROL);
	_widgetPriority = priority;
	_color.priority = priority;
	_color.control = control;
	_view = view_;
	_loop = loop_;
	_cel = cel_;
	_palette = palette_;

	_color.alpha = 0;
	_color.visual = PaletteEntry(0,0,0); // FIXME: black!

	if (halign == ALIGN_CENTER)
		xalignmod = width >> 1;
	else if (halign == ALIGN_RIGHT)
		xalignmod = width;
	else
		xalignmod = 0;

	if (valign == ALIGN_CENTER)
		yalignmod = height >> 1;
	else if (valign == ALIGN_BOTTOM)
		yalignmod = height;
	else
		yalignmod = 0;

	draw_bounds = gfx_rect(_pos.x - xalignmod, _pos.y - yalignmod - z_, width, height);
	_bounds = gfx_rect(_pos.x - offset.x - xalignmod, _pos.y - offset.y - yalignmod - z_, width, height);

	under_bitsp = NULL;
	under_bits = 0;
	signalp = NULL;
	signal = 0;
	_z = z_;
	sequence = sequence_;
	force_precedence = 0;

	_flags |= GFXW_FLAG_VISIBLE;

	_gfxw_set_ops_DYNVIEW(this);
}

//*** Text ***

static int _gfxwop_text_free(GfxWidget *widget) {
	GfxText *text = (GfxText *)widget;
	free(text->text);

	return _gfxwop_basic_free(widget);
}

static int _gfxwop_text_draw(GfxWidget *widget, Common::Point pos) {
	GfxText *text = (GfxText *)widget;
	DRAW_ASSERT(widget, GFXW_TEXT);

	GFX_ASSERT(gfxop_draw_text(text->_visual->gfx_state, text->text_handle, _move_rect(text->_bounds, pos)));

	return 0;
}

static int _gfxwop_text_alloc_and_draw(GfxWidget *widget, Common::Point pos) {
	GfxText *text = (GfxText *)widget;
	DRAW_ASSERT(widget, GFXW_TEXT);

	text->text_handle = gfxop_new_text(widget->_visual->gfx_state, text->font_nr, text->text, text->_bounds.width,
	                   text->halign, text->valign, text->_color1, text->_color2, text->_bgcolor, text->text_flags);

	text->draw = _gfxwop_text_draw;

	return _gfxwop_text_draw(widget, pos);
}

static int _gfxwop_text_print(GfxWidget *widget, int indentation) {
	_gfxw_print_widget(widget, indentation);
	sciprintf("TEXT:'%s'", ((GfxText *)widget)->text);

	return 0;
}

static int _gfxwop_text_equals(GfxWidget *widget, GfxWidget *other) {
	GfxText *wtext = (GfxText *)widget, *otext;
	if (other->_type != GFXW_TEXT)
		return 0;

	otext = (GfxText *)other;

	if ((wtext->_bounds.x != otext->_bounds.x) || (wtext->_bounds.y != otext->_bounds.y))
		return 0;

	if (wtext->halign != otext->halign || wtext->valign != otext->valign)
		return 0;

	if (wtext->text_flags != otext->text_flags)
		return 0;

	if (wtext->font_nr != otext->font_nr)
		return 0;

	/* if (!(_color_equals(wtext->_color1, otext->_color1) && _color_equals(wtext->_color2, otext->_color2)
			&& _color_equals(wtext->_bgcolor, otext->_bgcolor)))
		return 0; */

	return 1;
}

static int _gfxwop_text_should_replace(GfxWidget *widget, GfxWidget *other) {
	GfxText *wtext = (GfxText *)widget, *otext;

	if (other->_type != GFXW_TEXT)
		return 0;

	otext = (GfxText *)other;

	return strcmp(wtext->text, otext->text);
}

static int _gfxwop_text_compare_to(GfxWidget *widget, GfxWidget *other) {
	return 1;
}

void _gfxw_set_ops_TEXT(GfxWidget *widget) {
	_gfxw_set_ops(GFXW(widget), _gfxwop_text_alloc_and_draw, _gfxwop_text_free, _gfxwop_basic_tag,
	              _gfxwop_text_print, _gfxwop_text_compare_to, _gfxwop_text_equals,
	              _gfxwop_basic_superarea_of);
	widget->should_replace = _gfxwop_text_should_replace;
}

GfxText *gfxw_new_text(gfx_state_t *state, rect_t area, int font, const char *text, gfx_alignment_t halign,
	gfx_alignment_t valign, gfx_color_t color1, gfx_color_t color2, gfx_color_t bgcolor, int text_flags) {
	return new GfxText(state, area, font, text, halign, valign, color1, color2, bgcolor, text_flags);
}

GfxText::GfxText(gfx_state_t *state, rect_t area, int font, const char *text_, gfx_alignment_t halign_,
	gfx_alignment_t valign_, gfx_color_t color1_, gfx_color_t color2_, gfx_color_t bgcolor_, int text_flags_)
	: GfxWidget(GFXW_TEXT) {

	_widgetPriority = _gfxw_color_get_priority(color1_);
	font_nr = font;
	text = (char *)sci_malloc(strlen(text_) + 1);
	halign = halign_;
	valign = valign_;
	_color1 = color1_;
	_color2 = color2_;
	_bgcolor = bgcolor_;
	text_flags = text_flags_;
	text_handle = NULL;

	strcpy(text, text_);

	gfxop_get_text_params(state, font, text, area.width, &width, &height, text_flags,
	                      &lines_nr, &lineheight, &lastline_width);

	/* FIXME: Window is too big
	area.x += _calc_needmove(halign, area.width, width);
	area.y += _calc_needmove(valign, area.height, height);
	*/

	if (halign == ALIGN_LEFT)
		area.width = width;
	if (valign == ALIGN_TOP)
		area.height = height;

	_bounds = area;

	_flags |= GFXW_FLAG_VISIBLE;

	_gfxw_set_ops_TEXT(this);
}

void gfxw_text_info(gfx_state_t *state, GfxText *text, int *lines, int *lineheight, int *offset) {
	if (lines)
		*lines = text->lines_nr;
	if (lineheight)
		*lineheight = text->lineheight;
	if (offset)
		*offset = text->lastline_width;
}

//-- Container types --

static int _gfxwop_container_add_dirty_rel(GfxContainer *cont, rect_t rect, int propagate) {
	DDIRTY(stderr, "->container_add_dirty_rel(%d,%d,%d,%d, %d)\n", GFX_PRINT_RECT(rect), propagate);
	return cont->add_dirty_abs(cont, _move_rect(rect, Common::Point(cont->zone.x, cont->zone.y)), propagate);
}

static void _gfxw_set_container_ops(GfxContainer *container, gfxw_point_op *draw, gfxw_op *free, gfxw_op *tag,
	gfxw_op_int *print, gfxw_bin_op *compare_to, gfxw_bin_op *equals,
	gfxw_bin_op *superarea_of, gfxw_visual_op *set_visual,
	gfxw_unary_container_op *free_tagged, gfxw_unary_container_op *free_contents,
	gfxw_rect_op *add_dirty, gfxw_container_op *add) {
	_gfxw_set_ops(GFXW(container), draw, free, tag, print, compare_to, equals, superarea_of);

	container->free_tagged = free_tagged;
	container->free_contents = free_contents;
	container->add_dirty_abs = add_dirty;
	container->add_dirty_rel = _gfxwop_container_add_dirty_rel;
	container->add = add;
	container->set_visual = set_visual;
}

static int _w_gfxwop_container_print_contents(const char *name, GfxWidget *widget, int indentation) {
	GfxWidget *seeker = widget;

	indent(indentation);

	sciprintf("--%s:\n", name);

	while (seeker) {
		seeker->print(seeker, indentation + 1);
		sciprintf("\n");
		seeker = seeker->_next;
	}

	return 0;
}

static int _w_gfxwop_container_print(GfxWidget *widget, int indentation) {
	gfx_dirty_rect_t *dirty;
	GfxContainer *container = (GfxContainer *)widget;
	if (!GFXW_IS_CONTAINER(widget)) {
		GFXERROR("_w_gfxwop_container_print() called on type %d widget\n", widget->_type);
		return 1;
	}

	sciprintf(" viszone=((%d,%d),(%dx%d))\n", container->zone.x, container->zone.y,
	          container->zone.width, container->zone.height);

	indent(indentation);
	sciprintf("--dirty:\n");

	dirty = container->dirty;
	while (dirty) {
		indent(indentation + 1);
		sciprintf("dirty(%d,%d, (%dx%d))\n", dirty->rect.x, dirty->rect.y, dirty->rect.width, dirty->rect.height);
		dirty = dirty->next;
	}

	_w_gfxwop_container_print_contents("contents", container->contents, indentation);

	return 0;
}


GfxContainer::GfxContainer(rect_t area, gfxw_widget_type_t type_)
	: GfxWidget(type_) {
	_bounds = zone = area;
	contents = NULL;
	nextpp = &contents;
	dirty = NULL;
	
	free_tagged = NULL;
	free_contents = NULL;
	add_dirty_abs = NULL;
	add_dirty_rel = NULL;
	add = NULL;

	_flags |= GFXW_FLAG_VISIBLE | GFXW_FLAG_CONTAINER;
}

static void recursively_free_dirty_rects(gfx_dirty_rect_t *dirty) {
	if (dirty) {
		recursively_free_dirty_rects(dirty->next);
		free(dirty);
	}
}

static int _gfxw_dirty_rect_overlaps_normal_rect(rect_t port_zone, rect_t bounds, rect_t dirty) {
	bounds.x += port_zone.x;
	bounds.y += port_zone.y;

	return gfx_rects_overlap(bounds, dirty);
}

static int _gfxwop_container_draw_contents(GfxWidget *widget, GfxWidget *contents) {
	GfxContainer *container = (GfxContainer *)widget;
	gfx_dirty_rect_t *dirty = container->dirty;
	gfx_state_t *gfx_state = (widget->_visual) ? widget->_visual->gfx_state : ((GfxVisual *) widget)->gfx_state;
	int draw_ports;
	rect_t nullzone = {0, 0, 0, 0};

	if (!contents)
		return 0;

	while (dirty) {
		GfxWidget *seeker = contents;

		while (seeker) {
			if (_gfxw_dirty_rect_overlaps_normal_rect(GFXW_IS_CONTAINER(seeker) ? nullzone : container->zone,
			        // Containers have absolute coordinates, reflect this.
			        seeker->_bounds, dirty->rect)) {

				if (GFXW_IS_CONTAINER(seeker)) {// Propagate dirty rectangles /upwards/
					DDIRTY(stderr, "container_draw_contents: propagate upwards (%d,%d,%d,%d ,0)\n", GFX_PRINT_RECT(dirty->rect));
					((GfxContainer *)seeker)->add_dirty_abs((GfxContainer *)seeker, dirty->rect, 0);
				}

				seeker->_flags |= GFXW_FLAG_DIRTY;
			}

			seeker = seeker->_next;
		}

		dirty = dirty->next;
	}

	// The draw loop is executed twice: Once for normal data, and once for ports.
	for (draw_ports = 0; draw_ports < 2; draw_ports++) {
		dirty = container->dirty;

		while (dirty) {
			GfxWidget *seeker = contents;
			while (seeker && (draw_ports || !GFXW_IS_PORT(seeker))) {
				rect_t small_rect;
				byte draw_noncontainers;

				memcpy(&small_rect, &(dirty->rect), sizeof(rect_t));
				draw_noncontainers = !_gfxop_clip(&small_rect, container->_bounds);

				if (seeker->_flags & GFXW_FLAG_DIRTY) {

					if (!GFXW_IS_CONTAINER(seeker) && draw_noncontainers) {
						GFX_ASSERT(gfxop_set_clip_zone(gfx_state, small_rect));
					}
					/* Clip zone must be reset after each element, because we might
					** descend into containers.
					** Doing this is relatively cheap, though. */
					if (draw_noncontainers || GFXW_IS_CONTAINER(seeker))
						seeker->draw(seeker, Common::Point(container->zone.x, container->zone.y));

					if (!dirty->next)
						seeker->_flags &= ~GFXW_FLAG_DIRTY;
				}

				seeker = seeker->_next;
			}
			dirty = dirty->next;
		}
	}
	// Remember that the dirty rects should be freed afterwards!

	return 0;
}

static int _gfxwop_container_free(GfxWidget *widget) {
	GfxContainer *container = (GfxContainer *)widget;
	GfxWidget *seeker = container->contents;

	while (seeker) {
		GfxWidget *next = seeker->_next;
		seeker->widfree(seeker);
		seeker = next;
	}

	recursively_free_dirty_rects(container->dirty);
	container->dirty = NULL;

	return _gfxwop_basic_free(widget);
}

static int _gfxwop_container_tag(GfxWidget *widget) {
	GfxContainer *container = (GfxContainer *) widget;
	GfxWidget *seeker = container->contents;

	while (seeker) {
		seeker->tag(seeker);
		seeker = seeker->_next;
	}

	return 0;
}

static int _w_gfxwop_container_set_visual_contents(GfxWidget *contents, GfxVisual *visual) {
	while (contents) {
		contents->set_visual(contents, visual);
		contents = contents->_next;
	}
	return 0;
}

static int _gfxwop_container_set_visual(GfxWidget *widget, GfxVisual *visual) {
	GfxContainer *container = (GfxContainer *) widget;

	container->_visual = visual;
	if (widget->_parent) {
		if (!(GFXW_IS_LIST(widget) && !GFXWC(widget)->contents)) {
			DDIRTY(stderr, "set_visual::DOWNWARDS abs(%d,%d,%d,%d, 1)\n", GFX_PRINT_RECT(widget->_bounds));
			widget->_parent->add_dirty_abs(widget->_parent, widget->_bounds, 1);
		}
	}

	return _w_gfxwop_container_set_visual_contents(container->contents, visual);
}

static int _gfxwop_container_free_tagged(GfxContainer *container) {
	GfxWidget *seekerp = (container->contents);

	while (seekerp) {
		GfxWidget *redshirt = seekerp;

		if (redshirt->_flags & GFXW_FLAG_TAGGED) {
			seekerp = (redshirt->_next);
			redshirt->widfree(redshirt);
		} else
			seekerp = (seekerp)->_next;
	}

	return 0;
}

static int _gfxwop_container_free_contents(GfxContainer *container) {
	GfxWidget *seeker = container->contents;

	while (seeker) {
		GfxWidget *next = seeker->_next;
		seeker->widfree(seeker);
		seeker = next;
	}
	return 0;
}

static void _gfxw_dirtify_container(GfxContainer *container, GfxWidget *widget) {
	if (GFXW_IS_CONTAINER(widget))
		container->add_dirty_abs(GFXWC(container), widget->_bounds, 1);
	else
		container->add_dirty_rel(GFXWC(container), widget->_bounds, 1);
}

static int _parentize_widget(GfxContainer *container, GfxWidget *widget) {
	if (widget->_parent) {
		GFXERROR("_gfxwop_container_add(): Attempt to give second parent node to widget!\nWidget:");
		widget->print(GFXW(widget), 3);
		sciprintf("\nContainer:");
		container->print(GFXW(container), 3);

		return 1;
	}

	widget->_parent = GFXWC(container);

	if (GFXW_IS_VISUAL(container))
		widget->set_visual(widget, (GfxVisual *) container);
	else if (container->_visual)
		widget->set_visual(widget, container->_visual);

	return 0;
}

static int _gfxw_container_id_equals(GfxContainer *container, GfxWidget *widget) {
	GfxWidget **seekerp = &(container->contents);

	if (GFXW_IS_PORT(widget))
		return 0;

	if (widget->_ID == GFXW_NO_ID)
		return 0;

	while (*seekerp && ((*seekerp)->_ID != widget->_ID || (*seekerp)->_subID != widget->_subID))
		seekerp = &((*seekerp)->_next);

	if (!*seekerp)
		return 0;

	if ((*seekerp)->equals(*seekerp, widget) && !(*seekerp)->should_replace(*seekerp, widget)) {
		widget->widfree(widget);
		(*seekerp)->_flags &= ~GFXW_FLAG_TAGGED;
		return 1;
	} else {
		if (!(widget->_flags & GFXW_FLAG_MULTI_ID))
			(*seekerp)->widfree(*seekerp);
		return 0;
	}
}

static int _gfxwop_container_add_dirty(GfxContainer *container, rect_t dirty, int propagate) {
#if 0
	// This code has been disabled because containers may contain sub-containers with
	// bounds greater than their own.
	if (_gfxop_clip(&dirty, container->_bounds))
		return 0;
#endif

	DDIRTY(stderr, "Effectively adding dirty %d,%d,%d,%d %d to ID %d\n", GFX_PRINT_RECT(dirty), propagate, container->_ID);
	container->dirty = gfxdr_add_dirty(container->dirty, dirty, GFXW_DIRTY_STRATEGY);
	return 0;
}

static int _gfxwop_container_add(GfxContainer *container, GfxWidget *widget) {
	if (_gfxw_container_id_equals(container, widget))
		return 0;

	if (_parentize_widget(container, widget))
		return 1;

	if (!(GFXW_IS_LIST(widget) && (!GFXWC(widget)->contents))) { // Don't dirtify self on empty lists
		DDIRTY(stderr, "container_add: dirtify DOWNWARDS (%d,%d,%d,%d, 1)\n", GFX_PRINT_RECT(widget->_bounds));
		_gfxw_dirtify_container(container, widget);
	}

	*(container->nextpp) = widget;
	container->nextpp = &(widget->_next);

	return 0;
}

//*** Lists and sorted lists ***

static int _gfxwop_list_draw(GfxWidget *list, Common::Point pos) {
	DRAW_ASSERT(list, GFXW_LIST);

	_gfxwop_container_draw_contents(list, ((GfxList *)list)->contents);
	recursively_free_dirty_rects(GFXWC(list)->dirty);
	GFXWC(list)->dirty = NULL;
	list->_flags &= ~GFXW_FLAG_DIRTY;

	return 0;
}

static int _gfxwop_sorted_list_draw(GfxWidget *list, Common::Point pos) {
	DRAW_ASSERT(list, GFXW_SORTED_LIST);

	_gfxwop_container_draw_contents(list, ((GfxList *)list)->contents);
	recursively_free_dirty_rects(GFXWC(list)->dirty);
	GFXWC(list)->dirty = NULL;

	return 0;
}

static int _w_gfxwop_list_print(GfxWidget *list, const char *name, int indentation) {
	_gfxw_print_widget(list, indentation);
	sciprintf("%s", name);

	return _w_gfxwop_container_print(list, indentation);
}

static int _gfxwop_list_print(GfxWidget *list, int indentation) {
	return _w_gfxwop_list_print(list, "LIST", indentation);
}

static int _gfxwop_sorted_list_print(GfxWidget *list, int indentation) {
	return _w_gfxwop_list_print(list, "SORTED_LIST", indentation);
}

static int _gfxwop_list_equals(GfxWidget *widget, GfxWidget *other) {
	// Requires identical order of list elements.
	GfxList *wlist, *olist;

	if (widget->_type != other->_type)
		return 0;

	if (!GFXW_IS_LIST(widget)) {
		GFXWARN("_gfxwop_list_equals(): Method called on non-list!\n");
		widget->print(widget, 0);
		sciprintf("\n");
		return 0;
	}

	wlist = (GfxList *)widget;
	olist = (GfxList *)other;

	if (memcmp(&(wlist->_bounds), &(olist->_bounds), sizeof(rect_t)))
		return 0;

	widget = wlist->contents;
	other = olist->contents;

	while (widget && other) {
		if (!(widget->equals(widget, other) && !widget->should_replace(widget, other)))
			return 0;

		widget = widget->_next;
		other = other->_next;
	}

	return (!widget && !other); // True if both are finished now
}

static int _gfxwop_list_add_dirty(GfxContainer *container, rect_t dirty, int propagate) {
	// Lists add dirty boxes to both themselves and their parenting port/visual

	container->_flags |= GFXW_FLAG_DIRTY;

	DDIRTY(stderr, "list_add_dirty %d,%d,%d,%d %d\n", GFX_PRINT_RECT(dirty), propagate);
	if (propagate)
		if (container->_parent) {
			DDIRTY(stderr, "->PROPAGATING\n");
			container->_parent->add_dirty_abs(container->_parent, dirty, 1);
		}

	return _gfxwop_container_add_dirty(container, dirty, propagate);
}

int _gfxwop_ordered_add(GfxContainer *container, GfxWidget *widget, int compare_all) {
	// O(n)
	GfxWidget **seekerp = &(container->contents);

	if (widget->_next) {
		GFXERROR("_gfxwop_sorted_list_add(): Attempt to add widget to two lists!\nWidget:");
		widget->print(GFXW(widget), 3);
		sciprintf("\nList:");
		container->print(GFXW(container), 3);
		BREAKPOINT();

		return 1;
	}

	if (_gfxw_container_id_equals(container, widget))
		return 0;

	while (*seekerp && (compare_all || (widget->compare_to(widget, *seekerp) >= 0))) {

		if (widget->equals(GFXW(widget), GFXW(*seekerp))) {
			if (compare_all) {
				if ((*seekerp)->_visual)
					(*seekerp)->widfree(GFXW(*seekerp)); // If it's a fresh widget
				else
					gfxw_annihilate(GFXW(*seekerp));

				return _gfxwop_ordered_add(container, widget, compare_all); // We might have destroyed the container's contents
			} else {
				widget->_next = (*seekerp)->_next;
				(*seekerp)->widfree(GFXW(*seekerp));
				*seekerp = widget;
				return (_parentize_widget(container, widget));
			}
		}

		if (*seekerp)
			seekerp = &((*seekerp)->_next);
	}

	widget->_next = *seekerp;
	*seekerp = widget;

	return _parentize_widget(container, widget);
}

static int _gfxwop_sorted_list_add(GfxContainer *container, GfxWidget *widget) {
	// O(n)
	return _gfxwop_ordered_add(container, widget, 0);
}

void _gfxw_set_ops_LIST(GfxContainer *list, char sorted) {
	_gfxw_set_container_ops((GfxContainer *)list, sorted ? _gfxwop_sorted_list_draw : _gfxwop_list_draw,
	                        _gfxwop_container_free, _gfxwop_container_tag,
	                        sorted ? _gfxwop_sorted_list_print : _gfxwop_list_print,
	                        _gfxwop_basic_compare_to, sorted ? _gfxwop_basic_equals : _gfxwop_list_equals,
	                        _gfxwop_basic_superarea_of, _gfxwop_container_set_visual,
	                        _gfxwop_container_free_tagged, _gfxwop_container_free_contents,
	                        _gfxwop_list_add_dirty, sorted ? _gfxwop_sorted_list_add : _gfxwop_container_add);
}

GfxList *gfxw_new_list(rect_t area, int sorted) {
	return new GfxList(area, sorted);
}

GfxList::GfxList(rect_t area, bool sorted)
	: GfxContainer(area, sorted ? GFXW_SORTED_LIST : GFXW_LIST) {

	_gfxw_set_ops_LIST(this, sorted);
}



//*** Visuals ***

static int _gfxwop_visual_draw(GfxWidget *widget, Common::Point pos) {
	GfxVisual *visual = (GfxVisual *) widget;
	gfx_dirty_rect_t *dirty = visual->dirty;
	DRAW_ASSERT(widget, GFXW_VISUAL);

	while (dirty) {
		int err = gfxop_clear_box(visual->gfx_state, dirty->rect);

		if (err) {
			GFXERROR("Error while clearing dirty rect (%d,%d,(%dx%d))\n", dirty->rect.x,
			         dirty->rect.y, dirty->rect.width, dirty->rect.height);
			if (err == GFX_FATAL)
				return err;
		}

		dirty = dirty->next;
	}

	_gfxwop_container_draw_contents(widget, visual->contents);

	recursively_free_dirty_rects(visual->dirty);
	visual->dirty = NULL;
	widget->_flags &= ~GFXW_FLAG_DIRTY;

	return 0;
}

static int _gfxwop_visual_free(GfxWidget *widget) {
	GfxVisual *visual = (GfxVisual *) widget;
	int retval;

	if (!GFXW_IS_VISUAL(visual)) {
		GFXERROR("_gfxwop_visual_free() called on non-visual!Widget was: ");
		widget->print(widget, 3);
		return 1;
	}

	retval = _gfxwop_container_free(widget);

	return 0;
}

static int _gfxwop_visual_print(GfxWidget *widget, int indentation) {
	int comma = 0;
	GfxVisual *visual = (GfxVisual *) widget;

	if (!GFXW_IS_VISUAL(visual)) {
		GFXERROR("_gfxwop_visual_print() called on non-visual!Widget was: ");
		widget->print(widget, 3);
		return 1;
	}

	_gfxw_print_widget(widget, indentation);
	sciprintf("VISUAL; ports={");
	for (uint i = 0; i < visual->_portRefs.size(); i++) {
		if (visual->_portRefs[i]) {
			if (comma)
				sciprintf(",");
			else
				comma = 1;

			sciprintf("%d", i);
		}
	}
	sciprintf("}\n");

	return _w_gfxwop_container_print(widget, indentation);
}

static int _gfxwop_visual_set_visual(GfxWidget *self, GfxVisual *visual) {
	if (self != GFXW(visual)) {
		GFXWARN("Attempt to set a visual's parent visual to something else!\n");
	} else {
		GFXWARN("Attempt to set a visual's parent visual!\n");
	}

	return 1;
}

void _gfxw_set_ops_VISUAL(GfxContainer *visual) {
	_gfxw_set_container_ops((GfxContainer *)visual, _gfxwop_visual_draw, _gfxwop_visual_free,
	                        _gfxwop_container_tag, _gfxwop_visual_print, _gfxwop_basic_compare_to,
	                        _gfxwop_basic_equals, _gfxwop_basic_superarea_of, _gfxwop_visual_set_visual,
	                        _gfxwop_container_free_tagged, _gfxwop_container_free_contents,
	                        _gfxwop_container_add_dirty, _gfxwop_container_add);
}

GfxVisual *gfxw_new_visual(gfx_state_t *state, int font) {
	return new GfxVisual(state, font);
}

GfxVisual::GfxVisual(gfx_state_t *state, int font)
	: GfxContainer(gfx_rect(0, 0, 320, 200), GFXW_VISUAL) {

	font_nr = font;
	gfx_state = state;

	_gfxw_set_ops_VISUAL(this);
}

static int _visual_find_free_ID(GfxVisual *visual) {
	uint id = 0;

	while (id < visual->_portRefs.size() && visual->_portRefs[id])
		id++;

	if (id == visual->_portRefs.size()) { // Out of ports?
		visual->_portRefs.push_back(0);
	}

	return id;
}

static int _gfxwop_add_dirty_rects(GfxContainer *dest, gfx_dirty_rect_t *src) {
	DDIRTY(stderr, "Adding multiple dirty to #%d\n", dest->_ID);
	if (src) {
		dest->dirty = gfxdr_add_dirty(dest->dirty, src->rect, GFXW_DIRTY_STRATEGY);
		_gfxwop_add_dirty_rects(dest, src->next);
	}

	return 0;
}

//*** Ports ***

static int _gfxwop_port_draw(GfxWidget *widget, Common::Point pos) {
	GfxPort *port = (GfxPort *) widget;
	DRAW_ASSERT(widget, GFXW_PORT);

	if (port->decorations) {
		DDIRTY(stderr, "Getting/applying deco dirty (multi)\n");
		_gfxwop_add_dirty_rects(GFXWC(port->decorations), port->dirty);
		if (port->decorations->draw(GFXW(port->decorations), gfxw_point_zero)) {
			port->decorations->dirty = NULL;
			return 1;
		}
		port->decorations->dirty = NULL;
	}

	_gfxwop_container_draw_contents(widget, port->contents);

	recursively_free_dirty_rects(port->dirty);
	port->dirty = NULL;
	widget->_flags &= ~GFXW_FLAG_DIRTY;

	return 0;
}

static int _gfxwop_port_free(GfxWidget *widget) {
	GfxPort *port = (GfxPort *) widget;

	if (port->_visual) {
		GfxVisual *visual = port->_visual;
		int ID = port->_ID;

		if (ID < 0 || ID >= (int)visual->_portRefs.size()) {
			GFXWARN("Attempt to free port #%d; allowed: [0..%d]!\n", ID, visual->_portRefs.size());
			return GFX_ERROR;
		}

		if (visual->_portRefs[ID] != port) {
			GFXWARN("While freeing port %d: Port is at %p, but port list indicates %p", ID, (void *)port, (void *)visual->_portRefs[ID]);
		} else
			visual->_portRefs[ID] = NULL;

	}

	if (port->decorations)
		port->decorations->widfree(GFXW(port->decorations));

	return _gfxwop_container_free(widget);
}

static int _gfxwop_port_print(GfxWidget *widget, int indentation) {
	GfxPort *port = (GfxPort *)widget;

	_gfxw_print_widget(widget, indentation);
	sciprintf("PORT");
	sciprintf(" font=%d drawpos=(%d,%d)", port->font_nr, port->draw_pos.x, port->draw_pos.y);
	if (port->gray_text)
		sciprintf(" (gray)");
	_w_gfxwop_container_print(GFXW(port), indentation);

	return _w_gfxwop_container_print_contents("decorations", GFXW(port->decorations), indentation);
}

static int _gfxwop_port_superarea_of(GfxWidget *self, GfxWidget *other) {
	GfxPort *port = (GfxPort *) self;

	if (!port->port_bg)
		return _gfxwop_basic_superarea_of(self, other);

	return port->port_bg->superarea_of(port->port_bg, other);
}

static int _gfxwop_port_set_visual(GfxWidget *widget, GfxVisual *visual) {
	GfxList *decorations = ((GfxPort *) widget)->decorations;
	widget->_visual = visual;

	if (decorations)
		if (decorations->set_visual(GFXW(decorations), visual)) {
			GFXWARN("Setting the visual for decorations failed for port ");
			widget->print(widget, 1);
			return 1;
		}

	return _gfxwop_container_set_visual(widget, visual);
}

static int _gfxwop_port_add_dirty(GfxContainer *widget, rect_t dirty, int propagate) {
	GfxPort *self = (GfxPort *) widget;

	self->_flags |= GFXW_FLAG_DIRTY;

	_gfxwop_container_add_dirty(widget, dirty, propagate);

	DDIRTY(stderr, "Added dirty to ID %d\n", widget->_ID);
	DDIRTY(stderr, "dirty= (%d,%d,%d,%d) bounds (%d,%d,%d,%d)\n", dirty.x, dirty.x, dirty.width, dirty.height,
	       widget->_bounds.x, widget->_bounds.y, widget->_bounds.width, widget->_bounds.height);
#if 0
	// FIXME: This is a worthwhile optimization
	if (self->port_bg) {
		GfxWidget foo;

		foo.bounds = dirty; // Yeah, sub-elegant, I know
		foo.bounds.x -= self->zone.x;
		foo.bounds.y -= self->zone.y;
		if (self->port_bg->superarea_of(self->port_bg, &foo)) {
			GfxContainer *parent = self->_parent;
			while (parent) {
				fprintf(stderr, "Dirtifying parent id %d\n", parent->_ID);
				parent->_flags |= GFXW_FLAG_DIRTY;
				parent = parent->_parent;
			}
			return 0;
		}
	} // else propagate to the parent, since we're not 'catching' the dirty rect
#endif

	if (propagate)
		if (self->_parent) {
			DDIRTY(stderr, "PROPAGATE\n");
			return self->_parent->add_dirty_abs(self->_parent, dirty, 1);
		}

	return 0;
}

static int _gfxwop_port_add(GfxContainer *container, GfxWidget *widget) {
	// O(n)
	return _gfxwop_ordered_add(container, widget, 1);
}

void _gfxw_set_ops_PORT(GfxContainer *widget) {
	_gfxw_set_container_ops((GfxContainer *)widget, _gfxwop_port_draw, _gfxwop_port_free, _gfxwop_container_tag,
	                        _gfxwop_port_print, _gfxwop_basic_compare_to, _gfxwop_basic_equals, _gfxwop_port_superarea_of,
	                        _gfxwop_port_set_visual, _gfxwop_container_free_tagged, _gfxwop_container_free_contents,
	                        _gfxwop_port_add_dirty, _gfxwop_port_add);
}

GfxPort *gfxw_new_port(GfxVisual *visual, GfxPort *predecessor, rect_t area, gfx_color_t fgcolor, gfx_color_t bgcolor) {
	return new GfxPort(visual, area, fgcolor, bgcolor);
}

GfxPort::GfxPort(GfxVisual *visual_, rect_t area, gfx_color_t fgcolor, gfx_color_t bgcolor_)
	: GfxContainer(area, GFXW_PORT) {
	VERIFY_WIDGET(visual_);

	port_bg = NULL;
	_parent = NULL;
	decorations = NULL;
	title_text = NULL;
	draw_pos = Common::Point(0, 0);
	gray_text = 0;
	_color = fgcolor;
	_bgcolor = bgcolor_;
	font_nr = visual_->font_nr;
	_ID = _visual_find_free_ID(visual_);
	visual_->_portRefs[_ID] = this;

	_gfxw_set_ops_PORT(this);
}

void gfxw_port_auto_restore_background(GfxVisual *visual, GfxPort *window, rect_t auto_rect) {
	window->port_flags |= kWindowAutoRestore;
	window->restore_snap = gfxw_make_snapshot(visual, auto_rect);
}

GfxPort *gfxw_remove_port(GfxVisual *visual, GfxPort *port) {
	GfxPort *parent;
	VERIFY_WIDGET(visual);
	VERIFY_WIDGET(port);

	if (!visual->contents) {
		GFXWARN("Attempt to remove port from empty visual\n");
		return NULL;
	}

	parent = (GfxPort *)port->_parent;
	if (port->port_flags & kWindowAutoRestore)
		gfxw_restore_snapshot(visual, port->restore_snap);

	if (port->widfree(GFXW(port)))
		return parent;

	while (parent && !GFXW_IS_PORT(parent))
		parent = (GfxPort *)parent->_parent; // Ascend through ancestors

	return parent;
}

GfxPort *gfxw_find_port(GfxVisual *visual, int ID) {
	if (ID < 0 || ID >= (int)visual->_portRefs.size())
		return NULL;

	return visual->_portRefs[ID];
}

GfxPort *gfxw_find_default_port(GfxVisual *visual) {
	int id = visual->_portRefs.size();

	while (id--) {
		GfxPort *port = visual->_portRefs[id];

		if (port)
			return port;
	}

	return NULL;
}

// - other functions -

GfxWidget *gfxw_set_id(GfxWidget *widget, int ID, int subID) {
	if (widget) {
		widget->_ID = ID;
		widget->_subID = subID;
	}

	return widget;
}

GfxDynView *gfxw_dyn_view_set_params(GfxDynView *widget, int under_bits, void *under_bitsp, int signal, void *signalp) {
	if (!widget)
		return NULL;

	widget->under_bits = under_bits;
	widget->under_bitsp = under_bitsp;
	widget->signal = signal;
	widget->signalp = signalp;

	return widget;
}

GfxWidget *gfxw_remove_id(GfxContainer *container, int ID, int subID) {
	GfxWidget **wp = &(container->contents);

	while (*wp) {
		if ((*wp)->_ID == ID && (subID == GFXW_NO_ID || (*wp)->_subID == subID)) {
			GfxWidget *widget = *wp;

			*wp = (*wp)->_next;
			widget->_next = NULL;
			widget->_parent = NULL;
			widget->_visual = NULL;

			return widget;
		}

		wp = &((*wp)->_next);
	}

	return NULL;
}

GfxWidget *gfxw_hide_widget(GfxWidget *widget) {
	if (widget->_flags & GFXW_FLAG_VISIBLE) {
		widget->_flags &= ~GFXW_FLAG_VISIBLE;

		if (widget->_parent)
			widget->_parent->add_dirty_rel(widget->_parent, widget->_bounds, 1);
	}

	return widget;
}

GfxWidget *gfxw_show_widget(GfxWidget *widget) {
	if (!(widget->_flags & GFXW_FLAG_VISIBLE)) {
		widget->_flags |= GFXW_FLAG_VISIBLE;

		if (widget->_parent)
			widget->_parent->add_dirty_rel(widget->_parent, widget->_bounds, 1);
	}

	return widget;
}

gfxw_snapshot_t *gfxw_make_snapshot(GfxVisual *visual, rect_t area) {
	gfxw_snapshot_t *retval = (gfxw_snapshot_t*)sci_malloc(sizeof(gfxw_snapshot_t));

	retval->serial = widget_serial_number_counter++;

	retval->area = area;

	// Work around subset semantics in gfx_rect_subset.
	// This fixes the help icon in LSL5. */
	if (retval->area.width == 320)
		retval->area.width = 321;

	return retval;
}

int gfxw_widget_matches_snapshot(gfxw_snapshot_t *snapshot, GfxWidget *widget) {
	int free_below = (snapshot->serial < widget_serial_number_counter) ? 0 : widget_serial_number_counter;
	int free_above_eq = snapshot->serial;
	rect_t bounds = widget->_bounds;

	if (!GFXW_IS_CONTAINER(widget) && widget->_parent) {
		bounds.x += widget->_parent->_bounds.x;
		bounds.y += widget->_parent->_bounds.y;
	}

	// Note: the check for snapshot->area and bounds is NOT the same as contains() in Common::Rect
	// (this one includes equality too)
	return ((widget->_serial >= free_above_eq || widget->_serial < free_below) &&
			(snapshot->area.x <= bounds.x && snapshot->area.y <= bounds.y &&
			 snapshot->area.x + snapshot->area.width >= bounds.x + bounds.width &&
			 snapshot->area.y + snapshot->area.height >= bounds.y + bounds.height));
}

#define MAGIC_FREE_NUMBER -42

void _gfxw_free_contents_appropriately(GfxContainer *container, gfxw_snapshot_t *snapshot, int priority) {
	GfxWidget *widget = container->contents;

	while (widget) {
		GfxWidget *next = widget->_next;

		if (gfxw_widget_matches_snapshot(snapshot, widget) && !(widget->_flags & GFXW_FLAG_IMMUNE_TO_SNAPSHOTS)
		        && (priority == MAGIC_FREE_NUMBER || priority <= widget->_widgetPriority || widget->_widgetPriority == -1)) {
			widget->widfree(widget);
		} else {
			if (GFXW_IS_CONTAINER(widget))
				_gfxw_free_contents_appropriately(GFXWC(widget), snapshot, priority);
		}

		widget = next;
	}
}

gfxw_snapshot_t *gfxw_restore_snapshot(GfxVisual *visual, gfxw_snapshot_t *snapshot) {
	_gfxw_free_contents_appropriately(GFXWC(visual), snapshot, MAGIC_FREE_NUMBER);

	return snapshot;
}

void gfxw_annihilate(GfxWidget *widget) {
	GfxVisual *visual = widget->_visual;
	int widget_priority = 0;
	int free_overdrawn = 0;

	gfxw_snapshot_t snapshot;
	if (!GFXW_IS_CONTAINER(widget) && widget->_parent && visual && (widget->_flags & GFXW_FLAG_VISIBLE)) {
		snapshot.serial = 0;
		snapshot.area = widget->_bounds;
		snapshot.area.x += widget->_parent->zone.x;
		snapshot.area.y += widget->_parent->zone.y;
		free_overdrawn = 1;
		widget_priority = widget->_widgetPriority;
	}

	widget->widfree(GFXW(widget));

	if (free_overdrawn)
		_gfxw_free_contents_appropriately(GFXWC(visual), &snapshot, widget_priority);
}

GfxDynView *gfxw_picviewize_dynview(GfxDynView *dynview) {
	dynview->_type = GFXW_PIC_VIEW;
	dynview->_flags |= GFXW_FLAG_DIRTY;

	_gfxw_set_ops_PICVIEW(GFXW(dynview));

	if (dynview->_parent)
		_gfxw_dirtify_container(dynview->_parent, GFXW(dynview));

	return dynview;
}

} // End of namespace Sci
