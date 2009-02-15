/***************************************************************************
 sci_pic_0.c Copyright (C) 2000 Christoph Reichenbach

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

#include <sci_memory.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <gfx_resource.h>
#include <gfx_tools.h>

#undef GFXR_DEBUG_PIC0 /* Enable to debug pic0 messages */
#undef FILL_RECURSIVE_DEBUG /* Enable for verbose fill debugging */

#define GFXR_PIC0_PALETTE_SIZE 40
#define GFXR_PIC0_NUM_PALETTES 4

#define INTERCOL(a, b) ((int) sqrt((((3.3 * (a))*(a)) + ((1.7 * (b))*(b))) / 5.0))
/* Macro for color interpolation */

#define SCI_PIC0_MAX_FILL 30 /* Number of times to fill before yielding to scheduler */

#define SCI0_MAX_PALETTE 2

int sci0_palette = 0;

/* Copied from include/kernel.h */
#define SCI0_PRIORITY_BAND_FIRST_14_ZONES(nr) ((((nr) == 0)? 0 :  \
        ((first) + (((nr)-1) * (last - first)) / 14)))

/* Default color maps */
gfx_pixmap_color_t gfx_sci0_image_colors[SCI0_MAX_PALETTE+1][GFX_SCI0_IMAGE_COLORS_NR] = {
	{{GFX_COLOR_SYSTEM, 0x00, 0x00, 0x00}, {GFX_COLOR_SYSTEM, 0x00, 0x00, 0xaa},
	 {GFX_COLOR_SYSTEM, 0x00, 0xaa, 0x00}, {GFX_COLOR_SYSTEM, 0x00, 0xaa, 0xaa},
	 {GFX_COLOR_SYSTEM, 0xaa, 0x00, 0x00}, {GFX_COLOR_SYSTEM, 0xaa, 0x00, 0xaa},
	 {GFX_COLOR_SYSTEM, 0xaa, 0x55, 0x00}, {GFX_COLOR_SYSTEM, 0xaa, 0xaa, 0xaa},
	 {GFX_COLOR_SYSTEM, 0x55, 0x55, 0x55}, {GFX_COLOR_SYSTEM, 0x55, 0x55, 0xff},
	 {GFX_COLOR_SYSTEM, 0x55, 0xff, 0x55}, {GFX_COLOR_SYSTEM, 0x55, 0xff, 0xff},
	 {GFX_COLOR_SYSTEM, 0xff, 0x55, 0x55}, {GFX_COLOR_SYSTEM, 0xff, 0x55, 0xff},
	 {GFX_COLOR_SYSTEM, 0xff, 0xff, 0x55}, {GFX_COLOR_SYSTEM, 0xff, 0xff, 0xff}}, /* "Normal" EGA */


	{{GFX_COLOR_SYSTEM, 0x00, 0x00, 0x00}, {GFX_COLOR_SYSTEM, 0x00, 0x00, 0xff},
	 {GFX_COLOR_SYSTEM, 0x00, 0xaa, 0x00}, {GFX_COLOR_SYSTEM, 0x00, 0xaa, 0xaa},
	 {GFX_COLOR_SYSTEM, 0xce, 0x00, 0x00}, {GFX_COLOR_SYSTEM, 0xbe, 0x71, 0xde},
	 {GFX_COLOR_SYSTEM, 0x8d, 0x50, 0x00}, {GFX_COLOR_SYSTEM, 0xbe, 0xbe, 0xbe},
	 {GFX_COLOR_SYSTEM, 0x55, 0x55, 0x55}, {GFX_COLOR_SYSTEM, 0x00, 0xbe, 0xff},
	 {GFX_COLOR_SYSTEM, 0x00, 0xce, 0x55}, {GFX_COLOR_SYSTEM, 0x55, 0xff, 0xff},
	 {GFX_COLOR_SYSTEM, 0xff, 0x9d, 0x8d}, {GFX_COLOR_SYSTEM, 0xff, 0x55, 0xff},
	 {GFX_COLOR_SYSTEM, 0xff, 0xff, 0x00}, {GFX_COLOR_SYSTEM, 0xff, 0xff, 0xff}}, /* AGI Amiga-ish */

/* RGB and I intensities (former taken from the GIMP) */
#define GR 30
#define GG 59
#define GB 11
#define GI 15

#define FULL (GR+GG+GB+GI)

#define CC(x) (((x)*255)/FULL),(((x)*255)/FULL),(((x)*255)/FULL)         /* Combines color intensities */

	{{GFX_COLOR_SYSTEM, CC(0)           }, {GFX_COLOR_SYSTEM, CC(GB)          },
	 {GFX_COLOR_SYSTEM, CC(GG)          }, {GFX_COLOR_SYSTEM, CC(GB+GG)       },
	 {GFX_COLOR_SYSTEM, CC(GR)          }, {GFX_COLOR_SYSTEM, CC(GB+GR)       },
	 {GFX_COLOR_SYSTEM, CC(GG+GR)       }, {GFX_COLOR_SYSTEM, CC(GB+GG+GR)    },
	 {GFX_COLOR_SYSTEM, CC(GI)          }, {GFX_COLOR_SYSTEM, CC(GB+GI)       },
	 {GFX_COLOR_SYSTEM, CC(GG+GI)       }, {GFX_COLOR_SYSTEM, CC(GB+GG+GI)    },
	 {GFX_COLOR_SYSTEM, CC(GR+GI)       }, {GFX_COLOR_SYSTEM, CC(GB+GR+GI)    },
	 {GFX_COLOR_SYSTEM, CC(GG+GR+GI)    }, {GFX_COLOR_SYSTEM, CC(GB+GG+GR+GI) }}}; /* Grayscale */

#undef GR
#undef GG
#undef GB
#undef GI

#undef FULL

#undef C2
#undef C3
#undef C4

gfx_pixmap_color_t gfx_sci0_pic_colors[GFX_SCI0_PIC_COLORS_NR]; /* Initialized during initialization */

static int _gfxr_pic0_colors_initialized = 0;

#define SCI1_PALETTE_SIZE 1284

#ifdef FILL_RECURSIVE_DEBUG
/************************************/
int fillc = 100000000;
int fillmagc = 30000000;
/************************************/
#endif

/* Color mapping used while scaling embedded views. */
gfx_pixmap_color_t embedded_view_colors[16] = {
	{0x00, 0, 0, 0}, {0x11, 0, 0, 0}, {0x22, 0, 0, 0}, {0x33, 0, 0, 0},
	 {0x44, 0, 0, 0}, {0x55, 0, 0, 0}, {0x66, 0, 0, 0}, {0x77, 0, 0, 0},
	 {0x88, 0, 0, 0}, {0x99, 0, 0, 0}, {0xaa, 0, 0, 0}, {0xbb, 0, 0, 0},
	 {0xcc, 0, 0, 0}, {0xdd, 0, 0, 0}, {0xee, 0, 0, 0}, {0xff, 0, 0, 0}
};

void
gfxr_init_static_palette()
{
	int i;

	if (!_gfxr_pic0_colors_initialized) {
		for (i = 0; i < 256; i++) {
			gfx_sci0_pic_colors[i].global_index = GFX_COLOR_INDEX_UNMAPPED;
			gfx_sci0_pic_colors[i].r = INTERCOL(gfx_sci0_image_colors[sci0_palette][i & 0xf].r,
							    gfx_sci0_image_colors[sci0_palette][i >> 4].r);
			gfx_sci0_pic_colors[i].g = INTERCOL(gfx_sci0_image_colors[sci0_palette][i & 0xf].g,
							    gfx_sci0_image_colors[sci0_palette][i >> 4].g);
			gfx_sci0_pic_colors[i].b = INTERCOL(gfx_sci0_image_colors[sci0_palette][i & 0xf].b,
							    gfx_sci0_image_colors[sci0_palette][i >> 4].b);
		}
		WARNING("Uncomment me after fixing sci0_palette changes to reset me");
                /*  _gfxr_pic0_colors_initialized = 1; */
	}
}


gfxr_pic_t *
gfxr_init_pic(gfx_mode_t *mode, int ID, int sci1)
{
	gfxr_pic_t *pic = (gfxr_pic_t*)sci_malloc(sizeof(gfxr_pic_t));

	pic->mode = mode;

	pic->control_map = gfx_pixmap_alloc_index_data(gfx_new_pixmap(320, 200, ID, 2, 0));

	pic->priority_map = gfx_pixmap_alloc_index_data(gfx_new_pixmap(mode->xfact * 320, mode->yfact * 200,
								       ID, 1, 0));


	pic->visual_map = gfx_pixmap_alloc_index_data(gfx_new_pixmap(320 * mode->xfact,
								     200 * mode->yfact, ID, 0, 0));
	pic->visual_map->colors = gfx_sci0_pic_colors;
	pic->visual_map->colors_nr = GFX_SCI0_PIC_COLORS_NR;
	pic->visual_map->color_key = GFX_PIXMAP_COLOR_KEY_NONE;

	pic->visual_map->flags = GFX_PIXMAP_FLAG_EXTERNAL_PALETTE;
	pic->priority_map->flags = GFX_PIXMAP_FLAG_EXTERNAL_PALETTE;
	pic->control_map->flags = GFX_PIXMAP_FLAG_EXTERNAL_PALETTE;
	if (mode->xfact > 1 || mode->yfact > 1) {
		pic->visual_map->flags |= GFX_PIXMAP_FLAG_SCALED_INDEX;
		pic->priority_map->flags |= GFX_PIXMAP_FLAG_SCALED_INDEX;
	}

	pic->priority_map->colors = gfx_sci0_image_colors[sci0_palette];
	pic->priority_map->colors_nr = GFX_SCI0_IMAGE_COLORS_NR;
	pic->control_map->colors = gfx_sci0_image_colors[sci0_palette];
	pic->control_map->colors_nr = GFX_SCI0_IMAGE_COLORS_NR;

	/* Initialize colors */
	if (!sci1) {
		pic->ID = ID;
		gfxr_init_static_palette();
	}

	pic->undithered_buffer_size = pic->visual_map->index_xl * pic->visual_map->index_yl;
	pic->undithered_buffer = NULL;
	pic->internal = NULL;

	return pic;
}


/****************************/
/* Pic rendering operations */
/****************************/

void
gfxr_clear_pic0(gfxr_pic_t *pic, int sci_titlebar_size)
{
	memset(pic->visual_map->index_data, 0x00, (320 * pic->mode->xfact * sci_titlebar_size * pic->mode->yfact));
	memset(pic->visual_map->index_data + (320 * pic->mode->xfact * sci_titlebar_size * pic->mode->yfact),
	       0xff, pic->mode->xfact * 320 * pic->mode->yfact * (200 - sci_titlebar_size)); /* white */
	memset(pic->priority_map->index_data + (320 * pic->mode->xfact * sci_titlebar_size * pic->mode->yfact),
	       0x0, pic->mode->xfact * 320 * pic->mode->yfact * (200 - sci_titlebar_size));
	memset(pic->priority_map->index_data, 0x0a, sci_titlebar_size * (pic->mode->yfact * 320 * pic->mode->xfact));
	memset(pic->control_map->index_data, 0, GFXR_AUX_MAP_SIZE);
	memset(pic->aux_map, 0, GFXR_AUX_MAP_SIZE);
}


/*** Basic operations on the auxiliary buffer ***/

#define FRESH_PAINT 0x40
/* freshly filled or near to something that is */

#define LINEMACRO(startx, starty, deltalinear, deltanonlinear, linearvar, nonlinearvar, \
                  linearend, nonlinearstart, linearmod, nonlinearmod, operation) \
   x = (startx); y = (starty); \
   incrNE = ((deltalinear) > 0)? (deltalinear) : -(deltalinear); \
   incrNE <<= 1; \
   deltanonlinear <<= 1; \
   incrE = ((deltanonlinear) > 0) ? -(deltanonlinear) : (deltanonlinear);  \
   d = nonlinearstart-1;  \
   while (linearvar != (linearend)) { \
     buffer[linewidth * y + x] operation color; \
/* color ^= color2; color2 ^= color; color ^= color2; */ /* Swap colors */ \
     linearvar += linearmod; \
     if ((d+=incrE) < 0) { \
       d += incrNE; \
       nonlinearvar += nonlinearmod; \
     }; \
   }; \
  buffer[linewidth * y + x] operation color;

static void
_gfxr_auxbuf_line_draw(gfxr_pic_t *pic, rect_t line, int color, int color2, int sci_titlebar_size)
{
	int dx, dy, incrE, incrNE, d, finalx, finaly;
	int x = line.x;
	int y = line.y + sci_titlebar_size;
	unsigned char *buffer = pic->aux_map;
	int linewidth = 320;

	dx = line.xl;
	dy = line.yl;
	finalx = x + dx;
	finaly = y + dy;

	dx = abs(dx);
	dy = abs(dy);

	if (dx > dy) {
		if (finalx < x) {
			if (finaly < y) { /* llu == left-left-up */
				LINEMACRO(x, y, dx, dy, x, y, finalx, dx, -1, -1, |=);
			} else {         /* lld */
				LINEMACRO(x, y, dx, dy, x, y, finalx, dx, -1, 1, |=);
			}
		} else { /* x1 >= x */
			if (finaly < y) { /* rru */
				LINEMACRO(x, y, dx, dy, x, y, finalx, dx, 1, -1, |=);
			} else {         /* rrd */
				LINEMACRO(x, y, dx, dy, x, y, finalx, dx, 1, 1, |=);
			}
		}
	} else { /* dx <= dy */
		if (finaly < y) {
			if (finalx < x) { /* luu */
				LINEMACRO(x, y, dy, dx, y, x, finaly, dy, -1, -1, |=);
			} else {         /* ruu */
				LINEMACRO(x, y, dy, dx, y, x, finaly, dy, -1, 1, |=);
			}
		} else { /* y1 >= y */
			if (finalx < x) { /* ldd */
				LINEMACRO(x, y, dy, dx, y, x, finaly, dy, 1, -1, |=);
			} else {         /* rdd */
				LINEMACRO(x, y, dy, dx, y, x, finaly, dy, 1, 1, |=);
			}
		}
	}
}

static void
_gfxr_auxbuf_line_clear(gfxr_pic_t *pic, rect_t line, int color, int sci_titlebar_size)
{
	int dx, dy, incrE, incrNE, d, finalx, finaly;
	int x = line.x;
	int y = line.y + sci_titlebar_size;
	unsigned char *buffer = pic->aux_map;
	int linewidth = 320;
	int color2 = color;

	dx = line.xl;
	dy = line.yl;
	finalx = x + dx;
	finaly = y + dy;

	dx = abs(dx);
	dy = abs(dy);

	if (dx > dy) {
		if (finalx < x) {
			if (finaly < y) { /* llu == left-left-up */
				LINEMACRO(x, y, dx, dy, x, y, finalx, dx, -1, -1, &=);
			} else {         /* lld */
				LINEMACRO(x, y, dx, dy, x, y, finalx, dx, -1, 1, &=);
			}
		} else { /* x1 >= x */
			if (finaly < y) { /* rru */
				LINEMACRO(x, y, dx, dy, x, y, finalx, dx, 1, -1, &=);
			} else {         /* rrd */
				LINEMACRO(x, y, dx, dy, x, y, finalx, dx, 1, 1, &=);
			}
		}
	} else { /* dx <= dy */
		if (finaly < y) {
			if (finalx < x) { /* luu */
				LINEMACRO(x, y, dy, dx, y, x, finaly, dy, -1, -1, &=);
			} else {         /* ruu */
				LINEMACRO(x, y, dy, dx, y, x, finaly, dy, -1, 1, &=);
			}
		} else { /* y1 >= y */
			if (finalx < x) { /* ldd */
				LINEMACRO(x, y, dy, dx, y, x, finaly, dy, 1, -1, &=);
			} else {         /* rdd */
				LINEMACRO(x, y, dy, dx, y, x, finaly, dy, 1, 1, &=);
			}
		}
	}
}

#undef LINEMACRO


#ifdef WITH_PIC_SCALING
static void
_gfxr_auxbuf_propagate_changes(gfxr_pic_t *pic, int bitmask)
{
	/* Propagates all filled bits into the planes described by bitmask */
	unsigned long *data = (unsigned long *) pic->aux_map;
	unsigned long clearmask = 0x07070707;
	unsigned long andmask =
		(bitmask << 3)
		| (bitmask << (3+8))
		| (bitmask << (3+16))
		| (bitmask << (3+24));
	int i;

	if (sizeof(unsigned long) == 8) { /* UltraSparc, Alpha, newer MIPSens, etc */
		andmask |= (andmask << 32);
		clearmask |= (clearmask << 32);
	}

	for (i = 0; i < GFXR_AUX_MAP_SIZE / sizeof(unsigned long); i++) {
		unsigned long temp = *data & andmask;
		temp >>= 3;
		*data = (temp | *data) & clearmask;
		++data;
	}
}
#endif


static inline void
_gfxr_auxbuf_tag_line(gfxr_pic_t *pic, int pos, int width)
{
	int i;
	for (i = 0; i < width; i++)
		pic->aux_map[i+pos] |= FRESH_PAINT;
}


static void
_gfxr_auxbuf_spread(gfxr_pic_t *pic, int *min_x, int *min_y, int *max_x, int *max_y)
{
	/* Tries to spread by approximating the first derivation of the border function.
	** Draws to the current and the last line, thus taking up to twice as long as neccessary.
	** Other than that, it's O(n^2)
	*/

	int intervals_nr = 0, old_intervals_nr;
	int x, y, i, pos = 10*320;
	struct interval_struct {
		int xl, xr, tag;
	} intervals[2][160];

	*max_x = *max_y = -1;
	*min_x = *min_y = 320;

#ifdef FILL_RECURSIVE_DEBUG
	if (!fillmagc) {
		fprintf(stderr,"------------------------------------------------\n");
		fprintf(stderr,"LineID:   ");
		for (i = 0; i < 5; i++)
			fprintf(stderr,"  %d       ", i);
		fprintf(stderr,"\n");
	}
#endif

	for (y = 10; y < 200; y++) {
		int ivi = y & 1; /* InterVal Index: Current intervals; !ivi is the list of old ones */
		int old_intervals_start_offset = 0;
		int width = 0;

		old_intervals_nr = intervals_nr;
		intervals_nr = 0;

		for (x = 0; x < 321; x++)
			if (x < 320 && pic->aux_map[pos+x] & 0x10)
				width++;
			else if (width) { /* Found one interval */
				int xl = x - width;
				int xr = x - 1;
				int done = 0;
				int found_interval = 0;

				intervals[ivi][intervals_nr].xl = xl;
				intervals[ivi][intervals_nr].tag = 0;
				intervals[ivi][intervals_nr++].xr = xr;

				if (xl < *min_x)
					*min_x = xl;
				if (xr > *max_x)
					*max_x = xr;

				i = old_intervals_start_offset;
				while (!done && i < old_intervals_nr) {
					if (intervals[!ivi][i].xl > xr+1)
						done = 1;

					else if (intervals[!ivi][i].xr < xl-1) {
						int o_xl = intervals[!ivi][i].xl;
						int o_xr = intervals[!ivi][i].xr;
						if (o_xr == o_xl && !intervals[!ivi][i].tag) { /* thin bar */
							memcpy(intervals[ivi] + intervals_nr, intervals[ivi] + intervals_nr - 1, sizeof(struct interval_struct));
							memcpy(intervals[ivi] + intervals_nr - 1, intervals[!ivi] + i, sizeof(struct interval_struct));
							intervals[!ivi][i].tag = 1;
							pic->aux_map[pos - 320 + o_xl] |= FRESH_PAINT;
							++intervals_nr;
						}

						old_intervals_start_offset = i;
					}

					else {
						int k = i;
						int old_xl = intervals[!ivi][i].xl;
						int dwidth_l = abs(old_xl - xl);
						int old_xr, dwidth_r;
						int write_left_width, write_right_width;

						intervals[!ivi][i].tag = 1;
						while (k+1 < old_intervals_nr && intervals[!ivi][k+1].xl <= xr) {
							++k;
							intervals[!ivi][i].tag = 1;
						}

						old_xr = intervals[!ivi][k].xr;
						dwidth_r = abs(old_xr - xr);

						/* Current line */
						write_left_width = (dwidth_l > xl)? xl : dwidth_l;
						_gfxr_auxbuf_tag_line(pic, pos + xl - write_left_width, write_left_width);

						write_right_width = (dwidth_r + xr > 319)? 320 - xr : dwidth_r;
						_gfxr_auxbuf_tag_line(pic, pos + xr, write_right_width);

						if (xl - write_left_width < *min_x)
							*min_x = xl - write_left_width;
						if (xr + write_right_width > *max_x)
							*max_x = xr + write_right_width;

						/* Previous line */
						write_left_width = (dwidth_l > old_xl)? old_xl : dwidth_l;
						write_right_width = (dwidth_r + old_xr > 319)? 320 - old_xr : dwidth_r;

						if (i == k) { /* Only one predecessor interval */
							_gfxr_auxbuf_tag_line(pic, pos - 320 + old_xl - write_left_width, write_left_width);
							_gfxr_auxbuf_tag_line(pic, pos - 320 + old_xr, write_right_width);
						} else /* Fill entire line */
							_gfxr_auxbuf_tag_line(pic, pos - 320 + old_xl - write_left_width, old_xr - old_xl
									      + 1 + write_left_width + write_right_width);

						if (xl - write_left_width < *min_x)
							*min_x = xl - write_left_width;
						if (xr + write_right_width > *max_x)
							*max_x = xr + write_right_width;

						found_interval = done = 1;
					}
					i++;
				}
				width = 0;
			}

#ifdef FILL_RECURSIVE_DEBUG
		if (!fillmagc && intervals_nr) {
			fprintf(stderr,"AI L#%03d:", y);
			for (int j = 0; j < intervals_nr; j++)
				fprintf(stderr, "%c[%03d,%03d]", intervals[ivi][j].tag? ' ':'-', intervals[ivi][j].xl, intervals[ivi][j].xr);
			fprintf(stderr,"\n");
		}
#endif

		if (intervals_nr) {
			if (y < *min_y)
				*min_y = y;
			*max_y = y;
		}

		pos += 320;
	}

	for (pos = 320*200 - 1; pos >= 320; pos--)
		if (pic->aux_map[pos - 320] & 0x40)
			pic->aux_map[pos] |= 0x40;

	if (*max_y < 199)
		(*max_y)++;
}



/*** Regular drawing operations ***/


#define PATTERN_FLAG_RECTANGLE 0x10
#define PATTERN_FLAG_USE_PATTERN 0x20

#define PIC_OP_FIRST 0xf0

enum {
	PIC_OP_SET_COLOR = 0xf0,
	PIC_OP_DISABLE_VISUAL = 0xf1,
	PIC_OP_SET_PRIORITY = 0xf2,
	PIC_OP_DISABLE_PRIORITY = 0xf3,
	PIC_OP_SHORT_PATTERNS = 0xf4,
	PIC_OP_MEDIUM_LINES = 0xf5,
	PIC_OP_LONG_LINES = 0xf6,
	PIC_OP_SHORT_LINES = 0xf7,
	PIC_OP_FILL = 0xf8,
	PIC_OP_SET_PATTERN = 0xf9,
	PIC_OP_ABSOLUTE_PATTERN = 0xfa,
	PIC_OP_SET_CONTROL = 0xfb,
	PIC_OP_DISABLE_CONTROL = 0xfc,
	PIC_OP_MEDIUM_PATTERNS = 0xfd,
	PIC_OP_OPX = 0xfe,
	PIC_OP_TERMINATE = 0xff
};

enum {
	PIC_SCI0_OPX_SET_PALETTE_ENTRIES = 0,
	PIC_SCI0_OPX_SET_PALETTE = 1,
	PIC_SCI0_OPX_MONO0 = 2,
	PIC_SCI0_OPX_MONO1 = 3,
	PIC_SCI0_OPX_MONO2 = 4,
	PIC_SCI0_OPX_MONO3 = 5,
	PIC_SCI0_OPX_MONO4 = 6,
	PIC_SCI0_OPX_EMBEDDED_VIEW,
	PIC_SCI0_OPX_SET_PRIORITY_TABLE
};

/* We use this so we can keep OPX handling in one switch.
   We simply add this constant to the op number if we're running an SCI1 game,
   and offset the OPX constants below correspondingly. */
#define SCI1_OP_OFFSET 42

enum {
	PIC_SCI1_OPX_SET_PALETTE_ENTRIES = 0+SCI1_OP_OFFSET,
	PIC_SCI1_OPX_EMBEDDED_VIEW = 1+SCI1_OP_OFFSET,
	PIC_SCI1_OPX_SET_PALETTE = 2+SCI1_OP_OFFSET,
	PIC_SCI1_OPX_PRIORITY_TABLE_EQDIST = 3+SCI1_OP_OFFSET,
	PIC_SCI1_OPX_PRIORITY_TABLE_EXPLICIT = 4+SCI1_OP_OFFSET
};
	

#ifdef GFXR_DEBUG_PIC0
#define p0printf sciprintf
#else
#define p0printf if (0)
#endif


enum {
	ELLIPSE_SOLID, /* Normal filled ellipse */
	ELLIPSE_OR     /* color ORred to the buffer */
};

static void
_gfxr_fill_ellipse(gfxr_pic_t *pic, byte *buffer, int linewidth, int x, int y,
		   int rad_x, int rad_y, int color, int fillstyle)
{
	int xx = 0, yy = rad_y;
	int i, x_i, y_i;
	int xr = 2 * rad_x * rad_x;
	int yr = 2 * rad_y * rad_y;

	x_i = 1;
	y_i = xr * rad_y -1;
	i = y_i >> 1;

	while (yy >= 0) {
		int oldxx = xx;
		int oldyy = yy;

		if (i >= 0) {
			x_i += yr;
			i -= x_i + 1;
			++xx;
		}

		if (i < 0) {
			y_i -= xr;
			i += y_i - 1;
			--yy;
		}

		if (oldyy != yy) {
			int j;
			int offset0 = (y-oldyy) * linewidth;
			int offset1 = (y+oldyy) * linewidth;

			offset0 += x-oldxx;
			offset1 += x-oldxx;

			if (oldyy == 0)
				offset1 = 0; /* We never have to draw ellipses in the menu bar */

			oldyy = yy;

			switch (fillstyle) {

			case ELLIPSE_SOLID:
				memset(buffer + offset0, color, (oldxx << 1) + 1);
				if (offset1)
					memset(buffer + offset1, color, (oldxx << 1) + 1);
				break;

			case ELLIPSE_OR:
				for (j=0; j < (oldxx << 1) + 1; j++) {
					buffer[offset0 + j] |= color;
					if (offset1)
						buffer[offset1 + j] |= color;
				}
				break;

			default:
				fprintf(stderr,"%s L%d: Invalid ellipse fill mode!\n", __FILE__, __LINE__);
				return;

			}
		}
	}
}

static inline void
_gfxr_auxplot_brush(gfxr_pic_t *pic, byte *buffer, int yoffset, int offset, int plot,
		    int color, gfx_brush_mode_t brush_mode, int randseed)
{
	/* yoffset 63680, offset 320, plot 1, color 34, brush_mode 0, randseed 432)*/
	/* Auxplot: Used by plot_aux_pattern to plot to visual and priority */
	int xc, yc;
	int line_width = 320 * pic->mode->xfact;
	int full_offset = (yoffset * pic->mode->yfact + offset) * pic->mode->xfact;

	if (yoffset + offset >= 64000) {
		BREAKPOINT();
	}

	switch (brush_mode) {
	case GFX_BRUSH_MODE_SCALED:
		if (plot)
			for (yc = 0; yc < pic->mode->yfact; yc++) {
				memset(buffer + full_offset, color, pic->mode->xfact);
				full_offset += line_width;
			}
		break;

	case GFX_BRUSH_MODE_ELLIPSES:
		if (plot) {
			int x = offset * pic->mode->xfact + ((pic->mode->xfact -1) >> 1);
			int y = (yoffset / 320) * pic->mode->yfact + ((pic->mode->yfact -1) >> 1); /* Ouch! */

			_gfxr_fill_ellipse(pic, buffer, line_width, x, y, pic->mode->xfact >> 1, pic->mode->yfact >> 1,
					   color, ELLIPSE_SOLID);
		}
		break;

	case GFX_BRUSH_MODE_RANDOM_ELLIPSES:
		if (plot) {
			int x = offset * pic->mode->xfact + ((pic->mode->xfact -1) >> 1);
			int y = (yoffset / 320) * pic->mode->yfact + ((pic->mode->yfact -1) >> 1); /* Ouch! */
			int sizex = pic->mode->xfact >> 1;
			int sizey = pic->mode->yfact >> 1;

			srand(randseed);

			x -= (int) ((sizex * rand()*1.0)/(RAND_MAX + 1.0));
			x += (int) ((sizex * rand()*1.0)/(RAND_MAX + 1.0));
			y -= (int) ((sizey * rand()*1.0)/(RAND_MAX + 1.0));
			y += (int) ((sizey * rand()*1.0)/(RAND_MAX + 1.0));
			sizex = (int) ((sizex * rand()*1.0)/(RAND_MAX + 1.0));
			sizey = (int) ((sizey * rand()*1.0)/(RAND_MAX + 1.0));

			_gfxr_fill_ellipse(pic, buffer, line_width, x, y, pic->mode->xfact >> 1, pic->mode->yfact >> 1,
					   color, ELLIPSE_SOLID);
			srand(time(NULL)); /* Make sure we don't accidently forget to re-init the random number generator */
		}
		break;

	case GFX_BRUSH_MODE_MORERANDOM: {
		int mask = plot? 7 : 1;
		srand(randseed);
		for (yc = 0; yc < pic->mode->yfact; yc++) {
			for (xc = 0; xc < pic->mode->xfact; xc++)
				if ((rand() & 7) < mask)
					buffer[full_offset + xc] = color;
			full_offset += line_width;
		}
		srand(time(NULL)); /* Make sure we don't accidently forget to re-init the random number generator */
	}
	break;
	}
}

#define PLOT_AUX_PATTERN_NO_RANDOM -1

static void
_gfxr_plot_aux_pattern(gfxr_pic_t *pic, int x, int y, int size, int circle, int random,
		       int mask, int color, int priority, int control,
		       gfx_brush_mode_t brush_mode, int map_nr)
{
	/* Plots an appropriate pattern to the aux buffer and the control buffer,
	** if mask & GFX_MASK_CONTROL
	** random should be set to the random index, or -1 to disable
	*/

	/* These circle offsets uniquely identify the circles used by Sierra: */
	int circle_data[][8] = {
		{0},
		{1, 0},
		{2, 2, 1},
		{3, 3, 2, 1},
		{4, 4, 4, 3, 1},
		{5, 5, 4, 4, 3, 1},
		{6, 6, 6, 5, 5, 4, 2},
		{7, 7, 7, 6, 6, 5, 4, 2}};

	/* 'Random' fill patterns, provided by Carl Muckenhoupt: */
	byte random_data[32] = {
		0x20, 0x94, 0x02, 0x24, 0x90, 0x82, 0xa4, 0xa2, 0x82, 0x09, 0x0a, 0x22,
		0x12, 0x10, 0x42, 0x14, 0x91, 0x4a, 0x91, 0x11, 0x08, 0x12, 0x25, 0x10,
		0x22, 0xa8, 0x14, 0x24, 0x00, 0x50, 0x24, 0x04};

	/* 'Random' fill offsets, provided by Carl Muckenhoupt: */
	byte random_offset[128] = {
		0x00, 0x18, 0x30, 0xc4, 0xdc, 0x65, 0xeb, 0x48,
		0x60, 0xbd, 0x89, 0x05, 0x0a, 0xf4, 0x7d, 0x7d,
		0x85, 0xb0, 0x8e, 0x95, 0x1f, 0x22, 0x0d, 0xdf,
		0x2a, 0x78, 0xd5, 0x73, 0x1c, 0xb4, 0x40, 0xa1,
		0xb9, 0x3c, 0xca, 0x58, 0x92, 0x34, 0xcc, 0xce,
		0xd7, 0x42, 0x90, 0x0f, 0x8b, 0x7f, 0x32, 0xed,
		0x5c, 0x9d, 0xc8, 0x99, 0xad, 0x4e, 0x56, 0xa6,
		0xf7, 0x68, 0xb7, 0x25, 0x82, 0x37, 0x3a, 0x51,
		0x69, 0x26, 0x38, 0x52, 0x9e, 0x9a, 0x4f, 0xa7,
		0x43, 0x10, 0x80, 0xee, 0x3d, 0x59, 0x35, 0xcf,
		0x79, 0x74, 0xb5, 0xa2, 0xb1, 0x96, 0x23, 0xe0,
		0xbe, 0x05, 0xf5, 0x6e, 0x19, 0xc5, 0x66, 0x49,
		0xf0, 0xd1, 0x54, 0xa9, 0x70, 0x4b, 0xa4, 0xe2,
		0xe6, 0xe5, 0xab, 0xe4, 0xd2, 0xaa, 0x4c, 0xe3,
		0x06, 0x6f, 0xc6, 0x4a, 0xa4, 0x75, 0x97, 0xe1
	};

	int offset = 0, width = 0;
	int yoffset = (y - size) * 320;
	int i;
	int random_index = 0;
	gfx_pixmap_t *map = NULL;

	switch (map_nr) {
	case GFX_MASK_VISUAL: map = pic->visual_map; break;
	case GFX_MASK_PRIORITY: map = pic->priority_map; break;
	default: map = pic->control_map; break;
	}

	if (random >= 0)
		random_index = random_offset[random];

	if (!circle) {
		offset = -size;
		width = (size << 1) + 2;
	}

	for (i = -size; i <= size; i++) {
		int j;
		int height;

		if (circle) {
			offset = circle_data[size][abs(i)];
			height = width = (offset << 1) + 1;
			offset = -offset;
		} else height = width - 1;

		if (random == PLOT_AUX_PATTERN_NO_RANDOM) {

			if (mask & map_nr)
				memset(map->index_data + yoffset + offset + x, control, width);

			if (map_nr == GFX_MASK_CONTROL)
				for (j = x; j < x + width; j++)
					pic->aux_map[yoffset + offset + j] |= mask;

		} else { /* Semi-Random! */
			for (j = 0; j < height; j++) {
				if (random_data[random_index >> 3] & (0x80 >> (random_index & 7))) {
					/* The 'seemingly' random decision */
					if (mask & GFX_MASK_CONTROL)
						pic->control_map->index_data[yoffset + x + offset + j] = control;

					pic->aux_map[yoffset + x + offset + j] |= mask;

					if (mask & GFX_MASK_VISUAL)
						_gfxr_auxplot_brush(pic, pic->visual_map->index_data,
								    yoffset, x + offset + j,
								    1, color, brush_mode, random_index + x);

					if (mask & GFX_MASK_PRIORITY)
						_gfxr_auxplot_brush(pic, pic->priority_map->index_data,
								    yoffset, x + offset + j,
								    1, priority, brush_mode, random_index + x);

				} else {
					if (mask & GFX_MASK_VISUAL)
						_gfxr_auxplot_brush(pic, pic->visual_map->index_data,
								    yoffset, x + offset + j,
								    0, color, brush_mode, random_index + x);

					if (mask & GFX_MASK_PRIORITY)
						_gfxr_auxplot_brush(pic, pic->priority_map->index_data,
								    yoffset, x + offset + j,
								    0, priority, brush_mode, random_index + x);
				}
				random_index = (random_index + 1) & 0xff;
			}
		}

		yoffset += 320;
	}
}


static void
_gfxr_draw_pattern(gfxr_pic_t *pic, int x, int y, int color, int priority, int control, int drawenable,
		   int pattern_code, int pattern_size, int pattern_nr, gfx_brush_mode_t brush_mode,
		   int sci_titlebar_size)
{
	int xsize = (pattern_size + 1) * pic->mode->xfact - 1;
	int ysize = (pattern_size + 1) * pic->mode->yfact - 1;
	int scaled_x, scaled_y;
	rect_t boundaries;
	int max_x = (pattern_code & PATTERN_FLAG_RECTANGLE)? 318 : 319; /* Rectangles' width is size+1 */

	p0printf(stderr, "Pattern at (%d,%d) size %d, rand=%d, code=%02x\n", x, y, pattern_size, pattern_nr, pattern_code);

	y += sci_titlebar_size;

	if (x - pattern_size < 0)
		x = pattern_size;

	if (y - pattern_size < sci_titlebar_size)
		y = sci_titlebar_size + pattern_size;

	if (x + pattern_size > max_x)
		x = max_x - pattern_size;

	if (y + pattern_size > 199)
		y = 199 - pattern_size;

	scaled_x = x * pic->mode->xfact + ((pic->mode->xfact - 1) >> 1);
	scaled_y = y * pic->mode->yfact + ((pic->mode->yfact - 1) >> 1);

	if (scaled_x < xsize)
		scaled_x = xsize;

	if (scaled_y < ysize + sci_titlebar_size * pic->mode->yfact)
		scaled_y = ysize + sci_titlebar_size * pic->mode->yfact;

	if (scaled_x > (320 * pic->mode->xfact) - 1 - xsize)
		scaled_x = (320 * pic->mode->xfact) - 1 - xsize;

	if (scaled_y > (200 * pic->mode->yfact) - 1 - ysize)
		scaled_y = (200 * pic->mode->yfact) - 1 - ysize;

	if (pattern_code & PATTERN_FLAG_RECTANGLE) {
		/* Rectangle */
		boundaries.x = scaled_x - xsize;
		boundaries.y = scaled_y - ysize;
		boundaries.xl = ((xsize + 1) << 1) + 1;
		boundaries.yl = (ysize << 1) + 1;


		if (pattern_code & PATTERN_FLAG_USE_PATTERN) {
			_gfxr_plot_aux_pattern(pic, x, y, pattern_size, 0, pattern_nr,
					       drawenable, color, priority,
					       control, brush_mode, GFX_MASK_CONTROL);
		} else {

			_gfxr_plot_aux_pattern(pic, x, y, pattern_size, 0,
					       PLOT_AUX_PATTERN_NO_RANDOM,
					       drawenable, 0, 0, control,
					       GFX_BRUSH_MODE_SCALED,
					       GFX_MASK_CONTROL);

			if (drawenable & GFX_MASK_VISUAL)
				gfx_draw_box_pixmap_i(pic->visual_map, boundaries, color);

			if (drawenable & GFX_MASK_PRIORITY)
				gfx_draw_box_pixmap_i(pic->priority_map, boundaries, priority);
		}

	} else {
		/* Circle */

		if (pattern_code & PATTERN_FLAG_USE_PATTERN) {

			_gfxr_plot_aux_pattern(pic, x, y, pattern_size, 1, pattern_nr,
					       drawenable, color, priority,
					       control, brush_mode, GFX_MASK_CONTROL);
		} else {

			_gfxr_plot_aux_pattern(pic, x, y, pattern_size, 1,
					       PLOT_AUX_PATTERN_NO_RANDOM,
					       drawenable, 0, 0, control,
					       GFX_BRUSH_MODE_SCALED,
					       GFX_MASK_CONTROL);

			if (pic->mode->xfact == 1 && pic->mode->yfact == 1) {

				if (drawenable & GFX_MASK_VISUAL)
					_gfxr_plot_aux_pattern(pic, x, y, pattern_size, 1,
							       PLOT_AUX_PATTERN_NO_RANDOM,
							       drawenable, 0, 0, color,
							       GFX_BRUSH_MODE_SCALED,
							       GFX_MASK_VISUAL);

				if (drawenable & GFX_MASK_PRIORITY)
					_gfxr_plot_aux_pattern(pic, x, y, pattern_size, 1,
							       PLOT_AUX_PATTERN_NO_RANDOM,
							       drawenable, 0, 0, priority,
							       GFX_BRUSH_MODE_SCALED,
							       GFX_MASK_PRIORITY);
			} else {

				if (drawenable & GFX_MASK_VISUAL)
					_gfxr_fill_ellipse(pic, pic->visual_map->index_data, 320 * pic->mode->xfact,
							   scaled_x, scaled_y, xsize, ysize,
							   color, ELLIPSE_SOLID);

				if (drawenable & GFX_MASK_PRIORITY)
					_gfxr_fill_ellipse(pic, pic->priority_map->index_data, 320 * pic->mode->xfact,
							   scaled_x, scaled_y, xsize, ysize,
							   priority, ELLIPSE_SOLID);
			}
		}
	}
}


static inline void
_gfxr_draw_subline(gfxr_pic_t *pic, int x, int y, int ex, int ey, int color, int priority, int drawenable)
{
	point_t start;
	point_t end;
	
	start.x = x;
	start.y = y;
	end.x = ex;
	end.y = ey;

        if (ex >= pic->visual_map->index_xl || ey >= pic->visual_map->index_yl || x < 0 || y < 0) {
                fprintf(stderr,"While drawing pic0: INVALID LINE %d,%d,%d,%d\n",
                        GFX_PRINT_POINT(start), GFX_PRINT_POINT(end));
                return;
        }

	if (drawenable & GFX_MASK_VISUAL)
		gfx_draw_line_pixmap_i(pic->visual_map, start, end, color);

	if (drawenable & GFX_MASK_PRIORITY)
		gfx_draw_line_pixmap_i(pic->priority_map, start, end, priority);

}

static void
_gfxr_draw_line(gfxr_pic_t *pic, int x, int y, int ex, int ey, int color,
		int priority, int control, int drawenable, int line_mode,
		int cmd, int sci_titlebar_size)
{
	int scale_x = pic->mode->xfact;
	int scale_y = pic->mode->yfact;
	int xc, yc;
	rect_t line;
	int mask;
	int partially_white = (drawenable & GFX_MASK_VISUAL)
		&& (((color & 0xf0) == 0xf0) || ((color & 0x0f) == 0x0f));

	line.x = x;
	line.y = y;
	line.xl = ex - x;
	line.yl = ey - y;

	if (x > 319 || y > 199 || x < 0 || y < 0
	    || ex > 319 || ey > 199 || ex < 0 || ey < 0) {
		GFXWARN("While building pic: Attempt to draw line (%d,%d) to (%d,%d): cmd was %d\n", x, y, ex, ey, cmd);
		return;
	}

	y += sci_titlebar_size;
	ey += sci_titlebar_size;

	if (drawenable & GFX_MASK_CONTROL) {

		p0printf(" ctl:%x", control);
		gfx_draw_line_pixmap_i(pic->control_map, gfx_point(x, y), gfx_point(x + line.xl, y + line.yl), control);
	}


	/* Calculate everything that is changed to SOLID */
	mask = drawenable &
		(
			((color != 0xff)? 1 : 0)
			| ((priority)? 2 : 0)
			| ((control)? 4 : 0)
			);

	if (mask) {
		int mask2 = mask;
		if (partially_white)
			mask2 = mask &= ~GFX_MASK_VISUAL;
		_gfxr_auxbuf_line_draw(pic, line, mask, mask2, sci_titlebar_size);
	}

	/* Calculate everything that is changed to TRANSPARENT */
	mask = drawenable &
		(
			((color == 0xff)? 1 : 0)
			| ((!priority)? 2 : 0)
			| ((!control)? 4 : 0)
			);

	if (mask)
		_gfxr_auxbuf_line_clear(pic, line, ~mask, sci_titlebar_size);

	x *= scale_x;
	y *= scale_y;
	ex *= scale_x;
	ey *= scale_y;

	if (drawenable & GFX_MASK_VISUAL)
		p0printf(" col:%02x", color);

	if (drawenable & GFX_MASK_PRIORITY)
		p0printf(" pri:%x", priority);

	if (line_mode == GFX_LINE_MODE_FINE) {  /* Adjust lines to extend over the full visual */
		x = (x * ((320 + 1) * scale_x - 1)) / (320 * scale_x);
		y = (y * ((200 + 1) * scale_y - 1)) / (200 * scale_y);
		ex = (ex * ((320 + 1) * scale_x - 1)) / (320 * scale_x);
		ey = (ey * ((200 + 1) * scale_y - 1)) / (200 * scale_y);

		_gfxr_draw_subline(pic, x, y, ex, ey, color, priority, drawenable);
	} else {
		if (x == ex && y == ey) { /* Just one single point? */
			rect_t drawrect;
			drawrect.x = x;
			drawrect.y = y;
			drawrect.xl = scale_x;
			drawrect.yl = scale_y;

			if (drawenable & GFX_MASK_VISUAL)
				gfx_draw_box_pixmap_i(pic->visual_map, drawrect, color);

			if (drawenable & GFX_MASK_PRIORITY)
				gfx_draw_box_pixmap_i(pic->priority_map, drawrect, priority);

		} else {
			int width = scale_x;
			int height = scale_y;
			int x_offset = 0;
			int y_offset = 0;

			if (line_mode == GFX_LINE_MODE_FAST) {
				width = (width + 1) >> 1;
				height = (height + 1) >> 1;
				x_offset = (width >> 1);
				y_offset = (height >> 1);
			}

			for (xc = 0; xc < width; xc++)
				_gfxr_draw_subline(pic,
						   x + xc + x_offset, y + y_offset,
						   ex + xc + x_offset, ey + y_offset,
						   color, priority, drawenable);

			if (height > 0)
				for (xc = 0; xc < width; xc++)
					_gfxr_draw_subline(pic,
							   x + xc + x_offset, y + height - 1 + y_offset,
							   ex + xc + x_offset, ey + height - 1 + y_offset,
							   color, priority, drawenable);

			if (height > 1) {
				for (yc = 1; yc < height - 1; yc++)
					_gfxr_draw_subline(pic,
							   x + x_offset, y + yc + y_offset,
							   ex + x_offset, ey + yc + y_offset,
							   color, priority, drawenable);
				if (width > 0)
					for (yc = 1; yc < height - 1; yc++)
						_gfxr_draw_subline(pic,
								   x + width - 1 + x_offset, y + yc + y_offset,
								   ex + width - 1 + x_offset, ey + yc + y_offset,
								   color, priority, drawenable);
			}
		}
	}

	p0printf("\n");
}


#define IS_FILL_BOUNDARY(x) (((x) & legalmask) != legalcolor)


#ifdef WITH_PIC_SCALING

#define TEST_POINT(xx, yy) \
  if (pic->aux_map[(yy)*320 + (xx)] & FRESH_PAINT) { \
    mpos = (((yy) * 320 * pic->mode->yfact) + (xx)) * pic->mode->xfact; \
    for (iy = 0; iy < pic->mode->yfact; iy++) { \
      for (ix = 0; ix < pic->mode->xfact; ix++) \
	if (!IS_FILL_BOUNDARY(test_map[mpos + ix])) { \
	  *x = ix + (xx) * pic->mode->xfact; \
	  *y = iy + (yy) * pic->mode->yfact; \
	  return 0; \
	} \
      mpos += linewidth; \
    } \
  }

static inline int /* returns -1 on failure, 0 on success */
_gfxr_find_fill_point(gfxr_pic_t *pic, int min_x, int min_y, int max_x, int max_y, int x_320,
		      int y_200, int color, int drawenable, int *x, int *y)
{
	int linewidth = pic->mode->xfact * 320;
	int mpos, ix, iy;
	int size_x = (max_x - min_x + 1) >> 1;
	int size_y = (max_y - min_y + 1) >> 1;
	int mid_x = min_x + size_x;
	int mid_y = min_y + size_y;
	int max_size = (size_x > size_y)? size_x : size_y;
	int size;
	int legalcolor;
	int legalmask;
	byte *test_map;
	*x = x_320 * pic->mode->xfact;
	*y = y_200 * pic->mode->yfact;

	if (size_x < 0 || size_y < 0)
		return 0;

	if (drawenable & GFX_MASK_VISUAL) {
		test_map = pic->visual_map->index_data;

		if ((color & 0xf) == 0xf /* When dithering with white, do more
					 ** conservative checks  */
		    || (color & 0xf0) == 0xf0)
			legalcolor = 0xff;
		else
			legalcolor = 0xf0; /* Only check the second color */

		legalmask = legalcolor;
	} else if (drawenable & GFX_MASK_PRIORITY) {
		test_map = pic->priority_map->index_data;
		legalcolor = 0;
		legalmask = 0xf;
	} else return -3;

	TEST_POINT(x_320, y_200); /* Most likely candidate */
	TEST_POINT(mid_x, mid_y); /* Second most likely candidate */

	for (size = 1; size <= max_size; size++) {
		int i;

		if (size <= size_y) {
			int limited_size = (size > size_x)? size_x : size;

			for (i = mid_x - limited_size; i <= mid_x + limited_size; i++) {
				TEST_POINT(i, mid_y - size);
				TEST_POINT(i, mid_y + size);
			}
		}

		if (size <= size_x) {
			int limited_size = (size - 1 > size_y)? size_y : size - 1;

			for (i = mid_y - limited_size; i <= mid_y + limited_size; i++) {
				TEST_POINT(mid_x - size, i);
				TEST_POINT(mid_x + size, i);
			}
		}
	}

	return -1;
}

#undef TEST_POINT

/* Now include the actual filling code (with scaling support) */
#define FILL_FUNCTION _gfxr_fill_any
#define FILL_FUNCTION_RECURSIVE _gfxr_fill_any_recursive
#define AUXBUF_FILL_HELPER _gfxr_auxbuf_fill_any_recursive
#define AUXBUF_FILL _gfxr_auxbuf_fill_any
#define DRAW_SCALED
# include "sci_picfill_aux.c"
# include "sci_picfill.c"
#undef DRAW_SCALED
#undef AUXBUF_FILL
#undef AUXBUF_FILL_HELPER
#undef FILL_FUNCTION_RECURSIVE
#undef FILL_FUNCTION

#endif /* defined(WITH_PIC_SCALING) */

/* Include again, but this time without support for scaling */
#define FILL_FUNCTION _gfxr_fill_1
#define FILL_FUNCTION_RECURSIVE _gfxr_fill_1_recursive
#define AUXBUF_FILL_HELPER _gfxr_auxbuf_fill_1_recursive
#define AUXBUF_FILL _gfxr_auxbuf_fill_1
# include "sci_picfill_aux.c"
# include "sci_picfill.c"
#undef AUXBUF_FILL
#undef AUXBUF_FILL_HELPER
#undef FILL_FUNCTION_RECURSIVE
#undef FILL_FUNCTION


#define GET_ABS_COORDS(x, y) \
  temp = *(resource + pos++); \
  x = *(resource + pos++); \
  y = *(resource + pos++); \
  x |= (temp & 0xf0) << 4; \
  y |= (temp & 0x0f) << 8;

#define GET_REL_COORDS(x, y) \
  temp = *(resource + pos++); \
  if (temp & 0x80) \
    x -= ((temp >> 4) & 0x7); \
  else \
    x += (temp >> 4); \
  \
  if (temp & 0x08) \
    y -= (temp & 0x7); \
  else \
    y += (temp & 0x7);

#define GET_MEDREL_COORDS(oldx, oldy) \
  temp = *(resource + pos++); \
  if (temp & 0x80) \
    y = oldy - (temp & 0x7f); \
  else \
    y = oldy + temp; \
  x = oldx + *((signed char *) resource + pos++);


inline static void
check_and_remove_artifact(byte *dest, byte* srcp, int legalcolor, byte l, byte r, byte u, byte d)
{
	if (*dest == legalcolor) {
		if (*srcp == legalcolor)
			return;
		if (l) {
			if (srcp[-1] == legalcolor)
				return;
			if (u && srcp[-320 - 1] == legalcolor)
				return;
			if (d && srcp[320 - 1] == legalcolor)
				return;
		}
		if (r) {
			if (srcp[1] == legalcolor)
				return;
			if (u && srcp[-320 + 1] == legalcolor)
				return;
			if (d && srcp[320 + 1] == legalcolor)
				return;
		}

		if (u && srcp[-320] == legalcolor)
			return;

		if (d && srcp[-320] == legalcolor)
			return;

		*dest = *srcp;
	}
}


void
gfxr_remove_artifacts_pic0(gfxr_pic_t *dest, gfxr_pic_t *src)
{
	int x_320, y_200;
	int bound_x = dest->mode->xfact;
	int bound_y = dest->mode->yfact;
	int scaled_line_size = bound_x * 320;
	int read_offset = 0;

	assert(src->mode->xfact == 1);
	assert(src->mode->yfact == 1);

	if (bound_x == 1 && bound_y == 1) {
		/* D'Oh! */
		GFXWARN("attempt to remove artifacts from unscaled pic!\n");
		return;
	}

	for (y_200 = 0; y_200 < 200; y_200++) {
		for (x_320 = 0; x_320 < 320; x_320++) {
			int write_offset = (y_200 * bound_y * scaled_line_size) + (x_320 * bound_x);
			int sub_x, sub_y;
			byte *src_visualp = &(src->visual_map->index_data[read_offset]);
			byte *src_priorityp = &(src->priority_map->index_data[read_offset]);

			for (sub_y = 0; sub_y < bound_y; sub_y++) {
				for (sub_x = 0; sub_x < bound_x; sub_x++) {
					check_and_remove_artifact(dest->visual_map->index_data + write_offset,
								  src_visualp, (int)0xff,
								  (byte)x_320, (byte)(x_320 < 319), (byte)(y_200 > 10), (byte)(y_200 < 199));
					check_and_remove_artifact(dest->priority_map->index_data + write_offset,
								  src_priorityp, 0,
								  (byte)x_320, (byte)(x_320 < 319), (byte)(y_200 > 10), (byte)(y_200 < 199));
					++write_offset;
				}
				write_offset += scaled_line_size - bound_x;
			}
			++read_offset;
		}
	}

}

static void
view_transparentize(gfx_pixmap_t *view, byte *pic_index_data,
		    int posx, int posy,
		    int width, int height)
{
	int i,j;

	for (i=0;i<width;i++)
		for (j=0;j<height;j++)
		{
			if (view->index_data[j*width+i] == view->color_key)
			{
				view->index_data[j*width+i] = 
					pic_index_data[(j+posy)*width+i+posx];
			}
		}
}

extern gfx_pixmap_t *
gfxr_draw_cel0(int id, int loop, int cel, byte *resource, int size, gfxr_view_t *view, int mirrored);
extern gfx_pixmap_t *
gfxr_draw_cel1(int id, int loop, int cel, int mirrored, byte *resource, int size, gfxr_view_t *view, int amiga_game);
extern void
_gfx_crossblit_simple(byte *dest, byte *src, int dest_line_width, int src_line_width, int xl, int yl, int bpp);

void
gfxr_draw_pic01(gfxr_pic_t *pic, int flags, int default_palette, int size,
		byte *resource, gfxr_pic0_params_t *style, int resid, int sci1,
		gfx_pixmap_color_t *static_pal, int static_pal_nr)
{
	const int default_palette_table[GFXR_PIC0_PALETTE_SIZE] = {
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0x88,
		0x88, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x88,
		0x88, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
		0x08, 0x91, 0x2a, 0x3b, 0x4c, 0x5d, 0x6e, 0x88
	};

	const int default_priority_table[GFXR_PIC0_PALETTE_SIZE] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
	};
	int palette[GFXR_PIC0_NUM_PALETTES][GFXR_PIC0_PALETTE_SIZE];
	int priority_table[GFXR_PIC0_PALETTE_SIZE];
	int i;
	int drawenable = GFX_MASK_VISUAL | GFX_MASK_PRIORITY;
	int priority = 0;
	int color = 0;
	int pattern_nr = 0;
	int pattern_code = 0;
	int pattern_size = 0;
	int control = 0;
	int pos = 0;
	int x, y;
	int oldx, oldy;
	int pal, index;
	int temp;
	int line_mode = style->line_mode;
	int sci_titlebar_size = style->pic_port_bounds.y;
	int fill_count = 0;
	byte op, opx;

#ifdef FILL_RECURSIVE_DEBUG
	fillmagc = atoi(getenv("FOO"));
	fillc = atoi(getenv("FOO2"));
#endif /* FILL_RECURSIVE_DEBUG */

	/* Initialize palette */
	for (i = 0; i < GFXR_PIC0_NUM_PALETTES; i++)
		memcpy(palette[i], default_palette_table, sizeof(int) * GFXR_PIC0_PALETTE_SIZE);

	memcpy(priority_table, default_priority_table, sizeof(int) * GFXR_PIC0_PALETTE_SIZE);

	/* Main loop */
	while (pos < size) {
		op = *(resource + pos++);

		switch (op) {

		case PIC_OP_SET_COLOR:
			p0printf("Set color @%d\n", pos);

			if (!sci1) {
				pal = *(resource + pos++);
				index = pal % GFXR_PIC0_PALETTE_SIZE;
				pal /= GFXR_PIC0_PALETTE_SIZE;

				pal += default_palette;

				if (pal >= GFXR_PIC0_NUM_PALETTES) {
					GFXERROR("Attempt to access invalid palette %d\n", pal);
					return;
				}

				color = palette[pal][index];
			} else color = *(resource + pos++);
			p0printf("  color <- %02x [%d/%d]\n", color, pal, index);
			drawenable |= GFX_MASK_VISUAL;
			goto end_op_loop;


		case PIC_OP_DISABLE_VISUAL:
			p0printf("Disable visual @%d\n", pos);
			drawenable &= ~GFX_MASK_VISUAL;
			goto end_op_loop;


		case PIC_OP_SET_PRIORITY:
			p0printf("Set priority @%d\n", pos);

			if (!sci1) {
				pal = *(resource + pos++);
				index = pal % GFXR_PIC0_PALETTE_SIZE;
				pal /= GFXR_PIC0_PALETTE_SIZE; /* Ignore pal */
				
				priority = priority_table[index];
			} else priority = *(resource + pos++);

			p0printf("  priority <- %d [%d/%d]\n", priority, pal, index);
			drawenable |= GFX_MASK_PRIORITY;
			goto end_op_loop;


		case PIC_OP_DISABLE_PRIORITY:
			p0printf("Disable priority @%d\n", pos);
			drawenable &= ~GFX_MASK_PRIORITY;
			goto end_op_loop;


		case PIC_OP_SHORT_PATTERNS:
			p0printf("Short patterns @%d\n", pos);
			if (pattern_code & PATTERN_FLAG_USE_PATTERN) {
				pattern_nr = ((*(resource + pos++)) >> 1) & 0x7f;
				p0printf("  pattern_nr <- %d\n", pattern_nr);
			}

			GET_ABS_COORDS(x, y);

			_gfxr_draw_pattern(pic, x, y, color, priority, control, drawenable, pattern_code,
					   pattern_size, pattern_nr, style->brush_mode, sci_titlebar_size);

			while (*(resource + pos) < PIC_OP_FIRST) {
				if (pattern_code & PATTERN_FLAG_USE_PATTERN) {
					pattern_nr = ((*(resource + pos++)) >> 1) & 0x7f;
					p0printf("  pattern_nr <- %d\n", pattern_nr);
				}

				GET_REL_COORDS(x, y);

				_gfxr_draw_pattern(pic, x, y, color, priority, control, drawenable, pattern_code,
						   pattern_size, pattern_nr, style->brush_mode, sci_titlebar_size);
			}
			goto end_op_loop;


		case PIC_OP_MEDIUM_LINES:
			p0printf("Medium lines @%d\n", pos);
			GET_ABS_COORDS(oldx, oldy);
			while (*(resource + pos) < PIC_OP_FIRST) {
#if 0
				fprintf(stderr,"Medium-line: [%04x] from %d,%d, data %02x %02x (dx=%d)", pos, oldx, oldy,
					0xff & resource[pos], 0xff & resource[pos+1], *((signed char *) resource + pos + 1));
#endif
				GET_MEDREL_COORDS(oldx, oldy);
#if 0
				fprintf(stderr, " to %d,%d\n", x, y);
#endif
				_gfxr_draw_line(pic, oldx, oldy, x, y, color, priority, control, drawenable, line_mode, 
						PIC_OP_MEDIUM_LINES, sci_titlebar_size);
				oldx = x; oldy = y;
			}
			goto end_op_loop;


		case PIC_OP_LONG_LINES:
			p0printf("Long lines @%d\n", pos);
			GET_ABS_COORDS(oldx, oldy);
			while (*(resource + pos) < PIC_OP_FIRST) {
				GET_ABS_COORDS(x,y);
				_gfxr_draw_line(pic, oldx, oldy, x, y, color, priority, control, drawenable, line_mode, 
						PIC_OP_LONG_LINES, sci_titlebar_size);
				oldx = x; oldy = y;
			}
			goto end_op_loop;


		case PIC_OP_SHORT_LINES:
			p0printf("Short lines @%d\n", pos);
			GET_ABS_COORDS(oldx, oldy);
			x = oldx; y = oldy;
			while (*(resource + pos) < PIC_OP_FIRST) {
				GET_REL_COORDS(x,y);
				_gfxr_draw_line(pic, oldx, oldy, x, y, color, priority, control, drawenable, line_mode, 
						PIC_OP_SHORT_LINES, sci_titlebar_size);
				oldx = x; oldy = y;
			}
			goto end_op_loop;


		case PIC_OP_FILL:
			p0printf("Fill @%d\n", pos);
			while (*(resource + pos) < PIC_OP_FIRST) {
				/*fprintf(stderr,"####################\n"); */
				GET_ABS_COORDS(x, y);
				p0printf("Abs coords %d,%d\n", x, y);
				/*fprintf(stderr,"C=(%d,%d)\n", x, y + sci_titlebar_size);*/
#ifdef WITH_PIC_SCALING
				if (pic->mode->xfact > 1
				    || pic->mode->yfact > 1)
				_gfxr_fill_any(pic, x, y + sci_titlebar_size, (flags & DRAWPIC01_FLAG_FILL_NORMALLY)? 
					       color : 0, priority, control, drawenable, sci_titlebar_size);

				else
#endif
				_gfxr_fill_1(pic, x, y + sci_titlebar_size, (flags & DRAWPIC01_FLAG_FILL_NORMALLY)? 
					     color : 0, priority, control, drawenable, sci_titlebar_size);

				if (fill_count++ > SCI_PIC0_MAX_FILL) {
					sci_sched_yield();
					fill_count = 0;
				}

#ifdef FILL_RECURSIVE_DEBUG
				if (!fillmagc) {
					int x,y;
					if (getenv("FOO1"))
						for (x = 0; x < 320; x++)
							for (y = 0; y < 200; y++) {
								int aux = pic->aux_map[x + y*320];
								int pix = (aux & 0xf);
								int i;

								if (aux & 0x40) {
									if (x == 0 || !(pic->aux_map[x-1 + y * 320] & 0x40))
										for (i = 0; i < pic->mode->yfact; i++)
											pic->visual_map->index_data[(x + ((y*pic->mode->yfact)+i)*320) * pic->mode->xfact] ^= 0xff;

									if (x == 319 || !(pic->aux_map[x+1 + y * 320] & 0x40))
										for (i = 0; i < pic->mode->yfact; i++)
											pic->visual_map->index_data[pic->mode->xfact - 1 +(x + ((y*pic->mode->yfact)+i)*320) * pic->mode->xfact] ^= 0xff;

									if (y == 0 || !(pic->aux_map[x + (y-1) * 320] & 0x40))
										for (i = 0; i < pic->mode->yfact; i++)
											pic->visual_map->index_data[i+(x + ((y*pic->mode->yfact))*320) * pic->mode->xfact] ^= 0xff;

									if (y == 199 || !(pic->aux_map[x + (y+1) * 320] & 0x40))
										for (i = 0; i < pic->mode->yfact; i++)
											pic->visual_map->index_data[i+(x + ((y*pic->mode->yfact)+pic->mode->yfact - 1)*320) * pic->mode->xfact] ^= 0xff;
								}

								pix |= (aux & 0x40) >> 4;
								pix |= (pix << 4);

								pic->visual_map->index_data[x + y*320*pic->mode->xfact] = pix;
							}
					return;
				} --fillmagc;
#endif /* GFXR_DEBUG_PIC0 */
			}
			goto end_op_loop;


		case PIC_OP_SET_PATTERN:
			p0printf("Set pattern @%d\n", pos);
			pattern_code = (*(resource + pos++));
			pattern_size = pattern_code & 0x07;
			goto end_op_loop;


		case PIC_OP_ABSOLUTE_PATTERN:
			p0printf("Absolute pattern @%d\n", pos);
			while (*(resource + pos) < PIC_OP_FIRST) {
				if (pattern_code & PATTERN_FLAG_USE_PATTERN) {
					pattern_nr = ((*(resource + pos++)) >> 1) & 0x7f;
					p0printf("  pattern_nr <- %d\n", pattern_nr);
				}

				GET_ABS_COORDS(x, y);

				_gfxr_draw_pattern(pic, x, y, color, priority, control, drawenable, pattern_code,
						   pattern_size, pattern_nr, style->brush_mode, sci_titlebar_size);
			}
			goto end_op_loop;


		case PIC_OP_SET_CONTROL:
			p0printf("Set control @%d\n", pos);
			control = (*(resource + pos++)) & 0xf;
			drawenable |= GFX_MASK_CONTROL;
			goto end_op_loop;


		case PIC_OP_DISABLE_CONTROL:
			p0printf("Disable control @%d\n", pos);
			drawenable &= ~GFX_MASK_CONTROL;
			goto end_op_loop;


		case PIC_OP_MEDIUM_PATTERNS:
			p0printf("Medium patterns @%d\n", pos);
			if (pattern_code & PATTERN_FLAG_USE_PATTERN) {
				pattern_nr = ((*(resource + pos++)) >> 1) & 0x7f;
				p0printf("  pattern_nr <- %d\n", pattern_nr);
			}

			GET_ABS_COORDS(oldx, oldy);

			_gfxr_draw_pattern(pic, oldx, oldy, color, priority, control, drawenable, pattern_code,
					   pattern_size, pattern_nr, style->brush_mode, sci_titlebar_size);

			x = oldx; y = oldy;
			while (*(resource + pos) < PIC_OP_FIRST) {
				if (pattern_code & PATTERN_FLAG_USE_PATTERN) {
					pattern_nr = ((*(resource + pos++)) >> 1) & 0x7f;
					p0printf("  pattern_nr <- %d\n", pattern_nr);
				}

				GET_MEDREL_COORDS(x, y);

				_gfxr_draw_pattern(pic, x, y, color, priority, control, drawenable, pattern_code,
						   pattern_size, pattern_nr, style->brush_mode, sci_titlebar_size);
			}
			goto end_op_loop;


		case PIC_OP_OPX:
			opx = *(resource + pos++);
			p0printf("OPX: ");

			if (sci1) opx += SCI1_OP_OFFSET; /* See comment at the definition of SCI1_OP_OFFSET. */

			switch (opx) {

			case PIC_SCI1_OPX_SET_PALETTE_ENTRIES:
				GFXWARN("SCI1 Set palette entried not implemented\n");
				goto end_op_loop;

			case PIC_SCI0_OPX_SET_PALETTE_ENTRIES:
				p0printf("Set palette entry @%d\n", pos);
				while (*(resource + pos) < PIC_OP_FIRST) {
					index = *(resource + pos++);
					pal = index / GFXR_PIC0_PALETTE_SIZE;
					index %= GFXR_PIC0_PALETTE_SIZE;

					if (pal >= GFXR_PIC0_NUM_PALETTES) {
						GFXERROR("Attempt to write to invalid palette %d\n", pal);
						return;
					}
					palette[pal][index] = *(resource + pos++);
				}
				goto end_op_loop;


			case PIC_SCI0_OPX_SET_PALETTE:
				p0printf("Set palette @%d\n", pos);
				pal = *(resource + pos++);
				if (pal >= GFXR_PIC0_NUM_PALETTES) {
					GFXERROR("Attempt to write to invalid palette %d\n", pal);
					return;
				}

				p0printf("  palette[%d] <- (", pal);
				for (index = 0; index < GFXR_PIC0_PALETTE_SIZE; index++) {
					palette[pal][index] = *(resource + pos++);
					if (index > 0)
						p0printf(",");
					if (!(index & 0x7))
						p0printf("[%d]=", index);
					p0printf("%02x", palette[pal][index]);
				}
				p0printf(")\n");
				goto end_op_loop;

			case PIC_SCI1_OPX_SET_PALETTE:
				p0printf("Set palette @%d\n", pos);
				pic->visual_map->flags &= ~GFX_PIXMAP_FLAG_EXTERNAL_PALETTE;
				pic->visual_map->colors = gfxr_read_pal1(resid, &pic->visual_map->colors_nr,
									 resource+pos, SCI1_PALETTE_SIZE);
				pos += SCI1_PALETTE_SIZE;
				goto end_op_loop;

			case PIC_SCI0_OPX_MONO0:
				p0printf("Monochrome opx 0 @%d\n", pos);
				pos += 41;
				goto end_op_loop;


			case PIC_SCI0_OPX_MONO1:
			case PIC_SCI0_OPX_MONO3:
				++pos;
				p0printf("Monochrome opx %d @%d\n", opx, pos);
				goto end_op_loop;


			case PIC_SCI0_OPX_MONO2:
			case PIC_SCI0_OPX_MONO4: /* Monochrome ops: Ignored by us */
				p0printf("Monochrome opx %d @%d\n", opx, pos);
				goto end_op_loop;


			case PIC_SCI0_OPX_EMBEDDED_VIEW:
			case PIC_SCI1_OPX_EMBEDDED_VIEW:
			{
				int posx, posy;
				int bytesize;
/*				byte *vismap = pic->visual_map->index_data; */
				int nodraw = 0;

				gfx_pixmap_t *view;
				gfx_mode_t *mode;

				p0printf("Embedded view @%d\n", pos);

				/* Set up mode structure for resizing the view */
				mode = gfx_new_mode(
					pic->visual_map->index_xl/320,
					pic->visual_map->index_yl/200, 
					1, /* 1bpp, which handles masks and the rest for us */
					0, 0, 0, 0, 0, 0, 0, 0, 16, 0);

				GET_ABS_COORDS(posx, posy);
				bytesize = (*(resource + pos))+(*(resource + pos + 1) << 8);
				p0printf("(%d, %d)\n", posx, posy);
				pos += 2;
				if (!sci1 && !nodraw) 
					view = gfxr_draw_cel0(-1,-1,-1, resource + pos, bytesize, NULL, 0); 
				else
					view = gfxr_draw_cel1(-1,-1,-1, 0, resource + pos, bytesize, NULL,
							      static_pal_nr == GFX_SCI1_AMIGA_COLORS_NR);
				pos+=bytesize;
				if (nodraw) continue;
				p0printf("(%d, %d)-(%d, %d)\n", 
					 posx, 
					 posy, 
					 posx + view->index_xl,
					 posy + view->index_yl);

				/* we can only safely replace the palette if it's static
				 *if it's not for some reason, we should die
				 */
				if (!(view->flags & GFX_PIXMAP_FLAG_EXTERNAL_PALETTE) && !sci1) {
					sciprintf("gfx_draw_pic0(): can't set a non-static palette for an embedded view!\n");
				}

				/* For SCI0, use special color mapping to copy the low
				** nibble of the color index to the high
				** nibble. 
				*/
				if (sci1) {
					if (static_pal_nr == GFX_SCI1_AMIGA_COLORS_NR) {
						/* Assume Amiga game */
						pic->visual_map->colors = static_pal;
						pic->visual_map->colors_nr = static_pal_nr;
						pic->visual_map->flags |= GFX_PIXMAP_FLAG_EXTERNAL_PALETTE;
					}
					view->colors = pic->visual_map->colors;
					view->colors_nr = pic->visual_map->colors_nr;
				} else
					view->colors = embedded_view_colors;

				/* Hack to prevent overflowing the visual map buffer.
				   Yes, this does happen otherwise. */
				if (view->index_yl + sci_titlebar_size > 200)
					sci_titlebar_size = 0;

				gfx_xlate_pixmap(view, mode, GFX_XLATE_FILTER_NONE);

				if (flags & DRAWPIC01_FLAG_OVERLAID_PIC)
					view_transparentize(view, pic->visual_map->index_data, 
							    posx, sci_titlebar_size+posy, 
							    view->index_xl, view->index_yl);

				_gfx_crossblit_simple(pic->visual_map->index_data+(sci_titlebar_size*320)+
						      posy*320+posx,
						      view->index_data,
						      pic->visual_map->index_xl, view->index_xl,
						      view->index_xl,
						      view->index_yl,
						      1);

				gfx_free_mode(mode);
				gfx_free_pixmap(NULL, view);
			}
			goto end_op_loop;

			case PIC_SCI0_OPX_SET_PRIORITY_TABLE: 
			case PIC_SCI1_OPX_PRIORITY_TABLE_EXPLICIT: {
				int i;
				int *pri_table;

				p0printf("Explicit priority table @%d\n", pos);
				if (!pic->internal)
				{
					pic->internal = sci_malloc(16 * sizeof(int)); 
				} else
				{
					GFXERROR("pic->internal is not NULL (%08x); this only occurs with overlaid pics, otherwise it's a bug!\n", pic->internal); 
				}	

				pri_table = (int*)pic->internal;

				pri_table[0] = 0;
				pri_table[15] = 190;

				for (i = 1; i < 15; i++)
					pri_table[i] = resource[pos++];
			}
				goto end_op_loop;

			case PIC_SCI1_OPX_PRIORITY_TABLE_EQDIST:
			{
				int first = getInt16(resource + pos);
				int last = getInt16(resource + pos + 2);
				int nr;
				int *pri_table;

				if (!pic->internal)
				{
					pic->internal = sci_malloc(16 * sizeof(int)); 
				} else
				{
					GFXERROR("pic->internal is not NULL (%08x); possible memory corruption!\n", pic->internal); 
				}	

				pri_table = (int*)pic->internal;
				
				for (nr = 0; nr < 16; nr ++)
					pri_table[nr] = SCI0_PRIORITY_BAND_FIRST_14_ZONES(nr);					
				pos += 4;
				goto end_op_loop;
			}

			default: sciprintf("%s L%d: Warning: Unknown opx %02x\n", __FILE__, __LINE__, opx);
				return;
			}
			goto end_op_loop;

		case PIC_OP_TERMINATE:
			p0printf("Terminator\n");
			/* WARNING( "ARTIFACT REMOVAL CODE is commented out!") */
			/* _gfxr_vismap_remove_artifacts(); */
			return;

		default: GFXWARN("Unknown op %02x\n", op);
			return;
		}
end_op_loop: 
		{}
	}

	GFXWARN("Reached end of pic resource %04x\n", resid);
}

void
gfxr_draw_pic11(gfxr_pic_t *pic, int flags, int default_palette, int size,
	       byte *resource, gfxr_pic0_params_t *style, int resid,
	       gfx_pixmap_color_t *static_pal, int static_pal_nr)
{
	int has_bitmap = getUInt16(resource + 4);
	int vector_data_ptr = getUInt16(resource + 16);
	int palette_data_ptr = getUInt16(resource + 28);
	int bitmap_data_ptr = getUInt16(resource + 32);
	int sci_titlebar_size = style->pic_port_bounds.y;
	gfx_mode_t *mode;
	gfx_pixmap_t *view = NULL;
	/* Set up mode structure for resizing the view */
	mode = gfx_new_mode(
			    pic->visual_map->index_xl/320,
			    pic->visual_map->index_yl/200, 
			    1, /* 1bpp, which handles masks and the rest for us */
			    0, 0, 0, 0, 0, 0, 0, 0, 16, 0);

	pic->visual_map->colors = gfxr_read_pal11(-1, &(pic->visual_map->colors_nr), resource + palette_data_ptr, 1284);

	if (has_bitmap)
		view = gfxr_draw_cel11(-1, 0, 0, 0, resource, resource + bitmap_data_ptr, size - bitmap_data_ptr, NULL);

	if (view)
	{
		view->colors = pic->visual_map->colors;
		view->colors_nr = pic->visual_map->colors_nr;
		
		gfx_xlate_pixmap(view, mode, GFX_XLATE_FILTER_NONE);
		
		if (flags & DRAWPIC01_FLAG_OVERLAID_PIC)
			view_transparentize(view, pic->visual_map->index_data, 
					    0, 0, 
					    view->index_xl, view->index_yl);
		
		/* Hack to prevent overflowing the visual map buffer.
		   Yes, this does happen otherwise. */
		if (view->index_yl + sci_titlebar_size > 200)
			sci_titlebar_size = 0;

		_gfx_crossblit_simple(pic->visual_map->index_data+sci_titlebar_size*view->index_xl,
				      view->index_data,
				      pic->visual_map->index_xl, view->index_xl,
				      view->index_xl,
				      view->index_yl,
				      1);
	} else 
	{
		GFXWARN("No view was contained in SCI1.1 pic resource");
	}
		


	gfxr_draw_pic01(pic, flags, default_palette, size - vector_data_ptr,
			resource + vector_data_ptr, style, resid, 1,
			static_pal, static_pal_nr);
}

void
gfxr_dither_pic0(gfxr_pic_t *pic, int dmode, int pattern)
{
	int xl = pic->visual_map->index_xl;
	int yl = pic->visual_map->index_yl;
	int xfrob_max = (pattern == GFXR_DITHER_PATTERN_1)? 1 : pic->mode->xfact;
	int yfrob_max = (pattern == GFXR_DITHER_PATTERN_1)? 1 : pic->mode->yfact;
	int xfrobc = 0, yfrobc = 0;
	int selection = 0;
	int x, y;
	byte *data = pic->visual_map->index_data;

	if (dmode == GFXR_DITHER_MODE_F256)
		return; /* Nothing to do */

	if (dmode == GFXR_DITHER_MODE_D16) { /* Limit to 16 colors */
		pic->visual_map->colors = gfx_sci0_image_colors[sci0_palette];
		pic->visual_map->colors_nr = GFX_SCI0_IMAGE_COLORS_NR;
	}

	for (y = 0; y < yl; y++) {
		for (x = 0; x < xl; x++) {

			switch (dmode) {

			case GFXR_DITHER_MODE_D16:
				if (selection)
					*data = (*data & 0xf0) >> 4;
				else
					*data = (*data & 0xf);
				break;

			case GFXR_DITHER_MODE_D256:
				if (selection)
					*data = ((*data & 0xf) << 4) | ((*data & 0xf0) >> 4);
				break;

			default:
				GFXERROR("Invalid dither mode %d!\n", dmode);
				return;
			}

			++data;

			if (++xfrobc == xfrob_max) {
				selection = !selection;
				xfrobc = 0;
			}
		}

		if (++yfrobc == yfrob_max) {
			selection = !selection;
			yfrobc = 0;
		}
	}
}

