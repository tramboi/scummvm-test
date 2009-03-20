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


#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/intern.h"
#include "scumm/object.h"
#include "scumm/scumm_v0.h"
#include "scumm/verbs.h"

namespace Scumm {

#define OPCODE(x)	_OPCODE(ScummEngine_v0, x)

void ScummEngine_v0::setupOpcodes() {
	static const OpcodeEntryC64 opcodes[256] = {
		/* 00 */
		OPCODE(o5_stopObjectCode),
		OPCODE(o2_putActor),
		OPCODE(o5_startMusic),
		OPCODE(o_doSentence),
		/* 04 */
		OPCODE(o_isGreaterEqual),
		OPCODE(o_stopCurrentScript),
		OPCODE(o5_getDist),
		OPCODE(o5_getActorRoom),
		/* 08 */
		OPCODE(o_isNotEqual),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_setActorBitVar),
		/* 0C */
		OPCODE(o_loadSound),
		OPCODE(o_printEgo_c64),
		OPCODE(o_putActorAtObject),
		OPCODE(o2_clearState02),
		/* 10 */
		OPCODE(o5_breakHere),
		OPCODE(o_animateActor),
		OPCODE(o2_panCameraTo),
		OPCODE(o_lockCostume),
		/* 14 */
		OPCODE(o_print_c64),
		OPCODE(o5_walkActorToActor),
		OPCODE(o5_getRandomNr),
		OPCODE(o_clearState08),
		/* 18 */
		OPCODE(o_jumpRelative),
		OPCODE(o_stopCurrentScript),
		OPCODE(o5_move),
		OPCODE(o_getActorBitVar),
		/* 1C */
		OPCODE(o5_startSound),
		OPCODE(o_setBitVar),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifState04),
		/* 20 */
		OPCODE(o5_stopMusic),
		OPCODE(o2_putActor),
		OPCODE(o5_saveLoadGame),
		OPCODE(o_stopCurrentScript),
		/* 24 */
		OPCODE(o_unknown2),
		OPCODE(o5_loadRoom),
		OPCODE(o_getClosestObjActor),
		OPCODE(o2_getActorY),
		/* 28 */
		OPCODE(o_equalZero),
		OPCODE(o_setOwnerOf),
		OPCODE(o2_delay),
		OPCODE(o_setActorBitVar),
		/* 2C */
		OPCODE(o_stopCurrentScript),
		OPCODE(o2_putActorInRoom),
		OPCODE(o_print_c64),
		OPCODE(o2_ifState08),
		/* 30 */
		OPCODE(o_loadCostume),
		OPCODE(o_getBitVar),
		OPCODE(o2_setCameraAt),
		OPCODE(o_lockScript),
		/* 34 */
		OPCODE(o5_getDist),
		OPCODE(o_stopCurrentScript),
		OPCODE(o2_walkActorToObject),
		OPCODE(o2_clearState04),
		/* 38 */
		OPCODE(o_isLessEqual),
		OPCODE(o_stopCurrentScript),
		OPCODE(o2_subtract),
		OPCODE(o_stopCurrentScript),
		/* 3C */
		OPCODE(o5_stopSound),
		OPCODE(o_setBitVar),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifState02),
		/* 40 */
		OPCODE(o_cutscene),
		OPCODE(o2_putActor),
		OPCODE(o2_startScript),
		OPCODE(o_doSentence),
		/* 44 */
		OPCODE(o_isLess),
		OPCODE(o_stopCurrentScript),
		OPCODE(o5_increment),
		OPCODE(o2_getActorX),
		/* 48 */
		OPCODE(o_isEqual),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_loadRoom),
		OPCODE(o_setActorBitVar),
		/* 4C */
		OPCODE(o_loadScript),
		OPCODE(o_lockRoom),
		OPCODE(o_putActorAtObject),
		OPCODE(o2_clearState02),
		/* 50 */
		OPCODE(o_nop),
		OPCODE(o_animateActor),
		OPCODE(o5_actorFollowCamera),
		OPCODE(o_lockSound),
		/* 54 */
		OPCODE(o_setObjectName),
		OPCODE(o5_walkActorToActor),
		OPCODE(o_getActorMoving),
		OPCODE(o_clearState08),
		/* 58 */
		OPCODE(o_beginOverride),
		OPCODE(o_stopCurrentScript),
		OPCODE(o2_add),
		OPCODE(o_getActorBitVar),
		/* 5C */
		OPCODE(o5_startSound),
		OPCODE(o_setBitVar),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifState04),
		/* 60 */
		OPCODE(o_cursorCommand),
		OPCODE(o2_putActor),
		OPCODE(o2_stopScript),
		OPCODE(o_stopCurrentScript),
		/* 64 */
		OPCODE(o_ifActiveObject),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_getClosestObjActor),
		OPCODE(o5_getActorFacing),
		/* 68 */
		OPCODE(o5_isScriptRunning),
		OPCODE(o_setOwnerOf),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_setActorBitVar),
		/* 6C */
		OPCODE(o_stopCurrentScript),
		OPCODE(o2_putActorInRoom),
		OPCODE(o2_dummy),
		OPCODE(o2_ifState08),
		/* 70 */
		OPCODE(o_lights),
		OPCODE(o_getBitVar),
		OPCODE(o_nop),
		OPCODE(o5_getObjectOwner),
		/* 74 */
		OPCODE(o5_getDist),
		OPCODE(o_printEgo_c64),
		OPCODE(o2_walkActorToObject),
		OPCODE(o2_clearState04),
		/* 78 */
		OPCODE(o_isGreater),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_stopCurrentScript),
		/* 7C */
		OPCODE(o5_isSoundRunning),
		OPCODE(o_setBitVar),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifNotState02),
		/* 80 */
		OPCODE(o_stopCurrentScript),
		OPCODE(o2_putActor),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_doSentence),
		/* 84 */
		OPCODE(o_isGreaterEqual),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_nop),
		OPCODE(o5_getActorRoom),
		/* 88 */
		OPCODE(o_isNotEqual),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_setActorBitVar),
		/* 8C */
		OPCODE(o_loadSound),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_putActorAtObject),
		OPCODE(o2_setState02),
		/* 90 */
		OPCODE(o_pickupObject),
		OPCODE(o_animateActor),
		OPCODE(o2_panCameraTo),
		OPCODE(o_unlockCostume),
		/* 94 */
		OPCODE(o5_print),
		OPCODE(o2_actorFromPos),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_setState08),
		/* 98 */
		OPCODE(o2_restart),
		OPCODE(o_stopCurrentScript),
		OPCODE(o5_move),
		OPCODE(o_getActorBitVar),
		/* 9C */
		OPCODE(o5_startSound),
		OPCODE(o_setBitVar),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifNotState04),
		/* A0 */
		OPCODE(o5_stopObjectCode),
		OPCODE(o2_putActor),
		OPCODE(o5_saveLoadGame),
		OPCODE(o_stopCurrentScript),
		/* A4 */
		OPCODE(o_unknown2),
		OPCODE(o5_loadRoom),
		OPCODE(o_stopCurrentScript),
		OPCODE(o2_getActorY),
		/* A8 */
		OPCODE(o_notEqualZero),
		OPCODE(o_setOwnerOf),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_setActorBitVar),
		/* AC */
		OPCODE(o_stopCurrentScript),
		OPCODE(o2_putActorInRoom),
		OPCODE(o_print_c64),
		OPCODE(o2_ifNotState08),
		/* B0 */
		OPCODE(o_loadCostume),
		OPCODE(o_getBitVar),
		OPCODE(o2_setCameraAt),
		OPCODE(o_unlockScript),
		/* B4 */
		OPCODE(o5_getDist),
		OPCODE(o_stopCurrentScript),
		OPCODE(o2_walkActorToObject),
		OPCODE(o2_setState04),
		/* B8 */
		OPCODE(o_isLessEqual),
		OPCODE(o_stopCurrentScript),
		OPCODE(o2_subtract),
		OPCODE(o_stopCurrentScript),
		/* BC */
		OPCODE(o5_stopSound),
		OPCODE(o_setBitVar),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifNotState02),
		/* C0 */
		OPCODE(o_endCutscene),
		OPCODE(o2_putActor),
		OPCODE(o2_startScript),
		OPCODE(o_doSentence),
		/* C4 */
		OPCODE(o_isLess),
		OPCODE(o_stopCurrentScript),
		OPCODE(o5_decrement),
		OPCODE(o2_getActorX),
		/* C8 */
		OPCODE(o_isEqual),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_loadRoom),
		OPCODE(o_setActorBitVar),
		/* CC */
		OPCODE(o_loadScript),
		OPCODE(o_unlockRoom),
		OPCODE(o_putActorAtObject),
		OPCODE(o2_setState02),
		/* D0 */
		OPCODE(o_nop),
		OPCODE(o_animateActor),
		OPCODE(o5_actorFollowCamera),
		OPCODE(o_unlockSound),
		/* D4 */
		OPCODE(o_setObjectName),
		OPCODE(o2_actorFromPos),
		OPCODE(o_getActorMoving),
		OPCODE(o_setState08),
		/* D8 */
		OPCODE(o_stopCurrentScript),
		OPCODE(o_stopCurrentScript),
		OPCODE(o2_add),
		OPCODE(o_getActorBitVar),
		/* DC */
		OPCODE(o5_startSound),
		OPCODE(o_setBitVar),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifNotState04),
		/* E0 */
		OPCODE(o_cursorCommand),
		OPCODE(o2_putActor),
		OPCODE(o2_stopScript),
		OPCODE(o_stopCurrentScript),
		/* E4 */
		OPCODE(o_ifActiveObject),
		OPCODE(o_loadRoomWithEgo),
		OPCODE(o_stopCurrentScript),
		OPCODE(o5_getActorFacing),
		/* E8 */
		OPCODE(o5_isScriptRunning),
		OPCODE(o_setOwnerOf),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_setActorBitVar),
		/* EC */
		OPCODE(o_stopCurrentScript),
		OPCODE(o2_putActorInRoom),
		OPCODE(o2_dummy),
		OPCODE(o2_ifNotState08),
		/* F0 */
		OPCODE(o_lights),
		OPCODE(o_getBitVar),
		OPCODE(o_nop),
		OPCODE(o5_getObjectOwner),
		/* F4 */
		OPCODE(o5_getDist),
		OPCODE(o_stopCurrentScript),
		OPCODE(o2_walkActorToObject),
		OPCODE(o2_setState04),
		/* F8 */
		OPCODE(o_isGreater),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_stopCurrentScript),
		/* FC */
		OPCODE(o5_isSoundRunning),
		OPCODE(o_setBitVar),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifState02)
	};

	_opcodesC64 = opcodes;
}

#define SENTENCE_SCRIPT 2

#define PARAM_1 0x80
#define PARAM_2 0x40
#define PARAM_3 0x20

void ScummEngine_v0::executeOpcode(byte i) {
	OpcodeProcC64 op = _opcodesC64[i].proc;
	(this->*op) ();
}

int ScummEngine_v0::getVarOrDirectWord(byte mask) {
	return getVarOrDirectByte(mask);
}

uint ScummEngine_v0::fetchScriptWord() {
	return fetchScriptByte();
}

const char *ScummEngine_v0::getOpcodeDesc(byte i) {
	return _opcodesC64[i].desc;
}

int ScummEngine_v0::getObjectFlag() {
	if (_opcode & 0x40)
		return _activeObject;

	return fetchScriptByte();
}

void ScummEngine_v0::decodeParseString() {
	byte buffer[512];
	byte *ptr = buffer;
	byte c;
	bool insertSpace = false;

	while ((c = fetchScriptByte())) {

		insertSpace = (c & 0x80) != 0;
		c &= 0x7f;

		if (c == '/') {
			*ptr++ = 13;
		} else {
			*ptr++ = c;
		}

		if (insertSpace)
			*ptr++ = ' ';

	}
	*ptr = 0;

	int textSlot = 0;
	_string[textSlot].xpos = 0;
	_string[textSlot].ypos = 0;
	_string[textSlot].right = _screenWidth - 1;
	_string[textSlot].center = false;
	_string[textSlot].overhead = false;

	if (_actorToPrintStrFor == 0xFF)
		_string[textSlot].color = 14;

	actorTalk(buffer);
}

void ScummEngine_v0::setStateCommon(byte type) {
	int obj = getObjectFlag();
	putState(obj, getState(obj) | type);
}

void ScummEngine_v0::clearStateCommon(byte type) {
	int obj = getObjectFlag();
	putState(obj, getState(obj) & ~type);
}

void ScummEngine_v0::ifStateCommon(byte type) {
	int obj = getObjectFlag();

	if ((getState(obj) & type) != 0)
		ScummEngine::fetchScriptWord();
	else
		o_jumpRelative();
}

void ScummEngine_v0::ifNotStateCommon(byte type) {
	int obj = getObjectFlag();

	if ((getState(obj) & type) == 0)
		ScummEngine::fetchScriptWord();
	else
		o_jumpRelative();
}

void ScummEngine_v0::drawSentence() {
	Common::Rect sentenceline;
	const byte *temp;
	int sentencePrep = 0;

	if (!(_userState & 32))
		return;

	if (getResourceAddress(rtVerb, _activeVerb)) {
		strcpy(_sentenceBuf, (char*)getResourceAddress(rtVerb, _activeVerb));
	} else {
		return;
	}

	if (_activeObject > 0) {
		temp = getObjOrActorName(_activeObject);
		if (temp) {
			strcat(_sentenceBuf, " ");
			strcat(_sentenceBuf, (const char*)temp);
		}

		if (_verbs[_activeVerb].prep == 0xFF) {
			byte *ptr = getOBCDFromObject(_activeObject);
			assert(ptr);
			sentencePrep = (*(ptr + 11) >> 5);
		} else {
			sentencePrep = _verbs[_activeVerb].prep;
		}
	}

	if (sentencePrep > 0 && sentencePrep <= 4) {
		// The prepositions, like the fonts, were hard code in the engine. Thus
		// we have to do that, too, and provde localized versions for all the
		// languages MM/Zak are available in.
		const char *prepositions[][5] = {
			{ " ", " in", " with", " on", " to" },   // English
			{ " ", " mit", " mit", " mit", " zu" },  // German
			{ " ", " dans", " avec", " sur", " <" }, // French
			{ " ", " in", " con", " su", " a" },     // Italian
			{ " ", " en", " con", " en", " a" },     // Spanish
			};
		int lang;
		switch (_language) {
		case Common::DE_DEU:
			lang = 1;
			break;
		case Common::FR_FRA:
			lang = 2;
			break;
		case Common::IT_ITA:
			lang = 3;
			break;
		case Common::ES_ESP:
			lang = 4;
			break;
		default:
			lang = 0;	// Default to english
		}

		strcat(_sentenceBuf, prepositions[lang][sentencePrep]);
	}

	if (_activeInventory > 0) {
		temp = getObjOrActorName(_activeInventory);
		if (temp) {
			strcat(_sentenceBuf, " ");
			strcat(_sentenceBuf, (const char*)temp);
		}
	}

	_string[2].charset = 1;
	_string[2].ypos = _virtscr[kVerbVirtScreen].topline;
	_string[2].xpos = 0;
	_string[2].right = _virtscr[kVerbVirtScreen].w - 1;
	_string[2].color = 16;

	byte string[80];
	char *ptr = _sentenceBuf;
	int i = 0, len = 0;

	// Maximum length of printable characters
	int maxChars = 40;
	while (*ptr) {
		if (*ptr != '@')
			len++;
		if (len > maxChars) {
			break;
		}

		string[i++] = *ptr++;

	}
	string[i] = 0;

	sentenceline.top = _virtscr[kVerbVirtScreen].topline;
	sentenceline.bottom = _virtscr[kVerbVirtScreen].topline + 8;
	sentenceline.left = 0;
	sentenceline.right = _virtscr[kVerbVirtScreen].w - 1;
	restoreBackground(sentenceline);

	drawString(2, (byte*)string);
}

void ScummEngine_v0::o_setState08() {
	int obj = getObjectFlag();
	putState(obj, getState(obj) | kObjectState_08);
	markObjectRectAsDirty(obj);
	clearDrawObjectQueue();
}

void ScummEngine_v0::o_clearState08() {
	int obj = getObjectFlag();
	putState(obj, getState(obj) & ~kObjectState_08);
	markObjectRectAsDirty(obj);
	clearDrawObjectQueue();
}

void ScummEngine_v0::o_stopCurrentScript() {
	int script;

	script = vm.slot[_currentScript].number;

	if (_currentScript != 0 && vm.slot[_currentScript].number == script)
		stopObjectCode();
	else
		stopScript(script);
}

void ScummEngine_v0::o_loadSound() {
	int resid = fetchScriptByte();
	ensureResourceLoaded(rtSound, resid);
}

void ScummEngine_v0::o_lockSound() {
	int resid = fetchScriptByte();
	_res->lock(rtSound, resid);
}

void ScummEngine_v0::o_unlockSound() {
	int resid = fetchScriptByte();
	_res->unlock(rtSound, resid);
}

void ScummEngine_v0::o_loadCostume() {
	int resid = getVarOrDirectByte(PARAM_1);
	ensureResourceLoaded(rtCostume, resid);
}

void ScummEngine_v0::o_lockCostume() {
	int resid = fetchScriptByte();
	_res->lock(rtCostume, resid);
}

void ScummEngine_v0::o_unlockCostume() {
	int resid = fetchScriptByte();
	_res->unlock(rtCostume, resid);
}

void ScummEngine_v0::o_loadScript() {
	int resid = getVarOrDirectByte(PARAM_1);
	ensureResourceLoaded(rtScript, resid);
}

void ScummEngine_v0::o_lockScript() {
	int resid = fetchScriptByte();
	_res->lock(rtScript, resid);
}

void ScummEngine_v0::o_unlockScript() {
	int resid = fetchScriptByte();
	_res->unlock(rtScript, resid);
}

void ScummEngine_v0::o_loadRoom() {
	int resid = getVarOrDirectByte(PARAM_1);
	ensureResourceLoaded(rtRoom, resid);
}

void ScummEngine_v0::o_loadRoomWithEgo() {
	Actor *a;
	int obj, room, x, y, dir;

	obj = fetchScriptByte();
	room = fetchScriptByte();

	a = derefActor(VAR(VAR_EGO), "o_loadRoomWithEgo");

	a->putActor(0, 0, room);
	_egoPositioned = false;

	startScene(a->_room, a, obj);

	getObjectXYPos(obj, x, y, dir);
	AdjustBoxResult r = a->adjustXYToBeInBox(x, y);
	x = r.x;
	y = r.y;
	a->putActor(x, y, _currentRoom);
	a->setDirection(dir + 180);

	camera._dest.x = camera._cur.x = a->getPos().x;
	setCameraAt(a->getPos().x, a->getPos().y);
	setCameraFollows(a);

	_fullRedraw = true;

	resetSentence();

	if (x >= 0 && y >= 0) {
		a->startWalkActor(x, y, -1);
	}
}

void ScummEngine_v0::o_lockRoom() {
	int resid = fetchScriptByte();
	_res->lock(rtRoom, resid);
}

void ScummEngine_v0::o_unlockRoom() {
	int resid = fetchScriptByte();
	_res->unlock(rtRoom, resid);
}

void ScummEngine_v0::o_cursorCommand() {
	// TODO
	int state = 0;

	_currentMode = fetchScriptByte();
	switch (_currentMode) {
	case 0:
		state = 15;
		break;
	case 1:
		state = 31;
		break;
	case 2:
		break;
	case 3:
		state = 247;
		break;
	}

	setUserState(state);
	debug(0, "o_cursorCommand(%d)", _currentMode);
}

void ScummEngine_v0::o_lights() {
	int a;

	a = getVarOrDirectByte(PARAM_1);
	// Convert older light mode values into
	// equivalent values.of later games
	// 0 Darkness
	// 1 Flashlight
	// 2 Lighted area
	if (a == 2)
		_currentLights = 11;
	else if (a == 1)
		_currentLights = 4;
	else
		_currentLights = 0;

	_fullRedraw = true;
}

void ScummEngine_v0::o_animateActor() {
	int act = getVarOrDirectByte(PARAM_1);
	int anim = getVarOrDirectByte(PARAM_2);
	int unk = fetchScriptByte();
	debug(0,"o_animateActor: unk %d", unk);

	Actor *a = derefActor(act, "o_animateActor");
	a->animateActor(anim);
}

void ScummEngine_v0::o_getActorMoving() {
	getResultPos();
	int act = getVarOrDirectByte(PARAM_1);
	Actor *a = derefActor(act, "o_getActorMoving");
	if (a->_moving)
		setResult(1);
	else
		setResult(2);
}

void ScummEngine_v0::o_putActorAtObject() {
	int obj, x, y;
	Actor *a;

	a = derefActor(getVarOrDirectByte(PARAM_1), "o_putActorAtObject");

	obj = fetchScriptByte();
	if (whereIsObject(obj) != WIO_NOT_FOUND) {
		getObjectXYPos(obj, x, y);
		AdjustBoxResult r = a->adjustXYToBeInBox(x, y);
		x = r.x;
		y = r.y;
	} else {
		x = 30;
		y = 60;
	}

	a->putActor(x, y);
}

void ScummEngine_v0::o_pickupObject() {
	int obj = fetchScriptByte();
	if (obj == 0) {
		obj = _activeObject;
	}

	if (obj < 1) {
		error("pickupObject received invalid index %d (script %d)", obj, vm.slot[_currentScript].number);
	}

	if (getObjectIndex(obj) == -1)
		return;

	if (whereIsObject(obj) == WIO_INVENTORY)	/* Don't take an */
		return;					/* object twice */

	addObjectToInventory(obj, _roomResource);
	markObjectRectAsDirty(obj);
	putOwner(obj, VAR(VAR_EGO));
	putState(obj, getState(obj) | kObjectState_08 | kObjectStateUntouchable);
	clearDrawObjectQueue();

	runInventoryScript(1);
}

void ScummEngine_v0::o_setObjectName() {
	int obj = fetchScriptByte();
	setObjectName(obj);
}

void ScummEngine_v0::o_nop() {
}

// TODO: Maybe translate actor flags in future.
void ScummEngine_v0::o_setActorBitVar() {
	byte act = getVarOrDirectByte(PARAM_1);
	byte mask = getVarOrDirectByte(PARAM_2);
	byte mod = getVarOrDirectByte(PARAM_3);

	ActorC64 *a = (ActorC64 *)derefActor(act, "o_setActorBitVar");
	if (mod)
		a->_miscflags |= mask;
	else
		a->_miscflags &= ~mask;

	debug(0, "o_setActorBitVar(%d, %d, %d)", act, mask, mod);
}

void ScummEngine_v0::o_getActorBitVar() {
	getResultPos();
	byte act = getVarOrDirectByte(PARAM_1);
	byte mask = getVarOrDirectByte(PARAM_2);

	ActorC64 *a = (ActorC64 *)derefActor(act, "o_getActorBitVar");
	setResult((a->_miscflags & mask) ? 1 : 0);

	debug(0, "o_getActorBitVar(%d, %d, %d)", act, mask, (a->_miscflags & mask));
}

void ScummEngine_v0::o_setBitVar() {
	byte flag = getVarOrDirectByte(PARAM_1);
	byte mask = getVarOrDirectByte(PARAM_2);
	byte mod = getVarOrDirectByte(PARAM_3);

	if (mod)
		_bitVars[flag] |= (1 << mask);
	else
		_bitVars[flag] &= ~(1 << mask);

	debug(0, "o_setBitVar (%d, %d %d)", flag, mask, mod);
}

void ScummEngine_v0::o_getBitVar() {
	getResultPos();
	byte flag = getVarOrDirectByte(PARAM_1);
	byte mask = getVarOrDirectByte(PARAM_2);

	setResult((_bitVars[flag] & (1 << mask)) ? 1 : 0);

	debug(0, "o_getBitVar (%d, %d %d)", flag, mask, _bitVars[flag] & (1 << mask));
}

void ScummEngine_v0::o_print_c64() {
	_actorToPrintStrFor = fetchScriptByte();
	decodeParseString();
}

void ScummEngine_v0::o_printEgo_c64() {
	_actorToPrintStrFor = (byte)VAR(VAR_EGO);
	decodeParseString();
}

void ScummEngine_v0::o_doSentence() {
	byte entry = fetchScriptByte();
	byte obj = fetchScriptByte();
	fetchScriptByte();

	runObjectScript(obj, entry, false, false, NULL);
}

void ScummEngine_v0::o_unknown2() {
	byte var1 = fetchScriptByte();
	error("STUB: o_unknown2(%d)", var1);
}

void ScummEngine_v0::o_ifActiveObject() {
	byte obj = fetchScriptByte();

	if (obj == _activeInventory)
		ScummEngine::fetchScriptWord();
	else
		o_jumpRelative();
}

void ScummEngine_v0::o_getClosestObjActor() {
	int obj;
	int act;
	int dist;

	// This code can't detect any actors farther away than 255 units
	// (pixels in newer games, characters in older ones.) But this is
	// perfectly OK, as it is exactly how the original behaved.

	int closest_obj = 0xFF, closest_dist = 0xFF;

	getResultPos();

	act = getVarOrDirectByte(PARAM_1);
	obj = (_opcode & 0x40) ? 25 : 7;

	do {
		dist = getObjActToObjActDist(act, obj);
		if (dist < closest_dist) {
			closest_dist = dist;
			closest_obj = obj;
		}
	} while (--obj);

	setResult(closest_obj);
}

void ScummEngine_v0::o_cutscene() {
	vm.cutSceneData[0] = _userState | (_userPut ? 16 : 0);
	vm.cutSceneData[2] = _currentRoom;
	vm.cutSceneData[3] = camera._mode;

	// Hide inventory, freeze scripts, hide cursor
	setUserState(15);

	_sentenceNum = 0;
	stopScript(SENTENCE_SCRIPT);
	resetSentence();

	vm.cutScenePtr[0] = 0;
}

void ScummEngine_v0::o_endCutscene() {
	vm.cutSceneStackPointer = 0;

	VAR(VAR_OVERRIDE) = 0;
	vm.cutSceneScript[0] = 0;
	vm.cutScenePtr[0] = 0;

	// Reset user state to values before cutscene
	setUserState(vm.cutSceneData[0] | 7);

	camera._mode = (byte) vm.cutSceneData[3];
	if (camera._mode == kFollowActorCameraMode) {
		actorFollowCamera(VAR(VAR_EGO));
	} else if (vm.cutSceneData[2] != _currentRoom) {
		startScene(vm.cutSceneData[2], 0, 0);
	}
}

void ScummEngine_v0::o_beginOverride() {
	const int idx = vm.cutSceneStackPointer;
	assert(0 <= idx && idx < 5);

	vm.cutScenePtr[idx] = _scriptPointer - _scriptOrgPointer;
	vm.cutSceneScript[idx] = _currentScript;

	// Skip the jump instruction following the override instruction
	// (the jump is responsible for "skipping" cutscenes, and the reason
	// why we record the current script position in vm.cutScenePtr).
	fetchScriptByte();
	ScummEngine::fetchScriptWord();

	// This is based on disassembly
	VAR(VAR_OVERRIDE) = 0;
}

void ScummEngine_v0::o_isEqual() {
	int16 a, b;
	int var;

	var = fetchScriptByte();
	a = readVar(var);
	b = getVarOrDirectByte(PARAM_1);

	if (b == a)
		ScummEngine::fetchScriptWord();
	else
		o_jumpRelative();

}

void ScummEngine_v0::o_isGreater() {
	int16 a = getVar();
	int16 b = getVarOrDirectByte(PARAM_1);
	if (b > a)
		ScummEngine::fetchScriptWord();
	else
		o_jumpRelative();
}

void ScummEngine_v0::o_isGreaterEqual() {
	int16 a = getVar();
	int16 b = getVarOrDirectByte(PARAM_1);
	if (b >= a)
		ScummEngine::fetchScriptWord();
	else
		o_jumpRelative();
}

void ScummEngine_v0::o_isLess() {
	int16 a = getVar();
	int16 b = getVarOrDirectByte(PARAM_1);
	if (b < a)
		ScummEngine::fetchScriptWord();
	else
		o_jumpRelative();
}

void ScummEngine_v0::o_isLessEqual() {
	int16 a = getVar();
	int16 b = getVarOrDirectByte(PARAM_1);

	if (b <= a)
		ScummEngine::fetchScriptWord();
	else
		o_jumpRelative();
}

void ScummEngine_v0::o_isNotEqual() {
	int16 a = getVar();
	int16 b = getVarOrDirectByte(PARAM_1);
	if (b != a)
		ScummEngine::fetchScriptWord();
	else
		o_jumpRelative();
}

void ScummEngine_v0::o_notEqualZero() {
	int a = getVar();
	if (a != 0)
		ScummEngine::fetchScriptWord();
	else
		o_jumpRelative();
}

void ScummEngine_v0::o_equalZero() {
	int a = getVar();
	if (a == 0)
		ScummEngine::fetchScriptWord();
	else
		o_jumpRelative();
}

void ScummEngine_v0::o_jumpRelative() {
	int16 offset = (int16)ScummEngine::fetchScriptWord();
	_scriptPointer += offset;
}

void ScummEngine_v0::o_setOwnerOf() {
	int obj, owner;

	obj = getVarOrDirectWord(PARAM_1);
	owner = getVarOrDirectByte(PARAM_2);

	if (obj == 0)
		obj = _activeInventory;

	setOwnerOf(obj, owner);
}

void ScummEngine_v0::resetSentence() {
	_activeInventory = 0;
	_activeObject = 0;
	_activeVerb = 13;
}

#undef PARAM_1
#undef PARAM_2
#undef PARAM_3

} // End of namespace Scumm
