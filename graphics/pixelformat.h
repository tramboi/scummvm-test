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

#ifndef GRAPHICS_PIXELFORMAT_H
#define GRAPHICS_PIXELFORMAT_H

#include "common/scummsys.h"

namespace Graphics {

#ifdef ENABLE_16BIT
/**
 * A condensed bit format description.
 *
 * It includes the necessary information to create a PixelFormat and/or 
 * ColorMask which fully describe the given color format.
 *
 * It contains two components, the format (8Bit paletted, RGB555, etc)
 * and the order (palette, ARGB, ABGR, etc)
 *
 * Use (format & kFormatTypeMask) to get the type, and (format & kFormatOrderMask)
 * to get the applicable color order.
  */
enum ColorMode {
	kFormatCLUT8 = 0,		//256 color palette.
	kFormatRGB555 = 1,
	kFormatXRGB1555 = 2,	// Special case, high bit has special purpose, which may be alpha. 
							// Engines should probably handle this bit internally and pass RGB only, though
	kFormatRGB565 = 3,
	kFormatRGBA4444 = 4,	// since this mode is commonly supported in game hardware, some unimplemented engines may use it?
	kFormatRGB888 = 5,
	kFormatRGBA6666 = 6,	// I've never heard of this, but it's vaguely plausible
	kFormatRGBA8888 = 7
};
#endif

/**
 * A pixel format description.
 *
 * Like ColorMasks it includes the given values to create colors from RGB
 * values and to retrieve RGB values from colors.
 *
 * Unlike ColorMasks it is not dependend on knowing the exact pixel format
 * on compile time.
 *
 * A minor difference between ColorMasks and PixelFormat is that ColorMasks
 * stores the bit count per channel in 'kFooBits', while PixelFormat stores
 * the loss compared to 8 bits per channel in '#Loss'. It also doesn't
 * contain mask values.
 */
struct PixelFormat {
	byte bytesPerPixel; /**< Number of bytes used in the pixel format. */

	byte rLoss, gLoss, bLoss, aLoss; /**< Precision loss of each color component. */
	byte rShift, gShift, bShift, aShift; /**< Binary left shift of each color component in the pixel value. */

	inline bool operator==(const PixelFormat &fmt) const {
		// TODO: If aLoss==8, then the value of aShift is irrelevant, and should be ignored.
		return 0 == memcmp(this, &fmt, sizeof(PixelFormat));
	}

	inline bool operator!=(const PixelFormat &fmt) const {
		return !(*this == fmt);
	}

	inline uint32 RGBToColor(uint8 r, uint8 g, uint8 b) const {
		return
			((0xFF >> aLoss) << aShift) |
			((   r >> rLoss) << rShift) |
			((   g >> gLoss) << gShift) |
			((   b >> bLoss) << bShift);
	}

	inline uint32 ARGBToColor(uint8 a, uint8 r, uint8 g, uint8 b) const {
		return
			((a >> aLoss) << aShift) |
			((r >> rLoss) << rShift) |
			((g >> gLoss) << gShift) |
			((b >> bLoss) << bShift);
	}

	inline void colorToRGB(uint32 color, uint8 &r, uint8 &g, uint8 &b) const {
		r = ((color >> rShift) << rLoss) & 0xFF;
		g = ((color >> gShift) << gLoss) & 0xFF;
		b = ((color >> bShift) << bLoss) & 0xFF;
	}

	inline void colorToARGB(uint32 color, uint8 &a, uint8 &r, uint8 &g, uint8 &b) const {
		a = ((color >> aShift) << aLoss) & 0xFF;
		r = ((color >> rShift) << rLoss) & 0xFF;
		g = ((color >> gShift) << gLoss) & 0xFF;
		b = ((color >> bShift) << bLoss) & 0xFF;
	}

	//////////////////////////////////////////////////////////////////////
	// Convenience functions for getting number of color component bits //
	//////////////////////////////////////////////////////////////////////

	inline byte rBits() const {
		return (8 - rLoss);
	}

	inline byte gBits() const {
		return (8 - gLoss);
	}

	inline byte bBits() const {
		return (8 - bLoss);
	}

	inline byte aBits() const {
		return (8 - aLoss);
	}

	////////////////////////////////////////////////////////////////////////
	// Convenience functions for getting color components' maximum values //
	////////////////////////////////////////////////////////////////////////

	inline uint rMax() const {
		return (1 << rBits()) - 1;
	}

	inline uint gMax() const {
		return (1 << gBits()) - 1;
	}

	inline uint bMax() const {
		return (1 << bBits()) - 1;
	}

	inline uint aMax() const {
		return (1 << aBits()) - 1;
	}
};

} // end of namespace Graphics

#endif
