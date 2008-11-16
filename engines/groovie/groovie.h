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

#ifndef GROOVIE_H
#define GROOVIE_H

#include "common/advancedDetector.h"
#include "engines/engine.h"
#include "graphics/surface.h"

#include "groovie/cursor.h"
#include "groovie/debug.h"
#include "groovie/graphics.h"
#include "groovie/player.h"
#include "groovie/resource.h"
#include "groovie/script.h"

namespace Groovie {

class MusicPlayer;

enum kDebugLevels {
	kGroovieDebugAll = 1 << 0,
	kGroovieDebugVideo = 1 << 1,
	kGroovieDebugResource = 1 << 2,
	kGroovieDebugScript = 1 << 3,
	kGroovieDebugUnknown = 1 << 4,
	kGroovieDebugHotspots = 1 << 5,
	kGroovieDebugCursor = 1 << 6,
	kGroovieDebugMIDI = 1 << 7,
	kGroovieDebugScriptvars = 1 << 8
		// the current limitation is 32 debug levels (1 << 31 is the last one)
};

enum kEngineVersion {
	kGroovieT7G,
	kGroovieV2
};

struct GroovieGameDescription {
	Common::ADGameDescription desc;

	kEngineVersion version; // Version of the engine
	int indexEntry; // The index of the entry in disk.1 for V2 games
};

class GroovieEngine : public Engine {
public:
	GroovieEngine(OSystem *syst, GroovieGameDescription *gd);
	~GroovieEngine();

protected:
	Common::Error init();
	Common::Error go();

	void errorString(const char *buf_input, char *buf_output, int buf_output_size);

public:
	bool hasFeature(EngineFeature f) const;

	bool canLoadGameStateCurrently();
	Common::Error loadGameState(int slot);
	void syncSoundSettings();

	Debugger *getDebugger() { return _debugger; }

	void waitForInput();

	Script _script;
	ResMan *_resMan;
	CursorMan *_cursorMan;
	VideoPlayer *_videoPlayer;
	MusicPlayer *_musicPlayer;
	GraphicsMan *_graphicsMan;

private:
	GroovieGameDescription *_gameDescription;
	Debugger *_debugger;
	bool _waitingForInput;
};

} // End of namespace Groovie

#endif // GROOVIE_H
