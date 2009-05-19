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

#include "kyra/screen_v2.h"

#include "common/endian.h"

namespace Kyra {

Screen_v2::Screen_v2(KyraEngine_v1 *vm, OSystem *system) : Screen(vm, system), _wsaFrameAnimBuffer(0) {
	_wsaFrameAnimBuffer = new uint8[1024];
	assert(_wsaFrameAnimBuffer);
}

Screen_v2::~Screen_v2() {
	delete[] _wsaFrameAnimBuffer;
}

uint8 *Screen_v2::generateOverlay(const uint8 *palette, uint8 *buffer, int startColor, uint16 factor) {
	if (!palette || !buffer)
		return buffer;

	factor = MIN<uint16>(255, factor);
	factor >>= 1;
	factor &= 0xFF;

	const byte col1 = palette[startColor * 3 + 0];
	const byte col2 = palette[startColor * 3 + 1];
	const byte col3 = palette[startColor * 3 + 2];

	uint8 *dst = buffer;
	*dst++ = 0;

	for (int i = 1; i != 255; ++i) {
		uint8 processedPalette[3];
		const uint8 *src = palette + i*3;
		byte col;

		col = *src++;
		col -= ((((col - col1) * factor) << 1) >> 8) & 0xFF;
		processedPalette[0] = col;

		col = *src++;
		col -= ((((col - col2) * factor) << 1) >> 8) & 0xFF;
		processedPalette[1] = col;

		col = *src++;
		col -= ((((col - col3) * factor) << 1) >> 8) & 0xFF;
		processedPalette[2] = col;

		*dst++ = findLeastDifferentColor(processedPalette, palette+3, 255)+1;
	}

	return buffer;
}

void Screen_v2::applyOverlay(int x, int y, int w, int h, int pageNum, const uint8 *overlay) {
	if (pageNum == 0 || pageNum == 1)
		addDirtyRect(x, y, w, h);

	uint8 *dst = getPagePtr(pageNum) + y * 320 + x;
	while (h--) {
		for (int wi = 0; wi < w; ++wi) {
			uint8 index = *dst;
			*dst++ = overlay[index];
		}
		dst += 320 - w;
	}
}

int Screen_v2::findLeastDifferentColor(const uint8 *paletteEntry, const uint8 *palette, uint16 numColors, bool skipSpecialColors) {
	int m = 0x7fff;
	int r = 0x101;

	for (int i = 0; i < numColors; i++) {
		if (skipSpecialColors && i >= 0xc0 && i <= 0xc3)
			continue;

		int v = paletteEntry[0] - *palette++;
		int c = v * v;
		v = paletteEntry[1] - *palette++;
		c += (v * v);
		v = paletteEntry[2] - *palette++;
		c += (v * v);

		if (c <= m) {
			m = c;
			r = i;
		}
	}

	return r;
}

void Screen_v2::getFadeParams(const uint8 *palette, int delay, int &delayInc, int &diff) {

	int maxDiff = 0;
	diff = 0;
	int len = _use16ColorMode ? 48 : 768;
	for (int i = 0; i < len; ++i) {
		diff = ABS(palette[i] - _screenPalette[i]);
		maxDiff = MAX(maxDiff, diff);
	}

	delayInc = delay << 8;
	if (maxDiff != 0) {
		delayInc /= maxDiff;
		delayInc = MIN(delayInc, 0x7FFF);
	}

	delay = delayInc;
	for (diff = 1; diff <= maxDiff; ++diff) {
		if (delayInc >= 256)
			break;
		delayInc += delay;
	}
}

void Screen_v2::copyWsaRect(int x, int y, int w, int h, int dimState, int plotFunc, const uint8 *src,
							int unk1, const uint8 *unkPtr1, const uint8 *unkPtr2) {
	uint8 *dstPtr = getPagePtr(_curPage);
	uint8 *origDst = dstPtr;

	const ScreenDim *dim = getScreenDim(dimState);
	int dimX1 = dim->sx << 3;
	int dimX2 = dim->w << 3;
	dimX2 += dimX1;

	int dimY1 = dim->sy;
	int dimY2 = dim->h;
	dimY2 += dimY1;

	int temp = y - dimY1;
	if (temp < 0) {
		if ((temp += h) <= 0)
			return;
		else {
			SWAP(temp, h);
			y += temp - h;
			src += (temp - h) * w;
		}
	}

	temp = dimY2 - y;
	if (temp <= 0)
		return;

	if (temp < h)
		h = temp;

	int srcOffset = 0;
	temp = x - dimX1;
	if (temp < 0) {
		temp = -temp;
		srcOffset = temp;
		x += temp;
		w -= temp;
	}

	int srcAdd = 0;

	temp = dimX2 - x;
	if (temp <= 0)
		return;

	if (temp < w) {
		SWAP(w, temp);
		temp -= w;
		srcAdd = temp;
	}

	dstPtr += y * SCREEN_W + x;
	uint8 *dst = dstPtr;

	if (_curPage == 0 || _curPage == 1)
		addDirtyRect(x, y, w, h);

	clearOverlayRect(_curPage, x, y, w, h);

	temp = h;
	int curY = y;
	while (h--) {
		src += srcOffset;
		++curY;
		int cW = w;

		switch (plotFunc) {
		case 0:
			memcpy(dst, src, cW);
			dst += cW; src += cW;
			break;

		case 1:
			while (cW--) {
				uint8 d = *src++;
				uint8 t = unkPtr1[d];
				if (t != 0xFF)
					d = unkPtr2[*dst + (t << 8)];
				*dst++ = d;
			}
			break;

		case 4:
			while (cW--) {
				uint8 d = *src++;
				if (d)
					*dst = d;
				++dst;
			}
			break;

		case 5:
			while (cW--) {
				uint8 d = *src++;
				if (d) {
					uint8 t = unkPtr1[d];
					if (t != 0xFF)
						d = unkPtr2[*dst + (t << 8)];
					*dst = d;
				}
				++dst;
			}
			break;

		case 8:
		case 9:
			while (cW--) {
				uint8 d = *src++;
				uint8 t = _shapePages[0][dst - origDst] & 7;
				if (unk1 < t && (curY > _maskMinY && curY < _maskMaxY))
					d = _shapePages[1][dst - origDst];
				*dst++ = d;
			}
			break;

		case 12:
		case 13:
			while (cW--) {
				uint8 d = *src++;
				if (d) {
					uint8 t = _shapePages[0][dst - origDst] & 7;
					if (unk1 < t && (curY > _maskMinY && curY < _maskMaxY))
						d = _shapePages[1][dst - origDst];
					*dst++ = d;
				} else {
					d = _shapePages[1][dst - origDst];
					*dst++ = d;
				}
			}
			break;

		default:
			break;
		}

		dst = (dstPtr += SCREEN_W);
		src += srcAdd;
	}
}

const uint8 *Screen_v2::getPtrToShape(const uint8 *shpFile, int shape) {
	uint16 shapes = READ_LE_UINT16(shpFile);

	if (shapes <= shape)
		return 0;

	uint32 offset = READ_LE_UINT32(shpFile + (shape << 2) + 2);

	return shpFile + offset + 2;
}

uint8 *Screen_v2::getPtrToShape(uint8 *shpFile, int shape) {
	uint16 shapes = READ_LE_UINT16(shpFile);

	if (shapes <= shape)
		return 0;

	uint32 offset = READ_LE_UINT32(shpFile + (shape << 2) + 2);

	return shpFile + offset + 2;
}

int Screen_v2::getShapeScaledWidth(const uint8 *shpFile, int scale) {
	int width = READ_LE_UINT16(shpFile+3);
	return (width * scale) >> 8;
}

int Screen_v2::getShapeScaledHeight(const uint8 *shpFile, int scale) {
	int height = shpFile[2];
	return (height * scale) >> 8;
}

uint16 Screen_v2::getShapeSize(const uint8 *shp) {

	return READ_LE_UINT16(shp+6);
}

uint8 *Screen_v2::makeShapeCopy(const uint8 *src, int index) {

	const uint8 *shape = getPtrToShape(src, index);
	if (!shape)
		return 0;

	int size = getShapeSize(shape);

	uint8 *copy = new uint8[size];
	assert(copy);
	memcpy(copy, shape, size);

	return copy;
}

int Screen_v2::getLayer(int x, int y) {
	if (x < 0)
		x = 0;
	else if (x >= 320)
		x = 319;
	if (y < 0)
		y = 0;
	else if (y >= 144)
		y = 143;

	uint8 pixel = *(getCPagePtr(5) + y * 320 + x);
	pixel &= 0x7F;
	pixel >>= 3;

	if (pixel < 1)
		pixel = 1;
	else if (pixel > 15)
		pixel = 15;
	return pixel;
}

int Screen_v2::getRectSize(int w, int h) {
	if (w > 320 || h > 200)
		return 0;
	return w*h;
}

void Screen_v2::setTextColorMap(const uint8 *cmap) {
	setTextColor(cmap, 0, 15);
}

void Screen_v2::wsaFrameAnimationStep(int x1, int y1, int x2, int y2,
	int w1, int h1, int w2, int h2, int srcPage, int dstPage, int dim) {

	if (!(w1 || h1 || w2 || h2))
		return;

	ScreenDim cdm = *getScreenDim(dim);
	cdm.sx <<= 3;
	cdm.w <<= 3;

	int na = 0, nb = 0, nc = w2;

	if (!calcBounds(cdm.w, cdm.h, x2, y2, w2, h2, na, nb, nc))
		return;

	const uint8 *src = getPagePtr(srcPage) + y1 * 320;
	uint8 *dst = getPagePtr(dstPage) + (y2 + cdm.sy) * 320;

	int u = -1;

	do {
		int t = (nb * h1) / h2;
		if (t != u) {
			u = t;
			const uint8 *s = src + x1 + t * 320;
			uint8 *dt = (uint8 *)_wsaFrameAnimBuffer;

			t = w2 - w1;
			if (!t) {
				memcpy(dt, s, w2);
			} else if (t > 0) {
				if (w1 == 1) {
                    memset(dt, *s, w2);
				} else {
					t = ((((((w2 - w1 + 1) & 0xffff) << 8) / w1) + 0x100) & 0xffff) << 8;
					int bp = 0;
					for (int i = 0; i < w1; i++) {
						int cnt = (t >> 16);
						bp += (t & 0xffff);
						if (bp > 0xffff) {
							bp -= 0xffff;
							cnt++;
						}
						memset(dt, *s++, cnt);
						dt += cnt;
					}
				}
			} else {
				if (w2 == 1) {
					*dt = *s;
				} else {
					t = (((((w1 - w2) & 0xffff) << 8) / w2) & 0xffff) << 8;
					int bp = 0;
					for (int i = 0; i < w2; i++) {
						*dt++ = *s++;
						bp += (t & 0xffff);
						if (bp > 0xffff) {
							bp -= 0xffff;
							s++;
						}
						s += (t >> 16);
					}
				}
			}
		}
		memcpy(dst + x2 + cdm.sx, _wsaFrameAnimBuffer + na, w2);
		dst += 320;
	} while (++nb < h2);
}

bool Screen_v2::calcBounds(int w0, int h0, int &x1, int &y1, int &w1, int &h1, int &x2, int &y2, int &w2) {
	x2 = 0;
	y2 = 0;
	w2 = w1;

	int t = x1 + w1;
	if (t < 1) {
		w1 = h1 = -1;
	} else {
		if (t <= x1) {
			x2 = w1 - t;
			w1 = t;
			x1 = 0;
		}
		t = w0 - x1;
		if (t < 1) {
			w1 = h1 = -1;
		} else {
			if (t <= w1) {
				w1 = t;
			}
			w2 -= w1;
			t = h1 + y1;
			if (t < 1) {
				w1 = h1 = -1;
			} else {
				if (t <= y1) {
					y2 = h1 - t;
					h1 = t;
					y1 = 0;
				}
                t = h0 - y1;
				if (t < 1) {
					w1 = h1 = -1;
				} else {
					if (t <= h1) {
						h1 = t;
					}
				}
			}
		}
	}

	return (w1 == -1) ? false : true;
}

void Screen_v2::checkedPageUpdate(int srcPage, int dstPage) {

	const uint32 *src = (const uint32 *)getPagePtr(srcPage);
	uint32 *dst = (uint32 *)getPagePtr(dstPage);
	uint32 *page0 = (uint32 *)getPagePtr(0);

	bool updated = false;

	for (int y = 0; y < 200; ++y) {
		for (int x = 0; x < 80; ++x, ++src, ++dst, ++page0) {
			if (*src != *dst) {
				updated = true;
				*dst = *page0 = *src;
			}
		}
	}

	if (updated)
		addDirtyRect(0, 0, 320, 200);
}

} // end of namespace Kyra

