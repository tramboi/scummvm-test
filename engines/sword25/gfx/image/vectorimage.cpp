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

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/bs_stdint.h"
#include "sword25/gfx/image/vectorimage.h"

#include "graphics/colormasks.h"

#include <libart_lgpl/art_vpath_bpath.h>

namespace Sword25 {

#define BS_LOG_PREFIX "VECTORIMAGE"

#define BEZSMOOTHNESS 0.5

// -----------------------------------------------------------------------------
// SWF Datentypen
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Bitstream Hilfsklasse
// -----------------------------------------------------------------------------
// Das Parsen von SWF-Dateien erfordert sowohl bitweises Auslesen als auch an
// Bytegrenzen ausgerichtetes Lesen.
// Diese Klasse ist speziell daf�r ausgestattet.
// -----------------------------------------------------------------------------

class VectorImage::SWFBitStream {
public:
	SWFBitStream(const byte *pData, uint dataSize) :
		m_Pos(pData), m_End(pData + dataSize), m_WordMask(0)
	{}

	inline uint32 getBits(uint bitCount) {
		if (bitCount == 0 || bitCount > 32) {
			error("SWFBitStream::GetBits() must read at least 1 and at most 32 bits at a time");
		}

		uint32 value = 0;
		while (bitCount) {
			if (m_WordMask == 0)
				flushByte();

			value <<= 1;
			value |= ((m_Word & m_WordMask) != 0) ? 1 : 0;
			m_WordMask >>= 1;

			--bitCount;
		}

		return value;
	}

	inline int32 getSignedBits(uint bitCount) {
		// Bits einlesen
		uint32 temp = getBits(bitCount);

		// Falls das Sign-Bit gesetzt ist, den Rest des R�ckgabewertes mit 1-Bits auff�llen (Sign Extension)
		if (temp & 1 << (bitCount - 1))
			return (0xffffffff << bitCount) | temp;
		else
			return temp;
	}

	inline uint32 getUInt32() {
		uint32 byte1 = getByte();
		uint32 byte2 = getByte();
		uint32 byte3 = getByte();
		uint32 byte4 = getByte();

		return byte1 | (byte2 << 8) | (byte3 << 16) | (byte4 << 24);
	}

	inline uint16 getUInt16() {
		uint32 byte1 = getByte();
		uint32 byte2 = getByte();

		return byte1 | (byte2 << 8);
	}

	inline byte getByte() {
		flushByte();
		byte value = m_Word;
		m_WordMask = 0;
		flushByte();

		return value;
	}

	inline void flushByte() {
		if (m_WordMask != 128) {
			if (m_Pos >= m_End) {
				error("Attempted to read past end of file");
			} else {
				m_Word = *m_Pos++;
				m_WordMask = 128;
			}
		}
	}

	inline void skipBytes(uint skipLength) {
		flushByte();
		if (m_Pos + skipLength >= m_End) {
			error("Attempted to read past end of file");
		} else {
			m_Pos += skipLength;
			m_Word = *(m_Pos - 1);
		}
	}

private:
	const byte    *m_Pos;
	const byte    *m_End;

	byte m_Word;
	uint m_WordMask;
};


// -----------------------------------------------------------------------------
// Konstanten und Hilfsfunktionen
// -----------------------------------------------------------------------------

namespace {
// -----------------------------------------------------------------------------
// Konstanten
// -----------------------------------------------------------------------------

const uint32 MAX_ACCEPTED_FLASH_VERSION = 3;   // Die h�chste Flash-Dateiversion, die vom Lader akzeptiert wird


// -----------------------------------------------------------------------------
// Konvertiert SWF-Rechteckdaten in einem Bitstrom in Common::Rect-Objekte
// -----------------------------------------------------------------------------

Common::Rect flashRectToBSRect(VectorImage::SWFBitStream &bs) {
	bs.flushByte();

	// Feststellen mit wie vielen Bits die einzelnen Komponenten kodiert sind
	uint32 bitsPerValue = bs.getBits(5);

	// Die einzelnen Komponenten einlesen
	int32 xMin = bs.getSignedBits(bitsPerValue);
	int32 xMax = bs.getSignedBits(bitsPerValue);
	int32 yMin = bs.getSignedBits(bitsPerValue);
	int32 yMax = bs.getSignedBits(bitsPerValue);

	return Common::Rect(xMin, yMin, xMax + 1, yMax + 1);
}


// -----------------------------------------------------------------------------
// Konvertiert SWF-Farben in AntiGrain Farben
// -----------------------------------------------------------------------------

uint32 flashColorToAGGRGBA8(uint flashColor) {
	uint32 resultColor = Graphics::ARGBToColor<Graphics::ColorMasks<8888> >(flashColor >> 24, (flashColor >> 16) & 0xff, (flashColor >> 8) & 0xff, flashColor & 0xff);

	return resultColor;
}


// -----------------------------------------------------------------------------
// Berechnet die Bounding-Box eines BS_VectorImageElement
// -----------------------------------------------------------------------------

Common::Rect CalculateBoundingBox(const VectorImageElement &vectorImageElement) {
	double x0, y0, x1, y1;

	for (int j = vectorImageElement.getPathCount() - 1; j >= 0; j--) {
		ArtVpath *vec = vectorImageElement.getPathInfo(j).getVec();

		if (vec[0].code == ART_END) {
			continue;
		} else {
			x0 = x1 = vec[0].x;
			y0 = y1 = vec[0].y;
			for (int i = 1; vec[i].code != ART_END; i++) {
				if (vec[i].x < x0) x0 = vec[i].x;
				if (vec[i].x > x1) x1 = vec[i].x;
				if (vec[i].y < y0) y0 = vec[i].y;
				if (vec[i].y > y1) y1 = vec[i].y;
			}
		}
	}

	return Common::Rect(static_cast<int>(x0), static_cast<int>(y0), static_cast<int>(x1) + 1, static_cast<int>(y1) + 1);
}

}


// -----------------------------------------------------------------------------
// Konstruktion
// -----------------------------------------------------------------------------

VectorImage::VectorImage(const byte *pFileData, uint fileSize, bool &success) {
	success = false;

	// Bitstream-Objekt erzeugen
	// Im Folgenden werden die Dateidaten aus diesem ausgelesen.
	SWFBitStream bs(pFileData, fileSize);

	// SWF-Signatur �berpr�fen
	uint32 signature[3];
	signature[0] = bs.getByte();
	signature[1] = bs.getByte();
	signature[2] = bs.getByte();
	if (signature[0] != 'F' ||
		signature[1] != 'W' ||
		signature[2] != 'S') {
		BS_LOG_ERRORLN("File is not a valid SWF-file");
		return;
	}

	// Versionsangabe �berpr�fen
	uint32 version = bs.getByte();
	if (version > MAX_ACCEPTED_FLASH_VERSION) {
		BS_LOG_ERRORLN("File is of version %d. Highest accepted version is %d.", version, MAX_ACCEPTED_FLASH_VERSION);
		return;
	}

	// Dateigr��e auslesen und mit der tats�chlichen Gr��e vergleichen
	uint32 storedFileSize = bs.getUInt32();
	if (storedFileSize != fileSize) {
		BS_LOG_ERRORLN("File is not a valid SWF-file");
		return;
	}

	// SWF-Ma�e auslesen
	Common::Rect movieRect = flashRectToBSRect(bs);

	// Framerate und Frameanzahl auslesen
	/* uint32 frameRate = */bs.getUInt16();
	/* uint32 frameCount = */bs.getUInt16();

	// Tags parsen
	// Da wir uns nur f�r das erste DefineShape-Tag interessieren
	bool keepParsing = true;
	while (keepParsing) {
		// Tags beginnen immer an Bytegrenzen
		bs.flushByte();

		// Tagtyp und L�nge auslesen
		uint16 tagTypeAndLength = bs.getUInt16();
		uint32 tagType = tagTypeAndLength >> 6;
		uint32 tagLength = tagTypeAndLength & 0x3f;
		if (tagLength == 0x3f)
			tagLength = bs.getUInt32();

		switch (tagType) {
		case 2:
			// DefineShape
			success = parseDefineShape(2, bs);
			return;
		case 22:
			// DefineShape2
			success = parseDefineShape(2, bs);
			return;
		case 32:
			success = parseDefineShape(3, bs);
			return;
		default:
			// Unbekannte Tags ignorieren
			bs.skipBytes(tagLength);
		}
	}

	// Die Ausf�hrung darf nicht an dieser Stelle ankommen: Entweder es wird ein Shape gefunden, dann wird die Funktion mit vorher verlassen, oder
	// es wird keines gefunden, dann tritt eine Exception auf sobald �ber das Ende der Datei hinaus gelesen wird.
	BS_ASSERT(false);
}

VectorImage::~VectorImage() {
	for (int j = _elements.size() - 1; j >= 0; j--)
		for (int i = _elements[j].getPathCount() - 1; i >= 0; i--)
			if (_elements[j].getPathInfo(i).getVec())
				art_free(_elements[j].getPathInfo(i).getVec());
}


ArtBpath *ensureBezStorage(ArtBpath *bez, int nodes, int *allocated) {
	if (*allocated <= nodes) {
		(*allocated) += 20;

		return art_renew(bez, ArtBpath, *allocated);
	}

	return bez;
}

ArtBpath *VectorImage::storeBez(ArtBpath *bez, int lineStyle, int fillStyle0, int fillStyle1, int *bezNodes, int *bezAllocated) {
	(*bezNodes)++;

	bez = ensureBezStorage(bez, *bezNodes, bezAllocated);
	bez[*bezNodes].code = ART_END;

	ArtVpath *vec = art_bez_path_to_vec(bez, BEZSMOOTHNESS);

	_elements.back()._pathInfos.push_back(VectorPathInfo(vec, lineStyle, fillStyle0, fillStyle1));

	return bez;
}

#define SWF_SCALE_FACTOR		(1/20.0)

bool VectorImage::parseDefineShape(uint shapeType, SWFBitStream &bs) {
	/*uint32 shapeID = */bs.getUInt16();

	// Bounding Box auslesen
	_boundingBox = flashRectToBSRect(bs);

	// Erstes Image-Element erzeugen
	_elements.resize(1);

	// Styles einlesen
	uint numFillBits;
	uint numLineBits;
	if (!parseStyles(shapeType, bs, numFillBits, numLineBits))
		return false;

	uint lineStyle = 0;
	uint fillStyle0 = 0;
	uint fillStyle1 = 0;

	// Shaperecord parsen
	// ------------------

	double curX = 0;
	double curY = 0;
	int bezNodes = 0;
	int bezAllocated = 10;
	ArtBpath *bez = art_new(ArtBpath, bezAllocated);

	bool endOfShapeDiscovered = false;
	while (!endOfShapeDiscovered) {
		uint32 typeFlag = bs.getBits(1);

		// Non-Edge Record
		if (typeFlag == 0) {
			// Feststellen welche Parameter gesetzt werden
			uint32 stateNewStyles = bs.getBits(1);
			uint32 stateLineStyle = bs.getBits(1);
			uint32 stateFillStyle0 = bs.getBits(1);
			uint32 stateFillStyle1 = bs.getBits(1);
			uint32 stateMoveTo = bs.getBits(1);

			// End der Shape-Definition erreicht?
			if (!stateNewStyles && !stateLineStyle && !stateFillStyle0 && !stateFillStyle1 && !stateMoveTo) {
				endOfShapeDiscovered = true;
			// Parameter dekodieren
			} else {
				if (stateMoveTo) {
					uint32 moveToBits = bs.getBits(5);
					curX = bs.getSignedBits(moveToBits) * SWF_SCALE_FACTOR;
					curY = bs.getSignedBits(moveToBits) * SWF_SCALE_FACTOR;
				}

				if (stateFillStyle0) {
					if (numFillBits > 0)
						fillStyle0 = bs.getBits(numFillBits);
					else
						fillStyle0 = 0;
				}

				if (stateFillStyle1) {
					if (numFillBits > 0)
						fillStyle1 = bs.getBits(numFillBits);
					else
						fillStyle1 = 0;
				}

				if (stateLineStyle) {
					if (numLineBits)
						lineStyle = bs.getBits(numLineBits);
					else
						numLineBits = 0;
				}

				if (stateNewStyles) {
					// An dieser Stelle werden in Flash die alten Style-Definitionen verworfen und mit den neuen �berschrieben.
					// Es wird ein neues Element begonnen.
					_elements.resize(_elements.size() + 1);
					if (!parseStyles(shapeType, bs, numFillBits, numLineBits))
						return false;
				}

				// Ein neuen Pfad erzeugen, es sei denn, es wurden nur neue Styles definiert
				if (stateLineStyle || stateFillStyle0 || stateFillStyle1 || stateMoveTo) {
					// Store previous curve if any
					if (bezNodes) {
						bez = storeBez(bez, lineStyle, fillStyle0, fillStyle1, &bezNodes, &bezAllocated);
					}

					// Start new curve
					bez = ensureBezStorage(bez, 1, &bezAllocated);
					bez[0].code = ART_MOVETO_OPEN;
					bez[0].x3 = curX;
					bez[0].y3 = curY;
					bezNodes = 0;
				}
			}
		} else {
			// Edge Record
			uint32 edgeFlag = bs.getBits(1);
			uint32 numBits = bs.getBits(4) + 2;

			// Curved edge
			if (edgeFlag == 0) {
				double controlDeltaX = bs.getSignedBits(numBits) * SWF_SCALE_FACTOR;
				double controlDeltaY = bs.getSignedBits(numBits) * SWF_SCALE_FACTOR;
				double anchorDeltaX = bs.getSignedBits(numBits) * SWF_SCALE_FACTOR;
				double anchorDeltaY = bs.getSignedBits(numBits) * SWF_SCALE_FACTOR;

				double newX = curX + controlDeltaX;
				double newY = curY + controlDeltaY;
				double anchorX = curX + anchorDeltaX;
				double anchorY = curY + anchorDeltaY;

#define WEIGHT (2.0/3.0)

				bezNodes++;
				bez = ensureBezStorage(bez, bezNodes, &bezAllocated);
				bez[bezNodes].code = ART_CURVETO;
				bez[bezNodes].x1 = WEIGHT * anchorX + (1 - WEIGHT) * curX;
				bez[bezNodes].y1 = WEIGHT * anchorY + (1 - WEIGHT) * curY;
				bez[bezNodes].x2 = WEIGHT * anchorX + (1 - WEIGHT) * newX;
				bez[bezNodes].y2 = WEIGHT * anchorY + (1 - WEIGHT) * newY;
				bez[bezNodes].x3 = newX;
				bez[bezNodes].y3 = newY;

				curX = newX;
				curY = newY;
			} else {
				// Staight edge
				int32 deltaX = 0;
				int32 deltaY = 0;

				uint32 generalLineFlag = bs.getBits(1);
				if (generalLineFlag) {
					deltaX = bs.getSignedBits(numBits);
					deltaY = bs.getSignedBits(numBits);
				} else {
					uint32 vertLineFlag = bs.getBits(1);
					if (vertLineFlag)
						deltaY = bs.getSignedBits(numBits);
					else
						deltaX = bs.getSignedBits(numBits);
				}

				curX += deltaX * SWF_SCALE_FACTOR;
				curY += deltaY * SWF_SCALE_FACTOR;

				bezNodes++;
				bez = ensureBezStorage(bez, bezNodes, &bezAllocated);
				bez[bezNodes].code = ART_LINETO;
				bez[bezNodes].x3 = curX;
				bez[bezNodes].y3 = curY;
			}
		}
	}

	// Store last curve
	if (bezNodes)
		bez = storeBez(bez, lineStyle, fillStyle0, fillStyle1, &bezNodes, &bezAllocated);

	art_free(bez);

	// Bounding-Boxes der einzelnen Elemente berechnen
	Common::Array<VectorImageElement>::iterator it = _elements.begin();
	for (; it != _elements.end(); ++it)
		it->_boundingBox = CalculateBoundingBox(*it);

	return true;
}


// -----------------------------------------------------------------------------

bool VectorImage::parseStyles(uint shapeType, SWFBitStream &bs, uint &numFillBits, uint &numLineBits) {
	bs.flushByte();

	// Fillstyles parsen
	// -----------------

	// Anzahl an Fillstyles bestimmen
	uint fillStyleCount = bs.getByte();
	if (fillStyleCount == 0xff)
		fillStyleCount = bs.getUInt16();

	// Alle Fillstyles einlesen, falls ein Fillstyle mit Typ != 0 gefunden wird, wird das Parsen abgebrochen.
	// Es wird nur "solid fill" (Typ 0) unterst�tzt.
	_elements.back()._fillStyles.reserve(fillStyleCount);
	for (uint i = 0; i < fillStyleCount; ++i) {
		byte type = bs.getByte();
		uint32 color;
		if (shapeType == 3) {
			color = (bs.getByte() << 16) | (bs.getByte() << 8) | bs.getByte() | (bs.getByte() << 24);
		} else
			color = bs.getBits(24) | (0xff << 24);
		if (type != 0)
			return false;

		_elements.back()._fillStyles.push_back(flashColorToAGGRGBA8(color));
	}

	// Linestyles parsen
	// -----------------

	// Anzahl an Linestyles bestimmen
	uint lineStyleCount = bs.getByte();
	if (lineStyleCount == 0xff)
		lineStyleCount = bs.getUInt16();

	// Alle Linestyles einlesen
	_elements.back()._lineStyles.reserve(lineStyleCount);
	for (uint i = 0; i < lineStyleCount; ++i) {
		double width = bs.getUInt16();
		uint32 color;
		if (shapeType == 3)
			color = (bs.getByte() << 16) | (bs.getByte() << 8) | bs.getByte() | (bs.getByte() << 24);
		else
			color = bs.getBits(24) | (0xff << 24);

		_elements.back()._lineStyles.push_back(VectorImageElement::LineStyleType(width, flashColorToAGGRGBA8(color)));
	}

	// Bitbreite f�r die folgenden Styleindizes auslesen
	numFillBits = bs.getBits(4);
	numLineBits = bs.getBits(4);

	return true;
}


// -----------------------------------------------------------------------------

bool VectorImage::fill(const Common::Rect *pFillRect, uint color) {
	BS_LOG_ERRORLN("Fill() is not supported.");
	return false;
}


// -----------------------------------------------------------------------------

uint VectorImage::getPixel(int x, int y) {
	BS_LOG_ERRORLN("GetPixel() is not supported. Returning black.");
	return 0;
}

// -----------------------------------------------------------------------------

bool VectorImage::setContent(const byte *pixeldata, uint size, uint offset, uint stride) {
	BS_LOG_ERRORLN("SetContent() is not supported.");
	return 0;
}

} // End of namespace Sword25
