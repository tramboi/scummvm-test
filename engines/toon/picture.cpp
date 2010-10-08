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

#include "picture.h"
#include "tools.h"
#include "common/stack.h"

namespace Toon {

bool Picture::loadPicture(Common::String file, bool totalPalette /*= false*/) {
	debugC(1, kDebugPicture, "loadPicture(%s, %d)", file.c_str(), (totalPalette) ? 1 : 0);

	uint32 size = 0;
	uint8 *fileData = _vm->resources()->getFileData(file, &size);
	if (!fileData)
		return false;

	_useFullPalette = totalPalette;

	uint32 compId = READ_BE_UINT32(fileData);

	switch (compId) {
	case kCompLZSS: {
		uint32 dstsize = READ_LE_UINT32(fileData + 4);
		_data = new uint8[dstsize];
		decompressLZSS(fileData + 8, _data, dstsize);

		// size can only be 640x400 or 1280x400
		if (dstsize > 640 * 400 + 768)
			_width = 1280;
		else
			_width = 640;

		_height = 400;

		// do we have a palette ?
		_paletteEntries = (dstsize & 0x7ff) / 3;
		if (_paletteEntries) {
			_palette = new uint8[_paletteEntries * 3];
			memcpy(_palette, _data + dstsize - (dstsize & 0x7ff), _paletteEntries * 3);
			_vm->fixPaletteEntries(_palette, _paletteEntries);
		} else {
			_palette = 0;
		}
		return true;
		break;
	}
	case kCompSPCN: {
		uint32 decSize = READ_LE_UINT32(fileData + 10);
		_data = new uint8[decSize+100];
		_paletteEntries = READ_LE_UINT16(fileData + 14) / 3;

		if (_paletteEntries) {
			_palette = new uint8[_paletteEntries * 3];
			memcpy(_palette, fileData + 16, _paletteEntries * 3);
			_vm->fixPaletteEntries(_palette, _paletteEntries);
		}

		// size can only be 640x400 or 1280x400
		if (decSize > 640 * 400 + 768)
			_width = 1280;
		else
			_width = 640;

		_height = 400;

		// decompress the picture into our buffer
		decompressSPCN(fileData + 16 + _paletteEntries * 3, _data, decSize);
		return true;
		break;
	}
	case kCompRNC1: {
		Toon::RncDecoder rnc;

		// allocate enough place
		uint32 decSize = READ_BE_UINT32(fileData + 4);

		_data = new uint8[decSize];
		rnc.unpackM1(fileData, _data);

		// size can only be 640x400 or 1280x400
		if (decSize > 640 * 400 + 768)
			_width = 1280;
		else
			_width = 640;

		_height = 400;
		return true;
		break;
	}
	case kCompRNC2: {
		Toon::RncDecoder rnc;

		// allocate enough place
		uint32 decSize = READ_BE_UINT32(fileData + 4);

		_data = new uint8[decSize];

		decSize = rnc.unpackM2(fileData, _data);

		if (decSize > 640 * 400 + 768)
			_width = 1280;
		else
			_width = 640;

		_height = 400;
		return true;
		break;
	}
	}
	return false;
}

Picture::Picture(ToonEngine *vm) : _vm(vm) {

}

void Picture::setupPalette() {
	debugC(1, kDebugPicture, "setupPalette()");

	if (_useFullPalette)
		_vm->setPaletteEntries(_palette, 0, 255);
	else
		_vm->setPaletteEntries(_palette, 1, 128);
}

void Picture::drawMask(Graphics::Surface &surface, int32 x, int32 y, int32 dx, int32 dy) {
	debugC(1, kDebugPicture, "drawMask(surface, %d, %d, %d, %d)", x, y, dx, dy);

	for (int32 i = 0; i < 128; i++) {
		byte color[3];
		color[0] = i * 2;
		color[1] = i * 2;
		color[2] = 255 - i * 2;
		_vm->setPaletteEntries(color, i, 1);
	}

	int32 rx = MIN(_width, surface.w - x);
	int32 ry = MIN(_height, surface.h - y);

	if (rx < 0 || ry < 0)
		return;

	int32 destPitch = surface.pitch;
	int32 srcPitch = _width;
	uint8 *c = _data + _width * dy + dx ;
	uint8 *curRow = (uint8 *)surface.pixels + y * destPitch + x;

	for (int32 yy = 0; yy < ry; yy++) {
		uint8 *curSrc = c;
		uint8 *cur = curRow;
		for (int32 xx = 0; xx < rx; xx++) {
			//*cur = (*curSrc >> 5) * 8; // & 0x1f;
			*cur = (*curSrc & 0x1f) ? 127 : 0;

			curSrc++;
			cur++;
		}
		curRow += destPitch;
		c += srcPitch;
	}
}

void Picture::draw(Graphics::Surface &surface, int32 x, int32 y, int32 dx, int32 dy) {
	debugC(6, kDebugPicture, "draw(surface, %d, %d, %d, %d)", x, y, dx, dy);

	int32 rx = MIN(_width, surface.w - x);
	int32 ry = MIN(_height, surface.h - y);

	if (rx < 0 || ry < 0)
		return;

	int32 destPitch = surface.pitch;
	int32 srcPitch = _width;
	uint8 *c = _data + _width * dy + dx ;
	uint8 *curRow = (uint8 *)surface.pixels + y * destPitch + x;

	for (int32 yy = 0; yy < ry; yy++) {
		uint8 *curSrc = c;
		uint8 *cur = curRow;
		for (int32 xx = 0; xx < rx; xx++) {
			*cur = *curSrc ;
			curSrc++;
			cur++;
		}
		curRow += destPitch;
		c += srcPitch;
	}
}

uint8 Picture::getData(int32 x, int32 y) {
	debugC(6, kDebugPicture, "getData(%d, %d)", x, y);

	if (!_data)
		return 0;

	return _data[y * _width + x];
}

// use original work from johndoe
void Picture::floodFillNotWalkableOnMask( int32 x, int32 y ) {
	debugC(1, kDebugPicture, "floodFillNotWalkableOnMask(%d, %d)", x, y);

	// Stack-based floodFill algorithm based on
	// http://student.kuleuven.be/~m0216922/CG/files/floodfill.cpp
	Common::Stack<Common::Point> stack;
	bool spanLeft, spanRight;
	stack.push(Common::Point(x, y));
	while (!stack.empty()) {
		Common::Point pt = stack.pop();
		while (_data[pt.x + pt.y * _width] & 0x1F && pt.y >= 0)
			pt.y--;
		pt.y++;
		spanLeft = false;
		spanRight = false;
		while (_data[pt.x + pt.y * _width] & 0x1F && pt.y < _height) {
			_data[pt.x + pt.y * _width] &= 0xE0;
			if (!spanLeft && pt.x > 0 && _data[pt.x - 1 + pt.y * _width] & 0x1F) {
				stack.push(Common::Point(pt.x - 1, pt.y));
				spanLeft = 1;
			} else if (spanLeft && pt.x > 0 && !(_data[pt.x - 1 + pt.y * _width] & 0x1F)) {
				spanLeft = 0;
			}
			if (!spanRight && pt.x < _width - 1 && _data[pt.x + 1 + pt.y * _width] & 0x1F) {
				stack.push(Common::Point(pt.x + 1, pt.y));
				spanRight = 1;
			} else if (spanRight && pt.x < _width - 1 && !(_data[pt.x + 1 + pt.y * _width] & 0x1F)) {
				spanRight = 0;
			}
			pt.y++;
		}
	}
}

void Picture::drawLineOnMask( int32 x, int32 y, int32 x2, int32 y2, bool walkable ) {
	debugC(1, kDebugPicture, "drawLineOnMask(%d, %d, %d, %d, %d)", x, y, x2, y2, (walkable) ? 1 : 0);

	static int32 lastX = 0;
	static int32 lastY = 0;

	if (x == -1) {
		x = lastX;
		y = lastY;
	}

	uint32 bx = x << 16;
	int32 dx = x2 - x;
	uint32 by = y << 16;
	int32 dy = y2 - y;
	uint32 adx = abs(dx);
	uint32 ady = abs(dy);
	int32 t = 0;
	if (adx <= ady)
		t = ady;
	else
		t = adx;


	int32 cdx = (dx << 16) / t;
	int32 cdy = (dy << 16) / t;

	int32 i = t;
	while (i) {
		if (!walkable) {
			_data[_width * (by >> 16) + (bx >> 16)] &= 0xe0;
			_data[_width * (by >> 16) + (bx >> 16)+1] &= 0xe0;
		} else {
			int32 v = _data[_width * (by >> 16) + (bx >> 16) - 1];
			_data[_width * (by >> 16) + (bx >> 16)] = v;
			_data[_width * (by >> 16) + (bx >> 16)+1] = v;
		}
	
		bx += cdx;
		by += cdy;
		i--;
	}
}
} // End of namespace Toon
