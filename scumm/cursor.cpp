/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "common/system.h"
#include "common/util.h"
#include "scumm/bomp.h"
#include "scumm/charset.h"
#include "scumm/intern.h"
#include "scumm/object.h"
#include "scumm/resource_v7he.h"
#include "scumm/saveload.h"
#include "scumm/scumm.h"

namespace Scumm {

/*
 * Mouse cursor cycle colors (for the default crosshair).
 */
static const byte default_v1_cursor_colors[4] = {
	1, 1, 12, 11
};

static const byte default_cursor_colors[4] = {
	15, 15, 7, 8
};


static const uint16 default_cursor_images[4][16] = {
	/* cross-hair */
	{ 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0000, 0x7e3f,
	  0x0000, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0000 },
	/* hourglass */
	{ 0x0000, 0x7ffe, 0x6006, 0x300c, 0x1818, 0x0c30, 0x0660, 0x03c0,
	  0x0660, 0x0c30, 0x1998, 0x33cc, 0x67e6, 0x7ffe, 0x0000, 0x0000 },
	/* arrow */
	{ 0x0000, 0x4000, 0x6000, 0x7000, 0x7800, 0x7c00, 0x7e00, 0x7f00,
	  0x7f80, 0x78c0, 0x7c00, 0x4600, 0x0600, 0x0300, 0x0300, 0x0180 },
	/* hand */
	{ 0x1e00, 0x1200, 0x1200, 0x1200, 0x1200, 0x13ff, 0x1249, 0x1249,
	  0xf249, 0x9001, 0x9001, 0x9001, 0x8001, 0x8001, 0x8001, 0xffff },
};

static const byte default_cursor_hotspots[10] = {
	8, 7,   8, 7,   1, 1,   5, 0,
	8, 7, //zak256
};

static const uint16 default_he_cursor[64] = {
	0x0000, 0x0000, 0x3800, 0x0000, 0x7e00, 0x0000, 0x5f80, 0x0000,
	0x5fe0, 0x0000, 0x2ff8, 0x0000, 0x27fe, 0x0000, 0x17ff, 0x8000,
	0x13ff, 0xe000, 0x09ff, 0xf000, 0x09ff, 0xf800, 0x04ff, 0xf800,
	0x047f, 0xf000, 0x027f, 0xe000, 0x023f, 0xf000, 0x011f, 0xf800,
	0x0111, 0xfc00, 0x0080, 0xfc00, 0x0084, 0x0c00, 0x004a, 0x0800,
	0x0031, 0x1000, 0x0000, 0xe000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

static const byte default_v6_cursor[] = {
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0x00,0x0F,0x00, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0x00,0x0F,0x00, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0x00,0x0F,0x00, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0x00,0x0F,0x00, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0x00,0x0F,0x00, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x0F,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0xFF,
	0x0F,0x0F,0x0F,0x0F,0x0F,0x0F, 0x0F,0x0F,0x0F, 0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0xFF,
	0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x0F,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0x00,0x0F,0x00, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0x00,0x0F,0x00, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0x00,0x0F,0x00, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0x00,0x0F,0x00, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0x00,0x0F,0x00, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
};

ScummEngine_v5::ScummEngine_v5(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex)
 : ScummEngine(detector, syst, gs, md5sum, substResFileNameIndex) {

	for (int i = 0; i < 4; i++) {
		memcpy(_cursorImages[i], default_cursor_images[i], 32);
	}
	memcpy(_cursorHotspots, default_cursor_hotspots, 8);
}


void ScummEngine::setupCursor() {
	_cursor.animate = 1;
}

void ScummEngine_v5::animateCursor() {
	if (_cursor.animate) {
		if (!(_cursor.animateIndex & 0x1)) {
			setBuiltinCursor((_cursor.animateIndex >> 1) & 3);
		}
		_cursor.animateIndex++;
	}
}

void ScummEngine_v6::setCursorHotspot(int x, int y) {
	_cursor.hotspotX = x;
	_cursor.hotspotY = y;
}

void ScummEngine_v6::setCursorTransparency(int a) {
	int i, size;

	size = _cursor.width * _cursor.height;

	for (i = 0; i < size; i++)
		if (_grabbedCursor[i] == (byte)a)
			_grabbedCursor[i] = 0xFF;

	updateCursor();
}

void ScummEngine::updateCursor() {
	const int transColor = (_heversion >= 80) ? 5 : 255;
	_system->setMouseCursor(_grabbedCursor, _cursor.width, _cursor.height,
							_cursor.hotspotX, _cursor.hotspotY,
							(_platform == Common::kPlatformNES ? _grabbedCursor[63] : transColor),
							(_heversion == 70 ? 2 : 1));
}

void ScummEngine_v6::grabCursor(int x, int y, int w, int h) {
	VirtScreen *vs = findVirtScreen(y);

	if (vs == NULL) {
		debug(0, "grabCursor: invalid Y %d", y);
		return;
	}

	setCursorFromBuffer((byte *)vs->pixels + (y - vs->topline) * vs->pitch + x, w, h, vs->pitch);
}

void ScummEngine_v6::setDefaultCursor() {
	setCursorHotspot(7, 6);
	setCursorFromBuffer(default_v6_cursor, 16, 13, 16);
}

void ScummEngine::setCursorFromBuffer(const byte *ptr, int width, int height, int pitch) {
	uint size;
	byte *dst;

	size = width * height;
	if (size > sizeof(_grabbedCursor))
		error("grabCursor: grabbed cursor too big");

	_cursor.width = width;
	_cursor.height = height;
	_cursor.animate = 0;

	dst = _grabbedCursor;
	for (; height; height--) {
		memcpy(dst, ptr, width);
		dst += width;
		ptr += pitch;
	}

	updateCursor();
}

#ifndef DISABLE_HE
void ScummEngine_v70he::setCursorFromImg(uint img, uint room, uint imgindex) {
	_resExtractor->setCursor(img);
}

void ScummEngine_v90he::setDefaultCursor() {
	const uint16 *src;
	int i, j;
	static byte palette[] = { 0xff, 0xff, 0xff, 0,
							  0,    0,    0,    0};

	memset(_grabbedCursor, 5, sizeof(_grabbedCursor));

	_cursor.hotspotX = _cursor.hotspotY = 2;
	src = default_he_cursor;

	_cursor.width = 32;
	_cursor.height = 32;

	for (i = 0; i < 32; i++) {
		for (j = 0; j < 32; j++) {
			if (*src & (1 << (15 - (j % 16))))
				_grabbedCursor[32 * i + j] = 0xfe;
			if (j == 15)
				src++;
		}
		src++;
	}

	// Since white color position is not guaranteed
	// we setup our own palette if supported by backend
	if (_system->hasFeature(OSystem::kFeatureCursorHasPalette))
		_system->setCursorPalette(palette, 0xfe, 2);

	updateCursor();
}
#endif

void ScummEngine_v6::setCursorFromImg(uint img, uint room, uint imgindex) {
	int w, h;
	const byte *dataptr, *bomp;
	uint32 size;
	FindObjectInRoom foir;
	const ImageHeader *imhd;

	if (room == (uint) - 1)
		room = getObjectRoom(img);

	findObjectInRoom(&foir, foCodeHeader | foImageHeader | foCheckAlreadyLoaded, img, room);
	imhd = (const ImageHeader *)findResourceData(MKID('IMHD'), foir.obim);

	if (_version == 8) {
		setCursorHotspot(READ_LE_UINT32(&imhd->v8.hotspot[0].x),
		                  READ_LE_UINT32(&imhd->v8.hotspot[0].y));
		w = READ_LE_UINT32(&imhd->v8.width) / 8;
		h = READ_LE_UINT32(&imhd->v8.height) / 8;
	} else if (_version == 7) {
		setCursorHotspot(READ_LE_UINT16(&imhd->v7.hotspot[0].x),
		                  READ_LE_UINT16(&imhd->v7.hotspot[0].y));
		w = READ_LE_UINT16(&imhd->v7.width) / 8;
		h = READ_LE_UINT16(&imhd->v7.height) / 8;
	} else {
		if (_heversion == 0) {
			setCursorHotspot(READ_LE_UINT16(&imhd->old.hotspot[0].x),
		        	          READ_LE_UINT16(&imhd->old.hotspot[0].y));
		}
		w = READ_LE_UINT16(&foir.cdhd->v6.w) / 8;
		h = READ_LE_UINT16(&foir.cdhd->v6.h) / 8;
	}

	dataptr = getObjectImage(foir.obim, imgindex);
	assert(dataptr);
	if (_version == 8) {
		bomp = dataptr;
	} else {
		size = READ_BE_UINT32(dataptr + 4);
		if (size > sizeof(_grabbedCursor))
			error("setCursorFromImg: Cursor image too large");

		bomp = findResource(MKID('BOMP'), dataptr);
	}

	if (bomp != NULL)
		useBompCursor(bomp, w, h);
	else
		useIm01Cursor(dataptr, w, h);
}

void ScummEngine_v6::useIm01Cursor(const byte *im, int w, int h) {
	VirtScreen *vs = &virtscr[0];
	byte *buf, *dst;
	const byte *src;
	int i;

	w *= 8;
	h *= 8;

	// Backup the screen content
	dst = buf = (byte *)malloc(w * h);
	src = vs->getPixels(0, 0);

	for (i = 0; i < h; i++) {
		memcpy(dst, src, w);
		dst += w;
		src += vs->pitch;
	}

	// Do some drawing
	drawBox(0, 0, w - 1, h - 1, 0xFF);

	vs->hasTwoBuffers = false;
	gdi.disableZBuffer();
	gdi.drawBitmap(im, vs, _screenStartStrip, 0, w, h, 0, w / 8, 0);
	vs->hasTwoBuffers = true;
	gdi.enableZBuffer();

	// Grab the data we just drew and setup the cursor with it
	setCursorFromBuffer(vs->getPixels(0, 0), w, h, vs->pitch);

	// Restore the screen content
	src = buf;
	dst = vs->getPixels(0, 0);

	for (i = 0; i < h; i++) {
		memcpy(dst, src, w);
		dst += vs->pitch;
		src += w;
	}

	free(buf);
}

void ScummEngine_v6::useBompCursor(const byte *im, int width, int height) {
	uint size;

	width *= 8;
	height *= 8;

	size = width * height;
	if (size > sizeof(_grabbedCursor))
		error("useBompCursor: cursor too big (%d)", size);

	_cursor.width = width;
	_cursor.height = height;
	_cursor.animate = 0;

	// Skip the header
	if (_version == 8) {
		im += 16;
	} else {
		im += 18;
	}
	decompressBomp(_grabbedCursor, im, width, height);

	updateCursor();
}

void ScummEngine_v5::redefineBuiltinCursorFromChar(int index, int chr) {
	// Cursor image in both Looms are based on images from charset.
	if (_gameId != GID_LOOM && _gameId != GID_LOOM256) {
		// FIXME: Actually: is this opcode ever called by a non-Loom game?
		// Which V3-V5 game besides Loom makes use of custom cursors, ever?
		error("V3--V5 SO_CURSOR_IMAGE(%d,%d) called - tell Fingolfin where you saw this!", index, chr);
	}

	assert(index >= 0 && index < 4);

//	const int oldID = _charset->getCurID();

	if (_version == 3) {
		_charset->setCurID(0);
	} else if (_version >= 4) {
		_charset->setCurID(1);
	}

	Graphics::Surface s;
	byte buf[16*17];
	memset(buf, 123, 16*17);
	s.pixels = buf;
	s.w = _charset->getCharWidth(chr);
	s.h = _charset->getFontHeight();
	s.pitch = s.w;
	// s.h = 17 for FM-TOWNS Loom Japanese. Fixes bug #1166917
	assert(s.w <= 16 && s.h <= 17);
	s.bytesPerPixel = 1;

	_charset->drawChar(chr, s, 0, 0);

	uint16 *ptr = _cursorImages[index];
	memset(ptr, 0, 17 * sizeof(uint16));
	for (int h = 0; h < s.h; h++) {
		for (int w = 0; w < s.w; w++) {
			if (buf[s.pitch * h + w] != 123)
				*ptr |= 1 << (15 - w);
		}
		ptr++;
	}

//	_charset->setCurID(oldID);
}

void ScummEngine_v5::redefineBuiltinCursorHotspot(int index, int x, int y) {
	// Cursor image in both Looms are based on images from charset.
	if (_gameId != GID_LOOM && _gameId != GID_LOOM256) {
		// FIXME: Actually: is this opcode ever called by a non-Loom game?
		// Which V3-V5 game besides Loom makes use of custom cursors, ever?
		error("V3--V5 SO_CURSOR_HOTSPOT(%d,%d,%d) called - tell Fingolfin where you saw this!", index, x, y);
	}

	assert(index >= 0 && index < 4);

	_cursorHotspots[index * 2] = x;
	_cursorHotspots[index * 2 + 1] = y;
}

void ScummEngine_v5::setBuiltinCursor(int idx) {
	int i, j;
	byte color;

	memset(_grabbedCursor, 0xFF, sizeof(_grabbedCursor));

	if (_version == 1)
		color = default_v1_cursor_colors[idx];
	else
		color = default_cursor_colors[idx];

	if (_platform == Common::kPlatformNES) {
		_cursor.width = 8;
		_cursor.height = 8;
		_cursor.hotspotX = 0;
		_cursor.hotspotY = 0;

		byte *dst = _grabbedCursor;
		byte *src = &_NESPatTable[0][0xfa * 16];
		byte *palette = _NESPalette[1];

		for (i = 0; i < 8; i++) {
			byte c0 = src[i];
			byte c1 = src[i + 8];
			for (j = 0; j < 8; j++)
				*dst++ = palette[((c0 >> (7 - j)) & 1) | (((c1 >> (7 - j)) & 1) << 1) | ((idx == 3) ? 4 : 0)];
		}

	} else if (_version <= 2 && _platform == Common::kPlatformAmiga) {
		_cursor.width = 15;
		_cursor.height = 15;
		_cursor.hotspotX = 7;
		_cursor.hotspotY = 7;

		byte *hotspot = _grabbedCursor + _cursor.hotspotY * _cursor.width + _cursor.hotspotX;

		// Crosshair, symmetric
		// TODO: Instead of setting this up via code, we should simply extend
		//       default_cursor_images to contain this shape.
		for (i = 0; i < 5; i++) {
			*(hotspot - 3 - i) = color;
			*(hotspot + 3 + i) = color;
			*(hotspot - _cursor.width * (3 + i)) = color;
			*(hotspot + _cursor.width * (3 + i)) = color;
		}

		// Arrow heads, diagonal lines
		for (i = 1; i <= 2; i++) {
			*(hotspot - _cursor.width * i - (3 + i)) = color;
			*(hotspot + _cursor.width * i - (3 + i)) = color;
			*(hotspot - _cursor.width * i + (3 + i)) = color;
			*(hotspot + _cursor.width * i + (3 + i)) = color;
			*(hotspot - _cursor.width * (3 + i) - i) = color;
			*(hotspot + _cursor.width * (3 + i) - i) = color;
			*(hotspot - _cursor.width * (3 + i) + i) = color;
			*(hotspot + _cursor.width * (3 + i) + i) = color;
		}
	} else if (_version <= 2) {
		_cursor.width = 23;
		_cursor.height = 21;
		_cursor.hotspotX = 11;
		_cursor.hotspotY = 10;

		byte *hotspot = _grabbedCursor + _cursor.hotspotY * _cursor.width + _cursor.hotspotX;

		// Crosshair, slightly assymetric
		// TODO: Instead of setting this up via code, we should simply extend
		//       default_cursor_images to contain this shape.

		for (i = 0; i < 7; i++) {
			*(hotspot - 5 - i) = color;
			*(hotspot + 5 + i) = color;
		}

		for (i = 0; i < 8; i++) {
			*(hotspot - _cursor.width * (3 + i)) = color;
			*(hotspot + _cursor.width * (3 + i)) = color;
		}

		// Arrow heads, diagonal lines

		for (i = 1; i <= 3; i++) {
			*(hotspot - _cursor.width * i - 5 - i) = color;
			*(hotspot + _cursor.width * i - 5 - i) = color;
			*(hotspot - _cursor.width * i + 5 + i) = color;
			*(hotspot + _cursor.width * i + 5 + i) = color;
			*(hotspot - _cursor.width * (i + 3) - i) = color;
			*(hotspot - _cursor.width * (i + 3) + i) = color;
			*(hotspot + _cursor.width * (i + 3) - i) = color;
			*(hotspot + _cursor.width * (i + 3) + i) = color;
		}

		// Final touches

		*(hotspot - _cursor.width - 7) = color;
		*(hotspot - _cursor.width + 7) = color;
		*(hotspot + _cursor.width - 7) = color;
		*(hotspot + _cursor.width + 7) = color;
		*(hotspot - (_cursor.width * 5) - 1) = color;
		*(hotspot - (_cursor.width * 5) + 1) = color;
		*(hotspot + (_cursor.width * 5) - 1) = color;
		*(hotspot + (_cursor.width * 5) + 1) = color;
	} else {
		const uint16 *src;

		_cursor.hotspotX = _cursorHotspots[2 * _currentCursor];
		_cursor.hotspotY = _cursorHotspots[2 * _currentCursor + 1];
		src = _cursorImages[_currentCursor];

		_cursor.width = 16;
		_cursor.height = 16;

		for (i = 0; i < 16; i++) {
			for (j = 0; j < 16; j++) {
				if (src[i] & (1 << j))
					_grabbedCursor[16 * i + 15 - j] = color;
			}
		}
	}

	updateCursor();
}

} // End of namespace Scumm
