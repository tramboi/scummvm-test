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

#include "common/system.h"
#include "common/events.h"
#include "graphics/cursorman.h"
#include "graphics/video/avi_player.h"
#include "graphics/surface.h"

#include "sci/sci.h"
#include "sci/debug.h"	// for g_debug_sleeptime_factor
#include "sci/resource.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/gfx/gfx_gui.h"
#include "sci/gfx/gfx_widgets.h"
#include "sci/gfx/gfx_state_internal.h"	// required for GfxContainer, GfxPort, GfxVisual
#include "sci/gfx/seq_decoder.h"

namespace Sci {

#undef DEBUG_LSRECT

// This is the real width of a text with a specified width of 0
#define MAX_TEXT_WIDTH_MAGIC_VALUE 192

// Graph subfunctions
enum {
	K_GRAPH_GET_COLORS_NR = 2,
	K_GRAPH_DRAW_LINE = 4,
	K_GRAPH_SAVE_BOX = 7,
	K_GRAPH_RESTORE_BOX = 8,
	K_GRAPH_FILL_BOX_BACKGROUND = 9,
	K_GRAPH_FILL_BOX_FOREGROUND = 10,
	K_GRAPH_FILL_BOX_ANY = 11,
	K_GRAPH_UPDATE_BOX = 12,
	K_GRAPH_REDRAW_BOX = 13,
	K_GRAPH_ADJUST_PRIORITY = 14
};

// Control types and flags
enum {
	K_CONTROL_BUTTON = 1,
	K_CONTROL_TEXT = 2,
	K_CONTROL_EDIT = 3,
	K_CONTROL_ICON = 4,
	K_CONTROL_CONTROL = 6,
	K_CONTROL_CONTROL_ALIAS = 7,
	K_CONTROL_BOX = 10
};

#define ADD_TO_CURRENT_PORT(widget) \
	{if (s->port)				   \
		s->port->add((GfxContainer *)s->port, widget); \
	else \
		s->picture_port->add((GfxContainer *)s->visual, widget);}

#define ADD_TO_CURRENT_PICTURE_PORT(widget) \
	{if (s->port)				   \
		s->port->add((GfxContainer *)s->port, widget); \
	else \
		s->picture_port->add((GfxContainer *)s->picture_port, widget);}

#define ADD_TO_WINDOW_PORT(widget) \
	s->wm_port->add((GfxContainer *)s->wm_port, widget);

#define FULL_REDRAW()\
	if (s->visual) \
		s->visual->draw(gfxw_point_zero); \
	gfxop_update(s->gfx_state);

#if 0
// Used for debugging
#define FULL_INSPECTION()\
	if (s->visual) \
		s->visual->print(s->visual, 0);
#endif

static inline int sign_extend_byte(int value) {
	if (value & 0x80)
		return value - 256;
	else
		return value;
}

// was static
void assert_primary_widget_lists(EngineState *s) {
	if (!s->dyn_views) {
		rect_t bounds = s->picture_port->_bounds;

		s->dyn_views = gfxw_new_list(bounds, GFXW_LIST_SORTED);
		s->dyn_views->_flags |= GFXW_FLAG_IMMUNE_TO_SNAPSHOTS;
		ADD_TO_CURRENT_PICTURE_PORT(s->dyn_views);
	}

	if (!s->drop_views) {
		rect_t bounds = s->picture_port->_bounds;

		s->drop_views = gfxw_new_list(bounds, GFXW_LIST_SORTED);
		s->drop_views->_flags |= GFXW_FLAG_IMMUNE_TO_SNAPSHOTS;
		ADD_TO_CURRENT_PICTURE_PORT(s->drop_views);
	}
}

// static
void reparentize_primary_widget_lists(EngineState *s, GfxPort *newport) {
	if (!newport)
		newport = s->picture_port;

	if (s->dyn_views) {
		gfxw_remove_widget_from_container(s->dyn_views->_parent, s->dyn_views);

		newport->add((GfxContainer *)newport, s->dyn_views);
	}
}

int _find_view_priority(EngineState *s, int y) {
	/*if (s->version <= SCI_VERSION_LTU_PRIORITY_OB1)
		++y; */

	if (s->pic_priority_table) { // SCI01 priority table set?
		int j;
		for (j = 0; j < 15; j++)
			if (y < s->pic_priority_table[j+1])
				return j;
		return 14; // Maximum
	} else {
		if (!s->_kernel->usesOldGfxFunctions())
			return SCI0_VIEW_PRIORITY_14_ZONES(y);
		else
			return SCI0_VIEW_PRIORITY(y) == 15 ? 14 : SCI0_VIEW_PRIORITY(y);
	}
}

int _find_priority_band(EngineState *s, int nr) {
	if (!s->_kernel->usesOldGfxFunctions() && (nr < 0 || nr > 14)) {
		if (nr == 15)
			return 0xffff;
		else {
			warning("Attempt to get priority band %d", nr);
		}
		return 0;
	}

	if (s->_kernel->usesOldGfxFunctions() && (nr < 0 || nr > 15)) {
		warning("Attempt to get priority band %d", nr);
		return 0;
	}

	if (s->pic_priority_table) // SCI01 priority table set?
		return s->pic_priority_table[nr];
	else {
		int retval;

		if (!s->_kernel->usesOldGfxFunctions())
			retval = SCI0_PRIORITY_BAND_FIRST_14_ZONES(nr);
		else
			retval = SCI0_PRIORITY_BAND_FIRST(nr);

/*		if (s->version <= SCI_VERSION_LTU_PRIORITY_OB1)
			--retval; */
		return retval;
	}
}

reg_t graph_save_box(EngineState *s, rect_t area) {
	reg_t handle = kalloc(s->segMan, "graph_save_box()", sizeof(gfxw_snapshot_t *));
	gfxw_snapshot_t **ptr = (gfxw_snapshot_t **)kmem(s->segMan, handle);

	// FIXME: gfxw_make_snapshot returns a pointer. Now why do we store a
	// pointer to real memory inside the SCI heap?
	// If we save and the load again, this cannot work in general.
	// This seems like bad design. Either the snapshot data itself should be
	// stored in the heap, or a unique persistent id.
	*ptr = gfxw_make_snapshot(s->visual, area);

	return handle;
}

void graph_restore_box(EngineState *s, reg_t handle) {
	gfxw_snapshot_t **ptr;
	int port_nr = s->port->_ID;

	if (!handle.segment) {
		warning("Attempt to restore box with zero handle");
		return;
	}

	ptr = (gfxw_snapshot_t **)kmem(s->segMan, handle);

	if (!ptr) {
		warning("Attempt to restore invalid handle %04x:%04x", PRINT_REG(handle));
		return;
	}

	while (port_nr > 2 && !(s->port->_flags & GFXW_FLAG_IMMUNE_TO_SNAPSHOTS) && (gfxw_widget_matches_snapshot(*ptr, s->port))) {
		// This shouldn't ever happen, actually, since windows (ports w/ ID > 2) should all be immune
		GfxPort *newport = s->visual->getPort(port_nr);
		error("Port %d is not immune against snapshots", s->port->_ID);
		port_nr--;
		if (newport)
			s->port = newport;
	}

	if (s->dyn_views && gfxw_widget_matches_snapshot(*ptr, s->dyn_views->_parent)) {
		GfxContainer *parent = s->dyn_views->_parent;

		do {
			parent = parent->_parent;
		} while (parent && (gfxw_widget_matches_snapshot(*ptr, parent)));

		if (!parent) {
			error("Attempted widget mass destruction by a snapshot");
		}

		reparentize_primary_widget_lists(s, (GfxPort *) parent);
	}


	if (!ptr) {
		error("Attempt to restore invalid snaphot with handle %04x:%04x", PRINT_REG(handle));
		return;
	}

	gfxw_restore_snapshot(s->visual, *ptr);
	free(*ptr);
	*ptr = NULL;

	kfree(s->segMan, handle);
}

PaletteEntry get_pic_color(EngineState *s, int color) {
	if (!s->resMan->isVGA())
		return s->ega_colors[color].visual;

	if (color == -1 || color == 255)     // -1 occurs in Eco Quest 1. Not sure if this is the best approach, but it seems to work
		return PaletteEntry(255,255,255);
	else if (color < s->gfx_state->gfxResMan->getColorCount())
		return s->gfx_state->gfxResMan->getColor(color);
	else {
		// Happens in the beginning of EcoQuest 2, when the dialog box of the customs officer shows up
		warning("Color index %d out of bounds for pic %d (%d max)", color, s->gfx_state->pic_nr, s->gfx_state->gfxResMan->getColorCount());
		return PaletteEntry(0,0,0);
	}
}

static reg_t kSetCursorSci0(EngineState *s, int argc, reg_t *argv) {
	int16 cursor = argv[0].toSint16();

	if ((argc >= 2) && (argv[1].toSint16() == 0))
		cursor = GFXOP_NO_POINTER;

	gfxop_set_pointer_cursor(s->gfx_state, cursor);

	// Set pointer position, if requested
	if (argc >= 4) {
		Common::Point newPos = Common::Point(argv[2].toSint16() + s->port->_bounds.x, argv[3].toSint16() + s->port->_bounds.y);
		gfxop_set_pointer_position(s->gfx_state, newPos);
	}

	return s->r_acc;
}

static reg_t kSetCursorSci11(EngineState *s, int argc, reg_t *argv) {
	Common::Point *hotspot = NULL;

	switch (argc) {
	case 1:
		CursorMan.showMouse(argv[0].toSint16() != 0);
		break;
	case 2:
		gfxop_set_pointer_position(s->gfx_state,
				   Common::Point(argv[0].toUint16() + s->port->_bounds.x, argv[1].toUint16() + s->port->_bounds.y));
		break;
	case 4: {
		int16 top = argv[0].toSint16();
		int16 left = argv[1].toSint16();
		int16 bottom = argv[2].toSint16();
		int16 right = argv[3].toSint16();

		if ((right >= left) && (bottom >= top)) {
			Common::Rect rect = Common::Rect(left, top, right + 1, bottom + 1);
			gfxop_set_pointer_zone(s->gfx_state, rect);
		} else {
			warning("kSetCursor: Ignoring invalid mouse zone (%i, %i)-(%i, %i)", left, top, right, bottom);
		}
		break;
	}
	case 5:
	case 9:
		hotspot = new Common::Point(argv[3].toSint16(), argv[4].toSint16());
		// Fallthrough
	case 3:
		gfxop_set_pointer_view(s->gfx_state, argv[0].toUint16(), argv[1].toUint16(), argv[2].toUint16(), hotspot);
		if (hotspot)
			delete hotspot;
		break;
	default :
		warning("kSetCursor: Unhandled case: %d arguments given", argc);
		break;
	}
	return s->r_acc;
}

reg_t kSetCursor(EngineState *s, int argc, reg_t *argv) {
	switch (s->detectSetCursorType()) {
	case SCI_VERSION_0_EARLY:
		return kSetCursorSci0(s, argc, argv);
	case SCI_VERSION_1_1:
		return kSetCursorSci11(s, argc, argv);
	default:
		warning("Unknown SetCursor type");
		return NULL_REG;
	}
}

reg_t kMoveCursor(EngineState *s, int argc, reg_t *argv) {
	if (argc == 2)
		s->gui->moveCursor(argv[0].toSint16(), argv[1].toSint16());
	return s->r_acc;
}

reg_t kShow(EngineState *s, int argc, reg_t *argv) {
	int old_map = s->pic_visible_map;

	s->pic_visible_map = (argc > 0) ? (gfx_map_mask_t) argv[0].toUint16() : GFX_MASK_VISUAL;

	switch (s->pic_visible_map) {

	case GFX_MASK_VISUAL:
	case GFX_MASK_PRIORITY:
	case GFX_MASK_CONTROL:
		gfxop_set_visible_map(s->gfx_state, s->pic_visible_map);
		if (old_map != s->pic_visible_map) {

			if (s->pic_visible_map == GFX_MASK_VISUAL) // Full widget redraw
				s->visual->draw(Common::Point(0, 0));

			gfxop_update(s->gfx_state);
			debugC(2, kDebugLevelGraphics, "Switching visible map to %x\n", s->pic_visible_map);
		}
		break;

	default:
		warning("Show(%x) selects unknown map", s->pic_visible_map);

	}

	s->pic_not_valid = 2;
	return s->r_acc;
}

reg_t kPicNotValid(EngineState *s, int argc, reg_t *argv) {
	s->r_acc = make_reg(0, s->pic_not_valid);
	if (argc)
		s->pic_not_valid = (byte)argv[0].toUint16();

	return s->r_acc;
}

void _k_redraw_box(EngineState *s, int x1, int y1, int x2, int y2) {
	warning("_k_redraw_box(): Unimplemented");
#if 0
	int i;
	ViewObject *list = s->dyn_views;

	printf("Reanimating views\n", s->dyn_views_nr);

	for (i = 0;i < s->dyn_views_nr;i++) {
		*(list[i].underBitsp) = graph_save_box(s, list[i].nsLeft, list[i].nsTop, list[i].nsRight - list[i].nsLeft,
												list[i].nsBottom - list[i].nsTop, SCI_MAP_VISUAL | SCI_MAP_PRIORITY);
		draw_view0(s->pic, s->ports[0], list[i].nsLeft, list[i].nsTop, list[i].priority, list[i].loop,
		           list[i].cel, 0, list[i].view);
	}

	graph_update_box(s, x1, y1, x2 - x1, y2 - y1);

	for (i = 0;i < s->dyn_views_nr;i++)	{
		graph_restore_box(s, *(list[i].underBitsp));
		list[i].underBits = 0;
	}
#endif
}

reg_t kGraph(EngineState *s, int argc, reg_t *argv) {
	int rectLeft = 0, rectTop = 0, rectRight = 0, rectBottom = 0;
	uint16 flags;
	int16 priority, control, color, colorMask;

	Common::Rect rect;
	if (argc>=5) {
		rectLeft = argv[2].toSint16(); rectTop = argv[1].toSint16();
		rectRight = argv[4].toSint16(); rectBottom = argv[3].toSint16();
		// Fixup data, so that a valid rectangle is formed
		if (rectLeft > rectRight) {
			rectRight = rectLeft; rectLeft = argv[4].toSint16();
		}
		if (rectTop > rectBottom) {
			rectBottom = rectTop; rectTop = argv[3].toSint16();
		}
		rect = Common::Rect (rectLeft, rectTop, rectRight, rectBottom);
	}

	// old code, may be removed later after class migration
	int redraw_port = 0;
	rect_t area;
	area = gfx_rect(argv[2].toSint16(), argv[1].toSint16() , argv[4].toSint16(), argv[3].toSint16());
	area.width = area.width - area.x; // Since the actual coordinates are absolute
	area.height = area.height - area.y;

	switch (argv[0].toSint16()) {

	case K_GRAPH_GET_COLORS_NR:
		return make_reg(0, !s->resMan->isVGA() ? 0x10 : 0x100);
		break;

	case K_GRAPH_DRAW_LINE:
		priority = (argc > 6) ? argv[6].toSint16() : -1;
		control = (argc > 7) ? argv[7].toSint16() : -1;
		color = argv[5].toSint16();

		// FIXME: rect must be changed to 2 Common::Point
		s->gui->graphDrawLine(rect, color, priority, control);
		break;

	case K_GRAPH_SAVE_BOX:
		flags = (argc > 5) ? argv[5].toUint16() : 0;
		return s->gui->graphSaveBox(rect, flags);
		break;

	case K_GRAPH_RESTORE_BOX:
		s->gui->graphRestoreBox(argv[1]);
		break;

	case K_GRAPH_FILL_BOX_BACKGROUND:
		s->gui->graphFillBoxBackground(rect);
		break;

	case K_GRAPH_FILL_BOX_FOREGROUND:
		s->gui->graphFillBoxForeground(rect);
		break;

	case K_GRAPH_FILL_BOX_ANY:
		priority = (argc > 7) ? argv[7].toSint16() : -1;
		control = (argc > 8) ? argv[8].toSint16() : -1;
		color = argv[6].toSint16();
		colorMask = argv[5].toUint16();

		s->gui->graphFillBox(rect, colorMask, color, priority, control);
		break;

	case K_GRAPH_UPDATE_BOX: {

		debugC(2, kDebugLevelGraphics, "update_box(%d, %d, %d, %d)\n", argv[1].toSint16(), argv[2].toSint16(), argv[3].toSint16(), argv[4].toSint16());

		area.x += s->port->zone.x;
		area.y += s->port->zone.y;

		// FIXME: Change to class calling
		//gfxop_update_box(s->gfx_state, area);
	}
	break;

	case K_GRAPH_REDRAW_BOX: {
		debugC(2, kDebugLevelGraphics, "redraw_box(%d, %d, %d, %d)\n", argv[1].toSint16(), argv[2].toSint16(), argv[3].toSint16(), argv[4].toSint16());

		area.x += s->port->zone.x;
		area.y += s->port->zone.y;

		if (s->dyn_views && s->dyn_views->_parent == (GfxContainer *)s->port)
			s->dyn_views->draw(Common::Point(0, 0));

		// FIXME: Change to class calling
		//gfxop_update_box(s->gfx_state, area);
	}
	break;

	case K_GRAPH_ADJUST_PRIORITY:

		debugC(2, kDebugLevelGraphics, "adjust_priority(%d, %d)\n", argv[1].toSint16(), argv[2].toSint16());
		s->priority_first = argv[1].toSint16() - 10;
		s->priority_last = argv[2].toSint16() - 10;
		break;

	default:

		warning("Unhandled Graph() operation %04x", argv[0].toSint16());

	}

	if (redraw_port)
		FULL_REDRAW();

	gfxop_update(s->gfx_state);

	return s->r_acc;
}

reg_t kTextSize(EngineState *s, int argc, reg_t *argv) {
	int16 textWidth, textHeight;
	Common::String text = s->segMan->getString(argv[1]);
	reg_t *dest = s->segMan->derefRegPtr(argv[0], 4);
	int maxwidth = (argc > 3) ? argv[3].toUint16() : 0;
	int font_nr = argv[2].toUint16();

	Common::String sep_str;
	const char *sep = NULL;
	if ((argc > 4) && (argv[4].segment)) {
		sep_str = s->segMan->getString(argv[4]);
		sep = sep_str.c_str();
	}

	dest[0] = dest[1] = NULL_REG;

	if (text.empty() || !dest) { // Empty text
		dest[2] = dest[3] = make_reg(0, 0);
		debugC(2, kDebugLevelStrings, "GetTextSize: Empty string\n");
		return s->r_acc;
	}

	textWidth = dest[3].toUint16(); textHeight = dest[2].toUint16();
	s->gui->textSize(s->strSplit(text.c_str(), sep).c_str(), font_nr, maxwidth, &textWidth, &textHeight);
	debugC(2, kDebugLevelStrings, "GetTextSize '%s' -> %dx%d\n", text.c_str(), textWidth, textHeight);

	dest[2] = make_reg(0, textHeight);
	dest[3] = make_reg(0, textWidth);
	return s->r_acc;
}

reg_t kWait(EngineState *s, int argc, reg_t *argv) {
	int sleep_time = argv[0].toUint16();
#if 0
	uint32 time;

	time = g_system->getMillis();
	s->r_acc = make_reg(0, ((long)time - (long)s->last_wait_time) * 60 / 1000);
	s->last_wait_time = time;

	sleep_time *= g_debug_sleeptime_factor;
	gfxop_sleep(s->gfx_state, sleep_time * 1000 / 60);

	// Reset speed throttler: Game is playing along nicely anyway
	if (sleep_time > 0)
		s->speedThrottler->reset();
#endif

	// FIXME: we should not be asking from the GUI to wait. The kernel sounds
	// like a better place
	s->gui->wait(sleep_time);

	return s->r_acc;
}

reg_t kCoordPri(EngineState *s, int argc, reg_t *argv) {
	int y = argv[0].toSint16();

	return make_reg(0, _find_view_priority(s, y));
}

reg_t kPriCoord(EngineState *s, int argc, reg_t *argv) {
	int priority = argv[0].toSint16();

	return make_reg(0, _find_priority_band(s, priority));
}

void _k_dirloop(reg_t obj, uint16 angle, EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->segMan;
	int view = GET_SEL32V(obj, view);
	int signal = GET_SEL32V(obj, signal);
	int loop;
	int maxloops;
	bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);

	if (signal & _K_VIEW_SIG_FLAG_DOESNT_TURN)
		return;

	angle %= 360;

	if (!oldScriptHeader) {
		if (angle < 45)
			loop = 3;
		else if (angle < 136)
			loop = 0;
		else if (angle < 225)
			loop = 2;
		else if (angle < 316)
			loop = 1;
		else
			loop = 3;
	} else {
		if (angle >= 330 || angle <= 30)
			loop = 3;
		else if (angle <= 150)
			loop = 0;
		else if (angle <= 210)
			loop = 2;
		else if (angle < 330)
			loop = 1;
		else loop = 0xffff;
	}

	maxloops = gfxop_lookup_view_get_loops(s->gfx_state, view);

	if ((loop > 1) && (maxloops < 4))
		return;

	PUT_SEL32V(obj, loop, loop);
}

reg_t kDirLoop(EngineState *s, int argc, reg_t *argv) {
	_k_dirloop(argv[0], argv[1].toUint16(), s, argc, argv);

	return s->r_acc;
}

#define GASEOUS_VIEW_MASK_ACTIVE (_K_VIEW_SIG_FLAG_REMOVE | _K_VIEW_SIG_FLAG_IGNORE_ACTOR)
#define GASEOUS_VIEW_MASK_PASSIVE (_K_VIEW_SIG_FLAG_NO_UPDATE | _K_VIEW_SIG_FLAG_REMOVE | _K_VIEW_SIG_FLAG_IGNORE_ACTOR)

static Common::Rect nsrect_clip(EngineState *s, int y, Common::Rect retval, int priority);

static int collides_with(EngineState *s, Common::Rect area, reg_t other_obj, int use_nsrect, int view_mask, int argc, reg_t *argv) {
	SegManager *segMan = s->segMan;
	int other_signal = GET_SEL32V(other_obj, signal);
	int other_priority = GET_SEL32V(other_obj, priority);
	int y = (int16)GET_SEL32V(other_obj, y);
	Common::Rect other_area;

	if (use_nsrect) {
		other_area = get_nsrect(s, other_obj, 0);
		other_area = nsrect_clip(s, y, other_area, other_priority);
	} else {
		other_area.left = GET_SEL32V(other_obj, brLeft);
		other_area.right = GET_SEL32V(other_obj, brRight);
		other_area.top = GET_SEL32V(other_obj, brTop);
		other_area.bottom = GET_SEL32V(other_obj, brBottom);
	}

	if (other_area.right < 0 || other_area.bottom < 0 || area.right < 0 || area.bottom < 0)
		return 0; // Out of scope

	if (other_area.left >= 320 || other_area.top >= 190 || area.right >= 320 || area.bottom >= 190)
		return 0; // Out of scope

	debugC(2, kDebugLevelBresen, "OtherSignal=%04x, z=%04x obj=%04x:%04x\n", other_signal, (other_signal & view_mask), PRINT_REG(other_obj));

	if ((other_signal & (view_mask)) == 0) {
		// check whether the other object ignores actors

		debugC(2, kDebugLevelBresen, "  against (%d,%d) to (%d,%d)\n", other_area.left, other_area.top, other_area.right, other_area.bottom);

		if (area.intersects(other_area))
			return 1;
		/* CR (from :Bob Heitman:) Collision rects have Mac semantics, ((0,0),(1,1)) only
		** covers the coordinate (0,0) */
	}

	debugC(2, kDebugLevelBresen, " (no)\n");
	return 0;
}

reg_t kCanBeHere(EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->segMan;
	reg_t obj = argv[0];
	reg_t cliplist_ref = (argc > 1) ? argv[1] : NULL_REG;
	List *cliplist = NULL;
	GfxPort *port = s->picture_port;
	uint16 signal;
	int retval;

	Common::Rect abs_zone;
	rect_t zone;
	uint16 edgehit;
	uint16 illegal_bits;

	abs_zone.left = (int16)GET_SEL32V(obj, brLeft);
	abs_zone.right = (int16)GET_SEL32V(obj, brRight);
	abs_zone.top = (int16)GET_SEL32V(obj, brTop);
	abs_zone.bottom = (int16)GET_SEL32V(obj, brBottom);

	zone = gfx_rect(abs_zone.left + port->zone.x, abs_zone.top + port->zone.y, abs_zone.width(), abs_zone.height());

	signal = GET_SEL32V(obj, signal);
	debugC(2, kDebugLevelBresen, "Checking collision: (%d,%d) to (%d,%d) ([%d..%d]x[%d..%d]), obj=%04x:%04x, sig=%04x, cliplist=%04x:%04x\n",
	          GFX_PRINT_RECT(zone), abs_zone.left, abs_zone.right, abs_zone.top, abs_zone.bottom,
	          PRINT_REG(obj), signal, PRINT_REG(cliplist_ref));

	illegal_bits = GET_SEL32V(obj, illegalBits);

	retval = !(illegal_bits & (edgehit = gfxop_scan_bitmask(s->gfx_state, zone, GFX_MASK_CONTROL)));

	debugC(2, kDebugLevelBresen, "edgehit = %04x (illegalBits %04x)\n", edgehit, illegal_bits);
	if (retval == 0) {
		debugC(2, kDebugLevelBresen, " -> %04x\n", retval);
		return NULL_REG; // Can't BeHere
	}

	retval = 0;

	if ((illegal_bits & 0x8000) // If we are vulnerable to those views at all...
	        && s->dyn_views) { // ...check against all stop-updated dynviews
		GfxDynView *widget = (GfxDynView *)s->dyn_views->_contents;

		debugC(2, kDebugLevelBresen, "Checking vs dynviews:\n");

		while (widget) {
			if (widget->_ID && (widget->signal & _K_VIEW_SIG_FLAG_STOPUPD)
			        && ((widget->_ID != obj.segment) || (widget->_subID != obj.offset))
			        && s->segMan->isObject(make_reg(widget->_ID, widget->_subID)))
				if (collides_with(s, abs_zone, make_reg(widget->_ID, widget->_subID), 1, GASEOUS_VIEW_MASK_ACTIVE, argc, argv))
					return NULL_REG;

			widget = (GfxDynView *)widget->_next;
		}
	}

	if (signal & GASEOUS_VIEW_MASK_ACTIVE) {
		retval = signal & GASEOUS_VIEW_MASK_ACTIVE; // CanBeHere- it's either being disposed, or it ignores actors anyway
		debugC(2, kDebugLevelBresen, " -> %04x\n", retval);
		return make_reg(0, retval); // CanBeHere
	}

	if (cliplist_ref.segment)
		cliplist = lookup_list(s, cliplist_ref);

	if (cliplist) {
		Node *node = lookup_node(s, cliplist->first);

		retval = 0; // Assume that we Can'tBeHere...

		while (node) { // Check each object in the list against our bounding rectangle
			reg_t other_obj = node->value;
			debugC(2, kDebugLevelBresen, "  comparing against %04x:%04x\n", PRINT_REG(other_obj));

			if (!s->segMan->isObject(other_obj)) {
				warning("CanBeHere() cliplist contains non-object %04x:%04x", PRINT_REG(other_obj));
			} else if (other_obj != obj) { // Clipping against yourself is not recommended

				if (collides_with(s, abs_zone, other_obj, 0, GASEOUS_VIEW_MASK_PASSIVE, argc, argv)) {
					debugC(2, kDebugLevelBresen, " -> %04x\n", retval);
					return NULL_REG;
				}

			} // if (other_obj != obj)
			node = lookup_node(s, node->succ); // move on
		}
	}

	if (!retval)
		retval = 1;
	debugC(2, kDebugLevelBresen, " -> %04x\n", retval);

	return make_reg(0, retval);
}  // CanBeHere

reg_t kCantBeHere(EngineState *s, int argc, reg_t *argv) {
	// kCantBeHere does the same thing as kCanBeHere, except that
	// it returns the opposite result.
	reg_t result = kCanBeHere(s, argc, argv);
	result.offset = !result.offset;
	return result;
}

reg_t kIsItSkip(EngineState *s, int argc, reg_t *argv) {
	int view = argv[0].toSint16();
	int loop = argv[1].toSint16();
	int cel = argv[2].toSint16();
	int y = argv[3].toUint16();
	int x = argv[4].toUint16();
	gfxr_view_t *res = NULL;
	gfx_pixmap_t *pxm = NULL;

	res = s->gfx_state->gfxResMan->getView(view, &loop, &cel, 0);

	if (!res) {
		warning("[GFX] Attempt to get cel parameters for invalid view %d", view);
		return SIGNAL_REG;
	}

	pxm = res->loops[loop].cels[cel];
	if (x > pxm->index_width)
		x = pxm->index_width - 1;
	if (y > pxm->index_height)
		y = pxm->index_height - 1;

	return make_reg(0, pxm->index_data[y * pxm->index_width + x] == pxm->color_key);
}

reg_t kCelHigh(EngineState *s, int argc, reg_t *argv) {
	int view = argv[0].toSint16();
	int loop = argv[1].toSint16();
	int cel = argv[2].toSint16();
	int height, width;
	Common::Point offset;

	if (argc != 3) {
		warning("CelHigh called with %d parameters", argc);
	}

	gfxop_get_cel_parameters(s->gfx_state, view, loop, cel, &width, &height, &offset);
	return make_reg(0, height);
}

reg_t kCelWide(EngineState *s, int argc, reg_t *argv) {
	int view = argv[0].toSint16();
	int loop = argv[1].toSint16();
	int cel = argv[2].toSint16();
	int height, width;
	Common::Point offset;

	if (argc != 3) {
		warning("CelHigh called with %d parameters", argc);
	}

	gfxop_get_cel_parameters(s->gfx_state, view, loop, cel, &width, &height, &offset);
	return make_reg(0, width);
}

reg_t kNumLoops(EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->segMan;
	reg_t obj = argv[0];
	int view = GET_SEL32V(obj, view);
	int loops_nr = gfxop_lookup_view_get_loops(s->gfx_state, view);

	if (loops_nr < 0) {
		error("view.%d (0x%x) not found", view, view);
		return NULL_REG;
	}

	debugC(2, kDebugLevelGraphics, "NumLoops(view.%d) = %d\n", view, loops_nr);

	return make_reg(0, loops_nr);
}

reg_t kNumCels(EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->segMan;
	reg_t obj = argv[0];
	int loop = GET_SEL32V(obj, loop);
	int view = GET_SEL32V(obj, view);
	int cel = 0xffff;

	gfxop_check_cel(s->gfx_state, view, &loop, &cel);

	debugC(2, kDebugLevelGraphics, "NumCels(view.%d, %d) = %d\n", view, loop, cel + 1);

	return make_reg(0, cel + 1);
}

reg_t kOnControl(EngineState *s, int argc, reg_t *argv) {
	Common::Rect rect;
	byte screenMask;
	int argBase = 0;

	if ((argc == 2) || (argc == 4)) {
		screenMask = GFX_MASK_CONTROL;
	} else {
		screenMask = argv[0].toUint16();
		argBase = 1;
	}
	rect.left = argv[argBase].toSint16();
	rect.top = argv[argBase + 1].toSint16();
	if (argc > 3) {
		rect.right = argv[argBase + 2].toSint16() + 1;
		rect.bottom = argv[argBase + 3].toSint16() + 1;
	} else {
		rect.right = rect.left + 1;
		rect.bottom = rect.top + 1;
	}
	return make_reg(0, s->gui->onControl(screenMask, rect));
}

void _k_view_list_free_backgrounds(EngineState *s, ViewObject *list, int list_nr);

#define K_DRAWPIC_FLAG_MIRRORED (1 << 14)

reg_t kDrawPic(EngineState *s, int argc, reg_t *argv) {
	GUIResourceId pictureId = argv[0].toUint16();
	uint16 flags = 0;
	uint16 style = 1;
	int16 EGApaletteNo = -1;

	if (argc >= 2)
		style = argv[1].toUint16();
	if (argc >= 3) {
		if (!s->_kernel->usesOldGfxFunctions())
			flags = !argv[2].toUint16();
		else
			flags = argv[2].toUint16();
	}
	if (argc >= 4)
		EGApaletteNo = argv[3].toUint16();

	s->gui->drawPicture(pictureId, style, flags, EGApaletteNo);

	return s->r_acc;
}

Common::Rect set_base(EngineState *s, reg_t object) {
	SegManager *segMan = s->segMan;
	int x, y, original_y, z, ystep, xsize, ysize;
	int xbase, ybase, xend, yend;
	int view, loop, cel;
	int oldloop, oldcel;
	int xmod = 0, ymod = 0;
	Common::Rect retval;

	x = (int16)GET_SEL32V(object, x);
	original_y = y = (int16)GET_SEL32V(object, y);

	if (s->_kernel->_selectorCache.z > -1)
		z = (int16)GET_SEL32V(object, z);
	else
		z = 0;

	y -= z; // Subtract z offset

	ystep = (int16)GET_SEL32V(object, yStep);

	view = (int16)GET_SEL32V(object, view);
	oldloop = loop = sign_extend_byte(GET_SEL32V(object, loop));
	oldcel = cel = sign_extend_byte(GET_SEL32V(object, cel));

	Common::Point offset = Common::Point(0, 0);

	if (loop != oldloop) {
		loop = 0;
		PUT_SEL32V(object, loop, 0);
		debugC(2, kDebugLevelGraphics, "Resetting loop for %04x:%04x!\n", PRINT_REG(object));
	}

	if (cel != oldcel) {
		cel = 0;
		PUT_SEL32V(object, cel, 0);
	}

	gfxop_get_cel_parameters(s->gfx_state, view, loop, cel, &xsize, &ysize, &offset);

	xmod = offset.x;
	ymod = offset.y;

	xbase = x - xmod - (xsize >> 1);
	xend = xbase + xsize;
	yend = y /* - ymod */ + 1;
	ybase = yend - ystep;

	debugC(2, kDebugLevelBaseSetter, "(%d,%d)+/-(%d,%d), (%d x %d) -> (%d, %d) to (%d, %d)\n",
	          x, y, xmod, ymod, xsize, ysize, xbase, ybase, xend, yend);

	retval.left = xbase;
	retval.top = ybase;
	retval.right = xend;
	retval.bottom = yend;

	return retval;
}

void _k_base_setter(EngineState *s, reg_t object) {
	SegManager *segMan = s->segMan;
	Common::Rect absrect = set_base(s, object);

	if (lookup_selector(s->segMan, object, s->_kernel->_selectorCache.brLeft, NULL, NULL) != kSelectorVariable)
		return; // non-fatal

	// Note: there was a check here for a very old version of SCI, which supposedly needed
	// to subtract 1 from absrect.top. The original check was for version 0.000.256, which
	// does not exist (earliest one was KQ4 SCI, version 0.000.274). This code is left here
	// for reference only
#if 0
	if (getSciVersion() <= SCI_VERSION_0)
		--absrect.top; // Compensate for early SCI OB1 'bug'
#endif

	PUT_SEL32V(object, brLeft, absrect.left);
	PUT_SEL32V(object, brRight, absrect.right);
	PUT_SEL32V(object, brTop, absrect.top);
	PUT_SEL32V(object, brBottom, absrect.bottom);
}

reg_t kBaseSetter(EngineState *s, int argc, reg_t *argv) {
	reg_t object = argv[0];

	_k_base_setter(s, object);

	return s->r_acc;
} // kBaseSetter

static Common::Rect nsrect_clip(EngineState *s, int y, Common::Rect retval, int priority) {
	int pri_top;

	if (priority == -1)
		priority = _find_view_priority(s, y);

	pri_top = _find_priority_band(s, priority) + 1;
	// +1: Don't know why, but this seems to be happening

	if (retval.top < pri_top)
		retval.top = pri_top;

	if (retval.bottom < retval.top)
		retval.top = retval.bottom - 1;

	return retval;
}

static Common::Rect calculate_nsrect(EngineState *s, int x, int y, int view, int loop, int cel) {
	int xbase, ybase, xend, yend, xsize, ysize;
	int xmod = 0, ymod = 0;
	Common::Rect retval(0, 0, 0, 0);

	Common::Point offset = Common::Point(0, 0);

	gfxop_get_cel_parameters(s->gfx_state, view, loop, cel, &xsize, &ysize, &offset);

	xmod = offset.x;
	ymod = offset.y;

	xbase = x - xmod - (xsize >> 1);
	xend = xbase + xsize;
	yend = y - ymod + 1; // +1: magic modifier
	ybase = yend - ysize;

	retval.left = xbase;
	retval.top = ybase;
	retval.right = xend;
	retval.bottom = yend;

	return retval;
}

Common::Rect get_nsrect(EngineState *s, reg_t object, byte clip) {
	SegManager *segMan = s->segMan;
	int x, y, z;
	int view, loop, cel;
	Common::Rect retval;

	x = (int16)GET_SEL32V(object, x);
	y = (int16)GET_SEL32V(object, y);

	if (s->_kernel->_selectorCache.z > -1)
		z = (int16)GET_SEL32V(object, z);
	else
		z = 0;

	y -= z; // Subtract z offset

	view = (int16)GET_SEL32V(object, view);
	loop = sign_extend_byte((int16)GET_SEL32V(object, loop));
	cel = sign_extend_byte((int16)GET_SEL32V(object, cel));

	retval = calculate_nsrect(s, x, y, view, loop, cel);

	if (clip) {
		int priority = (int16)GET_SEL32V(object, priority);
		return nsrect_clip(s, y, retval, priority);
	}

	return retval;
}

reg_t kSetNowSeen(EngineState *s, int argc, reg_t *argv) {
	s->gui->setNowSeen(argv[0]);
	return s->r_acc;
}

reg_t kPalette(EngineState *s, int argc, reg_t *argv) {
//	warning("kPalette %d", argv[0].toUint16());
	switch (argv[0].toUint16()) {
	case 1:
		if (argc==3) {
			int resourceNo = argv[1].toUint16();
			int flags = argv[2].toUint16();
			s->gui->paletteSet(resourceNo, flags);
		}
		break;
	case 2:
		debug(5, "STUB: kPalette() effect 2, set flag to colors");
		break;
	case 3:
		debug(5, "STUB: kPalette() effect 3, clear flag to colors");
		break;
	case 4:	{	// Set palette intensity
#if 0
		// Colors 0 (black) and 255 (white) cannot be changed
		int16 from = CLIP<int16>(1, 255, argv[2].toUint16());
		int16 to = CLIP<int16>(1, 255, argv[3].toUint16());
		int16 intensity = argv[4].toUint16();

		if (argc < 5 || argv[5].toUint16() == 0) {
			s->gfx_state->gfxResMan->setPaletteIntensity(from, to, intensity);
		} else {
			warning("kPalette: argv[5] != 0");
		}

		return s->r_acc;
#endif
		debug(5, "STUB: kPalette() effect 4, set color intensity");
		break;
		}
	case 5: {	// Find closest color
		int r = argv[1].toUint16();
		int g = argv[2].toUint16();
		int b = argv[3].toUint16();

		return make_reg(0, s->gui->paletteFind(r, g, b));
	}
	case 6:
		if (argc==4) {
			int fromColor = argv[1].toUint16();
			int toColor = argv[2].toUint16();
			int speed = argv[3].toSint16();
			s->gui->paletteAnimate(fromColor, toColor, speed);
		}
		break;
	case 7:
		debug(5, "STUB: kPalette() effect 7, save palette to heap");
		break;
	case 8:
		debug(5, "STUB: kPalette() effect 8, set stored palette");
		break;
	default:
		warning("kPalette(): Unimplemented subfunction: %d", argv[0].toUint16());
	}

	return s->r_acc;
}

static void _k_draw_control(EngineState *s, reg_t obj, bool inverse);

static void disableCertainButtons(SegManager *segMan, Common::String gameName, reg_t obj) {
	reg_t text_pos = GET_SEL32(obj, text);
	Common::String text;
	if (!text_pos.isNull())
		text = segMan->getString(text_pos);
	int type = GET_SEL32V(obj, type);
	int state = GET_SEL32V(obj, state);

	/*
	 * WORKAROUND: The function is a "prevent the user from doing something
	 * nasty" type of thing, and goes back to the ugly way in which savegame
	 * deletion is implemented in SCI (and even worse in SQ4/Floppy, for
	 * which the workaround is intended). The result is basically that you
	 * can't implement savegame deletion for SQ4/Floppy unless you duplicate
	 * the exact naming scheme of savefiles (i.e. savefiles must be named
	 * SQ4SG.<number>) and the exact file format of the savegame index
	 * (SQ4SG.DIR). From the earlier discussions on file I/O handling -
	 * before as well as after the merge - I gather that this is not an
	 * option.
	 *
	 * SQ4/Floppy is special, being the first game to implement savegame
	 * deletion at all. For later games, we manage to implement deletion by
	 * using gross hacks in kDeviceInfo() (essentially repurposing a few
	 * subfunctions). I decided at the time that SQ4/Floppy was not worth the
	 * effort (see above), and to simply disable the delete functionality for
	 * that game - bringing the save/load dialog on a par with SCI0.
	 */
	// NOTE: This _only_ works with the English version
	if (type == K_CONTROL_BUTTON && (gameName == "sq4") &&
			getSciVersion() < SCI_VERSION_1_1 && text == " Delete ") {
		PUT_SEL32V(obj, state, (state | kControlStateDisabled) & ~kControlStateEnabled);
	}

	// Disable the "Change Directory" button, as we don't allow the game engine to
	// change the directory where saved games are placed
	// NOTE: This _only_ works with the English version
	if (type == K_CONTROL_BUTTON && text == "Change\r\nDirectory") {
		PUT_SEL32V(obj, state, (state | kControlStateDisabled) & ~kControlStateEnabled);
	}
}

reg_t kDrawControl(EngineState *s, int argc, reg_t *argv) {
	reg_t obj = argv[0];

	disableCertainButtons(s->segMan, s->_gameName, obj);
	_k_draw_control(s, obj, false);
//	FULL_REDRAW();
	return NULL_REG;
}

reg_t kHiliteControl(EngineState *s, int argc, reg_t *argv) {
	reg_t obj = argv[0];

	_k_draw_control(s, obj, true);

	return s->r_acc;
}

void update_cursor_limits(int *display_offset, int *cursor, int max_displayed) {
	if (*cursor < *display_offset + 4) {
		if (*cursor < 8)
			*display_offset = 0;
		else
			*display_offset = *cursor - 8;
	} else if (*cursor - *display_offset > max_displayed - 8)
		*display_offset = 12 + *cursor - max_displayed;
}

#define _K_EDIT_DELETE \
	if (cursor < textlen) { \
		text.deleteChar(cursor); \
	}

#define _K_EDIT_BACKSPACE \
	if (cursor) { \
		--cursor;    \
		text.deleteChar(cursor); \
		--textlen; \
	}

reg_t kEditControl(EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->segMan;
	reg_t obj = argv[0];
	reg_t event = argv[1];

	if (obj.segment) {
		uint16 ct_type = GET_SEL32V(obj, type);
		switch (ct_type) {

		case 0:
			break; // NOP

		case K_CONTROL_EDIT:
			if (event.segment && ((GET_SEL32V(event, type)) == SCI_EVT_KEYBOARD)) {
				int max_displayed = GET_SEL32V(obj, max);
				int max = max_displayed;
				int cursor = GET_SEL32V(obj, cursor);
				int modifiers = GET_SEL32V(event, modifiers);
				int key = GET_SEL32V(event, message);
				reg_t text_pos = GET_SEL32(obj, text);
				int display_offset = 0;

				Common::String text = s->segMan->getString(text_pos);
				int textlen;

#if 0
				if (!text) {
					warning("Could not draw control: %04x:%04x does not reference text", PRINT_REG(text_pos));
					return s->r_acc;
				}
#endif

				textlen = text.size();

				cursor += display_offset;

				if (cursor > textlen)
					cursor = textlen;

				if (modifiers & SCI_EVM_CTRL) {

					switch (tolower((char)key)) {
					case 'a':
						cursor = 0;
						break;
					case 'e':
						cursor = textlen;
						break;
					case 'f':
						if (cursor < textlen) ++cursor;
						break;
					case 'b':
						if (cursor > 0) --cursor;
						break;
					case 'k':
						text = Common::String(text.c_str(), cursor);
						break; // Terminate string
					case 'h':
						_K_EDIT_BACKSPACE;
						break;
					case 'd':
						_K_EDIT_DELETE;
						break;
					}
					PUT_SEL32V(event, claimed, 1);

				} else if (modifiers & SCI_EVM_ALT) { // Ctrl has precedence over Alt
					switch (key) {
					case 0x2100 /* A-f */:
						while ((cursor < textlen) && (text[cursor++] != ' '))
							;
						break;
					case 0x3000 /* A-b */:
						while ((cursor > 0) && (text[--cursor - 1] != ' '))
							;
						break;
					case 0x2000 /* A-d */: {
						while ((cursor < textlen) && (text[cursor] == ' ')) {
							_K_EDIT_DELETE;
							textlen--;
						}
						while ((cursor < textlen) && (text[cursor] != ' ')) {
							_K_EDIT_DELETE;
							textlen--;
						}
						break;
					}
					}
					PUT_SEL32V(event, claimed, 1);
				} else if (key < 31) {
					PUT_SEL32V(event, claimed, 1);
					switch (key) {
					case SCI_K_BACKSPACE:
						_K_EDIT_BACKSPACE;
						break;
					default:
						PUT_SEL32V(event, claimed, 0);
					}
				} else if (key & 0xff00) {
					switch (key) {
					case SCI_K_HOME:
						cursor = 0;
						break;
					case SCI_K_END:
						cursor = textlen;
						break;
					case SCI_K_RIGHT:
						if (cursor + 1 <= textlen)
							++cursor;
						break;
					case SCI_K_LEFT:
						if (cursor > 0)
							--cursor;
						break;
					case SCI_K_DELETE:
						_K_EDIT_DELETE;
						break;
					}
					PUT_SEL32V(event, claimed, 1);
				} else if ((key > 31) && (key < 128)) {
					int inserting = (modifiers & SCI_EVM_INSERT);

					modifiers &= ~(SCI_EVM_RSHIFT | SCI_EVM_LSHIFT | SCI_EVM_CAPSLOCK);

					if (cursor == textlen) {
						if (textlen < max) {
							text += key;
							cursor++;
						}
					} else if (inserting) {
						if (textlen < max) {
							int i;

							for (i = textlen + 2; i >= cursor; i--)
								text.setChar(text[i - 1], i);
							text.setChar(key, cursor++);

						}
					} else { // Overwriting
						text.setChar(key, cursor++);
					}

					if (max_displayed < max)
						update_cursor_limits(&display_offset, &cursor, max_displayed);

					cursor -= display_offset;

					PUT_SEL32V(event, claimed, 1);
				}

				PUT_SEL32V(obj, cursor, cursor); // Write back cursor position
				s->segMan->strcpy(text_pos, text.c_str()); // Write back string
			}
			if (event.segment) PUT_SEL32V(event, claimed, 1);
			_k_draw_control(s, obj, false);
			return NULL_REG;

		case K_CONTROL_ICON:
		case K_CONTROL_BOX:
		case K_CONTROL_BUTTON:
			return NULL_REG;

		case K_CONTROL_TEXT: {
			int state = GET_SEL32V(obj, state);
			PUT_SEL32V(obj, state, state | kControlStateDitherFramed);
			_k_draw_control(s, obj, false);
			PUT_SEL32V(obj, state, state);
		}
		break;

		default:
			warning("Attempt to edit control type %d", ct_type);
		}
	}

	return s->r_acc;
}

static void _k_draw_control(EngineState *s, reg_t obj, bool inverse) {
	SegManager *segMan = s->segMan;
	int x = (int16)GET_SEL32V(obj, nsLeft);
	int y = (int16)GET_SEL32V(obj, nsTop);
	int xl = (int16)GET_SEL32V(obj, nsRight) - x;
	int yl = (int16)GET_SEL32V(obj, nsBottom) - y;
	rect_t area = gfx_rect(x, y, xl, yl);

	Common::Rect rect;
	rect = Common::Rect (x, y, (int16)GET_SEL32V(obj, nsRight) + 1, (int16)GET_SEL32V(obj, nsBottom) + 1);

	int font_nr = GET_SEL32V(obj, font);
	reg_t text_pos = GET_SEL32(obj, text);
	Common::String text;
	if (!text_pos.isNull())
		text = s->segMan->getString(text_pos);
	int view = GET_SEL32V(obj, view);
	int cel = sign_extend_byte(GET_SEL32V(obj, cel));
	int loop = sign_extend_byte(GET_SEL32V(obj, loop));
	int mode;

	int type = GET_SEL32V(obj, type);
	int state = GET_SEL32V(obj, state);
	int cursor;
	int max;

	switch (type) {
	case K_CONTROL_BUTTON:
		debugC(2, kDebugLevelGraphics, "drawing button %04x:%04x to %d,%d\n", PRINT_REG(obj), x, y);
		s->gui->drawControlButton(rect, obj, s->strSplit(text.c_str(), NULL).c_str(), font_nr, state, inverse);
		return;

	case K_CONTROL_TEXT:
		mode = (gfx_alignment_t) GET_SEL32V(obj, mode);
		debugC(2, kDebugLevelGraphics, "drawing text %04x:%04x ('%s') to %d,%d, mode=%d\n", PRINT_REG(obj), text.c_str(), x, y, mode);
		s->gui->drawControlText(rect, obj, s->strSplit(text.c_str(), NULL).c_str(), font_nr, mode, state, inverse);
		return;

	case K_CONTROL_EDIT:
		debugC(2, kDebugLevelGraphics, "drawing edit control %04x:%04x (text %04x:%04x, '%s') to %d,%d\n", PRINT_REG(obj), PRINT_REG(text_pos), text.c_str(), x, y);

		max = GET_SEL32V(obj, max);
		cursor = GET_SEL32V(obj, cursor);

		if (cursor > (signed)text.size())
			cursor = text.size();

//		update_cursor_limits(&s->save_dir_edit_offset, &cursor, max);	FIXME: get rid of this?
		ADD_TO_CURRENT_PICTURE_PORT(sciw_new_edit_control(s->port, obj, area, text.c_str(), font_nr, (unsigned)cursor, (int8)inverse));
		break;

	case K_CONTROL_ICON:

		debugC(2, kDebugLevelGraphics, "drawing icon control %04x:%04x to %d,%d\n", PRINT_REG(obj), x, y - 1);

		ADD_TO_CURRENT_PICTURE_PORT(sciw_new_icon_control(s->port, obj, area, view, loop, cel,
		                          (int8)(state & kControlStateFramed), (int8)inverse));
		break;

	case K_CONTROL_CONTROL:
	case K_CONTROL_CONTROL_ALIAS: {
		int entries_nr;
		int lsTop;
		int list_top = 0;
		int selection = 0;
		int entry_size = GET_SEL32V(obj, x);
		int i;

		if (s->_kernel->_selectorCache.topString != -1) {
			// Games from early SCI1 onwards use topString
			lsTop = GET_SEL32V(obj, topString);
		} else {
			// Earlier games use lsTop
			lsTop = GET_SEL32V(obj, lsTop);
		}
		lsTop -= text_pos.offset;

		debugC(2, kDebugLevelGraphics, "drawing list control %04x:%04x to %d,%d, diff %d\n", PRINT_REG(obj), x, y, SCI_MAX_SAVENAME_LENGTH);
		cursor = GET_SEL32V(obj, cursor) - text_pos.offset;

		entries_nr = 0;

		// NOTE: most types of pointer dereferencing don't like odd offsets
		if (entry_size & 1) {
			warning("List control with odd entry_size %d. This is not yet implemented for all types of segments", entry_size);
		}

		reg_t seeker = text_pos;
		// Count string entries in NULL terminated string list
		while (s->segMan->strlen(seeker) > 0) {
			++entries_nr;
			seeker.offset += entry_size;
		}

		// TODO: This is rather convoluted... It would be a lot cleaner
		// if sciw_new_list_control would take a list of Common::String
		Common::String *strings = 0;
		const char **entries_list = NULL;

		if (entries_nr) { // determine list_top, selection, and the entries_list
			seeker = text_pos;
			entries_list = (const char**)malloc(sizeof(char *) * entries_nr);
			strings = new Common::String[entries_nr];
			for (i = 0; i < entries_nr; i++) {
				strings[i] = s->segMan->getString(seeker);
				entries_list[i] = strings[i].c_str();
				seeker.offset += entry_size;
				if ((seeker.offset - text_pos.offset) == lsTop)
					list_top = i + 1;
				if ((seeker.offset - text_pos.offset) == cursor)
					selection = i + 1;
			}
		}

		ADD_TO_CURRENT_PICTURE_PORT(sciw_new_list_control(s->port, obj, area, font_nr, entries_list, entries_nr,
		                          list_top, selection, (int8)inverse));
		free(entries_list);
		delete[] strings;
	}
	break;

	case K_CONTROL_BOX:
		break;

	default:
		warning("Unknown control type: %d at %04x:%04x, at (%d, %d) size %d x %d",
		         type, PRINT_REG(obj), x, y, xl, yl);
	}

	if (!s->pic_not_valid) {
		FULL_REDRAW();
	}
}

void _k_view_list_mark_free(EngineState *s, reg_t off) {
	if (s->dyn_views) {

		GfxDynView *w = (GfxDynView *)s->dyn_views->_contents;

		while (w) {
			if (w->_ID == off.segment
			        && w->_subID == off.offset) {
				w->under_bitsp.obj = NULL_REG;
			}

			w = (GfxDynView *)w->_next;
		}
	}
}

reg_t kAddToPic(EngineState *s, int argc, reg_t *argv) {
	GUIResourceId viewId;
	GUIViewLoopNo loopNo;
	GUIViewCellNo cellNo;
	int16 leftPos, topPos, priority, control;

	switch (argc) {
	case 0:
		break;
	case 1:
		s->gui->addToPicList(argv[0], argc, argv);
		break;
	case 7:
		viewId = argv[0].toUint16();
		loopNo = argv[1].toSint16();
		cellNo = argv[2].toSint16();
		leftPos = argv[3].toSint16();
		topPos = argv[4].toSint16();
		priority = argv[5].toSint16();
		control = argv[6].toSint16();
		s->gui->addToPicView(viewId, loopNo, cellNo, leftPos, topPos, priority, control);
		break;
	default:
		error("kAddToPic with unsupported parameter count %d", argc);		
	}
	return s->r_acc;
}

reg_t kGetPort(EngineState *s, int argc, reg_t *argv) {
	return s->gui->getPort();
}

reg_t kSetPort(EngineState *s, int argc, reg_t *argv) {
	uint16 portPtr;
	Common::Rect picRect;
	int16 picTop, picLeft;
	
	switch (argc) {
		case 1:
		portPtr = argv[0].toSint16();
		s->gui->setPort(portPtr);
		break;

		case 6:
		picRect.top = argv[0].toSint16();
		picRect.left = argv[1].toSint16();
		picRect.bottom = argv[2].toSint16() + 1;
		picRect.right = argv[3].toSint16() + 1;
		picTop = argv[4].toSint16();
		picLeft = argv[5].toSint16();
		s->gui->setPortPic(picRect, picTop, picLeft);
		break;

		default:
		error("SetPort was called with %d parameters", argc);
		break;
	}
	return NULL_REG;
}

reg_t kDrawCel(EngineState *s, int argc, reg_t *argv) {
	GUIResourceId viewId = argv[0].toSint16();
	GUIViewLoopNo loopNo = argv[1].toSint16();
	GUIViewCellNo cellNo = argv[2].toSint16();
	int x = argv[3].toSint16();
	int y = argv[4].toSint16();
	int priority = (argc > 5) ? argv[5].toUint16()  : -1;
	int paletteNo = (argc > 6) ? argv[6].toSint16() : 0;

	s->gui->drawCell(viewId, loopNo, cellNo, x, y, priority, paletteNo);

	return s->r_acc;
}

reg_t kDisposeWindow(EngineState *s, int argc, reg_t *argv) {
	int goner_nr = argv[0].toSint16();
	int arg2 = (argc != 2 || argv[2].toUint16() == 0 ? 0 : 1);

	s->gui->disposeWindow(goner_nr, arg2);
	return s->r_acc;
}

reg_t kNewWindow(EngineState *s, int argc, reg_t *argv) {
	Common::Rect rect1 (argv[1].toSint16(), argv[0].toSint16(), argv[3].toSint16() + 1, argv[2].toSint16() + 1);
	Common::Rect rect2;
	int argextra = argc == 13 ? 4 : 0; // Triggers in PQ3 and SCI1.1 games
	int	style = argv[5 + argextra].toSint16();
	int	priority = (argc > 6 + argextra) ? argv[6 + argextra].toSint16() : -1;
	int colorPen = (argc > 7 + argextra) ? argv[7 + argextra].toSint16() : 0;
	int colorBack = (argc > 8 + argextra) ? argv[8 + argextra].toSint16() : 255;

	//	const char *title = argv[4 + argextra].segment ? kernel_dereference_char_pointer(s, argv[4 + argextra], 0) : NULL;
	if (argc==13) {
		rect2 = Common::Rect (argv[5].toSint16(), argv[4].toSint16(), argv[7].toSint16() + 1, argv[6].toSint16() + 1);
	}

	Common::String title;
	if (argv[4 + argextra].segment) {
		title = s->segMan->getString(argv[4 + argextra]);
		title = s->strSplit(title.c_str(), NULL);
	}

	return s->gui->newWindow(rect1, rect2, style, priority, colorPen, colorBack, title.c_str());
}

reg_t kAnimate(EngineState *s, int argc, reg_t *argv) {
	reg_t castListReference = (argc > 0) ? argv[0] : NULL_REG;
	bool cycle = (argc > 1) ? ((argv[1].toUint16()) ? true : false) : false;

	s->gui->animate(castListReference, cycle, argc, argv);
	return s->r_acc;
}

#define SHAKE_DOWN 1
#define SHAKE_RIGHT 2

reg_t kShakeScreen(EngineState *s, int argc, reg_t *argv) {
	int shakes = (argc > 0) ? argv[0].toSint16() : 1;
	int directions = (argc > 1) ? argv[1].toSint16() : 1;
	gfx_pixmap_t *screen = gfxop_grab_pixmap(s->gfx_state, gfx_rect(0, 0, 320, 200));
	int i;

	if (directions & ~3)
		debugC(2, kDebugLevelGraphics, "ShakeScreen(): Direction bits are %x (unknown)\n", directions);

	gfxop_set_clip_zone(s->gfx_state, gfx_rect_fullscreen);

	for (i = 0; i < shakes; i++) {
		int shake_down = (directions & SHAKE_DOWN) ? 10 : 0;
		int shake_right = (directions & SHAKE_RIGHT) ? 10 : 0;

		if (directions & SHAKE_DOWN)
			gfxop_draw_box(s->gfx_state, gfx_rect(0, 0, 320, 10), s->ega_colors[0], s->ega_colors[0], GFX_BOX_SHADE_FLAT);

		if (directions & SHAKE_RIGHT)
			gfxop_draw_box(s->gfx_state, gfx_rect(0, 0, 10, 200), s->ega_colors[0], s->ega_colors[0], GFX_BOX_SHADE_FLAT);

		gfxop_draw_pixmap(s->gfx_state, screen, gfx_rect(0, 0, 320 - shake_right, 200 - shake_down),
		                  Common::Point(shake_right, shake_down));

		gfxop_update(s->gfx_state);
		gfxop_sleep(s->gfx_state, 50);

		gfxop_draw_pixmap(s->gfx_state, screen, gfx_rect(0, 0, 320, 200), Common::Point(0, 0));
		gfxop_update(s->gfx_state);
		gfxop_sleep(s->gfx_state, 50);
	}

	gfxop_free_pixmap(s->gfx_state, screen);
	gfxop_update(s->gfx_state);
	return s->r_acc;
}

reg_t kDisplay(EngineState *s, int argc, reg_t *argv) {
	reg_t textp = argv[0];
	int index = (argc > 1) ? argv[1].toUint16() : 0;

	Common::String text;

	if (textp.segment) {
		argc--; argv++;
		text = s->segMan->getString(textp);
	} else {
		argc--; argc--; argv++; argv++;
		text = kernel_lookup_text(s, textp, index);
	}

	s->gui->display(s->strSplit(text.c_str()).c_str(), argc, argv);
	return s->r_acc;
}

static reg_t kShowMovie_Windows(EngineState *s, int argc, reg_t *argv) {
	Common::String filename = s->segMan->getString(argv[1]);
	
	Graphics::AVIPlayer *player = new Graphics::AVIPlayer(g_system);
	
	if (!player->open(filename)) {
		warning("Failed to open movie file %s", filename.c_str());
		return s->r_acc;
	}
	
	uint32 startTime = g_system->getMillis();
	bool play = true;
	
	while (play && player->getCurFrame() < player->getFrameCount()) {
		uint32 elapsed = g_system->getMillis() - startTime;
		
		if (elapsed >= player->getCurFrame() * 1000 / player->getFrameRate()) {
			Graphics::Surface *surface = player->getNextFrame();

			Palette *palette = NULL;
			
			if (player->dirtyPalette()) {
				byte *rawPalette = player->getPalette();
				Palette *colors = new Palette(256);

				byte r, g, b;

				for (uint16 i = 0; i < 256; i++) {
					r = rawPalette[i * 4];
					g = rawPalette[i * 4 + 1];
					b = rawPalette[i * 4 + 2];
					colors->setColor(i, r, g, b);
				}

				palette->forceInto(s->gfx_state->driver->getMode()->palette);
			}
				
			if (surface) {
				// Allocate a pixmap
				gfx_pixmap_t *pixmap = gfx_new_pixmap(surface->w, surface->h, 0, 0, 0);
				assert(pixmap);
				gfx_pixmap_alloc_index_data(pixmap);
	
				// Copy data from the surface
				memcpy(pixmap->index_data, surface->pixels, surface->w * surface->h);
				pixmap->xoffset = (g_system->getWidth() - surface->w) / 2;
				pixmap->yoffset = (g_system->getHeight() - surface->h) / 2;
				pixmap->palette = palette;
				
				// Copy the frame to the screen
				gfx_xlate_pixmap(pixmap, s->gfx_state->driver->getMode());
				gfxop_draw_pixmap(s->gfx_state, pixmap, gfx_rect(0, 0, 320, 200), Common::Point(pixmap->xoffset, pixmap->yoffset));
				gfxop_update_box(s->gfx_state, gfx_rect(0, 0, 320, 200));
				gfx_free_pixmap(pixmap);
				
				// Surface is freed when the codec in the video is deleted
			}
		}
		
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
				case Common::EVENT_QUIT:
					play = false;
					quit_vm();
					break;
				default:
					break;
			}
		}
		
		g_system->delayMillis(10);
	}

	delete player;

	return s->r_acc;
}

static reg_t kShowMovie_DOS(EngineState *s, int argc, reg_t *argv) {
	Common::String filename = s->segMan->getString(argv[0]);
	int delay = argv[1].toUint16(); // Time between frames in ticks
	int frameNr = 0;
	SeqDecoder seq;

	if (!seq.loadFile(filename) && !seq.loadFile(Common::String("SEQ/") + filename)) {
		warning("Failed to open movie file %s", filename.c_str());
		return s->r_acc;
	}

	bool play = true;
	while (play) {
		uint32 startTime = g_system->getMillis();

		gfx_pixmap_t *pixmap = seq.getFrame(play);

		if (frameNr++ == 0)
			pixmap->palette->forceInto(s->gfx_state->driver->getMode()->palette);

		gfx_xlate_pixmap(pixmap, s->gfx_state->driver->getMode());
		gfxop_draw_pixmap(s->gfx_state, pixmap, gfx_rect(0, 0, 320, 200), Common::Point(pixmap->xoffset, pixmap->yoffset));
		gfxop_update_box(s->gfx_state, gfx_rect(0, 0, 320, 200));
		gfx_free_pixmap(pixmap);

		// Wait before showing the next frame
		while (play && (g_system->getMillis() < startTime + (delay * 1000 / 60))) {
			// FIXME: we should probably make a function that handles quitting in these kinds of situations
			Common::Event curEvent;
			Common::EventManager *eventMan = g_system->getEventManager();

			// Process quit events
			while (eventMan->pollEvent(curEvent)) {
				if (curEvent.type == Common::EVENT_QUIT) {
					play = false;
					quit_vm();
				}
			}

			g_system->delayMillis(10);
		}
	}

	return s->r_acc;
}

reg_t kShowMovie(EngineState *s, int argc, reg_t *argv) {
	// KQ6 Windows calls this with one argument. It doesn't seem
	// to have a purpose...
	if (argc == 1)
		return NULL_REG;

	// The Windows and DOS versions use different video format as well
	// as a different argument set.
	if (argv[0].toUint16() == 0)
		return kShowMovie_Windows(s, argc, argv);

	return kShowMovie_DOS(s, argc, argv);
}

reg_t kSetVideoMode(EngineState *s, int argc, reg_t *argv) {
	// This call is used for KQ6's intro. It has one parameter, which is
	// 1 when the intro begins, and 0 when it ends. It is suspected that
	// this is actually a flag to enable video planar memory access, as
	// the video decoder in KQ6 is specifically written for the planar
	// memory model. Planar memory mode access was used for VGA "Mode X"
	// (320x240 resolution, although the intro in KQ6 is 320x200).
	// Refer to http://en.wikipedia.org/wiki/Mode_X

	warning("STUB: SetVideoMode %d", argv[0].toUint16());

	return s->r_acc;
}

// New calls for SCI11. Using those is only needed when using text-codes so that one is able to change
//  font and/or color multiple times during kDisplay and kDrawControl
reg_t kTextFonts(EngineState *s, int argc, reg_t *argv) {
	s->gui->textFonts(argc, argv);
	return s->r_acc;
}

reg_t kTextColors(EngineState *s, int argc, reg_t *argv) {
	s->gui->textColors(argc, argv);
	return s->r_acc;
}

} // End of namespace Sci
