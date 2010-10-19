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

#include "sword25/gfx/image/imageloader.h"
#include "sword25/gfx/image/imageloader_ids.h"

namespace Sword25 {

#define BS_LOG_PREFIX "IMAGELOADER"

// Statische Elemente der Klasse BS_ImageLoader intialisieren.
Common::List<ImageLoader *> ImageLoader::_imageLoaderList;
bool ImageLoader::_imageLoaderListInitialized = false;

bool ImageLoader::loadImage(const byte *pFileData, uint fileSize,
                            GraphicEngine::COLOR_FORMATS colorFormat,
                            byte *&pUncompressedData,
                            int &width, int &height,
                            int &pitch) {
	// Falls die Liste der BS_ImageLoader noch nicht initialisiert wurde, wird dies getan.
	if (!_imageLoaderListInitialized)
		initializeLoaderList();

	// Passenden BS_ImageLoader finden und Bild dekodieren
	ImageLoader *pLoader = findSuitableImageLoader(pFileData, fileSize);
	if (pLoader) {
		return pLoader->decodeImage(pFileData, fileSize,
		                            colorFormat,
		                            pUncompressedData,
		                            width, height,
		                            pitch);
	}

	return false;
}

bool ImageLoader::extractImageProperties(const byte *pFileData, uint fileSize,
        GraphicEngine::COLOR_FORMATS &colorFormat,
        int &width, int &height) {
	// Falls die Liste der BS_ImageLoader noch nicht initialisiert wurde, wird dies getan.
	if (!_imageLoaderListInitialized)
		initializeLoaderList();

	// Passenden BS_ImageLoader finden und Bildeigenschaften auslesen.
	ImageLoader *pLoader = findSuitableImageLoader(pFileData, fileSize);
	if (pLoader) {
		return pLoader->imageProperties(pFileData, fileSize,
		                                colorFormat,
		                                width, height);
	}

	return false;
}

void ImageLoader::initializeLoaderList() {
	// Von jedem BS_ImageLoader wird eine Instanz erzeugt, diese f�gen sich selbst�ndig in die BS_ImageLoader-Liste ein.
	for (int i = 0; i < BS_IMAGELOADER_COUNT; i++)
		BS_IMAGELOADER_IDS[i]();

	// Die Liste als gef�llt markieren.
	_imageLoaderListInitialized = true;

	// Sicherstellen, dass beim Beenden alle BS_ImageLoader Instanzen zerst�rt werden.
	atexit(ImageLoader::deinitializeLoaderList);
}

void ImageLoader::deinitializeLoaderList() {
	while (!_imageLoaderList.empty()) {
		delete _imageLoaderList.back();
		_imageLoaderList.pop_back();
	}
}

ImageLoader *ImageLoader::findSuitableImageLoader(const byte *pFileData, uint fileSize) {
	// Alle BS_ImageLoader-Objekte durchgehen, bis eins gefunden wurde, dass das Bild laden kann
	Common::List<ImageLoader *>::iterator iter = _imageLoaderList.begin();
	for (; iter != _imageLoaderList.end(); ++iter) {
		// Falls ein geeigneter BS-ImageLoader gefunden wurde, wird er zur�ckgegeben.
		if ((*iter)->isCorrectImageFormat(pFileData, fileSize)) {
			return (*iter);
		}
	}

	// Es konnte kein passender BS_ImageLoader gefunden werden.
	BS_LOG_ERRORLN("Could not find suitable image loader for image data.");
	return NULL;
}

} // End of namespace Sword25
