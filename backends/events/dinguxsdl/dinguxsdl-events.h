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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/branches/gsoc2010-opengl/backends/events/dinguxsdl/dinguxsdl-events.h $
 * $Id: dinguxsdl-events.h 53730 2010-10-23 09:30:26Z hkz $
 *
 */

#ifndef BACKENDS_EVENTS_SDL_DINGUX_H
#define BACKENDS_EVENTS_SDL_DINGUX_H
#if defined(DINGUX)

#include "backends/platform/dingux/dingux.h"
#include "backends/events/dinguxsdl/dinguxsdl-events.h"

class DINGUXSdlEventSource : public SdlEventSource {
public:
	DINGUXSdlEventSource();
	void setCurrentGraphMan(DINGUXSdlGraphicsManager *_graphicManager);

protected:
	DINGUXSdlGraphicsManager *_grpMan;

	bool remapKey(SDL_Event &ev, Common::Event &event);
	void fillMouseEvent(Common::Event &event, int x, int y);
	void warpMouse(int x, int y);
};

#endif /* DINGUX */
#endif /* BACKENDS_EVENTS_SDL_DINGUX_H */
