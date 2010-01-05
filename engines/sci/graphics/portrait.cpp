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

#include "common/util.h"
#include "common/stack.h"
#include "graphics/primitives.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/portrait.h"

namespace Sci {

Portrait::Portrait(ResourceManager *resMan, Screen *screen, SciPalette *palette, Common::String resourceName)
	: _resMan(resMan), _screen(screen), _palette(palette), _resourceName(resourceName) {
	init();
}

Portrait::~Portrait() {
}

void Portrait::init() {
	// .BIN files are loaded from actors directory and from .\ directory
	// header:
	// 3 bytes "WIN"
	// 2 bytes main height (should be the same as first bitmap header height)
	// 2 bytes main width (should be the same as first bitmap header width)
	// 2 bytes animation count
	// 2 bytes unknown
	// 2 bytes unknown
	// 4 bytes paletteSize (base 1)
	// paletteSize bytes paletteData
	// 14 bytes bitmap header
	//  -> 4 bytes unknown
	//  -> 2 bytes height
	//  -> 2 bytes width
	//  -> 6 bytes unknown
	// height * width bitmap data
	// another animation count times bitmap header and data
}

} // End of namespace Sci
