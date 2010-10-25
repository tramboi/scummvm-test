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

#include "common/system.h"
#include "common/random.h"

#include "hugo/game.h"
#include "hugo/hugo.h"
#include "hugo/object.h"
#include "hugo/global.h"
#include "hugo/display.h"
#include "hugo/file.h"
#include "hugo/route.h"
#include "hugo/util.h"
#include "hugo/parser.h"

namespace Hugo {

ObjectHandler::ObjectHandler(HugoEngine *vm) : _vm(vm) {
}

ObjectHandler::~ObjectHandler() {
}

void ObjectHandler::saveSeq(object_t *obj) {
// Save sequence number and image number in given object
	debugC(1, kDebugFile, "saveSeq");

	bool found = false;
	for (int j = 0; !found && (j < obj->seqNumb); j++) {
		seq_t *q = obj->seqList[j].seqPtr;
		for (int k = 0; !found && (k < obj->seqList[j].imageNbr); k++) {
			if (obj->currImagePtr == q) {
				found = true;
				obj->curSeqNum = j;
				obj->curImageNum = k;
			} else {
				q = q->nextSeqPtr;
			}
		}
	}
}

void ObjectHandler::restoreSeq(object_t *obj) {
// Set up cur_seq_p from stored sequence and image number in object
	debugC(1, kDebugFile, "restoreSeq");

	seq_t *q = obj->seqList[obj->curSeqNum].seqPtr;
	for (int j = 0; j < obj->curImageNum; j++)
		q = q->nextSeqPtr;
	obj->currImagePtr = q;
}

// If status.objid = -1, pick up objid, else use status.objid on objid,
// if objid can't be picked up, use it directly
void ObjectHandler::useObject(int16 objId) {
	debugC(1, kDebugEngine, "useObject(%d)", objId);

	char *verb;                                     // Background verb to use directly
	object_t *obj = &_objects[objId];               // Ptr to object
	if (_vm->getGameStatus().inventoryObjId == -1) {
		// Get or use objid directly
		if ((obj->genericCmd & TAKE) || obj->objValue)  // Get collectible item
			sprintf(_line, "%s %s", _vm->_arrayVerbs[_vm->_take][0], _vm->_arrayNouns[obj->nounIndex][0]);
		else if (obj->genericCmd & LOOK)            // Look item
			sprintf(_line, "%s %s", _vm->_arrayVerbs[_vm->_look][0], _vm->_arrayNouns[obj->nounIndex][0]);
		else if (obj->genericCmd & DROP)            // Drop item
			sprintf(_line, "%s %s", _vm->_arrayVerbs[_vm->_drop][0], _vm->_arrayNouns[obj->nounIndex][0]);
		else if (obj->cmdIndex != 0)                // Use non-collectible item if able
			sprintf(_line, "%s %s", _vm->_arrayVerbs[_vm->_cmdList[obj->cmdIndex][1].verbIndex][0], _vm->_arrayNouns[obj->nounIndex][0]);
		else if ((verb = _vm->useBG(_vm->_arrayNouns[obj->nounIndex][0])) != 0)
			sprintf(_line, "%s %s", verb, _vm->_arrayNouns[obj->nounIndex][0]);
		else
			return;                                 // Can't use object directly
	} else {
		// Use status.objid on objid
		// Default to first cmd verb
		sprintf(_line, "%s %s %s", _vm->_arrayVerbs[_vm->_cmdList[_objects[_vm->getGameStatus().inventoryObjId].cmdIndex][1].verbIndex][0], 
			                       _vm->_arrayNouns[_objects[_vm->getGameStatus().inventoryObjId].nounIndex][0], 
								   _vm->_arrayNouns[obj->nounIndex][0]);

		// Check valid use of objects and override verb if necessary
		for (uses_t *use = _vm->_uses; use->objId != _vm->_numObj; use++) {
			if (_vm->getGameStatus().inventoryObjId == use->objId) {
				// Look for secondary object, if found use matching verb
				bool foundFl = false;
				for (target_t *target = use->targets; _vm->_arrayNouns[target->nounIndex] != 0; target++)
					if (_vm->_arrayNouns[target->nounIndex][0] == _vm->_arrayNouns[obj->nounIndex][0]) {
						foundFl = true;
						sprintf(_line, "%s %s %s", _vm->_arrayVerbs[target->verbIndex][0], 
							                       _vm->_arrayNouns[_objects[_vm->getGameStatus().inventoryObjId].nounIndex][0], 
												   _vm->_arrayNouns[obj->nounIndex][0]);
					}

				// No valid use of objects found, print failure string
				if (!foundFl) {
					// Deselect dragged icon if inventory not active
					if (_vm->getGameStatus().inventoryState != I_ACTIVE)
						_vm->getGameStatus().inventoryObjId  = -1;
					Utils::Box(BOX_ANY, "%s", _vm->_textData[use->dataIndex]);
					return;
				}
			}
		}
	}

	if (_vm->getGameStatus().inventoryState == I_ACTIVE)         // If inventory active, remove it
		_vm->getGameStatus().inventoryState = I_UP;
	_vm->getGameStatus().inventoryObjId  = -1;                   // Deselect any dragged icon
	_vm->_parser->lineHandler();                         // and process command
}

// Return object index of the topmost object under the cursor, or -1 if none
// Objects are filtered if not "useful"
int16 ObjectHandler::findObject(uint16 x, uint16 y) {
	debugC(3, kDebugEngine, "findObject(%d, %d)", x, y);

	int16     objIndex = -1;                        // Index of found object
	uint16    y2Max = 0;                            // Greatest y2
	object_t *obj = _objects;
	// Check objects on screen
	for (int i = 0; i < _vm->_numObj; i++, obj++) {
		// Object must be in current screen and "useful"
		if (obj->screenIndex == *_vm->_screen_p && (obj->genericCmd || obj->objValue || obj->cmdIndex)) {
			seq_t *curImage = obj->currImagePtr;
			// Object must have a visible image...
			if (curImage != 0 && obj->cycling != INVISIBLE) {
				// If cursor inside object
				if (x >= (uint16)obj->x && x <= obj->x + curImage->x2 && y >= (uint16)obj->y && y <= obj->y + curImage->y2) {
					// If object is closest so far
					if (obj->y + curImage->y2 > y2Max) {
						y2Max = obj->y + curImage->y2;
						objIndex = i;               // Found an object!
					}
				}
			} else {
				// ...or a dummy object that has a hotspot rectangle
				if (curImage == 0 && obj->vxPath != 0 && !obj->carriedFl) {
					// If cursor inside special rectangle
					if ((int16)x >= obj->oldx && (int16)x < obj->oldx + obj->vxPath && (int16)y >= obj->oldy && (int16)y < obj->oldy + obj->vyPath) {
						// If object is closest so far
						if (obj->oldy + obj->vyPath - 1 > (int16)y2Max) {
							y2Max = obj->oldy + obj->vyPath - 1;
							objIndex = i;           // Found an object!
						}
					}
				}
			}
		}
	}
	return objIndex;
}

// Issue "Look at <object>" command
// Note special case of swapped hero image
void ObjectHandler::lookObject(object_t *obj) {
	debugC(1, kDebugEngine, "lookObject");

	if (obj == _vm->_hero)
		// Hero swapped - look at other
		obj = &_objects[_vm->_heroImage];

	_vm->_parser->command("%s %s", _vm->_arrayVerbs[_vm->_look][0], _vm->_arrayNouns[obj->nounIndex][0]);
}

// Free all object images
void ObjectHandler::freeObjects() {
	debugC(1, kDebugEngine, "freeObjects");

	// Nothing to do if not allocated yet
	if (_vm->_hero->seqList[0].seqPtr == 0)
		return;

	// Free all sequence lists and image data
	for (int i = 0; i < _vm->_numObj; i++) {
		object_t *obj = &_objects[i];
		for (int j = 0; j < obj->seqNumb; j++) {    // for each sequence
			seq_t *seq = obj->seqList[j].seqPtr;    // Free image
			if (seq == 0)                           // Failure during database load
				break;
			do {
				free(seq->imagePtr);
				seq = seq->nextSeqPtr;
			} while (seq != obj->seqList[j].seqPtr);
			free(seq);                              // Free sequence record
		}
	}
}

// Compare function for the quicksort.  The sort is to order the objects in
// increasing vertical position, using y+y2 as the baseline
// Returns -1 if ay2 < by2 else 1 if ay2 > by2 else 0
int ObjectHandler::y2comp(const void *a, const void *b) {
	debugC(6, kDebugEngine, "y2comp");

//	const object_t *p1 = &s_Engine->_objects[*(const byte *)a];
//	const object_t *p2 = &s_Engine->_objects[*(const byte *)b];
	const object_t *p1 = &HugoEngine::get()._object->_objects[*(const byte *)a];
	const object_t *p2 = &HugoEngine::get()._object->_objects[*(const byte *)b];

	if (p1 == p2)
		// Why does qsort try the same indexes?
		return 0;

	if (p1->priority == BACKGROUND)
		return -1;

	if (p2->priority == BACKGROUND)
		return 1;

	if (p1->priority == FOREGROUND)
		return 1;

	if (p2->priority == FOREGROUND)
		return -1;

	int ay2 = p1->y + p1->currImagePtr->y2;
	int by2 = p2->y + p2->currImagePtr->y2;

	return ay2 - by2;
}

// Return TRUE if object being carried by hero
bool ObjectHandler::isCarrying(uint16 wordIndex) {
	debugC(1, kDebugParser, "isCarrying(%d)", wordIndex);

	for (int i = 0; i < _vm->_numObj; i++) {
		if ((wordIndex == _objects[i].nounIndex) && _objects[i].carriedFl)
			return true;
	}
	return false;
}

// Describe any takeable objects visible in this screen
void ObjectHandler::showTakeables() {
	debugC(1, kDebugParser, "showTakeables");

	for (int j = 0; j < _vm->_numObj; j++) {
		object_t *obj = &_objects[j];
		if ((obj->cycling != INVISIBLE) &&
		    (obj->screenIndex == *_vm->_screen_p) &&
		    (((TAKE & obj->genericCmd) == TAKE) || obj->objValue)) {
			Utils::Box(BOX_ANY, "You can also see:\n%s.", _vm->_arrayNouns[obj->nounIndex][LOOK_NAME]);
		}
	}
}

// Find a clear space around supplied object that hero can walk to
bool ObjectHandler::findObjectSpace(object_t *obj, int16 *destx, int16 *desty) {
	debugC(1, kDebugEngine, "findObjectSpace(obj, %d, %d)", *destx, *desty);

	seq_t *curImage = obj->currImagePtr;
	int16 y = obj->y + curImage->y2 - 1;

	bool foundFl = true;
	// Try left rear corner
	for (int16 x = *destx = obj->x + curImage->x1; x < *destx + HERO_MAX_WIDTH; x++) {
		if (BOUND(x, y))
			foundFl = false;
	}

	if (!foundFl) {                                 // Try right rear corner
		foundFl = true;
		for (int16 x = *destx = obj->x + curImage->x2 - HERO_MAX_WIDTH + 1; x <= obj->x + (int16)curImage->x2; x++) {
			if (BOUND(x, y))
				foundFl = false;
		}
	}

	if (!foundFl) {                                 // Try left front corner
		foundFl = true;
		y += 2;
		for (int16 x = *destx = obj->x + curImage->x1; x < *destx + HERO_MAX_WIDTH; x++) {
			if (BOUND(x, y))
				foundFl = false;
		}
	}

	if (!foundFl) {                                 // Try right rear corner
		foundFl = true;
		for (int16 x = *destx = obj->x + curImage->x2 - HERO_MAX_WIDTH + 1; x <= obj->x + (int16)curImage->x2; x++) {
			if (BOUND(x, y))
				foundFl = false;
		}
	}

	*desty = y;
	return foundFl;
}

void ObjectHandler::loadObject(Common::File &in) {
// TODO: For Hugo3, if not in story mode, set _objects[2].state to 3
	for (int varnt = 0; varnt < _vm->_numVariant; varnt++) {
		uint16 numElem = in.readUint16BE();
		if (varnt == _vm->_gameVariant) {
			_objects = (object_t *)malloc(sizeof(object_t) * numElem);
			for (int i = 0; i < numElem; i++) {
				_objects[i].nounIndex = in.readUint16BE();
				_objects[i].dataIndex = in.readUint16BE();
				uint16 numSubElem = in.readUint16BE();
				if (numSubElem == 0)
					_objects[i].stateDataIndex = 0;
				else
					_objects[i].stateDataIndex = (uint16 *)malloc(sizeof(uint16) * numSubElem);
				for (int j = 0; j < numSubElem; j++)
					_objects[i].stateDataIndex[j] = in.readUint16BE();
				_objects[i].pathType = (path_t) in.readSint16BE();
				_objects[i].vxPath = in.readSint16BE();
				_objects[i].vyPath = in.readSint16BE();
				_objects[i].actIndex = in.readUint16BE();
				_objects[i].seqNumb = in.readByte();
				_objects[i].currImagePtr = 0;
				if (_objects[i].seqNumb == 0) {
					_objects[i].seqList[0].imageNbr = 0;
					_objects[i].seqList[0].seqPtr = 0;
				}
				for (int j = 0; j < _objects[i].seqNumb; j++) {
					_objects[i].seqList[j].imageNbr = in.readUint16BE();
					_objects[i].seqList[j].seqPtr = 0;
				}
				_objects[i].cycling = (cycle_t)in.readByte();
				_objects[i].cycleNumb = in.readByte();
				_objects[i].frameInterval = in.readByte();
				_objects[i].frameTimer = in.readByte();
				_objects[i].radius = in.readByte();
				_objects[i].screenIndex = in.readByte();
				_objects[i].x = in.readSint16BE();
				_objects[i].y = in.readSint16BE();
				_objects[i].oldx = in.readSint16BE();
				_objects[i].oldy = in.readSint16BE();
				_objects[i].vx = in.readByte();
				_objects[i].vy = in.readByte();
				_objects[i].objValue = in.readByte();
				_objects[i].genericCmd = in.readSint16BE();
				_objects[i].cmdIndex = in.readUint16BE();
				_objects[i].carriedFl = (in.readByte() != 0);
				_objects[i].state = in.readByte();
				_objects[i].verbOnlyFl = (in.readByte() != 0);
				_objects[i].priority = in.readByte();
				_objects[i].viewx = in.readSint16BE();
				_objects[i].viewy = in.readSint16BE();
				_objects[i].direction = in.readSint16BE();
				_objects[i].curSeqNum = in.readByte();
				_objects[i].curImageNum = in.readByte();
				_objects[i].oldvx = in.readByte();
				_objects[i].oldvy = in.readByte();
			}
		} else {
			for (int i = 0; i < numElem; i++) {
				in.readUint16BE();
				in.readUint16BE();
				uint16 numSubElem = in.readUint16BE();
				for (int j = 0; j < numSubElem; j++)
					in.readUint16BE();
				in.readSint16BE();
				in.readSint16BE();
				in.readSint16BE();
				in.readUint16BE();
				numSubElem = in.readByte();
				for (int j = 0; j < numSubElem; j++)
					in.readUint16BE();
				in.readByte();
				in.readByte();
				in.readByte();
				in.readByte();
				in.readByte();
				in.readByte();
				in.readSint16BE();
				in.readSint16BE();
				in.readSint16BE();
				in.readSint16BE();
				in.readByte();
				in.readByte();
				in.readByte();
				in.readSint16BE();
				in.readUint16BE();
				in.readByte();
				in.readByte();
				in.readByte();
				in.readByte();
				in.readSint16BE();
				in.readSint16BE();
				in.readUint16BE();
				in.readByte();
				in.readByte();
				in.readByte();
				in.readByte();
			}
		}
	}
}

} // End of namespace Hugo
