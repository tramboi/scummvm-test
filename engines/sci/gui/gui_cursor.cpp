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

#include "graphics/cursorman.h"
#include "common/util.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/tools.h"
#include "sci/gui/gui_palette.h"
#include "sci/gui/gui_view.h"
#include "sci/gui/gui_cursor.h"

namespace Sci {

SciGuiCursor::SciGuiCursor(EngineState *state, SciGuiPalette *palette)
	: _s(state), _palette(palette) {
	init();
}

SciGuiCursor::~SciGuiCursor() {
}

void SciGuiCursor::init() {
	_rawBitmap = NULL;
}

void SciGuiCursor::show() {
	CursorMan.showMouse(true);
}

void SciGuiCursor::hide() {
	CursorMan.showMouse(false);
}

void SciGuiCursor::setShape(GuiResourceId resourceId) {
	Resource *resource;
	byte *resourceData;
	Common::Point hotspot = Common::Point(0, 0);
	byte colorMapping[4];
	int16 x, y;
	byte color;
	int16 maskA, maskB;
	byte *pOut;

	if (resourceId == -1) {
		// no resourceId given, so we actually hide the cursor
		hide();
		return;
	}
	
	// Load cursor resource...
	resource = _s->resMan->findResource(ResourceId(kResourceTypeCursor, resourceId), false);
	if (!resource)
		error("cursor resource %d not found", resourceId);
	if (resource->size != SCI_CURSOR_SCI0_RESOURCESIZE)
		error("cursor resource %d has invalid size", resourceId);

	resourceData = resource->data;
	// hotspot is specified for SCI1 cursors
	hotspot.x = READ_LE_UINT16(resourceData);
	hotspot.y = READ_LE_UINT16(resourceData + 2);
	// bit 0 of resourceData[3] is set on <SCI1 games, which means center hotspot
	if ((hotspot.x == 0) && (hotspot.y == 256))
		hotspot.x = hotspot.y = SCI_CURSOR_SCI0_HEIGHTWIDTH / 2;

	// Now find out what colors we are supposed to use
	colorMapping[0] = 0; // Black is hardcoded
	colorMapping[1] = _palette->matchColor(&_palette->_sysPalette, 255, 255, 255); // White
	colorMapping[2] = SCI_CURSOR_SCI0_TRANSPARENCYCOLOR;
	colorMapping[3] = _palette->matchColor(&_palette->_sysPalette, 170, 170, 170); // Grey
	
	// Seek to actual data
	resourceData += 4;

	if (!_rawBitmap)
		_rawBitmap = new byte[SCI_CURSOR_SCI0_HEIGHTWIDTH*SCI_CURSOR_SCI0_HEIGHTWIDTH];

	pOut = _rawBitmap;
	for (y = 0; y < SCI_CURSOR_SCI0_HEIGHTWIDTH; y++) {
		maskA = READ_LE_UINT16(resourceData + (y << 1));
		maskB = READ_LE_UINT16(resourceData + 32 + (y << 1));

		for (x = 0; x < SCI_CURSOR_SCI0_HEIGHTWIDTH; x++) {
			color = (((maskA << x) & 0x8000) | (((maskB << x) >> 1) & 0x4000)) >> 14;
			*pOut++ = colorMapping[color];
		}
	}

	CursorMan.replaceCursor(_rawBitmap, SCI_CURSOR_SCI0_HEIGHTWIDTH, SCI_CURSOR_SCI0_HEIGHTWIDTH, hotspot.x, hotspot.y, SCI_CURSOR_SCI0_TRANSPARENCYCOLOR);
	CursorMan.showMouse(true);
}

void SciGuiCursor::setPosition(Common::Point pos) {
	g_system->warpMouse(pos.x, pos.y);
}

} // End of namespace Sci
