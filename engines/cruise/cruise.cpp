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
#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"
#include "common/system.h"

#include "graphics/cursorman.h"

#include "sound/mididrv.h"
#include "sound/mixer.h"

#include "cruise/cruise.h"
#include "cruise/font.h"
#include "cruise/gfxModule.h"
#include "cruise/staticres.h"

namespace Cruise {

//SoundDriver *g_soundDriver;
//SfxPlayer *g_sfxPlayer;

CruiseEngine *_vm;

CruiseEngine::CruiseEngine(OSystem * syst, const CRUISEGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {

#ifdef PALMOS_MODE
	_currentVolumeFile = new Common::File();
#endif

	Common::addDebugChannel(kCruiseDebugScript, "Script",
	                             "Script debug level");

	// Setup mixer
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType,
	                              ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType,
	                              ConfMan.getInt("music_volume"));

	_vm = this;
	_debugger = new Debugger();
	_music = new MusicPlayer();
	_sound = new SoundPlayer();

	syst->getEventManager()->registerRandomSource(_rnd, "cruise");
}

CruiseEngine::~CruiseEngine() {
	delete _debugger;
	delete _music;
	delete _sound;

	freeSystem();
}

bool CruiseEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error CruiseEngine::run() {
	// Initialize backend
	initGraphics(320, 200, false);

	if (!loadLanguageStrings())
		return Common::kUnknownError;

	initialize();

	Cruise::changeCursor(Cruise::CURSOR_NORMAL);
	CursorMan.showMouse(true);


	lastTick = 0;
	lastTickDebug = 0;

	mainLoop();

	deinitialise();

	return Common::kNoError;
}

void CruiseEngine::initialize() {
	PCFadeFlag = 0;
	_gameSpeed = GAME_FRAME_DELAY_1;
	_speedFlag = false;

	/*volVar1 = 0;
	 * fileData1 = 0; */

	/*PAL_fileHandle = -1; */

	// video init stuff

	initSystem();

	// another bit of video init

	readVolCnf();

	// Setup mixer
	_musicVolume = ConfMan.getInt("music_volume");
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	int midiDriver = MidiDriver::detectMusicDriver(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MIDI);
	_mt32 = ((midiDriver == MD_MT32) || ConfMan.getBool("native_mt32"));
	_adlib = (midiDriver == MD_ADLIB);

	_driver = MidiDriver::createMidi(midiDriver);
	if (_mt32)
		_driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);
}

void CruiseEngine::deinitialise() {
	polyStructNorm.clear();
	polyStructExp.clear();
}

bool CruiseEngine::loadLanguageStrings() {
	Common::File f;

	// Give preference to a language file
	if (f.open("DELPHINE.LNG")) {
		char *data = (char *)malloc(f.size());
		f.read(data, f.size());
		char *ptr = data;

		for (int i = 0; i < MAX_LANGUAGE_STRINGS; ++i) {
			// Get the start of the next string
			while (*ptr != '"') ++ptr;
			const char *v = ++ptr;

			// Find the end of the string, and replace the end '"' with a NULL
			while (*ptr != '"') ++ptr;
			*ptr++ = '\0';

			// Add the string to the list
			_langStrings.push_back(v);
		}

		f.close();

	} else {
		// Try and use one of the pre-defined language lists
		const char **p = NULL;
		switch (getLanguage()) {
			case Common::EN_ANY:
				p = englishLanguageStrings;
				break;
			case Common::FR_FRA:
				p = frenchLanguageStrings;
				break;
			default:
				return false;
		}

		// Load in the located language set
		for (int i = 0; i < 13; ++i, ++p)
			_langStrings.push_back(*p);
	}

	return true;
}

void CruiseEngine::pauseEngine(bool pause) {
	Engine::pauseEngine(pause);

	if (pause) {
		// Draw the 'Paused' message
		drawSolidBox(64, 100, 256, 117, 0);
		drawString(10, 100, langString(ID_PAUSED), gfxModuleData.pPage00, itemColor, 300);
		gfxModuleData_flipScreen();

		_savedCursor = currentCursor;
		changeCursor(CURSOR_NOMOUSE);
	} else {
		processAnimation();
		flipScreen();
		changeCursor(_savedCursor);
	}
}

Common::Error CruiseEngine::loadGameState(int slot) {
	return loadSavegameData(slot);
}

bool CruiseEngine::canLoadGameStateCurrently() {
	return playerMenuEnabled != 0;
}

Common::Error CruiseEngine::saveGameState(int slot, const char *desc) {
	return saveSavegameData(slot, desc);
}

bool CruiseEngine::canSaveGameStateCurrently() {
	return (playerMenuEnabled != 0) && (userEnabled != 0);
}

const char *CruiseEngine::getSavegameFile(int saveGameIdx) {
	static char buffer[20];
	sprintf(buffer, "cruise.s%02d", saveGameIdx);
	return buffer;
}

} // End of namespace Cruise
