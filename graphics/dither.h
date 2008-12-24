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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#ifndef GRAPHICS_DITHER_H
#define GRAPHICS_DITHER_H

#include "common/util.h"
#include "common/stream.h"

namespace Graphics {

class PaletteLUT {
public:
	enum PaletteFormat {
		kPaletteRGB,
		kPaletteYUV
	};

	inline static void YUV2RGB(byte y, byte u, byte v, byte &r, byte &g, byte &b) {
		r = CLIP<int>(y + ((1357 * (v - 128)) >> 10), 0, 255);
		g = CLIP<int>(y - (( 691 * (v - 128)) >> 10) - ((333 * (u - 128)) >> 10), 0, 255);
		b = CLIP<int>(y + ((1715 * (u - 128)) >> 10), 0, 255);
	}
	inline static void RGB2YUV(byte r, byte g, byte b, byte &y, byte &u, byte &v) {
		y = CLIP<int>( ((r * 306) >> 10) + ((g * 601) >> 10) + ((b * 117) >> 10)      , 0, 255);
		u = CLIP<int>(-((r * 172) >> 10) - ((g * 340) >> 10) + ((b * 512) >> 10) + 128, 0, 255);
		v = CLIP<int>( ((r * 512) >> 10) - ((g * 429) >> 10) - ((b *  83) >> 10) + 128, 0, 255);
	}

	PaletteLUT(byte depth, PaletteFormat format);
	~PaletteLUT();

	void setPalette(const byte *palette, PaletteFormat format, byte depth);

	void buildNext();

	void getEntry(byte index, byte &c1, byte &c2, byte &c3) const;
	byte findNearest(byte c1, byte c2, byte c3);
	byte findNearest(byte c1, byte c2, byte c3, byte &nC1, byte &nC2, byte &nC3);

	bool save(Common::WriteStream &stream);
	bool load(Common::SeekableReadStream &stream);

private:
	byte _depth1, _depth2;
	byte _shift;

	uint32 _dim1, _dim2, _dim3;

	PaletteFormat _format;
	byte _lutPal[768];
	byte _realPal[768];

	uint32 _got;
	byte *_gots;
	byte *_lut;

	void build(int d1);
	inline int getIndex(byte c1, byte c2, byte c3) const;
	inline void plotEntry(int x, int y, int z, byte e, byte *filled, int &free);
};

// The Sierra-2-4A ("Filter Light") dithering algorithm
class SierraLight {
public:
	SierraLight(int16 width, int16 height, PaletteLUT *palLUT);
	~SierraLight();

	void newFrame();
	void nextLine();
	byte dither(byte c1, byte c2, byte c3, uint32 x);

protected:
	int16 _width, _height;

	PaletteLUT *_palLUT;

	int32 *_errorBuf;
	int32 *_errors[2];
	int _curLine;

	inline void getErrors(uint32 x, int32 &eC1, int32 &eC2, int32 &eC3);
	inline void addErrors(uint32 x, int32 eC1, int32 eC2, int32 eC3);
};

} // End of namespace Graphics

#endif
