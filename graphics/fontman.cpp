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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#include "graphics/fontman.h"
//#include "gui/consolefont.h"

DECLARE_SINGLETON(Graphics::FontManager)

namespace Graphics {

const ScummFont *g_scummfont = 0;
FORWARD_DECLARE_FONT(g_sysfont)
FORWARD_DECLARE_FONT(g_sysfont_big)
FORWARD_DECLARE_FONT(g_consolefont)

FontManager::FontManager() {
	// This assert should *never* trigger, because
	// FontManager is a singleton, thus there is only
	// one instance of it per time. (g_scummfont gets
	// reset to 0 in the desctructor of this class).
	assert(g_scummfont == 0);
	g_scummfont = new ScummFont;
	INIT_FONT(g_sysfont)
	INIT_FONT(g_sysfont_big)
	INIT_FONT(g_consolefont)
}

FontManager::~FontManager() {
	delete g_scummfont;
	g_scummfont = 0;
	delete g_sysfont;
	g_sysfont = 0;
	delete g_sysfont_big;
	g_sysfont_big = 0;
	delete g_consolefont;
	g_consolefont = 0;
}

const char *builtinFontNames[] = {
	"builtinOSD",
	"builtinConsole",
	"builtinGUI",
	"builtinBigGUI",
	0
};

const Font *FontManager::getFontByName(const Common::String &name) const {
	for (int i = 0; builtinFontNames[i]; i++)
		if (!strcmp(name.c_str(), builtinFontNames[i]))
			return getFontByUsage((FontUsage)i);

	if (!_fontMap.contains(name))
		return 0;
	return _fontMap[name];
}

const Font *FontManager::getFontByUsage(FontUsage usage) const {
	switch (usage) {
	case kOSDFont:
		return g_scummfont;
	case kConsoleFont:
		return g_consolefont;
	case kGUIFont:
		return g_sysfont;
	case kBigGUIFont:
		return g_sysfont_big;
	}

	return 0;
}

} // End of namespace Graphics
