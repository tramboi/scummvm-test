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

#include "cine/cine.h"
#include "cine/various.h"
#include "cine/pal.h"

namespace Cine {

Common::Array<PalEntry> palArray;
static byte paletteBuffer1[16];
static byte paletteBuffer2[16];

void loadPal(const char *fileName) {
	char buffer[20];

	removeExtention(buffer, fileName);

	strcat(buffer, ".PAL");
	palArray.clear();

	Common::File palFileHandle;
	if (!palFileHandle.open(buffer))
		error("loadPal(): Cannot open file %s", fileName);

	uint16 palEntriesCount = palFileHandle.readUint16LE();
	palFileHandle.readUint16LE(); // entry size

	palArray.resize(palEntriesCount);
	for (uint i = 0; i < palArray.size(); ++i) {
		palFileHandle.read(palArray[i].name, 10);
		palFileHandle.read(palArray[i].pal1, 16);
		palFileHandle.read(palArray[i].pal2, 16);
	}
	palFileHandle.close();
}

int16 findPaletteFromName(const char *fileName) {
	char buffer[10];
	uint16 position = 0;
	uint16 i;

	strcpy(buffer, fileName);

	while (position < strlen(fileName)) {
		if (buffer[position] > 'a' && buffer[position] < 'z') {
			buffer[position] += 'A' - 'a';
		}

		position++;
	}

	for (i = 0; i < palArray.size(); i++) {
		if (!strcmp(buffer, palArray[i].name)) {
			return i;
		}
	}

	return -1;

}

void loadRelatedPalette(const char *fileName) {
	char localName[16];
	byte i;
	int16 paletteIndex;

	removeExtention(localName, fileName);

	paletteIndex = findPaletteFromName(localName);

	if (paletteIndex == -1) {
		for (i = 0; i < 16; i++) {	// generate default palette
			paletteBuffer1[i] = paletteBuffer2[i] = (i << 4) + i;
		}
	} else {
		assert(paletteIndex < (int32)palArray.size());
		memcpy(paletteBuffer1, palArray[paletteIndex].pal1, 16);
		memcpy(paletteBuffer2, palArray[paletteIndex].pal2, 16);
	}
}

void palRotate(uint16 *pal, byte a, byte b, byte c) {
	assert(pal);

	if (c == 1) {
		uint16 currentColor = pal[b];

		for (int i = b; i > a; i--) {
			pal[i] = pal[i - 1];
		}

		pal[a] = currentColor;
	}
}

void palRotate(byte *pal, byte a, byte b, byte c) {
	assert(pal);

	if (c == 1) {
		byte currentR = pal[3 * b + 0];
		byte currentG = pal[3 * b + 1];
		byte currentB = pal[3 * b + 2];

		for (int i = b; i > a; i--) {
			pal[3 * i + 0] = pal[3 * (i - 1) + 0];
			pal[3 * i + 1] = pal[3 * (i - 1) + 1];
			pal[3 * i + 2] = pal[3 * (i - 1) + 2];
		}

		pal[3 * a + 0] = currentR;
		pal[3 * a + 1] = currentG;
		pal[3 * a + 2] = currentB;
	}
}

uint16 transformColor(uint16 baseColor, int r, int g, int b) {
	int8 oriR = CLIP( (baseColor & 0x007)       + b, 0, 7);
	int8 oriG = CLIP(((baseColor & 0x070) >> 4) + g, 0, 7);
	int8 oriB = CLIP(((baseColor & 0x700) >> 8) + r, 0, 7);

	return oriR | (oriG << 4) | (oriB << 8);
}

void transformPaletteRange(uint16 *dstPal, uint16 *srcPal, int startColor, int stopColor, int r, int g, int b) {
	assert(srcPal && dstPal);

	for (int i = startColor; i <= stopColor; i++)
		dstPal[i] = transformColor(srcPal[i], r, g, b);
}

void transformPaletteRange(byte *dstPal, byte *srcPal, int startColor, int stopColor, int r, int g, int b) {
	assert(srcPal && dstPal);

	for (int i = startColor; i <= stopColor; i++) {
		dstPal[3 * i + 0] = CLIP(srcPal[3 * i + 0] + r * 36, 0, 252);
		dstPal[3 * i + 1] = CLIP(srcPal[3 * i + 1] + g * 36, 0, 252);
		dstPal[3 * i + 2] = CLIP(srcPal[3 * i + 2] + b * 36, 0, 252);
	}
}

/*! \brief Shift byte to the left by given amount (Handles negative shifting amounts too, otherwise this would be trivial). */
byte shiftByteLeft(const byte value, const signed shiftLeft) {
	if (shiftLeft >= 0)
		return value << shiftLeft;
	else // right shift with negative shiftLeft values
		return value >> abs(shiftLeft);
}

/*! \brief Is given endian type big endian? (Handles native endian type too, otherwise this would be trivial). */
bool isBigEndian(const EndianType endian) {
	assert(endian == CINE_NATIVE_ENDIAN || endian == CINE_LITTLE_ENDIAN || endian == CINE_BIG_ENDIAN);

	// Handle explicit little and big endian types here
	if (endian != CINE_NATIVE_ENDIAN) {
		return (endian == CINE_BIG_ENDIAN);
	}

	// Handle native endian type here
#if defined(SCUMM_BIG_ENDIAN)
	return true;
#elif defined(SCUMM_LITTLE_ENDIAN)
	return false;
#else
	#error No endianness defined
#endif
}

/*! \brief Calculate byte position of given bit position in a multibyte variable using defined endianness. */
int bytePos(const int bitPos, const int numBytes, const bool bigEndian) {
	if (bigEndian)
		return (numBytes - 1) - (bitPos / 8);
	else // little endian
		return bitPos / 8;
}

// a.k.a. palRotate
Palette &Palette::rotateRight(byte firstIndex, byte lastIndex) {
	const Color lastColor = _colors[lastIndex];

	for (int i = lastIndex; i > firstIndex; i--)
		_colors[i] = _colors[i - 1];

	_colors[firstIndex] = lastColor;
	return *this;
}

uint Palette::colorCount() const {
	return _colors.size();
}

Palette &Palette::fillWithBlack() {
	for (uint i = 0; i < _colors.size(); i++) {
		_colors[i].r = 0;
		_colors[i].g = 0;
		_colors[i].b = 0;
	}

	return *this;
}

EndianType Palette::endianType() const {
	return (_bigEndian ? CINE_BIG_ENDIAN : CINE_LITTLE_ENDIAN);
}

Graphics::PixelFormat Palette::colorFormat() const {
	return _format;
}

void Palette::setColorFormat(const Graphics::PixelFormat format) {
	_format = format;

	_rBits = (8 - format.rLoss);
	_gBits = (8 - format.gLoss);
	_bBits = (8 - format.bLoss);

	_rMax = (1 << _rBits) - 1;
	_gMax = (1 << _gBits) - 1;
	_bMax = (1 << _bBits) - 1;
}

void Palette::setEndianType(const EndianType endian) {
	_bigEndian = isBigEndian(endian);
}

// a.k.a. transformPaletteRange
Palette &Palette::saturatedAddColor(Palette& output, byte firstIndex, byte lastIndex, signed r, signed g, signed b) {
	assert(firstIndex < colorCount() && lastIndex < colorCount());
	assert(firstIndex < output.colorCount() && lastIndex < output.colorCount());
	assert(output.colorFormat() == colorFormat());

	for (uint i = firstIndex; i <= lastIndex; i++)
		output._colors[i] = saturatedAddColor(_colors[i], r, g, b);

	return output;
}

Palette &Palette::saturatedAddNormalizedGray(Palette& output, byte firstIndex, byte lastIndex, int grayDividend, int grayDenominator) {
	assert(grayDenominator != 0);
	const signed r = _rMax * grayDividend / grayDenominator;
	const signed g = _gMax * grayDividend / grayDenominator;
	const signed b = _bMax * grayDividend / grayDenominator;

	return saturatedAddColor(output, firstIndex, lastIndex, r, g, b);
}

// a.k.a. transformColor
// Parameter color components (i.e. r, g and b) are in range [-7, 7]
// e.g. r = 7 sets the resulting color's red component to maximum
// e.g. r = -7 sets the resulting color's red component to minimum (i.e. zero)
Cine::Palette::Color Palette::saturatedAddColor(Cine::Palette::Color baseColor, signed r, signed g, signed b) const {
	Cine::Palette::Color result;
	result.r = CLIP<int>(baseColor.r + r, 0, _rMax);
	result.g = CLIP<int>(baseColor.g + g, 0, _gMax);
	result.b = CLIP<int>(baseColor.b + b, 0, _bMax);
	return result;
}

Palette &Palette::load(const byte *buf, const uint size, const Graphics::PixelFormat format, const uint numColors, const EndianType endian) {
	assert(format.bytesPerPixel * numColors <= size); // Make sure there's enough input space
	assert(format.aLoss == 8); // No alpha
	assert(format.rShift / 8 == (format.rShift + MAX<int>(0, 8 - format.rLoss - 1)) / 8); // R must be inside one byte
	assert(format.gShift / 8 == (format.gShift + MAX<int>(0, 8 - format.gLoss - 1)) / 8); // G must be inside one byte
	assert(format.bShift / 8 == (format.bShift + MAX<int>(0, 8 - format.bLoss - 1)) / 8); // B must be inside one byte

	setEndianType(endian);
	setColorFormat(format);

	_colors.clear();
	_colors.resize(numColors);

	const int rBytePos = bytePos(format.rShift, format.bytesPerPixel, isBigEndian(endian));
	const int gBytePos = bytePos(format.gShift, format.bytesPerPixel, isBigEndian(endian));
	const int bBytePos = bytePos(format.bShift, format.bytesPerPixel, isBigEndian(endian));
	
	for (uint i = 0; i < numColors; i++) {
		// _rMax, _gMax, _bMax are also used as masks here
		_colors[i].r = (buf[i * format.bytesPerPixel + rBytePos] >> (format.rShift % 8)) & _rMax;
		_colors[i].g = (buf[i * format.bytesPerPixel + gBytePos] >> (format.gShift % 8)) & _gMax;
		_colors[i].b = (buf[i * format.bytesPerPixel + bBytePos] >> (format.bShift % 8)) & _bMax;
	}

	return *this;
}

byte *Palette::save(byte *buf, const uint size, const EndianType endian) const {
	return save(buf, size, colorFormat(), colorCount(), endian);
}

byte *Palette::save(byte *buf, const uint size, const Graphics::PixelFormat format, const EndianType endian) const {
	return save(buf, size, format, colorCount(), endian);
}

byte *Palette::save(byte *buf, const uint size, const Graphics::PixelFormat format, const uint numColors, const EndianType endian, const byte firstIndex) const {
	assert(format.bytesPerPixel * numColors <= size); // Make sure there's enough output space
	assert(format.aLoss == 8); // No alpha
	assert(format.rShift / 8 == (format.rShift + MAX<int>(0, 8 - format.rLoss - 1)) / 8); // R must be inside one byte
	assert(format.gShift / 8 == (format.gShift + MAX<int>(0, 8 - format.gLoss - 1)) / 8); // G must be inside one byte
	assert(format.bShift / 8 == (format.bShift + MAX<int>(0, 8 - format.bLoss - 1)) / 8); // B must be inside one byte

	// Clear the part of the output palette we're going to be writing to with all black
	memset(buf, 0, format.bytesPerPixel * numColors);

	// Calculate how much bit shifting the color components need (for positioning them correctly)
	const signed rShiftLeft = (colorFormat().rLoss - (signed) format.rLoss) + (format.rShift % 8);
	const signed gShiftLeft = (colorFormat().gLoss - (signed) format.gLoss) + (format.gShift % 8);
	const signed bShiftLeft = (colorFormat().bLoss - (signed) format.bLoss) + (format.bShift % 8);

	// Calculate the byte masks for each color component (for masking away excess bits)
	const byte rMask = ((1 << (8 - format.rLoss)) - 1) << (format.rShift % 8);
	const byte gMask = ((1 << (8 - format.gLoss)) - 1) << (format.gShift % 8);
	const byte bMask = ((1 << (8 - format.bLoss)) - 1) << (format.bShift % 8);

	const int rBytePos = bytePos(format.rShift, format.bytesPerPixel, isBigEndian(endian));
	const int gBytePos = bytePos(format.gShift, format.bytesPerPixel, isBigEndian(endian));
	const int bBytePos = bytePos(format.bShift, format.bytesPerPixel, isBigEndian(endian));

	// Save the palette to the output in the specified format
	for (uint i = firstIndex; i < firstIndex + numColors; i++) {
		// _rMax, _gMax, _bMax are also used as masks here
		buf[i * format.bytesPerPixel + rBytePos] |= (shiftByteLeft(_colors[i].r, rShiftLeft) & rMask);
		buf[i * format.bytesPerPixel + gBytePos] |= (shiftByteLeft(_colors[i].g, gShiftLeft) & gMask);
		buf[i * format.bytesPerPixel + bBytePos] |= (shiftByteLeft(_colors[i].b, bShiftLeft) & bMask);
	}

	// Return the pointer to the output palette
	return buf;
}

} // End of namespace Cine
