/***************************************************************************
 sci_font.c Copyright (C) 2000 Christoph Reichenbach


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
#include <gfx_system.h>
#include <gfx_resource.h>
#include <gfx_tools.h>


extern int font_counter;


#define FONT_HEIGHT_OFFSET 4
#define FONT_MAXCHAR_OFFSET 2

static int
calc_char(byte *dest, int total_width, int total_height, byte *src, int size)
{
	int width = src[0];
	int height = src[1];
	int byte_width = (width + 7) >> 3;
	int y;

	src += 2;

	if ((width >> 3) > total_width || height > total_height) {
		GFXERROR("Weird character: width=%d/%d, height=%d/%d\n", width, total_width, height, total_height);
		return GFX_ERROR;
	}

	if (byte_width * height + 2 > size) {
		GFXERROR("Character extends to %d of %d allowed bytes\n", byte_width * height + 2, size);
		return GFX_ERROR;
	}

	for (y = 0; y < height; y++) {
		memcpy(dest, src, byte_width);
		src += byte_width;
		dest += total_width;
	}

	return GFX_OK;
}


gfx_bitmap_font_t *
gfxr_read_font(int id, byte *resource, int size)
{
	gfx_bitmap_font_t *font = (gfx_bitmap_font_t*)sci_calloc(sizeof(gfx_bitmap_font_t), 1);
	int chars_nr;
	int max_width = 0, max_height;
	int i;

	++font_counter;

	if (size < 6) {
		GFXERROR("Font %04x size is %d- this is a joke, right?\n", id, size);
		gfxr_free_font(font);
		return NULL;
	}

	font->chars_nr = chars_nr = get_int_16(resource + FONT_MAXCHAR_OFFSET);
	font->line_height = max_height = get_int_16(resource + FONT_HEIGHT_OFFSET);

	if (chars_nr < 0 || chars_nr > 256 || max_height < 0) {
		if (chars_nr < 0 || chars_nr > 256)
			GFXERROR("Font %04x: Invalid number of characters: %d\n", id,
				 chars_nr);
		if (max_height < 0)
			GFXERROR("Font %04x: Invalid font height: %d\n", id, max_height);
		gfxr_free_font(font);
		return NULL;
	}

	if (size < 6 + chars_nr * 2) {
		GFXERROR("Font %04x: Insufficient space for %d characters in font\n",
			 id, chars_nr);
		gfxr_free_font(font);
		return NULL;
	}

	font->ID = id;
	font->widths = (int*)sci_malloc(sizeof(int) * chars_nr);

	for (i = 0; i < chars_nr; i++) {
		int offset = get_int_16(resource + (i << 1) + 6);

		if (offset >= size) {
			GFXERROR("Font %04x: Error: Character 0x%02x is at offset 0x%04x (beyond 0x%04x)\n",
				 id, i, offset, size);
			gfxr_free_font(font);
			return NULL;
		}

		if ((resource[offset]) > max_width)
			max_width = resource[offset];
		if ((resource[offset + 1]) > max_height)
			max_height = resource[offset + 1];

		font->widths[i] = resource[offset];
	}

	font->height = max_height;
	font->row_size = (max_width + 7) >> 3;

	if (font->row_size == 3)
		font->row_size = 4;

	if (font->row_size > 4)
		font->row_size = (font->row_size + 3) & ~3;

	font->char_size = font->row_size * max_height;
	font->data = (byte*)sci_calloc(font->char_size, chars_nr);

	for (i = 0; i < chars_nr; i++) {
		int offset = get_int_16(resource + (i << 1) + 6);

		if (calc_char(font->data + (font->char_size * i), font->row_size, max_height,
			      resource + offset, size - offset)) {
			GFXERROR("Problem occured in font %04x, char %d/%d\n", id, i, chars_nr);
			gfxr_free_font(font);
			return NULL;
		}
	}

	return font;
}


