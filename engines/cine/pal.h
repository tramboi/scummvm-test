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

#ifndef CINE_PAL_H
#define CINE_PAL_H

#include "graphics/pixelformat.h"

namespace Cine {

// Constants related to kLowPalFormat
#define kLowPalBytesPerColor 2
#define kLowPalNumColors 16
#define kLowPalNumBytes ((kLowPalNumColors) * (kLowPalBytesPerColor))

/*! \brief Low resolution (9-bit) color format used in Cine's 16-color modes. */
static const Graphics::PixelFormat kLowPalFormat  = {kLowPalBytesPerColor, 5, 5, 5, 8, 8, 4, 0, 0};

// Constants related to kHighPalFormat
#define kHighPalBytesPerColor 3
#define kHighPalNumColors 256
#define kHighPalNumBytes ((kHighPalNumColors) * (kHighPalBytesPerColor))

/*! \brief High resolution (24-bit) color format used in Cine's 256-color modes. */
static const Graphics::PixelFormat kHighPalFormat = {kHighPalBytesPerColor, 0, 0, 0, 8, 0, 8, 16, 0};

/*! \brief The color format used by OSystem's setPalette-function. */
static const Graphics::PixelFormat kSystemPalFormat = {4, 0, 0, 0, 8, 0, 8, 16, 0};

/*! \brief Endian types. Used at least by Palette class's load and save functions.
 * TODO: Move somewhere more general as this is definitely not Cine-engine specific
 *
 * NOTE: It seems LITTLE_ENDIAN and/or BIG_ENDIAN were defined already on some platforms so
 * therefore renamed the enumerations to something not clashing by giving them "CINE_"-prefixes.
 */
enum EndianType {
	CINE_NATIVE_ENDIAN,
	CINE_LITTLE_ENDIAN,
	CINE_BIG_ENDIAN
};

struct PalEntry {
	char name[10];
	byte pal1[16];
	byte pal2[16];
};

extern Common::Array<PalEntry> palArray;

void loadPal(const char *fileName);

void loadRelatedPalette(const char *fileName);

void palRotate(uint16 *pal, byte a, byte b, byte c);
void palRotate(byte *pal, byte a, byte b, byte c);
uint16 transformColor(uint16 baseColor, int r, int g, int b);
void transformPaletteRange(uint16 *srcPal, uint16 *dstPal, int startColor, int stopColor, int r, int g, int b);
void transformPaletteRange(byte *srcPal, byte *dstPal, int startColor, int stopColor, int r, int g, int b);

// This class might be used for handling Cine-engine's palettes in the future. WIP!
// TODO: Document
// TODO: Make use of
// TODO: Test
class Palette {
private:
	struct Color {
		uint8 r, g, b;
	};

public:
	/*! \brief Create an initially black palette with the given color format and number of colors.
	 * \param format Color format
	 * \param numColors Number of colors
	 * \note For the default constructed object (i.e. no parameters given) this will hold: empty() && !isValid()
	 */
	Palette(const Graphics::PixelFormat format = Graphics::PixelFormat(), const uint numColors = 0);

	/*! \brief Clear the palette (Set color count to zero, release memory, overwrite color format with default value).
	 * \note This is very different from using fillWithBlack-function which fills the palette with black.
	 */
	Palette &clear();

	/*! \brief Load palette from buffer with given color format, endianness and number of colors.
	 * \param buf Input buffer
	 * \param size Input buffer size in bytes
	 * \param format Input color format
	 * \param numColors Number of colors to load
	 * \param endian The endianness of the colors in the input buffer
	 */
	Palette &load(const byte *buf, const uint size, const Graphics::PixelFormat format, const uint numColors, const EndianType endian);

	/*! \brief Save the whole palette to buffer in original color format using defined endianness.
	 * \param buf Output buffer
	 * \param size Output buffer size in bytes
	 * \param endian The endian type to use
	 */
	byte *save(byte *buf, const uint size, const EndianType endian) const;

	/*! \brief Save the whole palette to buffer in given color format using defined endianness.
	 * \param buf Output buffer
	 * \param size Output buffer size in bytes
	 * \param format Output color format
	 * \param endian The endian type to use
	 */
	byte *save(byte *buf, const uint size, const Graphics::PixelFormat format, const EndianType endian) const;

	/*! \brief Save (partial) palette to buffer in given color format using defined endianness.
	 * \param buf Output buffer
	 * \param size Output buffer size in bytes
	 * \param format Output color format
	 * \param numColors Number of colors to save
	 * \param endian The endian type to use
	 * \param firstIndex Starting color index (from which onwards to save the colors)
	 */
	byte *save(byte *buf, const uint size, const Graphics::PixelFormat format, const uint numColors, const EndianType endian, const byte firstIndex = 0) const;

	Palette &rotateRight(byte firstIndex, byte lastIndex);
	Palette &saturatedAddColor(Palette& output, byte firstIndex, byte lastIndex, signed r, signed g, signed b);

	/*! \brief Saturated add an RGB color in given color format to current palette's subset and save the modified colors in the given output palette.
	 * \param output The output palette (Only this palette is modified)
	 * \param firstIndex First color index of the palette's subset (Inclusive range)
	 * \param lastIndex Last color index of the palette's subset (Inclusive range)
	 * \param rSource The red color component in the source color format
	 * \param gSource The green color component in the source color format
	 * \param bSource The blue color component in the source color format
	 * \param sourceFormat The source color format (i.e. the color format of the given RGB color)
	 * \note This function basically converts the given color to the palette's internal color format
	 * and adds that using the normal saturatedAddColor-function.
	 */
	Palette &saturatedAddColor(Palette& output, byte firstIndex, byte lastIndex, signed rSource, signed gSource, signed bSource, const Graphics::PixelFormat &sourceFormat);

	/*! \brief Saturated add a normalized gray value to current palette's subset and save the modified colors in the given output palette.
	 * \param output The output palette (Only this palette is modified)
	 * \param firstIndex First color index of the palette's subset (Inclusive range)
	 * \param lastIndex Last color index of the palette's subset (Inclusive range)
	 * \param grayDividend Dividend of the normalized gray value
	 * \param grayDenominator Denominator of the normalized gray value
	 * \note The normalized gray value (i.e. in range [-1, +1]) is given as a fractional number
	 * (i.e. the normalized gray value is calculated by dividing grayDividend by grayDenominator).
	 */
	Palette &saturatedAddNormalizedGray(Palette& output, byte firstIndex, byte lastIndex, signed grayDividend, signed grayDenominator);

	bool empty() const;
	uint colorCount() const;

	Palette &fillWithBlack();

	/*! \brief Is the palette valid? (Mostly just checks the color format for correctness) */
	bool isValid() const;

	/*! \brief The original color format in which this palette was loaded. */
	const Graphics::PixelFormat &colorFormat() const;

	/*! \brief Sets current palette to global OSystem's palette using g_system->setPalette. */
	void setGlobalOSystemPalette() const;

private:
	void setColorFormat(const Graphics::PixelFormat format);
	Cine::Palette::Color saturatedAddColor(Cine::Palette::Color baseColor, signed r, signed g, signed b) const;

private:
	Graphics::PixelFormat _format; //!< The used source color format
	Common::Array<Color> _colors;  //!< The actual palette data
};

} // End of namespace Cine

#endif
