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
// INCLUDES
// -----------------------------------------------------------------------------

#include "sword25/package/packagemanager.h"
#include "sword25/gfx/image/imageloader.h"
#include "sword25/gfx/image/swimage.h"

namespace Sword25 {

#define BS_LOG_PREFIX "SWIMAGE"


// -----------------------------------------------------------------------------
// CONSTRUCTION / DESTRUCTION
// -----------------------------------------------------------------------------

SWImage::SWImage(const Common::String &filename, bool &result) :
	_imageDataPtr(0),
	_width(0),
	_height(0) {
	result = false;

	PackageManager *pPackage = Kernel::GetInstance()->GetPackage();
	BS_ASSERT(pPackage);

	// Datei laden
	byte *pFileData;
	uint fileSize;
	if (!(pFileData = (byte *)pPackage->getFile(filename, &fileSize))) {
		BS_LOG_ERRORLN("File \"%s\" could not be loaded.", filename.c_str());
		return;
	}

	// Bildeigenschaften bestimmen
	GraphicEngine::COLOR_FORMATS colorFormat;
	int pitch;
	if (!ImageLoader::ExtractImageProperties(pFileData, fileSize, colorFormat, _width, _height)) {
		BS_LOG_ERRORLN("Could not read image properties.");
		return;
	}

	// Das Bild dekomprimieren
	byte *pUncompressedData;
	if (!ImageLoader::LoadImage(pFileData, fileSize, GraphicEngine::CF_ARGB32, pUncompressedData, _width, _height, pitch)) {
		BS_LOG_ERRORLN("Could not decode image.");
		return;
	}

	// Dateidaten freigeben
	delete[] pFileData;

	_imageDataPtr = (uint *)pUncompressedData;

	result = true;
	return;
}

// -----------------------------------------------------------------------------

SWImage::~SWImage() {
	delete[] _imageDataPtr;
}


// -----------------------------------------------------------------------------

bool SWImage::blit(int posX, int posY,
                      int flipping,
                      Common::Rect *pPartRect,
                      uint color,
                      int width, int height) {
	BS_LOG_ERRORLN("Blit() is not supported.");
	return false;
}

// -----------------------------------------------------------------------------

bool SWImage::fill(const Common::Rect *pFillRect, uint color) {
	BS_LOG_ERRORLN("Fill() is not supported.");
	return false;
}

// -----------------------------------------------------------------------------

bool SWImage::setContent(const byte *pixeldata, uint size, uint offset, uint stride) {
	BS_LOG_ERRORLN("SetContent() is not supported.");
	return false;
}

// -----------------------------------------------------------------------------

uint SWImage::getPixel(int x, int y) {
	BS_ASSERT(x >= 0 && x < _width);
	BS_ASSERT(y >= 0 && y < _height);

	return _imageDataPtr[_width * y + x];
}

} // End of namespace Sword25
