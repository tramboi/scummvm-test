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

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#include <libart_lgpl/art_vpath_bpath.h>
#include <libart_lgpl/art_svp_vpath.h>
#include <libart_lgpl/art_svp_vpath_stroke.h>
#include <libart_lgpl/art_svp_render_aa.h>
#include <libart_lgpl/art_rgb_svp.h>
#include <libart_lgpl/art_rgb.h>

#include "sword25/gfx/image/vectorimage.h"

namespace Sword25 {

void
art_rgb_fill_run1(art_u8 *buf, art_u8 r, art_u8 g, art_u8 b, int n) {
	int i;

	if (r == g && g == b && r == 255) {
		memset(buf, g, n + n + n + n);
	} else {
		art_u32 *alt = (art_u32 *)buf;
		//art_u32 color = (r << 24) | (g << 16) | (b << 8) | 0xff;
		art_u32 color = (r << 0) | (g << 8) | (b << 16) | (0xff << 24);
		for (i = 0; i < n; i++)
			*alt++ = color;
	}
}

void
art_rgb_run_alpha1(art_u8 *buf, art_u8 r, art_u8 g, art_u8 b, int alpha, int n) {
	int i;
	int v;

	for (i = 0; i < n; i++) {
		v = *buf;
		*buf++ = v + (((r - v) * alpha + 0x80) >> 8);
		v = *buf;
		*buf++ = v + (((g - v) * alpha + 0x80) >> 8);
		v = *buf;
		*buf++ = v + (((b - v) * alpha + 0x80) >> 8);
		v = *buf;
		*buf++ = v + (((alpha - v) * alpha + 0x80) >> 8);
	}
}

typedef struct _ArtRgbSVPAlphaData ArtRgbSVPAlphaData;

struct _ArtRgbSVPAlphaData {
	int alphatab[256];
	art_u8 r, g, b, alpha;
	art_u8 *buf;
	int rowstride;
	int x0, x1;
};

static void
art_rgb_svp_alpha_callback1(void *callback_data, int y,
                            int start, ArtSVPRenderAAStep *steps, int n_steps) {
	ArtRgbSVPAlphaData *data = (ArtRgbSVPAlphaData *)callback_data;
	art_u8 *linebuf;
	int run_x0, run_x1;
	art_u32 running_sum = start;
	int x0, x1;
	int k;
	art_u8 r, g, b;
	int *alphatab;
	int alpha;

	linebuf = data->buf;
	x0 = data->x0;
	x1 = data->x1;

	r = data->r;
	g = data->g;
	b = data->b;
	alphatab = data->alphatab;

	if (n_steps > 0) {
		run_x1 = steps[0].x;
		if (run_x1 > x0) {
			alpha = (running_sum >> 16) & 0xff;
			if (alpha)
				art_rgb_run_alpha1(linebuf, r, g, b, alphatab[alpha], run_x1 - x0);
		}

		for (k = 0; k < n_steps - 1; k++) {
			running_sum += steps[k].delta;
			run_x0 = run_x1;
			run_x1 = steps[k + 1].x;
			if (run_x1 > run_x0) {
				alpha = (running_sum >> 16) & 0xff;
				if (alpha)
					art_rgb_run_alpha1(linebuf + (run_x0 - x0) * 4, r, g, b, alphatab[alpha], run_x1 - run_x0);
			}
		}
		running_sum += steps[k].delta;
		if (x1 > run_x1) {
			alpha = (running_sum >> 16) & 0xff;
			if (alpha)
				art_rgb_run_alpha1(linebuf + (run_x1 - x0) * 4, r, g, b, alphatab[alpha], x1 - run_x1);
		}
	} else {
		alpha = (running_sum >> 16) & 0xff;
		if (alpha)
			art_rgb_run_alpha1(linebuf, r, g, b, alphatab[alpha], x1 - x0);
	}

	data->buf += data->rowstride;
}

static void
art_rgb_svp_alpha_opaque_callback1(void *callback_data, int y,
                                   int start,
                                   ArtSVPRenderAAStep *steps, int n_steps) {
	ArtRgbSVPAlphaData *data = (ArtRgbSVPAlphaData *)callback_data;
	art_u8 *linebuf;
	int run_x0, run_x1;
	art_u32 running_sum = start;
	int x0, x1;
	int k;
	art_u8 r, g, b;
	int *alphatab;
	int alpha;

	linebuf = data->buf;
	x0 = data->x0;
	x1 = data->x1;

	r = data->r;
	g = data->g;
	b = data->b;
	alphatab = data->alphatab;

	if (n_steps > 0) {
		run_x1 = steps[0].x;
		if (run_x1 > x0) {
			alpha = running_sum >> 16;
			if (alpha) {
				if (alpha >= 255)
					art_rgb_fill_run1(linebuf, r, g, b, run_x1 - x0);
				else
					art_rgb_run_alpha1(linebuf, r, g, b, alphatab[alpha], run_x1 - x0);
			}
		}

		for (k = 0; k < n_steps - 1; k++) {
			running_sum += steps[k].delta;
			run_x0 = run_x1;
			run_x1 = steps[k + 1].x;
			if (run_x1 > run_x0) {
				alpha = running_sum >> 16;
				if (alpha) {
					if (alpha >= 255)
						art_rgb_fill_run1(linebuf + (run_x0 - x0) * 4, r, g, b, run_x1 - run_x0);
					else
						art_rgb_run_alpha1(linebuf + (run_x0 - x0) * 4, r, g, b, alphatab[alpha], run_x1 - run_x0);
				}
			}
		}
		running_sum += steps[k].delta;
		if (x1 > run_x1) {
			alpha = running_sum >> 16;
			if (alpha) {
				if (alpha >= 255)
					art_rgb_fill_run1(linebuf + (run_x1 - x0) * 4, r, g, b, x1 - run_x1);
				else
					art_rgb_run_alpha1(linebuf + (run_x1 - x0) * 4, r, g, b, alphatab[alpha], x1 - run_x1);
			}
		}
	} else {
		alpha = running_sum >> 16;
		if (alpha) {
			if (alpha >= 255)
				art_rgb_fill_run1(linebuf, r, g, b, x1 - x0);
			else
				art_rgb_run_alpha1(linebuf, r, g, b, alphatab[alpha], x1 - x0);
		}
	}

	data->buf += data->rowstride;
}

void
art_rgb_svp_alpha1(const ArtSVP *svp,
                   int x0, int y0, int x1, int y1,
                   art_u32 rgba,
                   art_u8 *buf, int rowstride,
                   ArtAlphaGamma *alphagamma) {
	ArtRgbSVPAlphaData data;
	int r, g, b, alpha;
	int i;
	int a, da;

	r = rgba >> 24;
	g = (rgba >> 16) & 0xff;
	b = (rgba >> 8) & 0xff;
	alpha = rgba & 0xff;

	data.r = r;
	data.g = g;
	data.b = b;
	data.alpha = alpha;

	a = 0x8000;
	da = (alpha * 66051 + 0x80) >> 8; /* 66051 equals 2 ^ 32 / (255 * 255) */

	for (i = 0; i < 256; i++) {
		data.alphatab[i] = a >> 16;
		a += da;
	}

	data.buf = buf;
	data.rowstride = rowstride;
	data.x0 = x0;
	data.x1 = x1;
	if (alpha == 255)
		art_svp_render_aa(svp, x0, y0, x1, y1, art_rgb_svp_alpha_opaque_callback1, &data);
	else
		art_svp_render_aa(svp, x0, y0, x1, y1, art_rgb_svp_alpha_callback1, &data);
}

void VectorImage::render(float scaleFactorX, float scaleFactorY, uint &width, uint &height) {
	width = static_cast<uint>(getWidth() * scaleFactorX);
	height = static_cast<uint>(getHeight() * scaleFactorY);

	if (_pixelData)
		free(_pixelData);

	_pixelData = (byte *)malloc(width * height * 4);
	memset(_pixelData, 0, width * height * 4);

	for (int j = _elements.size() - 1; j >= 0; j--)
		for (int i = _elements[j].getPathCount() - 1; i >= 0; i--) {
			if (!_elements[j].getPathInfo(i).getVec())
				continue;

			bool needfree = false;
			ArtVpath *vec = _elements[j].getPathInfo(i).getVec();

			// Upscale vector
			if (scaleFactorX != 1.0 || scaleFactorY != 1.0) {
				ArtVpath *vec1;
				int size;

				for (size = 0; vec[size].code != ART_END; size++);

				vec1 = art_new(ArtVpath, size + 1);

				int k;
				for (k = 0; k < size; k++) {
					vec1[k].code = vec[k].code;
					vec1[k].x = vec[k].x * scaleFactorX;
					vec1[k].y = vec[k].y * scaleFactorY;
				}

				vec1[k].code = ART_END;

				vec = vec1;
				needfree = true;
			}

			ArtSVP *svp1 = art_svp_from_vpath(vec);

			int penWidth = _elements[j].getLineStyleWidth(_elements[j].getPathInfo(i).getLineStyle());
			ArtSVP *svp2 = art_svp_vpath_stroke(vec, ART_PATH_STROKE_JOIN_ROUND, ART_PATH_STROKE_CAP_ROUND, penWidth, 1.0, 0.5);
			if (needfree)
				art_free(vec);

			int color1 = _elements[j].getFillStyleColor(_elements[j].getPathInfo(i).getFillStyle0());
			int color2 = _elements[j].getLineStyleColor(_elements[j].getPathInfo(i).getLineStyle());

			art_rgb_svp_alpha1(svp1, 0, 0, width, height, color1, _pixelData, width * 4, NULL);
			art_rgb_svp_alpha1(svp2, 0, 0, width, height, color2, _pixelData, width * 4, NULL);

			art_free(svp2);
			art_free(svp1);
		}
}


} // End of namespace Sword25
