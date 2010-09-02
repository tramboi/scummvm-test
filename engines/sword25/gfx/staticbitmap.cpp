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

#include "sword25/gfx/staticbitmap.h"
#include "sword25/gfx/bitmapresource.h"
#include "sword25/package/packagemanager.h"
#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/kernel/inputpersistenceblock.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Logging
// -----------------------------------------------------------------------------

#define BS_LOG_PREFIX "STATICBITMAP"

// -----------------------------------------------------------------------------
// Konstruktion / Destruktion
// -----------------------------------------------------------------------------

StaticBitmap::StaticBitmap(RenderObjectPtr<RenderObject> parentPtr, const Common::String &filename) :
	Bitmap(parentPtr, TYPE_STATICBITMAP) {
	// Das BS_Bitmap konnte nicht erzeugt werden, daher muss an dieser Stelle abgebrochen werden.
	if (!_initSuccess)
		return;

	_initSuccess = initBitmapResource(filename);
}

// -----------------------------------------------------------------------------

StaticBitmap::StaticBitmap(InputPersistenceBlock &reader, RenderObjectPtr<RenderObject> parentPtr, uint handle) :
	Bitmap(parentPtr, TYPE_STATICBITMAP, handle) {
	_initSuccess = unpersist(reader);
}

// -----------------------------------------------------------------------------

bool StaticBitmap::initBitmapResource(const Common::String &filename) {
	// Bild-Resource laden
	Resource *resourcePtr = Kernel::GetInstance()->GetResourceManager()->RequestResource(filename);
	if (!resourcePtr) {
		BS_LOG_ERRORLN("Could not request resource \"%s\".", filename.c_str());
		return false;
	}
	if (resourcePtr->GetType() != Resource::TYPE_BITMAP) {
		BS_LOG_ERRORLN("Requested resource \"%s\" is not a bitmap.", filename.c_str());
		return false;
	}

	BitmapResource *bitmapPtr = static_cast<BitmapResource *>(resourcePtr);

	// Den eindeutigen Dateinamen zum sp�teren Referenzieren speichern
	_resourceFilename = bitmapPtr->getFileName();

	// RenderObject Eigenschaften aktualisieren
	_originalWidth = _width = bitmapPtr->getWidth();
	_originalHeight = _height = bitmapPtr->getHeight();

	// Bild-Resource freigeben
	bitmapPtr->release();

	return true;
}

// -----------------------------------------------------------------------------

StaticBitmap::~StaticBitmap() {
}

// -----------------------------------------------------------------------------

bool StaticBitmap::doRender() {
	// Bitmap holen
	Resource *resourcePtr = Kernel::GetInstance()->GetResourceManager()->RequestResource(_resourceFilename);
	BS_ASSERT(resourcePtr);
	BS_ASSERT(resourcePtr->GetType() == Resource::TYPE_BITMAP);
	BitmapResource *bitmapResourcePtr = static_cast<BitmapResource *>(resourcePtr);

	// Framebufferobjekt holen
	GraphicEngine *gfxPtr = static_cast<GraphicEngine *>(Kernel::GetInstance()->GetService("gfx"));
	BS_ASSERT(gfxPtr);

	// Bitmap zeichnen
	bool result;
	if (_scaleFactorX == 1.0f && _scaleFactorY == 1.0f) {
		result = bitmapResourcePtr->blit(_absoluteX, _absoluteY,
		                                 (_flipV ? BitmapResource::FLIP_V : 0) |
		                                 (_flipH ? BitmapResource::FLIP_H : 0),
		                                 0, _modulationColor, -1, -1);
	} else {
		result = bitmapResourcePtr->blit(_absoluteX, _absoluteY,
		                                 (_flipV ? BitmapResource::FLIP_V : 0) |
		                                 (_flipH ? BitmapResource::FLIP_H : 0),
		                                 0, _modulationColor, _width, _height);
	}

	// Resource freigeben
	bitmapResourcePtr->release();

	return result;
}

// -----------------------------------------------------------------------------

uint StaticBitmap::getPixel(int x, int y) const {
	BS_ASSERT(x >= 0 && x < _width);
	BS_ASSERT(y >= 0 && y < _height);

	Resource *pResource = Kernel::GetInstance()->GetResourceManager()->RequestResource(_resourceFilename);
	BS_ASSERT(pResource->GetType() == Resource::TYPE_BITMAP);
	BitmapResource *pBitmapResource = static_cast<BitmapResource *>(pResource);
	uint result = pBitmapResource->getPixel(x, y);
	pResource->release();
	return result;
}

// -----------------------------------------------------------------------------

bool StaticBitmap::setContent(const byte *pixeldata, uint size, uint offset, uint stride) {
	BS_LOG_ERRORLN("SetContent() ist not supported with this object.");
	return false;
}

// -----------------------------------------------------------------------------
// Auskunftsmethoden
// -----------------------------------------------------------------------------

bool StaticBitmap::isAlphaAllowed() const {
	Resource *pResource = Kernel::GetInstance()->GetResourceManager()->RequestResource(_resourceFilename);
	BS_ASSERT(pResource->GetType() == Resource::TYPE_BITMAP);
	bool result = static_cast<BitmapResource *>(pResource)->isAlphaAllowed();
	pResource->release();
	return result;
}

// -----------------------------------------------------------------------------

bool StaticBitmap::isColorModulationAllowed() const {
	Resource *pResource = Kernel::GetInstance()->GetResourceManager()->RequestResource(_resourceFilename);
	BS_ASSERT(pResource->GetType() == Resource::TYPE_BITMAP);
	bool result = static_cast<BitmapResource *>(pResource)->isColorModulationAllowed();
	pResource->release();
	return result;
}

// -----------------------------------------------------------------------------

bool StaticBitmap::isScalingAllowed() const {
	Resource *pResource = Kernel::GetInstance()->GetResourceManager()->RequestResource(_resourceFilename);
	BS_ASSERT(pResource->GetType() == Resource::TYPE_BITMAP);
	bool result = static_cast<BitmapResource *>(pResource)->isScalingAllowed();
	pResource->release();
	return result;
}

// -----------------------------------------------------------------------------
// Persistenz
// -----------------------------------------------------------------------------

bool StaticBitmap::persist(OutputPersistenceBlock &writer) {
	bool result = true;

	result &= Bitmap::persist(writer);
	writer.write(_resourceFilename);

	result &= RenderObject::persistChildren(writer);

	return result;
}

bool StaticBitmap::unpersist(InputPersistenceBlock &reader) {
	bool result = true;

	result &= Bitmap::unpersist(reader);
	Common::String resourceFilename;
	reader.read(resourceFilename);
	result &= initBitmapResource(resourceFilename);

	result &= RenderObject::unpersistChildren(reader);

	return reader.isGood() && result;
}

} // End of namespace Sword25
