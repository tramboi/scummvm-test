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

#ifndef SCI_GRAPHICS_CONTROLS_H
#define SCI_GRAPHICS_CONTROLS_H

namespace Sci {

class GfxPorts;
class GfxPaint16;
class Font;
class GfxText16;
class Controls {
public:
	Controls(SegManager *segMan, GfxPorts *ports, GfxPaint16 *paint16, GfxText16 *text16);
	~Controls();

	void drawListControl(Common::Rect rect, reg_t obj, int16 maxChars, int16 count, const char **entries, GuiResourceId fontId, int16 upperPos, int16 cursorPos, bool isAlias);
	void TexteditCursorDraw(Common::Rect rect, const char *text, uint16 curPos);
	void TexteditCursorErase();
	void TexteditChange(reg_t controlObject, reg_t eventObject);

private:
	void init();
	void TexteditSetBlinkTime();

	SegManager *_segMan;
	GfxPorts *_ports;
	GfxPaint16 *_paint16;
	GfxText16 *_text16;

	// Textedit-Control related
	Common::Rect _texteditCursorRect;
	bool _texteditCursorVisible;
	uint32 _texteditBlinkTime;
};

} // End of namespace Sci

#endif
