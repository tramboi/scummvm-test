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

#ifndef SWORD25_GL_IMAGE_H
#define SWORD25_GL_IMAGE_H

// -----------------------------------------------------------------------------
// INCLUDES
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/gfx/image/image.h"
#include "sword25/gfx/graphicengine.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// FORWARD DECLARATION
// -----------------------------------------------------------------------------

typedef void *GLS_Sprite;

// -----------------------------------------------------------------------------
// CLASS DEFINITION
// -----------------------------------------------------------------------------

class GLImage : public Image {
public:
	GLImage(const Common::String &filename, bool &result);

	/**
	    @brief Erzeugt ein leeres BS_GLImage

	    @param Width die Breite des zu erzeugenden Bildes.
	    @param Height die H�he des zu erzeugenden Bildes
	    @param Result gibt dem Aufrufer bekannt, ob der Konstruktor erfolgreich ausgef�hrt wurde. Wenn es nach dem Aufruf false enthalten sollte,
	                  d�rfen keine Methoden am Objekt aufgerufen werden und das Objekt ist sofort zu zerst�ren.
	*/
	GLImage(uint width, uint height, bool &result);
	GLImage();

	virtual ~GLImage();

	virtual int getWidth() const {
		return _width;
	}
	virtual int getHeight() const {
		return _height;
	}
	virtual GraphicEngine::COLOR_FORMATS getColorFormat() const {
		return GraphicEngine::CF_ARGB32;
	}

	virtual bool blit(int posX = 0, int posY = 0,
	                  int flipping = Image::FLIP_NONE,
	                  Common::Rect *pPartRect = NULL,
	                  uint color = BS_ARGB(255, 255, 255, 255),
	                  int width = -1, int height = -1);
	virtual bool fill(const Common::Rect *pFillRect, uint color);
	virtual bool setContent(const byte *pixeldata, uint size, uint offset = 0, uint stride = 0);
	void replaceContent(byte *pixeldata, int width, int height);
	virtual uint getPixel(int x, int y);

	virtual bool isBlitSource() const               {
		return true;
	}
	virtual bool isBlitTarget() const               {
		return false;
	}
	virtual bool isScalingAllowed() const           {
		return true;
	}
	virtual bool isFillingAllowed() const           {
		return false;
	}
	virtual bool isAlphaAllowed() const             {
		return true;
	}
	virtual bool isColorModulationAllowed() const   {
		return true;
	}
	virtual bool isSetContentAllowed() const        {
		return true;
	}
private:
	byte *_data;
	int  _width;
	int  _height;
	bool _doCleanup;

	Graphics::Surface *_backSurface;
};

} // End of namespace Sword25

#endif
