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

#include "common/system.h"

#include "common/config-manager.h"

#include "parallaction/parallaction.h"
#include "parallaction/input.h"
#include "parallaction/saveload.h"
#include "parallaction/sound.h"
#include "parallaction/walk.h"


namespace Parallaction {

#define INITIAL_FREE_SARCOPHAGUS_SLOT_X	200


class LocationName {

	Common::String _slide;
	Common::String _character;
	Common::String _location;

	bool _hasCharacter;
	bool _hasSlide;
	char *_buf;

public:
	LocationName() {
		_buf = 0;
		_hasSlide = false;
		_hasCharacter = false;
	}

	~LocationName() {
		free(_buf);
	}

	void bind(const char*);

	const char *location() const {
		return _location.c_str();
	}

	bool hasCharacter() const {
		return _hasCharacter;
	}

	const char *character() const {
		return _character.c_str();
	}

	bool hasSlide() const {
		return _hasSlide;
	}

	const char *slide() const {
		return _slide.c_str();
	}

	const char *c_str() const {
		return _buf;
	}
};



/*
	bind accept the following input formats:

    1 - [S].slide.[L]{.[C]}
	2 - [L]{.[C]}

    where:

	[S] is the slide to be shown
    [L] is the location to switch to (immediately in case 2, or right after slide [S] in case 1)
    [C] is the character to be selected, and is optional

    The routine tells one form from the other by searching for the '.slide.'

    NOTE: there exists one script in which [L] is not used in the case 1, but its use
		  is commented out, and would definitely crash the current implementation.
*/
void LocationName::bind(const char *s) {

	free(_buf);

	_buf = strdup(s);
	_hasSlide = false;
	_hasCharacter = false;

	Common::StringList list;
	char *tok = strtok(_buf, ".");
	while (tok) {
		list.push_back(tok);
		tok = strtok(NULL, ".");
	}

	if (list.size() < 1 || list.size() > 4)
		error("changeLocation: ill-formed location name '%s'", s);

	if (list.size() > 1) {
		if (list[1] == "slide") {
			_hasSlide = true;
			_slide = list[0];

			list.remove_at(0);		// removes slide name
			list.remove_at(0);		// removes 'slide'
		}

		if (list.size() == 2) {
			_hasCharacter = true;
			_character = list[1];
		}
	}

	_location = list[0];

	strcpy(_buf, s);		// kept as reference
}

Parallaction_ns::Parallaction_ns(OSystem* syst, const PARALLACTIONGameDescription *gameDesc) : Parallaction(syst, gameDesc),
	_locationParser(0), _programParser(0), _builder(0), _walker(0) {
}

Common::Error Parallaction_ns::init() {

	_screenWidth = 320;
	_screenHeight = 200;

	if (getPlatform() == Common::kPlatformPC) {
		_disk = new DosDisk_ns(this);
	} else {
		if (getFeatures() & GF_DEMO) {
			strcpy(_location._name, "fognedemo");
		}
		_disk = new AmigaDisk_ns(this);
	}

	_disk->init();

	if (getPlatform() == Common::kPlatformPC) {
		int midiDriver = MidiDriver::detectMusicDriver(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MIDI);
		MidiDriver *driver = MidiDriver::createMidi(midiDriver);
		_soundMan = new DosSoundMan(this, driver);
		_soundMan->setMusicVolume(ConfMan.getInt("music_volume"));
	} else {
		_soundMan = new AmigaSoundMan(this);
	}

	initResources();
	initFonts();
	_locationParser = new LocationParser_ns(this);
	_locationParser->init();
	_programParser = new ProgramParser_ns(this);
	_programParser->init();

	_cmdExec = new CommandExec_ns(this);
	_cmdExec->init();
	_programExec = new ProgramExec_ns(this);
	_programExec->init();

	_builder = new PathBuilder_NS(&_char);
	_walker = new PathWalker_NS(&_char);

	_sarcophagusDeltaX = 0;
	_movingSarcophagus = false;
	_freeSarcophagusSlotX = INITIAL_FREE_SARCOPHAGUS_SLOT_X;

	num_foglie = 0;

	_intro = false;
	_inTestResult = false;

	_location._animations.push_front(_char._ani);

	_saveLoad = new SaveLoad_ns(this, _saveFileMan);

	Parallaction::init();

	return Common::kNoError;
}

Parallaction_ns::~Parallaction_ns() {
	freeFonts();
	freeCharacter();

	delete _locationParser;
	delete _programParser;
	freeLocation(true);

	_location._animations.remove(_char._ani);

	delete _builder;
	delete _walker;
}


void Parallaction_ns::freeFonts() {

	delete _dialogueFont;
	delete _labelFont;
	delete _menuFont;
	delete _introFont;

	_menuFont  = 0;
	_dialogueFont = 0;
	_labelFont = 0;
	_introFont = 0;
}


void Parallaction_ns::callFunction(uint index, void* parm) {
	assert(index < 25);	// magic value 25 is maximum # of callables for Nippon Safes

	(this->*_callables[index])(parm);
}


Common::Error Parallaction_ns::go() {
	_saveLoad->renameOldSavefiles();

	_globalFlagsNames = _disk->loadTable("global");

	startGui();

	while (!shouldQuit()) {
		runGame();
	}

	return Common::kNoError;
}

void Parallaction_ns::switchBackground(const char* background, const char* mask) {
//	printf("switchBackground(%s)", name);

	Palette pal;

	uint16 v2 = 0;
	if (!scumm_stricmp(background, "final")) {
		_gfx->clearScreen();
		for (uint16 _si = 0; _si < 32; _si++) {
			pal.setEntry(_si, v2, v2, v2);
			v2 += 4;
		}

		_system->delayMillis(20);
		_gfx->setPalette(pal);
		_gfx->updateScreen();
	}

	setBackground(background, mask, mask);

	return;
}


void Parallaction_ns::runPendingZones() {
	if (_activeZone) {
		ZonePtr z = _activeZone;	// speak Zone or sound
		_activeZone = nullZonePtr;
		runZone(z);
	}
}

//	changeLocation handles transitions between locations, and is able to display slides
//	between one and the other.
//
void Parallaction_ns::changeLocation(char *location) {
	debugC(1, kDebugExec, "changeLocation(%s)", location);

	MouseTriState oldMouseState = _input->getMouseState();
	_input->setMouseState(MOUSE_DISABLED);

	_soundMan->playLocationMusic(location);

	_input->stopHovering();
	_gfx->freeLabels();

	_zoneTrap = nullZonePtr;

	_input->setArrowCursor();

	_gfx->showGfxObj(_char._ani->gfxobj, false);

	LocationName locname;
	locname.bind(location);

	bool fullCleanup = false;
	if (locname.hasCharacter()) {
		CharacterName newName(locname.character());
		fullCleanup = (strcmp(newName.getBaseName(), _char.getBaseName()));
	}

	if (fullCleanup) {
		cleanupGame();
	} else {
		freeLocation(false);
	}


	if (locname.hasSlide()) {
		showSlide(locname.slide());
		uint id = _gfx->createLabel(_menuFont, _location._slideText[0], 1);
		_gfx->showLabel(id, CENTER_LABEL_HORIZONTAL, 14);
		_gfx->updateScreen();

		_input->waitForButtonEvent(kMouseLeftUp);
		_gfx->freeLabels();
	}

	if (locname.hasCharacter()) {
		changeCharacter(locname.character());
	}

	strcpy(_saveData1, locname.location());
	parseLocation(_saveData1);

	if (_location._startPosition.x != -1000) {
		_char._ani->setX(_location._startPosition.x);
		_char._ani->setY(_location._startPosition.y);
		_char._ani->setF(_location._startFrame);
		_location._startPosition.y = -1000;
		_location._startPosition.x = -1000;
	}


	_gfx->setBlackPalette();
	_gfx->updateScreen();

	// BUG #1837503: kEngineChangeLocation flag must be cleared *before* commands
	// and acommands are executed, so that it can be set again if needed.
	_engineFlags &= ~kEngineChangeLocation;

	_cmdExec->run(_location._commands);

	doLocationEnterTransition();

	_cmdExec->run(_location._aCommands);

	if (_location._hasSound)
		_soundMan->playSfx(_location._soundFile, 0, true);

	if (!_intro) {
		_input->setMouseState(oldMouseState);
	}

	debugC(1, kDebugExec, "changeLocation() done");
}


void Parallaction_ns::parseLocation(const char *filename) {
	debugC(1, kDebugParser, "parseLocation('%s')", filename);

	allocateLocationSlot(filename);
	Script *script = _disk->loadLocation(filename);

	// TODO: the following two lines are specific to Nippon Safes
	// and should be moved into something like 'initializeParsing()'
	_location._hasSound = false;

	_locationParser->parse(script);

	delete script;

	// this loads animation scripts
	AnimationList::iterator it = _location._animations.begin();
	for ( ; it != _location._animations.end(); ++it) {
		if ((*it)->_scriptName) {
			loadProgram(*it, (*it)->_scriptName);
		}
	}

	debugC(1, kDebugParser, "parseLocation('%s') done", filename);
	return;
}



void Parallaction_ns::changeCharacter(const char *name) {
	debugC(1, kDebugExec, "changeCharacter(%s)", name);

	_char.setName(name);

	if (!scumm_stricmp(_char.getFullName(), _characterName1)) {
		debugC(3, kDebugExec, "changeCharacter: nothing done");
		return;
	}

	freeCharacter();

	_char._ani->gfxobj = _gfx->loadCharacterAnim(_char.getFullName());

	if (!_char.dummy()) {
		_char._head = _disk->loadHead(_char.getBaseName());
		_char._talk = _disk->loadTalk(_char.getBaseName());
		_objects = _disk->loadObjects(_char.getBaseName());
		_objectsNames = _disk->loadTable(_char.getBaseName());

		_soundMan->playCharacterMusic(_char.getBaseName());

		// The original engine used to reload 'common' only on loadgames. We are reloading here since 'common'
		// contains character specific stuff. This causes crashes like bug #1816899, because parseLocation tries
		// to reload scripts but the data archive selected is occasionally wrong. This has been solved by having
		// parseLocation only load scripts when they aren't already loaded - which it should have done since the
		// beginning nevertheless.
		if (!(getFeatures() & GF_DEMO))
			parseLocation("common");
	}

	strcpy(_characterName1, _char.getFullName());

	debugC(3, kDebugExec, "changeCharacter: switch completed");

	return;
}

void Parallaction_ns::freeCharacter() {
	_gfx->freeCharacterObjects();

	delete _char._talk;
	delete _char._head;
	delete _char._ani->gfxobj;
	delete _objects;
	delete _objectsNames;

	_char._talk = 0;
	_char._head = 0;
	_char._ani->gfxobj = 0;

	_objects = 0;
	_objectsNames = 0;
}

void Parallaction_ns::freeLocation(bool removeAll) {
	debugC(2, kDebugExec, "freeLocation");

	_soundMan->stopSfx(0);
	_soundMan->stopSfx(1);
	_soundMan->stopSfx(2);
	_soundMan->stopSfx(3);

	_localFlagNames->clear();

	_gfx->freeLocationObjects();

	_location._animations.remove(_char._ani);
	_location.cleanup(removeAll);
	_location._animations.push_front(_char._ani);
}

void Parallaction_ns::cleanupGame() {
	_soundMan->stopMusic();

	_inTestResult = false;
	_engineFlags &= ~kEngineTransformedDonna;

	_numLocations = 0;
	_globalFlags = 0;
	memset(_localFlags, 0, sizeof(_localFlags));
	memset(_locationNames, 0, sizeof(_locationNames));

	freeLocation(true);

	_score = 0;
	_freeSarcophagusSlotX = INITIAL_FREE_SARCOPHAGUS_SLOT_X;
	_movingSarcophagus = false;
}

void Parallaction_ns::updateWalkers() {
	_walker->walk();
}


void Parallaction_ns::scheduleWalk(int16 x, int16 y, bool fromUser) {
	AnimationPtr a = _char._ani;

	if ((a->_flags & kFlagsRemove) || (a->_flags & kFlagsActive) == 0) {
		return;
	}

	_builder->buildPath(x, y);
	_engineFlags |= kEngineWalking;
}

}// namespace Parallaction
