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

#include "gui/theme.h"
#include "gui/eval.h"
#include "common/file.h"

#include "common/archive.h"
#include "common/unzip.h"

namespace GUI {

Theme::Theme() : _drawArea(), _stylefile(""), _configFile(), _loadedThemeX(0), _loadedThemeY(0) {
	Common::MemoryReadStream s((const byte *)_defaultConfigINI, strlen(_defaultConfigINI));
	_defaultConfig.loadFromStream(s);

	_evaluator = new Eval();
}

Theme::~Theme() {
	delete _evaluator;
}

void Theme::getColorFromConfig(const Common::String &value, OverlayColor &color) {
	const char *postfixes[] = {".r", ".g", ".b"};
	int rgb[3];

	for (int cnt = 0; cnt < 3; cnt++)
		rgb[cnt] = _evaluator->getVar(value + postfixes[cnt], 0);

	color = g_system->RGBToColor(rgb[0], rgb[1], rgb[2]);
}

void Theme::getColorFromConfig(const Common::String &value, uint8 &r, uint8 &g, uint8 &b) {
	r = _evaluator->getVar(value + ".r", 0);
	g = _evaluator->getVar(value + ".g", 0);
	b = _evaluator->getVar(value + ".b", 0);
}

const Graphics::Font *Theme::loadFont(const char *filename) {
	const Graphics::NewFont *font = 0;
	Common::String cacheFilename = genCacheFilename(filename);
	Common::File fontFile;

	if (!cacheFilename.empty()) {
		if (fontFile.open(cacheFilename))
			font = Graphics::NewFont::loadFromCache(fontFile);
		if (font)
			return font;

#ifdef USE_ZLIB
		Common::ZipArchive zipArchive(_stylefile + ".zip");
		if (zipArchive.hasFile(cacheFilename)) {
			Common::FilePtr stream(zipArchive.openFile(cacheFilename));
			font = Graphics::NewFont::loadFromCache(*stream.get());
		}
#endif
		if (font)
			return font;
	}

	// normal open
	if (fontFile.open(filename)) {
		font = Graphics::NewFont::loadFont(fontFile);
	}

#ifdef USE_ZLIB
	if (!font) {
		Common::ZipArchive zipArchive(_stylefile + ".zip");
		if (zipArchive.hasFile(filename)) {
			Common::FilePtr stream(zipArchive.openFile(filename));
			font = Graphics::NewFont::loadFont(*stream.get());
		}
	}
#endif

	if (font) {
		if (!cacheFilename.empty()) {
			if (!Graphics::NewFont::cacheFontData(*font, cacheFilename)) {
				warning("Couldn't create cache file for font '%s'", filename);
			}
		}
	}

	return font;
}

Common::String Theme::genCacheFilename(const char *filename) {
	Common::String cacheName(filename);
	for (int i = cacheName.size() - 1; i >= 0; --i) {
		if (cacheName[i] == '.') {
			while ((uint)i < cacheName.size() - 1) {
				cacheName.deleteLastChar();
			}

			cacheName += "fcc";
			return cacheName;
		}
	}

	return "";
}

bool Theme::loadConfigFile(const Common::String &stylefile) {
	if (ConfMan.hasKey("themepath"))
		Common::File::addDefaultDirectory(ConfMan.get("themepath"));

#ifdef DATA_PATH
	Common::File::addDefaultDirectoryRecursive(DATA_PATH);
#endif

	if (ConfMan.hasKey("extrapath"))
		Common::File::addDefaultDirectoryRecursive(ConfMan.get("extrapath"));

	if (_configFile.loadFromFile(stylefile + ".ini"))
		return true;

#ifdef USE_ZLIB
	// Maybe find a nicer solution to this
	Common::ZipArchive zipArchive(stylefile + ".zip");
	if (zipArchive.hasFile(stylefile + ".ini")) {
		Common::FilePtr stream(zipArchive.openFile(stylefile + ".ini"));
		if (_configFile.loadFromStream(*stream.get()))
			return true;
	}
#endif

	return false;
}

bool Theme::themeConfigUseable(const Common::String &stylefile, const Common::String &style, Common::String *cStyle, Common::ConfigFile *cfg) {
	if (ConfMan.hasKey("themepath"))
		Common::File::addDefaultDirectory(ConfMan.get("themepath"));

#ifdef DATA_PATH
	Common::File::addDefaultDirectoryRecursive(DATA_PATH);
#endif

	if (ConfMan.hasKey("extrapath"))
		Common::File::addDefaultDirectoryRecursive(ConfMan.get("extrapath"));

	Common::File file;
	Common::ConfigFile configFile;
	if (!cfg && (cStyle || !style.empty()))
		cfg = &configFile;

	if (!file.open(stylefile + ".ini")) {
#ifdef USE_ZLIB
		// Maybe find a nicer solution to this
		Common::ZipArchive zipArchive(stylefile + ".zip");
		if (zipArchive.hasFile(stylefile + ".ini")) {
			if (!style.empty() || cStyle || cfg) {
				Common::FilePtr stream(zipArchive.openFile(stylefile + ".ini"));
				if (!cfg->loadFromStream(*stream.get()))
					return false;
			}
		} else {
			return false;
		}
#else
		return false;
#endif
	}

	if (!style.empty() || cStyle || cfg) {
		if (file.isOpen()) {
			if (!cfg->loadFromStream(file))
				return false;
			file.close();
		}

		Common::String temp;
		if (!cfg->getKey("type", "theme", temp))
			return false;
		if (cStyle)
			*cStyle = temp;
		if (0 != temp.compareToIgnoreCase(style) && !style.empty())
			return false;
	}

	return true;
}

} // End of namespace GUI

