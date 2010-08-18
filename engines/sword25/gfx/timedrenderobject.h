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
// System Includes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Engine Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/gfx/renderobject.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Class Definition
// -----------------------------------------------------------------------------

/**
    @brief
*/

class TimedRenderObject : public RenderObject {
public:
	TimedRenderObject(RenderObjectPtr<RenderObject> pParent, TYPES Type, unsigned int Handle = 0);
	~TimedRenderObject();

	/**
	    @brief Teilt dem Objekt mit, dass ein neuer Frame begonnen wird.

	    Diese Methode wird jeden Frame an jedem BS_TimedRenderObject aufgerufen um diesen zu ermöglichen
	    ihren Zustand Zeitabhängig zu verändern (z.B. Animationen).<br>
	    @param int TimeElapsed gibt an wie viel Zeit (in Microsekunden) seit dem letzten Frame vergangen ist.
	*/
	virtual void FrameNotification(int TimeElapsed) = 0;
};

} // End of namespace Sword25
