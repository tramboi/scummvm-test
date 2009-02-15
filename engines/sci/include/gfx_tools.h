/***************************************************************************
 gfx_tools.h Copyright (C) 2000 Christoph Reichenbach

 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/
/* FreeSCI 0.3.1+ graphics subsystem helper functions */


#ifndef _GFX_TOOLS_H_
#define _GFX_TOOLS_H_

#include <gfx_system.h>
#include <gfx_driver.h>

typedef enum {
	GFX_XLATE_FILTER_NONE,
	GFX_XLATE_FILTER_LINEAR,
	GFX_XLATE_FILTER_TRILINEAR
} gfx_xlate_filter_t;

typedef enum {
	GFXR_ANTIALIASING_NONE,
	GFXR_ANTIALIASING_SIMPLE
} gfxr_antialiasing_t;


extern DLLEXTERN int gfx_crossblit_alpha_threshold; /* Crossblitting functions use this value as threshold
		 			  ** for distinguishing between transparent and opaque
					  ** wrt alpha values */

gfx_mode_t *
gfx_new_mode(int xfact, int yfact, int bytespp, unsigned int red_mask, unsigned int green_mask,
	     unsigned int blue_mask, unsigned int alpha_mask, int red_shift, int green_shift,
	     int blue_shift, int alpha_shift, int palette, int flags);
/* Allocates a new gfx_mode_t structure with the specified parameters
** Parameters: (int x int) xfact x yfact: Horizontal and vertical scaling factors
**             (int) bytespp: Bytes per pixel
**             (unsigned int) red_mask: Red bit mask
**             (unsigned int) green_mask: Green bit mask
**             (unsigned int) blue_mask: Blue bit mask
**             (unsigned int) Alpha_mask: Alpha bit mask, or 0 if the alpha channel is not supported
**             (int) red_shift: Red shift value
**             (int) green_shift: Green shift value
**             (int) blue_shift: Blue shift value
**             (int) alpha_shift: Alpha shift value
**             (int) palette: Number of palette colors, 0 if we're not in palette mode
**             (int) flags: GFX_MODE_FLAG_* values ORred together, or just 0
** Returns   : (gfx_mode_t *) A newly allocated gfx_mode_t structure
*/


void
gfx_clip_box_basic(rect_t *box, int maxx, int maxy);
/* Clips a rect_t
** Parameters: (rect_t *) box: Pointer to the box to clip
**             (int x int) maxx, maxy: Maximum allowed width and height
** Returns   : (void)
*/


void
gfx_free_mode(gfx_mode_t *mode);
/* Frees all memory allocated by a mode structure
** Parameters: (gfx_mode_t *) mode: The mode to free
** Returns   : (void)
*/


gfx_pixmap_t *
gfx_new_pixmap(int xl, int yl, int resid, int loop, int cel);
/* Creates a new pixmap structure
** Parameters: (int x int) xl x yl: The dimensions (in SCI coordinates) of the pixmap
**             (int) resid: The pixmap's resource ID, or GFX_RESID_NONE
**             (int) loop: For views: The pixmap's loop number
**             (int) cel: For cels: The pixmap's cel number
** Returns   : (gfx_pixmap_t *) The newly allocated pixmap
** The following fiels are initialized:
** ID, loop, cel, index_xl, index_yl, xl, yl, data <- NULL,
** alpha_map <- NULL, internal.handle <- 0, internal.info <- NULL, colors <- NULL,
** index_scaled <- 0
*/

gfx_pixmap_t *
gfx_clone_pixmap(gfx_pixmap_t *pixmap, gfx_mode_t *mode);
/* Clones a pixmap, minus its index data, palette and driver-specific handles
** Parameters: (gfx_pixmap_t *) pixmap: The pixmap to clone
**             (gfx_mode_t *) mode: The mode to be applied to the pixmap
** Returns   : (gfx_pixmap_t *) The clone
*/


gfx_pixmap_t *
gfx_pixmap_alloc_index_data(gfx_pixmap_t *pixmap);
/* Allocates the index_data field of a pixmap
** Parameters: (gfx_pixmap_t *) pixmap: The pixmap to allocate for
** Returns   : (gfx_pixmap_t *) pixmap
*/

gfx_pixmap_t *
gfx_pixmap_free_index_data(gfx_pixmap_t *pixmap);
/* Frees the index_data field of a pixmap
** Parameters: (gfx_pixmap_t *) pixmap: The pixmap to modify
** Returns   : (gfx_pixmap_t *) pixmap
*/

gfx_pixmap_t *
gfx_pixmap_alloc_data(gfx_pixmap_t *pixmap, gfx_mode_t *mode);
/* Allocates the data field of a pixmap
** Parameters: (gfx_pixmap_t *) pixmap: The pixmap to allocate for
**             (gfx_mode_t *) mode: The mode the memory is to be allocated for
** Returns   : (gfx_pixmap_t *) pixmap
*/

gfx_pixmap_t *
gfx_pixmap_free_data(gfx_pixmap_t *pixmap);
/* Frees the memory allocated for a pixmap's data field
** Parameters: (gfx_pixmap_t *) pixmap: The pixmap to modify
** Returns   : (gfx_pixmap_t *) pixmap
*/

void
gfx_free_pixmap(gfx_driver_t *driver, gfx_pixmap_t *pxm);
/* Frees all memory associated with a pixmap
** Parameters: (gfx_driver_t *) driver: The driver the pixmap is to be removed from
**             (gfx_pixmap_t *) pxm: The pixmap to free
** Returns   : (void)
*/

void
gfx_draw_line_pixmap_i(gfx_pixmap_t *pxm, point_t start, point_t end, int color);
/* Draws a line to a pixmap's index data buffer
** Parameters: (gfx_pixmap_t *) pxm: The pixmap to draw to
**             (point_t) start: Starting point of the line to draw
**             (point_t) end: End point of the line to draw
**             (int) color: The byte value to write
** Returns   : (void)
** Remember, this only draws to the /index/ buffer, not to the drawable buffer.
** The line is not clipped. Invalid x, y, x1, y1 values will result in memory corruption.
*/

void
gfx_draw_line_buffer(byte *buffer, int linewidth, int pixelwidth,
		     point_t start, point_t end, unsigned int color);
/* Draws a line to a linear pixel buffer
** Parameters: (byte *) buffer: Pointer to the start of the buffer to draw to
**             (int) linewidth: Number of bytes per pixel line in the buffer
**             (int) pixelwidth: Number of bytes per pixel
**             (point_t) start: Starting point of the line to draw
**             (point_t) end: End point of the line to draw
**             (rect_t) Coordinates: the line should be drawn to (must be clipped already)
**                      xl and yl describe relative offsets, as usual.
**             (unsigned int) color: The color to draw (only the lowest 8 * pixelwidth bits are relevant)
** Returns   : (void)
** This function assumes 1 <= pixelwidth <= 4
*/

void
gfx_draw_box_pixmap_i(gfx_pixmap_t *pxm, rect_t box, int color);
/* Draws a filled rectangular area to a pixmap's index buffer
** Parameters: (gfx_pixmap_t *) pxm: The pixmap to draw to
**             (rect_t) box: The box to fill
**             (int) color: The color to use for drawing
** Returns   : (void)
** This function only draws to the index buffer.
*/

void
gfx_copy_pixmap_box_i(gfx_pixmap_t *dest, gfx_pixmap_t *src, rect_t box);
/* Copies part of a pixmap to another pixmap, with clipping
** Parameters: (gfx_pixmap_t *) dest: The destination pixmap
**             (gfx_pixmap_t *) src: The source pixmap
**             (rect_t) box: The area to copy
** Returns   : (void)
*/

void
gfx_xlate_pixmap(gfx_pixmap_t *pxm, gfx_mode_t *mode, gfx_xlate_filter_t filter);
/* Translates a pixmap's index data to drawable graphics data
** Parameters: (gfx_pixmap_t *) pxm: The pixmap to translate
**             (gfx_mode_t *) mode: The mode according which to scale
**             (gfx_xlate_filter_t) filter: How to filter the data
** Returns   : (void)
*/

void
gfxr_antialiase(gfx_pixmap_t *pixmap, gfx_mode_t *mode, gfxr_antialiasing_t type);
/* Performs antialiasing on a pixmap
** Parameters: (gfx_pixmap_t *) pixmap: The pixmap to antialiase
**             (gfx_mode_t *) mode: The current mode
**             (gfxr_antialiasing_t) type: Antialiasing algorithm to use
** Returns   : (void)
*/

#define GFX_CROSSBLIT_FLAG_DATA_IS_HOMED (1<<0)
/* Means that the first byte in the visual data refers to the
** point corresponding to (dest.x, dest.y) */

int
gfx_crossblit_pixmap(gfx_mode_t *mode, gfx_pixmap_t *pxm, int priority,
		     rect_t src_coords, rect_t dest_coords, byte *dest,
		     int dest_line_width, byte *priority_dest,
		     int priority_line_width, int priority_skip,
                     int flags);
/* Transfers the non-transparent part of a pixmap to a linear pixel buffer
** Parameters: (gfx_mode_t *) mode: The graphics mode of the target buffer
**             (gfx_pixmap_t *) pxm: The pixmap to transfer
**             (int priority): The pixmap's priority
**             (rect_t) src_coords: The source coordinates within the pixmap
**             (rect_t) dest_coords: The destination coordinates (no scaling)
**             (byte *) dest: Memory position of the upper left pixel of the
**                      linear pixel buffer
**             (int) dest_line_width: Byte offset of the very first pixel in the
**                                    second line of the linear pixel buffer,
**                                    relative to dest.
**             (byte *) priority_dest: Destination buffer for the pixmap's priority
**                                     values
**             (int) priority_line_width: Byte offset of the first pixel in the
**                                        second line of the priority buffer
**             (int) priority_skip: Amount of bytes allocated by each priority value
**             (int) flags: Any crossblit flags
** Returns   : (int) GFX_OK, or GFX_ERROR if the specified mode was invalid or unsupported
** A 'linear buffer' in this context means a data buffer containing an entire
** screen (visual or priority), with fixed offsets between each data row, and
** linear access.
*/

int
gfx_alloc_color(gfx_palette_t *pal, gfx_pixmap_color_t *color);
/* Allocates a color entry for the specified pixmap color
** Parameters: (gfx_palette_t *) pal: The palette structure the color should be allocated in
**             (gfx_pixmap_color_t *) color: The color to allocate
** Returns   : (int) GFX_ERROR if any error occured, GFX_OK if the color could be mapped to an
**                   existing color or a positive value if a new color was allocated in the
**                   palette.
*/

int
gfx_free_color(gfx_palette_t *pal, gfx_pixmap_color_t *color);
/* Frees the color entry allocated for the specified pixmap color
** Parameters: (gfx_palette_t *) pal: The palette structure the color was previously allocated in
**             (gfx_pixmap_color_t *) color: The color to free
** Returns   : (int) GFX_ERROR if any error occured, GFX_OK otherwise
*/

gfx_pixmap_t *
gfx_pixmap_scale_index_data(gfx_pixmap_t *pixmap, gfx_mode_t *mode);
/* Scales the index data associated with a pixmap
** Parameters: (gfx_pixmap_t *) pixmap: The pixmap whose index data should be scaled
**             (gfx_mode_t *) mode: The mode to scale it to
** Returns   : (gfx_pixmap_t *) pixmap
*/


#ifdef HAVE_ALPHA_EV6_SUPPORT

extern int axp_have_mvi; /* set to 1 iff the local system has the MVI instruction set extension */

void
alpha_mvi_crossblit_32(byte *dest, byte *src, int bytes_per_dest_line, int bytes_per_src_line,
		       int xl, int yl, byte *alpha, int bytes_per_alpha_line, int bytes_per_alpha_pixel,
		       unsigned int alpha_test_mask, int alpha_shift);
/* Internal function for accellerated 32 bit cross-blitting on Alpha hardware */

void
alpha_mvi_crossblit_32_P(byte *dest, byte *src, int bytes_per_dest_line, int bytes_per_src_line,
			 int xl, int yl, byte *alpha, int bytes_per_alpha_line, int bytes_per_alpha_pixel,
			 unsigned int alpha_test_mask, int alpha_shift,
			 byte *priority_pos, int bytes_per_priority_line, int bytes_per_priority_pixel, int priority);
/* Internal function for accellerated 32 bit cross-blitting on Alpha hardware (with priority) */
#endif /* __alpha__ */


#endif /* !_GFX_TOOLS_H_ */
