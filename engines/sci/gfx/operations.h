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

/* Graphical operations, called from the widget state manager */

#ifndef SCI_GFX_GFX_OPERATIONS_H
#define SCI_GFX_GFX_OPERATIONS_H

#include "sci/gfx/gfx_resmgr.h"
#include "sci/gfx/gfx_tools.h"
#include "sci/gfx/gfx_options.h"
#include "sci/gfx/gfx_system.h"
#include "sci/uinput.h"

#include "common/list.h"

namespace Sci {

struct text_fragment_t;

#define GFXOP_NO_POINTER -1

/* Threshold in color index mode to differentiate between visible and non-visible stuff.
** GFXOP_ALPHA_THRESHOLD itself should be treated as non-visible.
*/
#define GFXOP_ALPHA_THRESHOLD 0xff

struct gfx_text_handle_t {
	char *text; /* Copy of the actual text */

	int lines_nr;
	int line_height;
	text_fragment_t *lines; /* Text offsets */
	gfx_bitmap_font_t *font;
	gfx_pixmap_t **text_pixmaps;

	int width, height;

	int priority, control;
	gfx_alignment_t halign, valign;
};

/* Unless individually stated otherwise, the following applies:
** All operations herein apply to the standard 320x200 coordinate system.
** All operations perform clipping relative to state->clip_zone.
*/

enum gfx_box_shade_t {
	GFX_BOX_SHADE_FLAT,
	GFX_BOX_SHADE_RIGHT,
	GFX_BOX_SHADE_LEFT,
	GFX_BOX_SHADE_DOWN,
	GFX_BOX_SHADE_UP
#if 0
	/* possible with alphaing, but there is no way to check for
	** alpha capability of gfx_driver->draw_filled_rect() yet
	*/
	, GFX_BOX_SHADE_RIGHT_DOWN,
	GFX_BOX_SHADE_LEFT_DOWN,
	GFX_BOX_SHADE_RIGHT_UP,
	GFX_BOX_SHADE_LEFT_UP
#endif
};


typedef Common::List<rect_t> DirtyRectList;


struct GfxState {
	gfx_options_t *options;

	Common::Point pointer_pos; /* Mouse pointer coordinates */

	rect_t clip_zone_unscaled; /* The current UNSCALED clipping zone */
	rect_t clip_zone; /* The current SCALED clipping zone; a cached scaled version of clip_zone_unscaled */

	gfx_driver_t *driver;

	int visible_map;

	GfxResManager *gfxResMan;

	gfx_pixmap_t *priority_map; /* back buffer priority map (unscaled) */
	gfx_pixmap_t *static_priority_map; /* static buffer priority map (unscaled) */
	gfx_pixmap_t *control_map; /* back buffer control map (only exists unscaled in the first place) */


	int tag_mode; /* Set to 1 after a new pic is drawn and the resource manager
		      ** has tagged all resources. Reset after the next front buffer
		      ** update is done, when all resources that are still tagged are
		      ** flushed.  */

	int disable_dirty; /* Set to 1 to disable dirty rect accounting */

	int pic_nr; /* Number of the current pic */
	int palette_nr; /* Palette number of the current pic */

	Common::List<sci_event_t> _events;

	gfx_pixmap_t *fullscreen_override; /* An optional override picture which must have unscaled
					   ** full-screen size, which overrides all other visibility, and
					   ** which is generally slow */

	gfxr_pic_t *pic, *pic_unscaled; /* The background picture and its unscaled equivalent */
	rect_t pic_port_bounds;  /* Picture port bounds */

	DirtyRectList _dirtyRects; /* Dirty rectangles */
};


/**************************/
/* Fundamental operations */
/**************************/

int gfxop_init(int version, GfxState *state, gfx_options_t *options, ResourceManager *resManager,
			   int xfact = 1, int yfact = 1, gfx_color_mode_t bpp = GFX_COLOR_MODE_INDEX);
/* Initializes a graphics mode
** Parameters: (int) version: The interpreter version
**             (GfxState *) state: The state to initialize
**             (int x int) xfact, yfact: Horizontal and vertical scale factors
**             (gfx_color_mode_t) bpp: Bytes per pixel to initialize with, or
**                                     0 (GFX_COLOR_MODE_AUTO) to auto-detect
**             (gfx_options_t *) options: Rendering options
**             (void *) misc_info: Additional information for the interpreter
**                      part of the resource loader
** Returns   : (int) GFX_OK on success, GFX_ERROR if that particular mode is
**                   unavailable, or GFX_FATAL if the graphics driver is unable
**                   to provide any useful graphics support
*/

int gfxop_set_parameter(GfxState *state, char *attribute, char *value);
/* Sets a driver-specific parameter
** Parameters: (GfxState *) state: The state, encapsulating the driver object to manipulate
**             (char *) attribute: The attribute to set
**             (char *) value: The value the attribute should be set to
** Returns   : (int) GFX_OK on success, GFX_FATAL on fatal error conditions triggered
**                   by the command
*/

int gfxop_exit(GfxState *state);
/* Deinitializes a currently active driver
** Parameters: (GfxState *) state: The state encapsulating the driver in question
** Returns   : (int) GFX_OK
*/

int gfxop_scan_bitmask(GfxState *state, rect_t area, gfx_map_mask_t map);
/* Calculates a bit mask calculated from some pixels on the specified map
** Parameters: (GfxState *) state: The state containing the pixels to scan
**             (rect_t) area: The area to check
**             (gfx_map_mask_t) map: The GFX_MASKed map(s) to test
** Returns   : (int) An integer value where, for each 0<=i<=15, bit #i is set
**             iff there exists a map for which the corresponding bit was set
**             in the 'map' parameter and for which there exists a pixel within
**             the specified area so that the pixel's lower 4 bits, interpreted
**             as an integer value, equal i.
** (Short version: This is an implementation of "on_control()").
*/

int gfxop_set_visible_map(GfxState *state, gfx_map_mask_t map);
/* Sets the currently visible map
** Parameters: (GfxState *) state: The state to modify
**             (gfx_map_mask_t) map: The GFX_MASK to set
** Returns   : (int) GFX_OK, or GFX_ERROR if map was invalid
** 'visible_map' can be any of GFX_MASK_VISUAL, GFX_MASK_PRIORITY and GFX_MASK_CONTROL; the appropriate
** map (as far as its contents are known to the graphics subsystem) is then subsequently drawn to the
** screen at each update. If this is set to anything other than GFX_MASK_VISUAL, slow full-screen updates
** are performed. Mostly useful for debugging.
** The screen needs to be updated for the changes to take effect.
*/

int gfxop_set_clip_zone(GfxState *state, rect_t zone);
/* Sets a new clipping zone
** Parameters: (GfxState *) state: The affected state
**             (rect_t) zone: The new clipping zone
** Returns   : (int) GFX_OK
*/

/******************************/
/* Generic drawing operations */
/******************************/

int gfxop_draw_line(GfxState *state,
	Common::Point start, Common::Point end, gfx_color_t color,
	gfx_line_mode_t line_mode, gfx_line_style_t line_style);
/* Renders a clipped line to the back buffer
** Parameters: (GfxState *) state: The state affected
**             (Common::Point) start: Starting point of the line
**	       (Common::Point) end: End point of the line
**             (gfx_color_t) color: The color to use for drawing
**             (gfx_line_mode_t) line_mode: Any valid line mode to use
**             (gfx_line_style_t) line_style: The line style to use
** Returns   : (int) GFX_OK or GFX_FATAL
*/

int gfxop_draw_rectangle(GfxState *state, rect_t rect, gfx_color_t color, gfx_line_mode_t line_mode,
	gfx_line_style_t line_style);
/* Draws a non-filled rectangular box to the back buffer
** Parameters: (GfxState *) state: The affected state
**             (rect_t) rect: The rectangular area the box is drawn to
**             (gfx_color_t) color: The color the box is to be drawn in
**             (gfx_line_mode_t) line_mode: The line mode to use
**             (gfx_line_style_t) line_style: The line style to use for the box
** Returns   : (int) GFX_OK or GFX_FATAL
** Boxes drawn in thin lines will surround the minimal area described by rect.
*/

int gfxop_draw_box(GfxState *state, rect_t box, gfx_color_t color1, gfx_color_t color2,
	gfx_box_shade_t shade_type);
/* Draws a filled box to the back buffer
** Parameters: (GfxState *) state: The affected state
**             (rect_t) box: The area to draw to
**             (gfx_color_t) color1: The primary color to use for drawing
**             (gfx_color_t) color2: The secondary color to draw in
**             (gfx_box_shade_t) shade_type: The shading system to use
**                               (e.g. GFX_BOX_SHADE_FLAT)
** Returns   : (int) GFX_OK or GFX_FATAL
** The draw mask, control, and priority values are derived from color1.
*/

int gfxop_fill_box(GfxState *state, rect_t box, gfx_color_t color);
/* Fills a box in the back buffer with a specific color
** Parameters: (GfxState *) state: The state to draw to
**             (rect_t) box: The box to fill
**             (gfx_color_t) color: The color to use for filling
** Returns   : (int) GFX_OK or GFX_FATAL
** This is a simple wrapper function for gfxop_draw_box
*/

int gfxop_clear_box(GfxState *state, rect_t box);
/* Copies a box from the static buffer to the back buffer
** Parameters: (GfxState *) state: The affected state
**             (rect_t) box: The box to propagate from the static buffer
** Returns   : (int) GFX_OK or GFX_FATAL
*/


int gfxop_update(GfxState *state);
/* Updates all dirty rectangles
** Parameters: (GfxState) *state: The relevant state
** Returns   : (int) GFX_OK or GFX_FATAL if reported by the driver
** In order to track dirty rectangles, they must be enabled in the options.
** This function instructs the resource manager to free all tagged data
** on certain occasions (see gfxop_new_pic).
*/


int gfxop_update_box(GfxState *state, rect_t box);
/* Propagates a box from the back buffer to the front (visible) buffer
** Parameters: (GfxState *) state: The affected state
**             (rect_t) box: The box to propagate to the front buffer
** Returns   : (int) GFX_OK or GFX_FATAL
** This function instructs the resource manager to free all tagged data
** on certain occasions (see gfxop_new_pic).
** When called with dirty rectangle management enabled, it will automatically
** propagate all dirty rectangles as well, UNLESS dirty frame accounting has
** been disabled explicitly.
*/

int gfxop_enable_dirty_frames(GfxState *state);
/* Enables dirty frame accounting
** Parameters: (GfxState *) state: The state dirty frame accounting is to be enabled in
** Returns   : (int) GFX_OK or GFX_ERROR if state was invalid
** Dirty frame accounting is enabled by default.
*/

int gfxop_disable_dirty_frames(GfxState *state);
/* Disables dirty frame accounting
** Parameters: (GfxState *) state: The state dirty frame accounting is to be disabled in
** Returns   : (int) GFX_OK or GFX_ERROR if state was invalid
*/


/********************/
/* Color operations */
/********************/

int gfxop_set_color(GfxState *state, gfx_color_t *color, int r, int g, int b, int a,
	int priority, int control);
/* Maps an r/g/b value to a color and sets a gfx_color_t structure
** Parameters: (GfxState *) state: The current state
**             (gfx_color_t *) color: Pointer to the structure to write to
**             (int x int x int) r,g,b: The red/green/blue color intensity values
**                               of the result color (0x00 (minimum) to 0xff (max))
**                               If any of these values is less than zero, the
**                               resulting color will not affect the visual map when
**                               used for drawing
**             (int) a: The alpha (transparency) value, with 0x00 meaning absolutely
**                      opaque and 0xff meaning fully transparent. Alpha blending support
**                      is optional for drivers, so these are the only two values that
**                      are guaranteed to work as intended. Any value in between them
**                      must guarantee the following opaqueness:
**                      opaqueness(x-1) >= opaqueness(x) >= opaqueness (x+1)
**                      (i.e. ([0,255], less-transparent-than) must define a partial order)
**             (int) priority: The priority to use for drawing, or -1 for none
**             (int) control: The control to use for drawing, or -1 to disable drawing to the
**                            control map
** Returns   : (int) GFX_OK or GFX_ERROR if state is invalid
** In palette mode, this may allocate a new color. Use gfxop_free_color() described below to
** free that color.
*/

int gfxop_set_system_color(GfxState *state, unsigned int index, gfx_color_t *color);
/* Designates a color as a 'system color'
** Parameters: (GfxState *) state: The affected state
**             (unsigned int) index: The index for the new system color
**             (gfx_color_t *) color: The color to designate as a system color
** Returns   : (int) GFX_OK or GFX_ERROR if state is invalid
** System colors are permanent colors that cannot be deallocated. As such, they must be used
** with caution.
*/

int gfxop_free_color(GfxState *state, gfx_color_t *color);
/* Frees a color allocated by gfxop_set_color()
** Parmaeters: (GfxState *) state: The state affected
**             (gfx_color_t *) color: The color to de-allocate
** Returns   : (int) GFX_OK or GFX_ERROR if state is invalid
** This function is a no-op in non-index mode, or if color is a system color.
*/


/**********************/
/* Pointer and IO ops */
/**********************/

int gfxop_sleep(GfxState *state, uint32 msecs);
/* Suspends program execution for the specified amount of milliseconds
** Parameters: (GfxState *) state: The state affected
**             (uint32) usecs: The amount of milliseconds to wait
** Returns   : (int) GFX_OK or GFX_ERROR
** The mouse pointer will be redrawn continually, if applicable
*/

int gfxop_set_pointer_cursor(GfxState *state, int nr);
/* Sets the mouse pointer to a cursor resource
** Parameters: (GfxState *) state: The affected state
**             (int) nr: Number of the cursor resource to use
** Returns   : (int) GFX_OK, GFX_ERROR if the resource did not
**                   exist and was not GFXOP_NO_POINTER, or GFX_FATAL on
**                   fatal error conditions.
** Use nr = GFX_NO_POINTER to disable the mouse pointer (default).
*/

int gfxop_set_pointer_view(GfxState *state, int nr, int loop, int cel, Common::Point *hotspot);
/* Sets the mouse pointer to a view resource
** Parameters: (GfxState *) state: The affected state
**             (int) nr: Number of the view resource to use
**             (int) loop: View loop to use
**             (int) cel: View cel to use
**             (Common::Point *) hotspot: Manually set hotspot to use, or NULL for default.
** Returns   : (int) GFX_OK or GFX_FATAL
** Use gfxop_set_pointer_cursor(state, GFXOP_NO_POINTER) to disable the
** pointer.
*/

int gfxop_set_pointer_position(GfxState *state, Common::Point pos);
/* Teleports the mouse pointer to a specific position
** Parameters: (GfxState *) state: The state the pointer is in
**             (Common::Point) pos: The position to teleport it to
** Returns   : (int) Any error code or GFX_OK
** Depending on the graphics driver, this operation may be without
** any effect
*/

sci_event_t gfxop_get_event(GfxState *state, unsigned int mask);
/* Retrieves the next input event from the driver
** Parameters: (GfxState *) state: The affected state
**             (int) mask: The event mask to poll from (see uinput.h)
** Returns   : (sci_event_t) The next event in the driver's event queue, or
**             a NONE event if no event matching the mask was found.
*/


/*******************/
/* View operations */
/*******************/

int gfxop_lookup_view_get_loops(GfxState *state, int nr);
/* Determines the number of loops associated with a view
** Parameters: (GfxState *) state: The state to use
**             (int) nr: Number of the view to investigate
** Returns   : (int) The number of loops, or GFX_ERROR if the view didn't exist
*/

int gfxop_lookup_view_get_cels(GfxState *state, int nr, int loop);
/* Determines the number of cels associated stored in a loop
** Parameters: (GfxState *) state: The state to look up in
**             (int) nr: Number of the view to look up in
**             (int) loop: Number of the loop the number of cels of
**                         are to  be investigated
** Returns   : (int) The number of cels in that loop, or GFX_ERROR if either
**                   the view or the loop didn't exist
*/

int gfxop_check_cel(GfxState *state, int nr, int *loop, int *cel);
/* Clips the view/loop/cel position of a cel
** Parameters: (GfxState *) state: The state to use
**             (int) nr: Number of the view to use
**             (int *) loop: Pointer to the variable storing the loop
**                           number to verify
**             (int *) cel: Pointer to the variable storing the cel
**                          number to check
** Returns   : (int) GFX_OK or GFX_ERROR if the view didn't exist
** *loop is clipped first, then *cel. The resulting setup will be a valid
** view configuration.
*/

int gfxop_overflow_cel(GfxState *state, int nr, int *loop, int *cel);
/* Resets loop/cel values to zero if they have become invalid
** Parameters: (GfxState *) state: The state to use
**             (int) nr: Number of the view to use
**             (int *) loop: Pointer to the variable storing the loop
**                           number to verify
**             (int *) cel: Pointer to the variable storing the cel
**                          number to check
** Returns   : (int) GFX_OK or GFX_ERROR if the view didn't exist
** *loop is clipped first, then *cel. The resulting setup will be a valid
** view configuration.
*/

int gfxop_get_cel_parameters(GfxState *state, int nr, int loop, int cel,
	int *width, int *height, Common::Point *offset);
/* Retrieves the width and height of a cel
** Parameters: (GfxState *) state: The state to use
**             (int) nr: Number of the view
**             (int) loop: Loop number to examine
**             (int) cel: The cel (inside the loop) to look up
**             (int *) width: The variable the width will be stored in
**             (int *) height: The variable the height will be stored in
**             (Common::Point *) offset: The variable the cel's x/y offset will be stored in
** Returns   : (int) GFX_OK if the lookup succeeded, GFX_ERROR if the nr/loop/cel
**             combination was invalid
*/

int gfxop_draw_cel(GfxState *state, int nr, int loop, int cel, Common::Point pos,
	gfx_color_t color, int palette);
/* Draws (part of) a cel to the back buffer
** Parameters: (GfxState *) state: The state encapsulating the driver to draw with
**             (int) nr: Number of the view to draw
**             (int) loop: Loop of the cel to draw
**             (int) cel: The cel number of the cel to draw
**             (Common::Point) pos: The positino the cel is to be drawn to
**             (gfx_color_t color): The priority and control values to use for drawing
**	       (int) palette: The palette to use
** Returns   : (int) GFX_OK or GFX_FATAL
*/


int gfxop_draw_cel_static(GfxState *state, int nr, int loop, int cel, Common::Point pos,
	gfx_color_t color, int palette);
/* Draws a cel to the static buffer; no clipping is performed
** Parameters: (GfxState *) state: The state encapsulating the driver to draw with
**             (int) nr: Number of the view to draw
**             (int) loop: Loop of the cel to draw
**             (int) cel: The cel number of the cel to draw
**             (Common::Point) pos: The positino the cel is to be drawn to
**             (gfx_color_t color): The priority and control values to use for drawing
**	       (int) palette: The palette to use
** Returns   : (int) GFX_OK or GFX_FATAL
** Let me repeat, no clipping (except for the display borders) is performed.
*/


int gfxop_draw_cel_static_clipped(GfxState *state, int nr, int loop, int cel, Common::Point pos,
	gfx_color_t color, int palette);
/* Draws (part of) a clipped cel to the static buffer
** Parameters: (GfxState *) state: The state encapsulating the driver to draw with
**             (int) nr: Number of the view to draw
**             (int) loop: Loop of the cel to draw
**             (int) cel: The cel number of the cel to draw
**             (Common::Point) pos: The positino the cel is to be drawn to
**             (gfx_color_t color): The priority and control values to use for drawing
**	       (int) palette: The palette to use
** Returns   : (int) GFX_OK or GFX_FATAL
** This function does clip.
*/


/******************/
/* Pic operations */
/******************/
/* These operations are exempt from clipping */

int gfxop_new_pic(GfxState *state, int nr, int flags, int default_palette);
/* Draws a pic and writes it over the static buffer
** Parameters: (GfxState *) state: The state affected
**             (int) nr: Number of the pic to draw
**             (int) flags: Interpreter-dependant flags to use for drawing
**             (int) default_palette: The default palette for drawing
** Returns   : (int) GFX_OK or GFX_FATAL
** This function instructs the resource manager to tag all data as "unused".
** See the resource manager tag functions for a full description.
*/

int *gfxop_get_pic_metainfo(GfxState *state);
/* Retrieves all meta-information assigned to the current pic
** Parameters: (GfxState *) state: The state affected
** Returns   : (int *) NULL if the pic doesn't exist or has no meta-information,
**             the meta-info otherwise
** This meta-information is referred to as 'internal data' in the pic code
*/

int gfxop_add_to_pic(GfxState *state, int nr, int flags, int default_palette);
/* Adds a pic to the static buffer
** Parameters: (GfxState *) state: The state affected
**             (int) nr: Number of the pic to add
**             (int) flags: Interpreter-dependant flags to use for drawing
**             (int) default_palette: The default palette for drawing
** Returns   : (int) GFX_OK or GFX_FATAL
*/




/*******************/
/* Text operations */
/*******************/


int gfxop_get_font_height(GfxState *state, int font_nr);
/* Returns the fixed line height for one specified font
** Parameters: (GfxState *) state: The state to work on
**             (int) font_nr: Number of the font to inspect
** Returns   : (int) GFX_ERROR, GFX_FATAL, or the font line height
*/

int gfxop_get_text_params(GfxState *state, int font_nr, const char *text,
	int maxwidth, int *width, int *height, int flags,
	int *lines_nr, int *lineheight, int *lastline_width);
/* Calculates the width and height of a specified text in a specified font
** Parameters: (GfxState *) state: The state to use
**             (int) font_nr: Font number to use for the calculation
**             (const char *) text: The text to examine
**             (int) flags: ORred GFXR_FONT_FLAGs
**             (int) maxwidth: The maximum pixel width to allow for the text
** Returns   : (int) GFX_OK or GFX_ERROR if the font didn't exist
**             (int) *width: The resulting width
**             (int) *height: The resulting height
**             (int) *lines_nr: Number of lines used in the text
**             (int) *lineheight: Pixel height (SCI scale) of each text line
**             (int) *lastline_wdith: Pixel offset (SCI scale) of the space
**                   after the last character in the last line
*/

gfx_text_handle_t *gfxop_new_text(GfxState *state, int font_nr, char *text, int maxwidth,
	gfx_alignment_t halign, gfx_alignment_t valign, gfx_color_t color1,
	gfx_color_t color2, gfx_color_t bg_color, int flags);
/* Generates a new text handle that can be used to draw any text
** Parameters: (GfxState *) state: The state to use
**             (int) font_nr: Font number to use for the calculation
**             (char *) text: The text to examine
**             (int) maxwidth: The maximum pixel width to allow for the text
**             (gfx_alignment_t) halign: The horizontal text alignment
**             (gfx_alignment_t) valign: The vertical text alignment
**             (gfx_color_t x gfx_color_t) color1, color2: The text's foreground colors
**                                         (the function will dither between those two)
**             (gfx_color_t) bg_color: The background color
**             (int) flags: ORred GFXR_FONT_FLAGs
** Returns   : (gfx_text_handle_t *) A newly allocated gfx_text_handle_t, or
**             NULL if font_nr was invalid
** The control and priority values for the text will be extracted from color1.
** Note that the colors must have been allocated properly, or the text may display in
** incorrect colors.
*/

int gfxop_free_text(GfxState *state, gfx_text_handle_t *handle);
/* Frees a previously allocated text handle and all related resources
** Parameters: (GfxState *) state: The state to use
**             (gfx_text_handle_t *) handle: The handle to free
** Returns   : (int) GFX_OK
*/

int gfxop_draw_text(GfxState *state, gfx_text_handle_t *handle, rect_t zone);
/* Draws text stored in a text handle
** Parameters: (GfxState *) state: The target state
**             (gfx_text_handle_t *) handle: The text handle to use for drawing
**             (rect_t) zone: The rectangular box to draw to. In combination with
**                            halign and valign, this defines where the text is
**                            drawn to.
** Returns   : (int) GFX_OK or GFX_FATAL
*/


/****************************/
/* Manual pixmap operations */
/****************************/

gfx_pixmap_t *gfxop_grab_pixmap(GfxState *state, rect_t area);
/* Grabs a screen section from the back buffer and stores it in a pixmap
** Parameters: (GfxState *) state: The affected state
**             (rect_t) area: The area to grab
** Returns   : (gfx_pixmap_t *) A result pixmap, or NULL on error
** Obviously, this only affects the visual map
*/

int gfxop_draw_pixmap(GfxState *state, gfx_pixmap_t *pxm, rect_t zone, Common::Point pos);
/* Draws part of a pixmap to the screen
** Parameters: (GfxState *) state: The affected state
**             (gfx_pixmap_t *) pxm: The pixmap to draw
**             (rect_t) zone: The segment of the pixmap to draw
**             (Common::Point) pos: The position the pixmap should be drawn to
** Returns   : (int) GFX_OK or any error code
*/

int gfxop_free_pixmap(GfxState *state, gfx_pixmap_t *pxm);
/* Frees a pixmap returned by gfxop_grab_pixmap()
** Parameters: (GfxState *) state: The affected state
**             (gfx_pixmap_t *) pxm: The pixmap to free
** Returns   : (int) GFX_OK, or GFX_ERROR if the state was invalid
*/

/******************************/
/* Dirty rectangle operations */
/******************************/

/**
 * Adds a dirty rectangle to 'base' according to a strategy.
 * @param list		the list to add to
 * @param box		the dirty frame to addable
 * @param strategy	the dirty frame heuristic to use (see gfx_options.h)
 */
void gfxdr_add_dirty(DirtyRectList &list, rect_t box, int strategy);

int _gfxop_clip(rect_t *rect, rect_t clipzone);
/* Clips a rectangle against another one
** Parameters: (rect_t *) rect: The rectangle to clip
**             (rect_t) clipzone: The outer bounds rect must be in
** Reuturns  : (int) 1 if rect is empty now, 0 otherwise
*/

} // End of namespace Sci

#endif // SCI_GFX_GFX_OPERATIONS_H
