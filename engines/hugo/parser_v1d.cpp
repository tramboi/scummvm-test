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
#include "common/events.h"

#include "hugo/hugo.h"
#include "hugo/parser.h"
#include "hugo/file.h"
#include "hugo/schedule.h"
#include "hugo/display.h"
#include "hugo/util.h"
#include "hugo/route.h"
#include "hugo/sound.h"
#include "hugo/object.h"

namespace Hugo {

Parser_v1d::Parser_v1d(HugoEngine *vm) : Parser(vm) {
}

Parser_v1d::~Parser_v1d() {
}

/**
* Locate word in list of nouns and return ptr to string in noun list
* If n is NULL, start at beginning of list, else with n
*/
char *Parser_v1d::findNextNoun(char *noun) {
	debugC(1, kDebugParser, "findNextNoun(%s)", noun);

	int currNounIndex = -1;
	if (noun) {                                        // If noun not NULL, find index
		for (currNounIndex = 0; _vm->_arrayNouns[currNounIndex]; currNounIndex++) {
			if (noun == _vm->_arrayNouns[currNounIndex][0])
				break;
		}
	}
	for (int i = currNounIndex + 1; _vm->_arrayNouns[i]; i++) {
		for (int j = 0; strlen(_vm->_arrayNouns[i][j]); j++) {
			if (strstr(_line, _vm->_arrayNouns[i][j]))
				return _vm->_arrayNouns[i][0];
		}
	}
	return 0;
}

/**
* Test whether hero is close to object.  Return TRUE or FALSE
* If no noun specified, check context flag in object before other tests.
* If object not near, return suitable string; may be similar object closer
* If radius is -1, treat radius as infinity
*/
bool Parser_v1d::isNear(char *verb, char *noun, object_t *obj, char *comment) {
	debugC(1, kDebugParser, "isNear(%s, %s, obj, %s)", verb, noun, comment);

	if (!noun && !obj->verbOnlyFl) {                // No noun specified & object not context senesitive
		return false;
	} else if (noun && (noun != _vm->_arrayNouns[obj->nounIndex][0])) { // Noun specified & not same as object
		return false;
	} else if (obj->carriedFl) {                    // Object is being carried
		return true;
	} else if (obj->screenIndex != *_vm->_screen_p) { // Not in same screen
		if (obj->objValue)
			strcpy (comment, _vm->_textParser[kCmtAny4]);
		return false;
	}

	if (obj->cycling == INVISIBLE) {
		if (obj->seqNumb) {                         // There is an image
			strcpy(comment, _vm->_textParser[kCmtAny5]);
			return false;
		} else {                                    // No image, assume visible
			if ((obj->radius < 0) ||
			   ((abs(obj->x - _vm->_hero->x) <= obj->radius) &&
			   (abs(obj->y - _vm->_hero->y - _vm->_hero->currImagePtr->y2) <= obj->radius))) {
			   return true;
			} else {
				// User is either not close enough (stationary, valueless objects)
				// or is not carrying it (small, portable objects of value)
				if (noun) {                         // Don't say unless object specified
					if (obj->objValue && (verb != _vm->_arrayVerbs[_vm->_take][0]))
						strcpy(comment, _vm->_textParser[kCmtAny4]);
					else
						strcpy(comment, _vm->_textParser[kCmtClose]);
					}
				return false;
			}
		}
	}

	if ((obj->radius < 0) ||
	    ((abs(obj->x - _vm->_hero->x) <= obj->radius) &&
	    (abs(obj->y + obj->currImagePtr->y2 - _vm->_hero->y - _vm->_hero->currImagePtr->y2) <= obj->radius))) {
	   return true;
	} else {
		// User is either not close enough (stationary, valueless objects)
		// or is not carrying it (small, portable objects of value)
		if (noun) {                                 // Don't say unless object specified
			if (obj->objValue && (verb != _vm->_arrayVerbs[_vm->_take][0]))
				strcpy(comment, _vm->_textParser[kCmtAny4]);
			else
				strcpy(comment, _vm->_textParser[kCmtClose]);
		}
		return false;
	}

	return true;
}

/**
* Test whether supplied verb is one of the common variety for this object
* say_ok needed for special case of take/drop which may be handled not only
* here but also in a cmd_list with a donestr string simultaneously
*/
bool Parser_v1d::isGenericVerb(char *word, object_t *obj) {
	debugC(1, kDebugParser, "isGenericVerb(%s, object_t *obj)", word);

	if (!obj->genericCmd)
		return false;

	// Following is equivalent to switch, but couldn't do one
	if (word == _vm->_arrayVerbs[_vm->_look][0]) {
		if ((LOOK & obj->genericCmd) == LOOK)
			Utils::Box(BOX_ANY, "%s", _vm->_textData[obj->dataIndex]);
		else
			Utils::Box(BOX_ANY, "%s", _vm->_textParser[kTBUnusual_1d]);
	} else if (word == _vm->_arrayVerbs[_vm->_take][0]) {
		if (obj->carriedFl)
			Utils::Box(BOX_ANY, "%s", _vm->_textParser[kTBHave]);
		else if ((TAKE & obj->genericCmd) == TAKE)
			takeObject(obj);
		else if (!obj->verbOnlyFl)                  // Make sure not taking object in context!
			Utils::Box(BOX_ANY, "%s", _vm->_textParser[kTBNoUse]);
		else
			return false;
	} else if (word == _vm->_arrayVerbs[_vm->_drop][0]) {
		if (!obj->carriedFl)
			Utils::Box(BOX_ANY, "%s", _vm->_textParser[kTBDontHave]);
		else if ((DROP & obj->genericCmd) == DROP)
			dropObject(obj);
		else
			Utils::Box(BOX_ANY, "%s", _vm->_textParser[kTBNeed]);
	} else {                                        // It was not a generic cmd
		return false;
	}

	return true;
}

/**
* Test whether supplied verb is included in the list of allowed verbs for
* this object.  If it is, then perform the tests on it from the cmd list
* and if it passes, perform the actions in the action list.  If the verb
* is catered for, return TRUE
*/
bool Parser_v1d::isObjectVerb(char *word, object_t *obj) {
	debugC(1, kDebugParser, "isObjectVerb(%s, object_t *obj)", word);

	// First, find matching verb in cmd list
	uint16 cmdIndex = obj->cmdIndex;                // ptr to list of commands
	if (!cmdIndex)                                  // No commands for this obj
		return false;

	int i;
	for (i = 0; _vm->_cmdList[cmdIndex][i].verbIndex != 0; i++) { // For each cmd
		if (!strcmp(word, _vm->_arrayVerbs[_vm->_cmdList[cmdIndex][i].verbIndex][0])) // Is this verb catered for?
			break;
	}

	if (_vm->_cmdList[cmdIndex][i].verbIndex == 0)  // No
		return false;

	// Verb match found, check all required objects are being carried
	cmd *cmnd = &_vm->_cmdList[cmdIndex][i];        // ptr to struct cmd
	if (cmnd->reqIndex) {                           // At least 1 thing in list
		uint16 *reqs = _vm->_arrayReqs[cmnd->reqIndex]; // ptr to list of required objects
		for (i = 0; reqs[i]; i++) {                 // for each obj
			if (!_vm->_object->isCarrying(reqs[i])) {
				Utils::Box(BOX_ANY, "%s", _vm->_textData[cmnd->textDataNoCarryIndex]);
				return true;
			}
		}
	}

	// Required objects are present, now check state is correct
	if ((obj->state != cmnd->reqState) && (cmnd->reqState != DONT_CARE)){
		Utils::Box(BOX_ANY, "%s", _vm->_textData[cmnd->textDataWrongIndex]);
		return true;
	}

	// Everything checked.  Change the state and carry out any actions
	if (cmnd->reqState != DONT_CARE)                // Don't change new state if required state didn't care
		obj->state = cmnd->newState;
	Utils::Box(BOX_ANY, "%s", _vm->_textData[cmnd->textDataDoneIndex]);
	_vm->_scheduler->insertActionList(cmnd->actIndex);
	// Special case if verb is Take or Drop.  Assume additional generic actions
	if ((word == _vm->_arrayVerbs[_vm->_take][0]) || (word == _vm->_arrayVerbs[_vm->_drop][0]))
		isGenericVerb(word, obj);
	return true;
}

/**
* Print text for possible background object.  Return TRUE if match found
* Only match if both verb and noun found.  Test_ca will match verb-only
*/
bool Parser_v1d::isBackgroundWord(char *noun, char *verb, objectList_t obj) {
	debugC(1, kDebugParser, "isBackgroundWord(%s, %s, object_list_t obj)", noun, verb);

	if (!noun)
		return false;

	for (int i = 0; obj[i].verbIndex; i++) {
		if ((verb == _vm->_arrayVerbs[obj[i].verbIndex][0]) && (noun == _vm->_arrayNouns[obj[i].nounIndex][0])) {
			Utils::Box(BOX_ANY, "%s", _vm->_file->fetchString(obj[i].commentIndex));
			return true;
		}
	}
	return false;
}

/**
* Do all things necessary to carry an object
*/
void Parser_v1d::takeObject(object_t *obj) {
	debugC(1, kDebugParser, "takeObject(object_t *obj)");

	obj->carriedFl = true;
	if (obj->seqNumb)                               // Don't change if no image to display
		obj->cycling = ALMOST_INVISIBLE;

	_vm->adjustScore(obj->objValue);

	Utils::Box(BOX_ANY, TAKE_TEXT, _vm->_arrayNouns[obj->nounIndex][TAKE_NAME]);
}

/**
* Do all necessary things to drop an object
*/
void Parser_v1d::dropObject(object_t *obj) {
	debugC(1, kDebugParser, "dropObject(object_t *obj)");

	obj->carriedFl = false;
	obj->screenIndex = *_vm->_screen_p;
	if (obj->seqNumb)                               // Don't change if no image to display
		obj->cycling = NOT_CYCLING;
	obj->x = _vm->_hero->x - 1;
	obj->y = _vm->_hero->y + _vm->_hero->currImagePtr->y2 - 1;
	_vm->adjustScore(-obj->objValue);
	Utils::Box(BOX_ANY, "%s", _vm->_textParser[kTBOk]);
}

/**
* Print text for possible background object.  Return TRUE if match found
* If test_noun TRUE, must have a noun given
*/
bool Parser_v1d::isCatchallVerb(bool testNounFl, char *noun, char *verb, objectList_t obj) {
	debugC(1, kDebugParser, "isCatchallVerb(%d, %s, %s, object_list_t obj)", (testNounFl) ? 1 : 0, noun, verb);

	if (testNounFl && !noun)
		return false;

	for (int i = 0; obj[i].verbIndex; i++) {
		if ((verb == _vm->_arrayVerbs[obj[i].verbIndex][0]) && ((noun == _vm->_arrayNouns[obj[i].nounIndex][0]) || (obj[i].nounIndex == 0))) {
			Utils::Box(BOX_ANY, "%s", _vm->_file->fetchString(obj[i].commentIndex));
			return true;
		}
	}
	return false;
}

void Parser_v1d::keyHandler(Common::Event event) {
	debugC(1, kDebugParser, "keyHandler(%d)", event.kbd.keycode);

	status_t &gameStatus = _vm->getGameStatus();
	uint16 nChar = event.kbd.keycode;

	// Process key down event - called from OnKeyDown()
	switch (nChar) {                                // Set various toggle states
	case Common::KEYCODE_ESCAPE:                    // Escape key, may want to QUIT
		if (gameStatus.inventoryState == I_ACTIVE)  // Remove inventory, if displayed
			gameStatus.inventoryState = I_UP;
		gameStatus.inventoryObjId = -1;             // Deselect any dragged icon
		break;
	case Common::KEYCODE_END:
	case Common::KEYCODE_HOME:
	case Common::KEYCODE_PAGEUP:
	case Common::KEYCODE_PAGEDOWN:
	case Common::KEYCODE_KP1:
	case Common::KEYCODE_KP7:
	case Common::KEYCODE_KP9:
	case Common::KEYCODE_KP3:
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_UP:
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_KP4:
	case Common::KEYCODE_KP6:
	case Common::KEYCODE_KP8:
	case Common::KEYCODE_KP2:
		gameStatus.routeIndex = -1;                 // Stop any automatic route
		_vm->_route->setWalk(nChar);                // Direction of hero travel
		break;
	case Common::KEYCODE_F1:                        // User Help (DOS)
		if (_checkDoubleF1Fl)
			_vm->_file->instructions();
		else
			_vm->_screen->userHelp();
		_checkDoubleF1Fl = !_checkDoubleF1Fl;
		break;
	case Common::KEYCODE_F2:                        // Toggle sound
		_vm->_sound->toggleSound();
		_vm->_sound->toggleMusic();
		break;
	case Common::KEYCODE_F3:                        // Repeat last line
		gameStatus.recallFl = true;
		break;
	case Common::KEYCODE_F4:                        // Save game
		if (gameStatus.viewState == V_PLAY)
			_vm->_file->saveGame(-1, Common::String());
		break;
	case Common::KEYCODE_F5:                        // Restore game
		_vm->_file->restoreGame(-1);
		_vm->_scheduler->restoreScreen(*_vm->_screen_p);
		gameStatus.viewState = V_PLAY;
		break;
	case Common::KEYCODE_F6:                        // Inventory
		showDosInventory();
		break;
	case Common::KEYCODE_F8:                        // Turbo mode
		_config.turboFl = !_config.turboFl;
		break;
	case Common::KEYCODE_F9:                        // Boss button
		warning("STUB: F9 (DOS) - BossKey");
		break;
	default:                                        // Any other key
		if (!gameStatus.storyModeFl) {              // Keyboard disabled
			// Add printable keys to ring buffer
			uint16 bnext = _putIndex + 1;
			if (bnext >= sizeof(_ringBuffer))
				bnext = 0;
			if (bnext != _getIndex) {
				_ringBuffer[_putIndex] = event.kbd.ascii;
				_putIndex = bnext;
			}
		}
		break;
	}
	if (_checkDoubleF1Fl && (nChar != Common::KEYCODE_F1))
		_checkDoubleF1Fl = false;
}

/**
* Parse the user's line of text input.  Generate events as necessary
*/
void Parser_v1d::lineHandler() {
	debugC(1, kDebugParser, "lineHandler()");

	status_t &gameStatus = _vm->getGameStatus();

	// Toggle God Mode
	if (!strncmp(_line, "PPG", 3)) {
		_vm->_sound->playSound(!_vm->_soundTest, BOTH_CHANNELS, HIGH_PRI);
		gameStatus.godModeFl = !gameStatus.godModeFl;
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
			for (int i = 0; i < _vm->_numScreens; i++) {
				if (!scumm_stricmp(&_line[strlen("goto") + 1], _vm->_screenNames[i])) {
					_vm->_scheduler->newScreen(i);
					return;
				}
			}
		}

		// Special code to allow me to get objects from anywhere
		if (strstr(_line, "fetch all")) {
			for (int i = 0; i < _vm->_object->_numObj; i++) {
				if (_vm->_object->_objects[i].genericCmd & TAKE)
					takeObject(&_vm->_object->_objects[i]);
			}
			return;
		}

		if (strstr(_line, "fetch")) {
			for (int i = 0; i < _vm->_object->_numObj; i++) {
				if (!scumm_stricmp(&_line[strlen("fetch") + 1], _vm->_arrayNouns[_vm->_object->_objects[i].nounIndex][0])) {
					takeObject(&_vm->_object->_objects[i]);
					return;
				}
			}
		}

		// Special code to allow me to goto objects
		if (strstr(_line, "find")) {
			for (int i = 0; i < _vm->_object->_numObj; i++) {
				if (!scumm_stricmp(&_line[strlen("find") + 1], _vm->_arrayNouns[_vm->_object->_objects[i].nounIndex][0])) {
					_vm->_scheduler->newScreen(_vm->_object->_objects[i].screenIndex);
					return;
				}
			}
		}
	}

	if (!strcmp("exit", _line) || strstr(_line, "quit")) {
		if (Utils::Box(BOX_YESNO, "%s", _vm->_textParser[kTBExit_1d]) != 0)
			_vm->endGame();
		return;
	}

	// SAVE/RESTORE
	if (!strcmp("save", _line)) {
		if (gameStatus.gameOverFl)
			Utils::gameOverMsg();
		else
			_vm->_file->saveGame(-1, Common::String());
		return;
	}

	if (!strcmp("restore", _line)) {
		_vm->_file->restoreGame(-1);
		_vm->_scheduler->restoreScreen(*_vm->_screen_p);
		gameStatus.viewState = V_PLAY;
		return;
	}

	if (*_line == '\0')                             // Empty line
		return;

	if (strspn(_line, " ") == strlen(_line))        // Nothing but spaces!
		return;

	if (gameStatus.gameOverFl) {                    // No commands allowed!
		Utils::gameOverMsg();
		return;
	}

	// Find the first verb in the line
	char *verb = findVerb();
	char *noun = 0;                                 // Noun not found yet
	char farComment[XBYTES * 5] = "";               // hold 5 line comment if object not nearby

	if (verb) {                                     // OK, verb found.  Try to match with object
		do {
			noun = findNextNoun(noun);              // Find a noun in the line
			// Must try at least once for objects allowing verb-context
			for (int i = 0; i < _vm->_object->_numObj; i++) {
				object_t *obj = &_vm->_object->_objects[i];
				if (isNear(verb, noun, obj, farComment)) {
					if (isObjectVerb(verb, obj)     // Foreground object
					 || isGenericVerb(verb, obj))   // Common action type
						return;
				}
			}
			if ((*farComment == '\0') && isBackgroundWord(noun, verb, _vm->_backgroundObjects[*_vm->_screen_p]))
				return;
		} while (noun);
	}
	noun = findNextNoun(noun);
	if (*farComment != '\0')                        // An object matched but not near enough
		Utils::Box(BOX_ANY, "%s", farComment);
	else if (!isCatchallVerb(true, noun, verb, _vm->_catchallList) &&
		     !isCatchallVerb(false, noun, verb, _vm->_backgroundObjects[*_vm->_screen_p])  &&
		     !isCatchallVerb(false, noun, verb, _vm->_catchallList))
		Utils::Box(BOX_ANY, "%s", _vm->_textParser[kTBEh_1d]);
}

} // End of namespace Hugo
