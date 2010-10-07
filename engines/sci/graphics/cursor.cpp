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

#include "common/events.h"
#include "common/macresman.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/cursorman.h"

#include "sci/sci.h"
#include "sci/event.h"
#include "sci/engine/state.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/coordadjuster.h"
#include "sci/graphics/view.h"
#include "sci/graphics/cursor.h"

namespace Sci {

GfxCursor::GfxCursor(ResourceManager *resMan, GfxPalette *palette, GfxScreen *screen)
	: _resMan(resMan), _palette(palette), _screen(screen) {

	_upscaledHires = _screen->getUpscaledHires();
	_isVisible = true;

	// center mouse cursor
	setPosition(Common::Point(_screen->getWidth() / 2, _screen->getHeight() / 2));
	_moveZoneActive = false;

	_zoomZoneActive = false;
	_zoomZone = Common::Rect();
	_zoomCursorView = 0;
	_zoomCursorLoop = 0;
	_zoomCursorCel = 0;
	_zoomPicView = 0;
	_zoomColor = 0;
	_zoomMultiplier = 0;
	_cursorSurface = 0;
}

GfxCursor::~GfxCursor() {
	purgeCache();
	kernelClearZoomZone();
}

void GfxCursor::init(GfxCoordAdjuster *coordAdjuster, EventManager *event) {
	_coordAdjuster = coordAdjuster;
	_event = event;
}

void GfxCursor::kernelShow() {
	CursorMan.showMouse(true);
	_isVisible = true;
}

void GfxCursor::kernelHide() {
	CursorMan.showMouse(false);
	_isVisible = false;
}

bool GfxCursor::isVisible() {
	return _isVisible;
}

void GfxCursor::purgeCache() {
	for (CursorCache::iterator iter = _cachedCursors.begin(); iter != _cachedCursors.end(); ++iter) {
		delete iter->_value;
		iter->_value = 0;
	}

	_cachedCursors.clear();
}

void GfxCursor::kernelSetShape(GuiResourceId resourceId) {
	Resource *resource;
	byte *resourceData;
	Common::Point hotspot = Common::Point(0, 0);
	byte colorMapping[4];
	int16 x, y;
	byte color;
	int16 maskA, maskB;
	byte *pOut;
	byte *rawBitmap = new byte[SCI_CURSOR_SCI0_HEIGHTWIDTH * SCI_CURSOR_SCI0_HEIGHTWIDTH];
	int16 heightWidth;

	if (resourceId == -1) {
		// no resourceId given, so we actually hide the cursor
		kernelHide();
		delete[] rawBitmap;
		return;
	}

	// Load cursor resource...
	resource = _resMan->findResource(ResourceId(kResourceTypeCursor, resourceId), false);
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
	colorMapping[1] = _screen->getColorWhite(); // White is also hardcoded
	colorMapping[2] = SCI_CURSOR_SCI0_TRANSPARENCYCOLOR;
	colorMapping[3] = _palette->matchColor(170, 170, 170); // Grey

	// Seek to actual data
	resourceData += 4;

	pOut = rawBitmap;
	for (y = 0; y < SCI_CURSOR_SCI0_HEIGHTWIDTH; y++) {
		maskA = READ_LE_UINT16(resourceData + (y << 1));
		maskB = READ_LE_UINT16(resourceData + 32 + (y << 1));

		for (x = 0; x < SCI_CURSOR_SCI0_HEIGHTWIDTH; x++) {
			color = (((maskA << x) & 0x8000) | (((maskB << x) >> 1) & 0x4000)) >> 14;
			*pOut++ = colorMapping[color];
		}
	}

	heightWidth = SCI_CURSOR_SCI0_HEIGHTWIDTH;

	if (_upscaledHires) {
		// Scale cursor by 2x - note: sierra didn't do this, but it looks much better
		heightWidth *= 2;
		hotspot.x *= 2;
		hotspot.y *= 2;
		byte *upscaledBitmap = new byte[heightWidth * heightWidth];
		_screen->scale2x(rawBitmap, upscaledBitmap, SCI_CURSOR_SCI0_HEIGHTWIDTH, SCI_CURSOR_SCI0_HEIGHTWIDTH);
		delete[] rawBitmap;
		rawBitmap = upscaledBitmap;
	}

	CursorMan.replaceCursor(rawBitmap, heightWidth, heightWidth, hotspot.x, hotspot.y, SCI_CURSOR_SCI0_TRANSPARENCYCOLOR);
	kernelShow();

	delete[] rawBitmap;
}

void GfxCursor::kernelSetView(GuiResourceId viewNum, int loopNum, int celNum, Common::Point *hotspot) {
	if (_cachedCursors.size() >= MAX_CACHED_CURSORS)
		purgeCache();

	if (!_cachedCursors.contains(viewNum))
		_cachedCursors[viewNum] = new GfxView(_resMan, _screen, _palette, viewNum);

	GfxView *cursorView = _cachedCursors[viewNum];

	const CelInfo *celInfo = cursorView->getCelInfo(loopNum, celNum);
	int16 width = celInfo->width;
	int16 height = celInfo->height;
	byte clearKey = celInfo->clearKey;
	Common::Point *cursorHotspot = hotspot;

	if (!cursorHotspot)
		// Compute hotspot from xoffset/yoffset
		cursorHotspot = new Common::Point((celInfo->width >> 1) - celInfo->displaceX, celInfo->height - celInfo->displaceY - 1);

	// Eco Quest 1 uses a 1x1 transparent cursor to hide the cursor from the
	// user. Some scalers don't seem to support this
	if (width < 2 || height < 2) {
		kernelHide();
		delete cursorHotspot;
		return;
	}

	const byte *rawBitmap = cursorView->getBitmap(loopNum, celNum);
	if (_upscaledHires) {
		// Scale cursor by 2x - note: sierra didn't do this, but it looks much better
		width *= 2;
		height *= 2;
		cursorHotspot->x *= 2;
		cursorHotspot->y *= 2;
		byte *cursorBitmap = new byte[width * height];
		_screen->scale2x(rawBitmap, cursorBitmap, celInfo->width, celInfo->height);
		CursorMan.replaceCursor(cursorBitmap, width, height, cursorHotspot->x, cursorHotspot->y, clearKey);
		delete[] cursorBitmap;
	} else {
		CursorMan.replaceCursor(rawBitmap, width, height, cursorHotspot->x, cursorHotspot->y, clearKey);
	}

	kernelShow();

	delete cursorHotspot;
}

void GfxCursor::kernelSetMacCursor(GuiResourceId viewNum, int loopNum, int celNum, Common::Point *hotspot) {
	// See http://developer.apple.com/legacy/mac/library/documentation/mac/QuickDraw/QuickDraw-402.html
	// for more information.

	// View 998 seems to be a fake resource used to call for Mac cursor resources.
	// For other resources, they're still in the views, so use them.
	if (viewNum != 998) {
		kernelSetView(viewNum, loopNum, celNum, hotspot);
		return;
	}

	// TODO: What about the 2000 resources? Inventory items? How to handle?
	// TODO: 1000 + celNum won't work for GK1

	Resource *resource = _resMan->findResource(ResourceId(kResourceTypeCursor, 1000 + celNum), false);

	if (!resource) {
		warning("Mac cursor %d not found", 1000 + celNum);
		return;
	}

	assert(resource);

	if (resource->size == 32 * 2 + 4) {
		// Mac CURS cursor
		byte *cursorBitmap = new byte[16 * 16];
		byte *data = resource->data;

		// Get B&W data
		for (byte i = 0; i < 32; i++) {
			byte imageByte = *data++;
			for (byte b = 0; b < 8; b++)
				cursorBitmap[i * 8 + b] = (byte)((imageByte & (0x80 >> b)) > 0 ? 0x00 : 0xFF);
		}

		// Apply mask data
		for (byte i = 0; i < 32; i++) {
			byte imageByte = *data++;
			for (byte b = 0; b < 8; b++)
				if ((imageByte & (0x80 >> b)) == 0)
					cursorBitmap[i * 8 + b] = SCI_CURSOR_SCI0_TRANSPARENCYCOLOR; // Doesn't matter, just is transparent
		}

		uint16 hotspotX = READ_BE_UINT16(data);
		uint16 hotspotY = READ_BE_UINT16(data + 2);

		CursorMan.replaceCursor(cursorBitmap, 16, 16, hotspotX, hotspotY, SCI_CURSOR_SCI0_TRANSPARENCYCOLOR);

		delete[] cursorBitmap;
	} else {
		// Mac crsr cursor
		byte *cursorBitmap, *palette;
		int width, height, hotspotX, hotspotY, palSize, keycolor;
		Common::MacResManager::convertCrsrCursor(resource->data, resource->size, &cursorBitmap, &width, &height, &hotspotX, &hotspotY, &keycolor, true, &palette, &palSize);
		CursorMan.replaceCursor(cursorBitmap, width, height, hotspotX, hotspotY, keycolor);
		CursorMan.replaceCursorPalette(palette, 0, palSize);
		free(cursorBitmap);
		free(palette);
	}

	kernelShow();
}

// this list contains all mandatory set cursor changes, that need special handling
//  ffs. GfxCursor::setPosition (below)
//    Game,            newPosition, validRect
static const SciCursorSetPositionWorkarounds setPositionWorkarounds[] = {
    { GID_ISLANDBRAIN, 84, 109,     46, 76, 174, 243 }, // island of dr. brain / game menu
    { GID_LSL5,        23, 171,     0, 0, 26, 320 },    // larry 5 / skip forward helper
    { GID_QFG1VGA,     64, 174,     40, 37, 74, 284 },  // Quest For Glory 1 VGA / run/walk/sleep sub-menu
    { (SciGameId)0,    -1, -1,     -1, -1, -1, -1 }
};

void GfxCursor::setPosition(Common::Point pos) {
	// Don't set position, when cursor is not visible.
	// This fixes eco quest 1 (floppy) right at the start, which is setting
	// mouse cursor to (0,0) all the time during the intro. It's escapeable
	// (now) by moving to the left or top, but it's getting on your nerves. This
	// could theoretically break some things, although sierra normally sets
	// position only when showing the cursor.
	if (!_isVisible)
		return;

	if (!_upscaledHires) {
		g_system->warpMouse(pos.x, pos.y);
	} else {
		_screen->adjustToUpscaledCoordinates(pos.y, pos.x);
		g_system->warpMouse(pos.x, pos.y);
	}

	// Some games display a new menu, set mouse position somewhere within and
	//  expect it to be in there. This is fine for a real mouse, but on wii using
	//  wii-mote or touch interfaces this won't work. In fact on those platforms
	//  the menus will close immediately because of that behaviour.
	// We identify those cases and set a reaction-rect. If the mouse it outside
	//  of that rect, we won't report the position back to the scripts.
	//  As soon as the mouse was inside once, we will revert to normal behaviour
	// Currently this code is enabled for all platforms, especially because we can't
	//  differentiate between e.g. Windows used via mouse and Windows used via touchscreen
	// The workaround won't hurt real-mouse platforms
	const SciGameId gameId = g_sci->getGameId();
	const SciCursorSetPositionWorkarounds *workaround;
	workaround = setPositionWorkarounds;
	while (workaround->newPositionX != -1) {
		if (workaround->gameId == gameId
			&& ((workaround->newPositionX == pos.x) && (workaround->newPositionY == pos.y))) {
			EngineState *s = g_sci->getEngineState();
			s->_cursorWorkaroundActive = true;
			s->_cursorWorkaroundPoint = pos;
			s->_cursorWorkaroundRect = Common::Rect(workaround->rectLeft, workaround->rectTop, workaround->rectRight, workaround->rectBottom);
			return;
		}
		workaround++;
	}
}

Common::Point GfxCursor::getPosition() {
	Common::Point mousePos = g_system->getEventManager()->getMousePos();

	if (_upscaledHires)
		_screen->adjustBackUpscaledCoordinates(mousePos.y, mousePos.x);

	return mousePos;
}

void GfxCursor::refreshPosition() {
	Common::Point mousePoint = getPosition();

	if (_moveZoneActive) {
		bool clipped = false;

		if (mousePoint.x < _moveZone.left) {
			mousePoint.x = _moveZone.left;
			clipped = true;
		} else if (mousePoint.x >= _moveZone.right) {
			mousePoint.x = _moveZone.right - 1;
			clipped = true;
		}

		if (mousePoint.y < _moveZone.top) {
			mousePoint.y = _moveZone.top;
			clipped = true;
		} else if (mousePoint.y >= _moveZone.bottom) {
			mousePoint.y = _moveZone.bottom - 1;
			clipped = true;
		}

		// FIXME: Do this only when mouse is grabbed?
		if (clipped)
			setPosition(mousePoint);
	}

	if (_zoomZoneActive) {
		// Cursor
		const CelInfo *cursorCelInfo = _zoomCursorView->getCelInfo(_zoomCursorLoop, _zoomCursorCel);
		const byte *cursorBitmap = _zoomCursorView->getBitmap(_zoomCursorLoop, _zoomCursorCel);
		// Pic
		const CelInfo *picCelInfo = _zoomPicView->getCelInfo(0, 0);
		const byte *rawPicBitmap = _zoomPicView->getBitmap(0, 0);
		// Compute hotspot from xoffset/yoffset
		Common::Point cursorHotspot = Common::Point((cursorCelInfo->width >> 1) - cursorCelInfo->displaceX, cursorCelInfo->height - cursorCelInfo->displaceY - 1);

		uint16 targetX = CLIP<int>((mousePoint.x - _zoomZone.left) * _zoomMultiplier, 0, picCelInfo->width - cursorCelInfo->width);
		uint16 targetY =  CLIP<int>((mousePoint.y - _zoomZone.top) * _zoomMultiplier, 0, picCelInfo->height - cursorCelInfo->height);

		// Replace the special magnifier color with the associated magnified pixels
		for (int x = 0; x < cursorCelInfo->width; x++) {
			for (int y = 0; y < cursorCelInfo->height; y++) {
				int curPos = cursorCelInfo->width * y + x;
				if (cursorBitmap[curPos] == _zoomColor) {
					_cursorSurface[curPos] = rawPicBitmap[picCelInfo->width * (targetY + y) + (targetX + x)];
				}
			}
		}

		CursorMan.replaceCursor((const byte *)_cursorSurface, cursorCelInfo->width, cursorCelInfo->height, cursorHotspot.x, cursorHotspot.y, cursorCelInfo->clearKey);
	}
}

void GfxCursor::kernelResetMoveZone() {
	_moveZoneActive = false;
}

void GfxCursor::kernelSetMoveZone(Common::Rect zone) {
	_moveZone = zone;
	_moveZoneActive = true;
}

void GfxCursor::kernelClearZoomZone() {
	kernelResetMoveZone();
	_zoomZone = Common::Rect();
	_zoomColor = 0;
	_zoomMultiplier = 0;
	_zoomZoneActive = false;
	delete _zoomCursorView;
	_zoomCursorView = 0;
	delete _zoomPicView;
	_zoomPicView = 0;
	delete[] _cursorSurface;
	_cursorSurface = 0;
}

void GfxCursor::kernelSetZoomZone(byte multiplier, Common::Rect zone, GuiResourceId viewNum, int loopNum, int celNum, GuiResourceId picNum, byte zoomColor) {
	kernelClearZoomZone();

	_zoomMultiplier = multiplier;

	_zoomCursorView = new GfxView(_resMan, _screen, _palette, viewNum);
	_zoomCursorLoop = (byte)loopNum;
	_zoomCursorCel = (byte)celNum;
	_zoomPicView = new GfxView(_resMan, _screen, _palette, picNum);
	const CelInfo *cursorCelInfo = _zoomCursorView->getCelInfo(_zoomCursorLoop, _zoomCursorCel);
	const byte *cursorBitmap = _zoomCursorView->getBitmap(_zoomCursorLoop, _zoomCursorCel);
	_cursorSurface = new byte[cursorCelInfo->width * cursorCelInfo->height];
	memcpy(_cursorSurface, cursorBitmap, cursorCelInfo->width * cursorCelInfo->height);

	_zoomZone = zone;
	kernelSetMoveZone(_zoomZone);

	_zoomColor = zoomColor;
	_zoomZoneActive = true;
}

void GfxCursor::kernelSetPos(Common::Point pos) {
	_coordAdjuster->setCursorPos(pos);
	kernelMoveCursor(pos);
}

void GfxCursor::kernelMoveCursor(Common::Point pos) {
	_coordAdjuster->moveCursor(pos);
	if (pos.x > _screen->getWidth() || pos.y > _screen->getHeight()) {
		warning("attempt to place cursor at invalid coordinates (%d, %d)", pos.y, pos.x);
		return;
	}

	setPosition(pos);

	// Trigger event reading to make sure the mouse coordinates will
	// actually have changed the next time we read them.
	_event->getSciEvent(SCI_EVENT_PEEK);
}

} // End of namespace Sci
