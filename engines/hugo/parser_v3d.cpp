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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

// parser.c - handles all keyboard/command input

#include "common/system.h"

#include "hugo/hugo.h"
#include "hugo/parser.h"
#include "hugo/schedule.h"
#include "hugo/util.h"
#include "hugo/sound.h"

namespace Hugo {

Parser_v3d::Parser_v3d(HugoEngine &vm) : Parser_v1w(vm) {
}

Parser_v3d::~Parser_v3d() {
}

// Parse the user's line of text input.  Generate events as necessary
void Parser_v3d::lineHandler() {
	debugC(1, kDebugParser, "lineHandler()");

	status_t &gameStatus = _vm.getGameStatus();

	// Toggle God Mode
	if (!strncmp(_line, "PPG", 3)) {
		_vm.sound().playSound(!_vm._soundTest, BOTH_CHANNELS, HIGH_PRI);
		gameStatus.godModeFl ^= 1;
		return;
	}

	Utils::strlwr(_line);                           // Convert to lower case

	// God Mode cheat commands:
	// goto <screen>                                Takes hero to named screen
	// fetch <object name>                          Hero carries named object
	// fetch all                                    Hero carries all possible objects
	// find <object name>                           Takes hero to screen containing named object
	if (gameStatus.godModeFl) {
		// Special code to allow me to go straight to any screen
		if (strstr(_line, "goto")) {
			for (int i = 0; i < _vm._numScreens; i++) {
				if (!strcmp(&_line[strlen("goto") + 1], _vm._screenNames[i])) {
					_vm.scheduler().newScreen(i);
					return;
				}
			}
		}

		// Special code to allow me to get objects from anywhere
		if (strstr(_line, "fetch all")) {
			for (int i = 0; i < _vm._numObj; i++) {
				if (_vm._objects[i].genericCmd & TAKE)
					takeObject(&_vm._objects[i]);
			}
			return;
		}

		if (strstr(_line, "fetch")) {
			for (int i = 0; i < _vm._numObj; i++) {
				if (!strcmp(&_line[strlen("fetch") + 1], _vm._arrayNouns[_vm._objects[i].nounIndex][0])) {
					takeObject(&_vm._objects[i]);
					return;
				}
			}
		}

		// Special code to allow me to goto objects
		if (strstr(_line, "find")) {
			for (int i = 0; i < _vm._numObj; i++) {
				if (!strcmp(&_line[strlen("find") + 1], _vm._arrayNouns[_vm._objects[i].nounIndex][0])) {
					_vm.scheduler().newScreen(_vm._objects[i].screenIndex);
					return;
				}
			}
		}
	}

	// Special meta commands
	// EXIT/QUIT
	if (!strcmp("exit", _line) || strstr(_line, "quit")) {
		if (Utils::Box(BOX_YESNO, "%s", _vm._textParser[kTBExit_1d]) != 0)
			_vm.endGame();
		else
			return;
	}

	// SAVE/RESTORE
	if (!strcmp("save", _line)) {
		_config.soundFl = false;
		if (gameStatus.gameOverFl)
			Utils::gameOverMsg();
		else
//			_vm.file().saveOrRestore(true);
			warning("STUB: saveOrRestore()");
		return;
	}

	if (!strcmp("restore", _line)) {
		_config.soundFl = false;
//		_vm.file().saveOrRestore(false);
		warning("STUB: saveOrRestore()");
		return;
	}

	// Empty line
	if (*_line == '\0')                             // Empty line
		return;
	if (strspn(_line, " ") == strlen(_line))        // Nothing but spaces!
		return;

	if (gameStatus.gameOverFl) {
		// No commands allowed!
		Utils::gameOverMsg();
		return;
	}

	char farComment[XBYTES * 5] = "";               // hold 5 line comment if object not nearby

	// Test for nearby objects referenced explicitly
	for (int i = 0; i < _vm._numObj; i++) {
		object_t *obj = &_vm._objects[i];
		if (isWordPresent(_vm._arrayNouns[obj->nounIndex])) {
			if (isObjectVerb(obj, farComment) || isGenericVerb(obj, farComment))
				return;
		}
	}

	// Test for nearby objects that only require a verb
	// Note comment is unused if not near.
	for (int i = 0; i < _vm._numObj; i++) {
		object_t *obj = &_vm._objects[i];
		if (obj->verbOnlyFl) {
			char contextComment[XBYTES * 5] = "";   // Unused comment for context objects
			if (isObjectVerb(obj, contextComment) || isGenericVerb(obj, contextComment))
				return;
		}
	}

	// No objects match command line, try background and catchall commands
	if (isBackgroundWord(_vm._backgroundObjects[*_vm._screen_p]))
		return;
	if (isCatchallVerb(_vm._backgroundObjects[*_vm._screen_p]))
		return;
	if (isBackgroundWord(_vm._catchallList))
		return;
	if (isCatchallVerb(_vm._catchallList))
		return;

	// If a not-near comment was generated, print it
	if (*farComment != '\0') {
		Utils::Box(BOX_ANY, "%s", farComment);
		return;
	}

	// Nothing matches.  Report recognition success to user.
	char *verb = findVerb();
	char *noun = findNoun();
	
	if (verb && noun) {                             // A combination I didn't think of
		Utils::Box(BOX_ANY, "%s", _vm._textParser[kTBNoPoint]);
	} else if (noun) {
		Utils::Box(BOX_ANY, "%s", _vm._textParser[kTBNoun]);
	} else if (verb) {
		Utils::Box(BOX_ANY, "%s", _vm._textParser[kTBVerb]);
	} else {
		Utils::Box(BOX_ANY, "%s", _vm._textParser[kTBEh]);
	}
}

} // End of namespace Hugo
