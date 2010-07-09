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

#ifndef BACKENDS_GRAPHICS_OPENGLSDL_H
#define BACKENDS_GRAPHICS_OPENGLSDL_H

#include "backends/graphics/opengl/opengl-graphics.h"

/**
 * SDL OpenGL graphics manager
 */
class OpenGLSDLGraphicsManager : public OpenGLGraphicsManager {
public:
	OpenGLSDLGraphicsManager();
	virtual ~OpenGLSDLGraphicsManager();

	virtual void init();

	virtual void forceFullRedraw();
	virtual bool handleScalerHotkeys(const SDL_KeyboardEvent &key);
	virtual bool isScalerHotkey(const Common::Event &event);
	virtual void adjustMouseEvent(Common::Event &event);
	virtual void setMousePos(int x, int y);
	virtual void toggleFullScreen();
	virtual bool saveScreenshot(const char *filename);
};

#endif