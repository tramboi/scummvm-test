/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "scumm.h"
#include "actor.h"
#include "akos.h"
#include "boxes.h"
#include "charset.h"
#include "costume.h"
#include "resource.h"
#include "saveload.h"
#include "sound.h"
#include "usage_bits.h"

#include <math.h>

byte Actor::kInvalidBox = 0;
Scumm *Actor::_vm = 0;

void Actor::initActorClass(Scumm *scumm) {
	_vm = scumm;
	if (_vm->_features & GF_SMALL_HEADER) {
		kInvalidBox = 255;
	}
}

Actor::Actor() {
	assert(_vm != 0);
	top = bottom = 0;
	number = 0;
	needRedraw = needBgReset = costumeNeedsInit = visible = false;
	flip = false;
	speedx = 8;
	speedy = 2;
	frame = 0;
	walkbox = 0;
	animProgress = 0;
	memset(animVariable, 0, sizeof(animVariable));
	memset(palette, 0, sizeof(palette));
	memset(sound, 0, sizeof(sound));
	memset(&cost, 0, sizeof(CostumeData));
	memset(&walkdata, 0, sizeof(ActorWalkData));
	walkdata.point3x = 32000;

	walkScript = 0;

	initActor(1);
}

void Actor::initActor(int mode) {
	if (mode == 1) {
		costume = 0;
		room = 0;
		x = 0;
		y = 0;
		facing = 180;
		newDirection = 180;
		talkFrequency = 256;
	} else if (mode == 2) {
		facing = 180;
		newDirection = 180;
	}

	elevation = 0;
	width = 24;
	talkColor = 15;
	talkPosX = 0;
	talkPosY = -80;
	scaley = scalex = 0xFF;
	charset = 0;
	memset(sound, 0, sizeof(sound));
	newDirection = 0;

	stopActorMoving();

	shadow_mode = 0;
	layer = 0;

	setActorWalkSpeed(8, 2);
	animSpeed = 0;

	ignoreBoxes = false;
	forceClip = 0;
	ignoreTurns = false;
	
	if (_vm->_version <= 2) {
		initFrame = 2;
		walkFrame = 0;
		standFrame = 1;
		talkStartFrame = 5;
		talkStopFrame = 4;
	} else {
		initFrame = 1;
		walkFrame = 2;
		standFrame = 3;
		talkStartFrame = 4;
		talkStopFrame = 5;
	}

	walkScript = 0;
	talkScript = 0;

	_vm->_classData[number] = (_vm->_version >= 7) ? _vm->_classData[0] : 0;
}

void Actor::stopActorMoving() {
	_vm->stopScript(walkScript);
	moving = 0;
}

void Actor::setActorWalkSpeed(uint newSpeedX, uint newSpeedY) {
	if (newSpeedX == speedx && newSpeedY == speedy)
		return;

	speedx = newSpeedX;
	speedy = newSpeedY;

	if (moving) {
		calcMovementFactor(walkdata.newx, walkdata.newy);
	}
}

int Scumm::getAngleFromPos(int x, int y) const {
	if (_gameId == GID_DIG || _gameId == GID_CMI) {
		double temp = atan2((double)x, (double)-y);
		return normalizeAngle((int)(temp * 180 / 3.1415926535));
	} else {
		if (abs(y) * 2 < abs(x)) {
			if (x > 0)
				return 90;
			return 270;
		} else {
			if (y > 0)
				return 180;
			return 0;
		}
	}
}

int Actor::calcMovementFactor(int newX, int newY) {
	int actorX, actorY;
	int diffX, diffY;
	int32 deltaXFactor, deltaYFactor;

	actorX = x;
	actorY = y;

	if (actorX == newX && actorY == newY)
		return 0;

	diffX = newX - actorX;
	diffY = newY - actorY;
	deltaYFactor = speedy << 16;

	if (diffY < 0)
		deltaYFactor = -deltaYFactor;

	deltaXFactor = deltaYFactor * diffX;
	if (diffY != 0) {
		deltaXFactor /= diffY;
	} else {
		deltaYFactor = 0;
	}

	if ((uint) abs((int)(deltaXFactor >> 16)) > speedx) {
		deltaXFactor = speedx << 16;
		if (diffX < 0)
			deltaXFactor = -deltaXFactor;

		deltaYFactor = deltaXFactor * diffY;
		if (diffX != 0) {
			deltaYFactor /= diffX;
		} else {
			deltaXFactor = 0;
		}
	}

	walkdata.x = actorX;
	walkdata.y = actorY;
	walkdata.newx = newX;
	walkdata.newy = newY;
	walkdata.deltaXFactor = deltaXFactor;
	walkdata.deltaYFactor = deltaYFactor;
	walkdata.xfrac = 0;
	walkdata.yfrac = 0;

	newDirection = _vm->getAngleFromPos(deltaXFactor, deltaYFactor);

	return actorWalkStep();
}

int Actor::remapDirection(int dir, bool is_walking) {
	int specdir;
	byte flags;
	bool flipX;
	bool flipY;

	// FIXME - It seems that at least in The Dig the original code does
	// check ignoreBoxes here. However, it breaks some animations in Loom,
	// causing Bobbin to face towards the camera instead of away from it
	// in some places: After the tree has been destroyed by lightning, and
	// when entering the dark tunnels beyond the dragon's lair at the very
	// least. Possibly other places as well.
	//
	// The Dig also checks if the actor is in the current room, but that's
	// not necessary here because we never call the function unless the
	// actor is in the current room anyway.
	
	if (!ignoreBoxes || _vm->_gameId == GID_LOOM || _vm->_gameId == GID_LOOM256) {
		specdir = _vm->_extraBoxFlags[walkbox];
		if (specdir) {
			if (specdir & 0x8000) {
				dir = specdir & 0x3FFF;
			} else {
				specdir = specdir & 0x3FFF;
				if (specdir - 90 < dir && dir < specdir + 90)
					dir = specdir;
				else
					dir = specdir + 180;
			}
		}

		flags = _vm->getBoxFlags(walkbox);

		flipX = (walkdata.deltaXFactor > 0);
		flipY = (walkdata.deltaYFactor > 0);

		// FIXME - this special cases for the class might be necesary
		// for other games besides Loom!

		// Check for X-Flip
		if ((flags & kBoxXFlip) || isInClass(kObjectClassXFlip)) {
			dir = 360 - dir;
			flipX = !flipX;
		}
		// Check for Y-Flip
		if ((flags & kBoxYFlip) || isInClass(kObjectClassYFlip)) {
			dir = 180 - dir;
			flipY = !flipY;
		}

		switch (flags & 7) {
		case 1:
			if (_vm->_version >= 7) {
				if (dir < 180)
					return 90;
				else
					return 270;
			} else {
				if (is_walking)	                       // Actor is walking
					return flipX ? 90 : 270;
				else	                               // Actor is standing/turning
					return (dir == 90) ? 90 : 270;
			}
		case 2:
			if (_vm->_version >= 7) {
				if (dir > 90 && dir < 270)
					return 180;
				else
					return 0;
			} else {
				if (is_walking)	                       // Actor is walking
					return flipY ? 180 : 0;
				else	                               // Actor is standing/turning
					return (dir == 0) ? 0 : 180;
			}
		case 3:
			return 270;
		case 4:
			return 90;
		case 5:
			return 0;
		case 6:
			return 180;
		}
	}
	// OR 1024 in to signal direction interpolation should be done
	return normalizeAngle(dir) | 1024;
}

int Actor::updateActorDirection(bool is_walking) {
	int from;
	int dirType;
	int dir;
	bool shouldInterpolate;

	dirType = (_vm->_features & GF_NEW_COSTUMES) ? _vm->akos_hasManyDirections(this) : false;

	from = toSimpleDir(dirType, facing);
	dir = remapDirection(newDirection, is_walking);

	if (_vm->_features & GF_NEW_COSTUMES)
		// Direction interpolation interfers with walk scripts in Dig; they perform
		// (much better) interpolation themselves.
		shouldInterpolate = false;	
	else
		shouldInterpolate = (dir & 1024) ? true : false;
	dir &= 1023;

	if (shouldInterpolate) {
		int to = toSimpleDir(dirType, dir);
		int num = dirType ? 8 : 4;

		// Turn left or right, depending on which is shorter.
		int diff = to - from;
		if (abs(diff) > (num >> 1))
			diff = -diff;

		if (diff > 0) {
			to = from + 1;
		} else if (diff < 0){
			to = from - 1;
		}

		dir = fromSimpleDir(dirType, (to + num) % num);
	}

	return dir;
}

void Actor::setBox(int box) {
	walkbox = box;
	setupActorScale();
}

int Actor::actorWalkStep() {
	int tmpX, tmpY;
	int actorX, actorY;
	int distX, distY;
	int direction;

	needRedraw = true;

	direction = updateActorDirection(true);
	if (!(moving & MF_IN_LEG) || facing != direction) {
		if (walkFrame != frame || facing != direction) {
			startWalkAnim(1, direction);
		}
		moving |= MF_IN_LEG;
	}

	actorX = x;
	actorY = y;

	if (walkbox != walkdata.curbox && _vm->checkXYInBoxBounds(walkdata.curbox, actorX, actorY)) {
		setBox(walkdata.curbox);
	}

	distX = abs(walkdata.newx - walkdata.x);
	distY = abs(walkdata.newy - walkdata.y);

	if (abs(actorX - walkdata.x) >= distX && abs(actorY - walkdata.y) >= distY) {
		moving &= ~MF_IN_LEG;
		return 0;
	}

	tmpX = (actorX << 16) + walkdata.xfrac + (walkdata.deltaXFactor >> 8) * scalex;
	walkdata.xfrac = (uint16)tmpX;
	actorX = (tmpX >> 16);

	tmpY = (actorY << 16) + walkdata.yfrac + (walkdata.deltaYFactor >> 8) * scaley;
	walkdata.yfrac = (uint16)tmpY;
	actorY = (tmpY >> 16);

	if (abs(actorX - walkdata.x) > distX) {
		actorX = walkdata.newx;
	}

	if (abs(actorY - walkdata.y) > distY) {
		actorY = walkdata.newy;
	}

	x = actorX;
	y = actorY;
	return 1;
}


void Actor::setupActorScale() {
	uint16 scale;

	if (_vm->_features & GF_NO_SCALING) {
		scalex = 0xFF;
		scaley = 0xFF;
		return;
	}

	if (ignoreBoxes)
		return;

	if (_vm->getBoxFlags(walkbox) & kBoxPlayerOnly)
		return;

	scale = _vm->getScale(walkbox, x, y);
	if (_vm->_version == 8) {
		// At least in COMI, scale values are clipped to range 1-255
		if (scale < 1)
			scale = 1;
		else if (scale > 255)
			scale = 255;
	}

	// FIXME - Hack for The Dig 'Tomb' (room 88)
	//	Otherwise walking to the far-left door causes the actor
	//	to shrink to a one-pixel dot. (!?!?)
	if (_vm->_gameId == GID_DIG && _vm->_currentRoom == 88) {
		scale = 0xFF;
	}


	// FIXME - Quick fix to ft's fuel tower bug (by yazoo)
	//
	// Ben's Y position can be anything between 272 and 398 inclusive
	// (which by the way means that we're always looking at the same
	// element in the scale table... hmmm...)
	//
	// When standing at the bottom of the ladder, Ben's Y position is
	// 356, and by the looks of it he ought to be unscaled there.

	if (_vm->_gameId == GID_FT && scale == 1 && _vm->_currentRoom == 76) {
		scale = 0xff;
		if (y < 356)
			scale -= 2 * (356 - y);
	}

	if (scale > 255) {
			warning("Actor %d at %d, scale %d out of range", number, y, scale);
	}
	scalex = (byte)scale;
	scaley = (byte)scale;
}

void Actor::startAnimActor(int f) {
	if (_vm->_features & GF_NEW_COSTUMES) {
		switch (f) {
		case 1001:
			f = initFrame;
			break;
		case 1002:
			f = walkFrame;
			break;
		case 1003:
			f = standFrame;
			break;
		case 1004:
			f = talkStartFrame;
			break;
		case 1005:
			f = talkStopFrame;
			break;
		}

		frame = f;

		if (costume != 0) {
			animProgress = 0;
			needRedraw = true;
			if (f == initFrame)
				cost.reset();
			_vm->akos_decodeData(this, f, (uint) - 1);
		}

	} else {
		switch (f) {
		case 0x38:
			f = initFrame;
			break;
		case 0x39:
			f = walkFrame;
			break;
		case 0x3A:
			f = standFrame;
			break;
		case 0x3B:
			f = talkStartFrame;
			break;
		case 0x3C:
			f = talkStopFrame;
			break;
		}
		
		assert(f != 0x3E);
		frame = f;

		if (isInCurrentRoom() && costume != 0) {
			animProgress = 0;
			cost.animCounter = 0;
			needRedraw = true;
			if (f == initFrame)
				cost.reset();
			_vm->cost_decodeData(this, f, (uint) - 1);
		}
	}
}

void Actor::animateActor(int anim) {
	int cmd, dir;

	if (_vm->_features & GF_NEW_COSTUMES) {

		if (anim == 0xFF)
			anim = 2000;

		cmd = anim / 1000;
		dir = anim % 1000;

	} else {

		cmd = anim / 4;
		dir = oldDirToNewDir(anim % 4);

		// Convert into old cmd code
		cmd = 0x3F - cmd + 2;

	}

	switch (cmd) {
	case 2:				// stop walking
		stopActorMoving();
		startAnimActor(standFrame);
		break;
	case 3:				// change direction immediatly
		moving &= ~MF_TURN;
		setDirection(dir);
		break;
	case 4:				// turn to new direction
		turnToDirection(dir);
		break;
	default:
		if (_vm->_version <= 2)
			startAnimActor(anim / 4);
		else
			startAnimActor(anim);
	}
}

void Actor::setDirection(int direction) {
	uint aMask;
	int i;
	uint16 vald;

	if (facing == direction)
		return;

	facing = normalizeAngle(direction);

	if (costume == 0)
		return;

	aMask = 0x8000;
	for (i = 0; i < 16; i++, aMask >>= 1) {
		vald = cost.frame[i];
		if (vald == 0xFFFF)
			continue;
		if (_vm->_features & GF_NEW_COSTUMES)
			_vm->akos_decodeData(this, vald, aMask);
		else
			_vm->cost_decodeData(this, vald, aMask);
	}

	needRedraw = true;
}

void Actor::putActor(int dstX, int dstY, byte newRoom) {
	if (visible && _vm->_currentRoom != newRoom && _vm->VAR(_vm->VAR_TALK_ACTOR) == number) {
		_vm->clearMsgQueue();
	}

	// HACK: The green transparency of the tank in the Hall of Oddities is
	// is positioned one pixel too far to the left. This appears to be a
	// bug in the original game as well.

	if (_vm->_gameId == GID_SAMNMAX && newRoom == 16 && number == 5 && dstX == 235 && dstY == 236)
		dstX++;

	x = dstX;
	y = dstY;
	room = newRoom;
	needRedraw = true;

	if (_vm->VAR(_vm->VAR_EGO) == number) {
		_vm->_egoPositioned = true;
	}

	if (visible) {
		if (isInCurrentRoom()) {
			if (moving) {
				startAnimActor(standFrame);
				moving = 0;
			}
			adjustActorPos();
		} else {
			hideActor();
		}
	} else {
		if (isInCurrentRoom())
			showActor();
	}
}

int Actor::getActorXYPos(int &xPos, int &yPos) const {
	if (!isInCurrentRoom())
		return -1;

	xPos = x;
	yPos = y;
	return 0;
}

AdjustBoxResult Actor::adjustXYToBeInBox(int dstX, int dstY) {
	const uint thresholdTable[] = { 30, 80, 0 };
	AdjustBoxResult abr;
	int16 tmpX, tmpY;
	int tmpDist, bestDist, threshold, numBoxes;
	byte flags, bestBox;
	int box;
	const int firstValidBox = (_vm->_features & GF_SMALL_HEADER) ? 0 : 1;

	abr.x = dstX;
	abr.y = dstY;
	abr.box = kInvalidBox;

	if (ignoreBoxes)
		return abr;

	for (int tIdx = 0; tIdx < ARRAYSIZE(thresholdTable); tIdx++) {
		threshold = thresholdTable[tIdx];

		numBoxes = _vm->getNumBoxes() - 1;
		if (numBoxes < firstValidBox)
			return abr;

		bestDist = 0xFFFF;
		if (_vm->_version <= 2)
			bestDist *= 8*2;	// Adjust for the fact that we multiply x by 8 and y by 2
		bestBox = kInvalidBox;

		// We iterate (backwards) over all boxes, searching the one closest
		// to the desired coordinates.
		for (box = numBoxes; box >= firstValidBox; box--) {
			flags = _vm->getBoxFlags(box);

			// Skip over invisible boxes
			if (flags & kBoxInvisible && !(flags & kBoxPlayerOnly && !isPlayer()))
				continue;
			
			// For increased performance, we perform a quick test if
			// the coordinates can even be within a distance of 'threshold'
			// pixels of the box.
			if (threshold > 0 && _vm->inBoxQuickReject(box, dstX, dstY, threshold))
				continue;

			// Check if the point is contained in the box. If it is,
			// we don't have to search anymore.
			if (_vm->checkXYInBoxBounds(box, dstX, dstY)) {
				abr.x = dstX;
				abr.y = dstY;
				abr.box = box;
				return abr;
			}

			// Find the point in the box which is closest to our point.
			tmpDist = _vm->getClosestPtOnBox(box, dstX, dstY, tmpX, tmpY);

			// Check if the box is closer than the previous boxes.
			if (tmpDist < bestDist) {
				abr.x = tmpX;
				abr.y = tmpY;
	
				if (tmpDist == 0) {
					abr.box = box;
					return abr;
				}
				bestDist = tmpDist;
				bestBox = box;
			}
		}

		// If the closest ('best') box we found is within the threshold, or if
		// we are on the last run (i.e. threshold == 0), return that box.
		if (threshold == 0 || threshold * threshold >= bestDist) {
			abr.box = bestBox;
			return abr;
		}
	}

	return abr;
}

void Actor::adjustActorPos() {
	AdjustBoxResult abr;

	abr = adjustXYToBeInBox(x, y);

	x = abr.x;
	y = abr.y;
	walkdata.destbox = abr.box;

	setBox(abr.box);

	walkdata.destx = -1;

	moving = 0;
	cost.soundCounter = 0;

	if (_vm->_features & GF_NEW_COSTUMES) {
		stopActorMoving();
	}

	if (walkbox != kInvalidBox) {
		byte flags = _vm->getBoxFlags(walkbox);
		if (flags & 7) {
			turnToDirection(facing);
		}
	}
}

void Actor::faceToObject(int obj) {
	int x2, y2, dir;
	
	if (!isInCurrentRoom())
		return;

	if (_vm->getObjectOrActorXY(obj, x2, y2) == -1)
		return;

	dir = (x2 > x) ? 90 : 270;
	turnToDirection(dir);
}

void Actor::turnToDirection(int newdir) {
	if (newdir == -1)
		return;

	moving &= ~MF_TURN;

	if (newdir != facing) {
		if (_vm->_version <= 3)
			moving = MF_TURN;
		else
			moving |= MF_TURN;
		newDirection = newdir;
	}
}

void Actor::hideActor() {
	if (!visible)
		return;

	if (moving) {
		startAnimActor(standFrame);
		moving = 0;
	}
	visible = false;
	cost.soundCounter = 0;
	needRedraw = false;
	needBgReset = true;
}

void Actor::showActor() {
	if (_vm->_currentRoom == 0 || visible)
		return;

	adjustActorPos();

	_vm->ensureResourceLoaded(rtCostume, costume);

	if (costumeNeedsInit) {
		startAnimActor(initFrame);
		if (_vm->_version <= 2) {
			startAnimActor(standFrame);
			startAnimActor(talkStopFrame);
		}
		costumeNeedsInit = false;
	}
	moving = 0;
	visible = true;
	needRedraw = true;
}

void Scumm::showActors() {
	int i;

	for (i = 1; i < _numActors; i++) {
		if (_actors[i].isInCurrentRoom())
			_actors[i].showActor();
	}
}

void Scumm::walkActors() {
	int i;

	for (i = 1; i < _numActors; i++) {
		if (_actors[i].isInCurrentRoom())
			if (_version <= 3)
				_actors[i].walkActorOld();
			else
				_actors[i].walkActor();
	}
}

/* Used in Scumm v5 only. Play sounds associated with actors */
void Scumm::playActorSounds() {
	int i;

	for (i = 1; i < _numActors; i++) {
		if (_actors[i].cost.soundCounter && _actors[i].isInCurrentRoom() && _actors[i].sound) {
			_currentScript = 0xFF;
			_sound->addSoundToQueue(_actors[i].sound[0]);
			for (i = 1; i < _numActors; i++) {
				_actors[i].cost.soundCounter = 0;
			}
			return;
		}
	}
}

Actor *Scumm::derefActor(int id, const char *errmsg) const {
	if (id < 1 || id >= _numActors || _actors[id].number != id) {
		if (errmsg)
			error("Invalid actor %d in %s", id, errmsg);
		else
			error("Invalid actor %d", id);
	}
	return &_actors[id];
}

Actor *Scumm::derefActorSafe(int id, const char *errmsg) const {
	if (id < 1 || id >= _numActors || _actors[id].number != id) {
		debug(2, "Invalid actor %d in %s (script %d, opcode 0x%x) - This is potentially a BIG problem.",
			 id, errmsg, vm.slot[_curExecScript].number, _opcode);
		return NULL;
	}
	return &_actors[id];
}

static int compareDrawOrder(const void* a, const void* b)
{
	const Actor* actor1 = *(const Actor *const*)a;
	const Actor* actor2 = *(const Actor *const*)b;
	int diff;

	// The actor in the higher layer is ordered lower
	diff = actor1->layer - actor2->layer;
	if (diff < 0)
		return +1;
	if (diff > 0)
		return -1;

	// The actor with higher y value is ordered higher
	diff = actor1->y - actor2->y;
	if (diff < 0)
		return -1;
	if (diff > 0)
		return +1;

	// The qsort() function is not guaranteed to be stable (i.e. it may
	// re-order "equal" elements in an array it sorts). Hence we use the
	// actor number as tie-breaker. This is needed for the Sam & Max intro,
	// and possibly other cases as well. See bug #758167.

	return actor1->number - actor2->number;
}

void Scumm::processActors() {
	int numactors = 0;

	// TODO : put this actors as a member array. It never has to grow or shrink
	// since _numActors is constant within a game.
	Actor** actors = new Actor * [_numActors];
	
	// Make a list of all actors in this room
	for (int i = 1; i < _numActors; i++) {
		if (_version == 8 && _actors[i].layer < 0)
			continue;
		if (_actors[i].isInCurrentRoom())
			actors[numactors++] = &_actors[i];
	}
	if (!numactors) {
		delete [] actors;
		return;
	}

	// Sort actors by position before we draw them (to ensure that actors in
	// front are drawn after those "behind" them).
	qsort(actors, numactors, sizeof (Actor*), compareDrawOrder);

	Actor** end = actors + numactors;

	// Finally draw the now sorted actors
	for (Actor** ac = actors; ac != end; ++ac) {
		Actor* a = *ac;
		if (a->costume) {
			CHECK_HEAP
			a->drawActorCostume();
			CHECK_HEAP
			a->animateCostume();
		}
	}
	
	delete [] actors;
}

// Used in Scumm v8, to allow the verb coin to be drawn over the inventory
// chest. I'm assuming that draw order won't matter here.
void Scumm::processUpperActors() {
	int i;

	for (i = 1; i < _numActors; i++) {
		if (_actors[i].isInCurrentRoom() && _actors[i].costume && _actors[i].layer < 0) {
			CHECK_HEAP
			_actors[i].drawActorCostume();
			CHECK_HEAP
			_actors[i].animateCostume();
		}
	}
}

void Actor::drawActorCostume() {
	if (!needRedraw)
		return;

	needRedraw = false;

	setupActorScale();

	BaseCostumeRenderer* bcr = _vm->_costumeRenderer;

	bcr->updateNbStrips();

	bcr->_actorX = x - _vm->virtscr[0].xstart;
	bcr->_actorY = y - elevation;

	if (_vm->_version <= 2) {
		// FIXME: We have to adjust the x position by one strip (8 pixels) in V2 games
		// However, it is not quite clear to me why. And to fully match the original,
		// it seems we have to offset by 2 strips when facing left (270 degree).
		if (facing == 270)
			bcr->_actorX += 16;
		else
			bcr->_actorX += 8;
	}

	bcr->_scaleX = scalex;
	bcr->_scaleY = scaley;

	bcr->_shadow_mode = shadow_mode;
	if (_vm->_features & GF_SMALL_HEADER)
		bcr->_shadow_table = NULL;
	else
		bcr->_shadow_table = _vm->_shadowPalette;

	bcr->setCostume(costume);
	bcr->setPalette(palette);
	bcr->setFacing(this);

	bcr->_dirty_id = number;

	if (!(_vm->_features & GF_NEW_COSTUMES)) {

		if (forceClip)
			bcr->_zbuf = forceClip;
		else if (isInClass(kObjectClassNeverClip))
			bcr->_zbuf = 0;
		else {
			bcr->_zbuf = _vm->getMaskFromBox(walkbox);
			if (bcr->_zbuf > _vm->gdi._numZBuffer)
				bcr->_zbuf = _vm->gdi._numZBuffer;
		}

		bcr->_draw_top = top = 0xFF;
	} else {

		bcr->_zbuf = forceClip;
		if (bcr->_zbuf == 100) {
			bcr->_zbuf = _vm->getMaskFromBox(walkbox);
			if (bcr->_zbuf > _vm->gdi._numZBuffer)
				bcr->_zbuf = _vm->gdi._numZBuffer;
		}

		bcr->_draw_top = top = 0x7fffffff;
	}

	bcr->_draw_bottom = bottom = 0;

	bcr->_outptr = _vm->virtscr[0].screenPtr + _vm->virtscr[0].xstart;
	bcr->_outwidth = _vm->virtscr[0].width;
	bcr->_outheight = _vm->virtscr[0].height;

	// If the actor is partially hidden, redraw it next frame.
	// Only done for pre-AKOS, though.
	if (bcr->drawCostume(cost) & 1) {
		needRedraw = !(_vm->_features & GF_NEW_COSTUMES);
	}

	// Record the vertical extent of the drawn actor
	top = bcr->_draw_top;
	bottom = bcr->_draw_bottom;
}

void Actor::animateCostume() {
	if (costume == 0)
		return;

	animProgress++;
	if (animProgress >= animSpeed) {
		animProgress = 0;

		if (_vm->_features & GF_NEW_COSTUMES) {
			byte *akos = _vm->getResourceAddress(rtCostume, costume);
			assert(akos);
			if (_vm->akos_increaseAnims(akos, this)) {
				needRedraw = true;
			}
		} else {
			LoadedCostume lc(_vm);
			lc.loadCostume(costume);
			if (lc.increaseAnims(this)) {
				needRedraw = true;
			}
		}
	}
}

void Actor::animateLimb(int limb, int f) {
	// This methods is very similiar to animateCostume(). 
	// However, instead of animating *all* the limbs, it only animates
	// the specified limb to be at the frame specified by "f". 

	if (!f)
		return;

	animProgress++;
	if (animProgress >= animSpeed) {
		animProgress = 0;

		if (costume == 0)
			return;

		const byte *aksq, *akfo;
		uint size;
		byte *akos = _vm->getResourceAddress(rtCostume, costume);
		assert(akos);

		aksq = _vm->findResourceData(MKID('AKSQ'), akos);
		akfo = _vm->findResourceData(MKID('AKFO'), akos);
	
		size = _vm->getResourceDataSize(akfo) >> 1;
	
		while (f--) {
			if (cost.active[limb] != 0)
				_vm->akos_increaseAnim(this, limb, aksq, (const uint16 *)akfo, size);
		}

//		needRedraw = true;
//		needBgReset = true;
	}
}

void Scumm::setActorRedrawFlags() {
	int i, j;

	if (_fullRedraw) {
		for (j = 1; j < _numActors; j++) {
			_actors[j].needRedraw = true;
		}
	} else {
		for (i = 0; i < gdi._numStrips; i++) {
			int strip = _screenStartStrip + i;
			if (testGfxAnyUsageBits(strip)) {
				for (j = 1; j < _numActors; j++) {
					if (testGfxUsageBit(strip, j) && testGfxOtherUsageBits(strip, j)) {
						_actors[j].needRedraw = true;
					}
				}
			}
		}
	}
}

int Scumm::getActorFromPos(int x, int y) {
	int i;

	if (!testGfxAnyUsageBits(x >> 3))
		return 0;
	for (i = 1; i < _numActors; i++) {
		if (testGfxUsageBit(x >> 3, i) && !getClass(i, kObjectClassUntouchable)
			&& y >= _actors[i].top && y <= _actors[i].bottom) {
			if (_version > 2 || i != VAR(VAR_EGO))
				return i;
		}
	}
	return 0;
}

void Scumm::actorTalk() {
	Actor *a;

	_msgPtrToAdd = _charsetBuffer;
	_messagePtr = addMessageToStack(_messagePtr);
	assert((int)(_msgPtrToAdd - _charsetBuffer) < (int)(sizeof(_charsetBuffer)));

	if (_actorToPrintStrFor == 0xFF) {
		if (!_keepText)
			stopTalk();
		VAR(VAR_TALK_ACTOR) = 0xFF;
	} else {
		int oldact;
		
		// FIXME: Workaround for bug #770724
		if (_gameId == GID_LOOM && _roomResource == 23 &&
			vm.slot[_currentScript].number == 232 && _actorToPrintStrFor == 0) {
			_actorToPrintStrFor = 2;	// Could be anything from 2 to 5. Maybe compare to original?
		}
		
		a = derefActor(_actorToPrintStrFor, "actorTalk");
		if (!a->isInCurrentRoom() && !(_features & GF_NEW_COSTUMES)) {
			oldact = 0xFF;
		} else {
			if (!_keepText)
				stopTalk();
			VAR(VAR_TALK_ACTOR) = a->number;
			if (!_string[0].no_talk_anim) {
				a->startAnimActor(a->talkStartFrame);
				_useTalkAnims = true;
			}
			oldact = VAR(VAR_TALK_ACTOR);
		}
		if (oldact >= 0x80)
			return;
	}

	if (VAR(VAR_TALK_ACTOR) > 0x7F) {
		_charsetColor = (byte)_string[0].color;
	} else {
		a = derefActor(VAR(VAR_TALK_ACTOR), "actorTalk(2)");
		_charsetColor = a->talkColor;
	}
	_charsetBufPos = 0;
	_talkDelay = 0;
	_haveMsg = 0xFF;
	VAR(VAR_HAVE_MSG) = 0xFF;
	if (VAR_CHARCOUNT != 0xFF)
		VAR(VAR_CHARCOUNT) = 0;
	CHARSET_1();
}

void Scumm::stopTalk() {
	int act;

	_sound->stopTalkSound();

	_haveMsg = 0;
	_talkDelay = 0;

	act = VAR(VAR_TALK_ACTOR);
	if (act && act < 0x80) {
		Actor *a = derefActor(act, "stopTalk");
		if ((a->isInCurrentRoom() && _useTalkAnims) || (_features & GF_NEW_COSTUMES)) {
			a->startAnimActor(a->talkStopFrame);
			_useTalkAnims = false;
		}
		VAR(VAR_TALK_ACTOR) = 0xFF;
	}
	_keepText = false;
	restoreCharsetBg();
}

void Scumm::clearMsgQueue() {
	_messagePtr = (const byte *)" ";
	stopTalk();
}

void Actor::setActorCostume(int c) {
	int i;

	costumeNeedsInit = true;
	
	if (_vm->_version == 8) {
		cost.reset();
		memset(animVariable, 0, sizeof(animVariable));
		costume = c;
		
		if (visible) {
			if (costume) {
				_vm->ensureResourceLoaded(rtCostume, costume);
			}
			startAnimActor(initFrame);
		}
	} else {
		if (visible) {
			hideActor();
			cost.reset();
			costume = c;
			showActor();
		} else {
			costume = c;
			cost.reset();
		}
	}


	if (_vm->_features & GF_NEW_COSTUMES) {
		for (i = 0; i < 256; i++)
			palette[i] = 0xFF;
	} else if (_vm->_features & GF_OLD_BUNDLE) {
		for (i = 0; i < 16; i++)
			palette[i] = i;
	} else {
		for (i = 0; i < 32; i++)
			palette[i] = 0xFF;
	}
}

void Actor::startWalkActor(int destX, int destY, int dir) {
	AdjustBoxResult abr;

	if (_vm->_version <= 3) {
		abr.x = destX;
		abr.y = destY;
	} else {
		abr = adjustXYToBeInBox(destX, destY);
	}

	if (!isInCurrentRoom()) {
		x = abr.x;
		y = abr.y;
		if (dir != -1)
			setDirection(dir);
		return;
	}

	if (ignoreBoxes) {
		abr.box = kInvalidBox;
		walkbox = kInvalidBox;
	} else {
		if (_vm->checkXYInBoxBounds(walkdata.destbox, abr.x, abr.y)) {
			abr.box = walkdata.destbox;
		} else {
			abr = adjustXYToBeInBox(abr.x, abr.y);
		}
		if (moving && walkdata.destdir == dir && walkdata.destx == abr.x && walkdata.desty == abr.y)
			return;
	}

	if (x == abr.x && y == abr.y) {
		turnToDirection(dir);
		return;
	}

	walkdata.destx = abr.x;
	walkdata.desty = abr.y;
	walkdata.destbox = abr.box;
	walkdata.destdir = dir;
	moving = (moving & MF_IN_LEG) | MF_NEW_LEG;
	walkdata.point3x = 32000;

	walkdata.curbox = walkbox;
}

void Actor::startWalkAnim(int cmd, int angle) {
	if (angle == -1)
		angle = facing;

	/* FIXME: (yazoo/fingolfin): using the walk script is buggy in Dig,
	 * troubles while walking. It's disabled until we can figure out how
	 * to fix this properly.
	 * Note: walk scripts aren't required to make the game
	 * work as usual
	 */
#if 1
	if (walkScript != 0) {
		int args[16];
		args[0] = number;
		args[1] = cmd;
		args[2] = angle;
		_vm->runScript(walkScript, 1, 0, args);
	} else
#endif
	{
		switch (cmd) {
		case 1:										/* start walk */
			setDirection(angle);
			startAnimActor(walkFrame);
			break;
		case 2:										/* change dir only */
			setDirection(angle);
			break;
		case 3:										/* stop walk */
			turnToDirection(angle);
			startAnimActor(standFrame);
			break;
		}
	}
}

void Actor::walkActor() {
	int new_dir, box;
	int16 foundPathX, foundPathY;

	if (_vm->_version >= 7) {
		// FIXME - this is kind of a hack right now but it fixes the
		// walk scripts in The Dig.
		if (moving & MF_FROZEN) {
			if (moving & MF_TURN) {
				new_dir = updateActorDirection(false);
				if (facing != new_dir)
					setDirection(new_dir);
				else
					moving &= ~MF_TURN;
			}
			return;
		}
	}

	if (!moving)
		return;

	if (!(moving & MF_NEW_LEG)) {
		if (moving & MF_IN_LEG && actorWalkStep())
			return;

		if (moving & MF_LAST_LEG) {
			moving = 0;
			setBox(walkdata.destbox);
			startWalkAnim(3, walkdata.destdir);
			return;
		}

		if (moving & MF_TURN) {
			new_dir = updateActorDirection(false);
			if (facing != new_dir)
				setDirection(new_dir);
			else
				moving = 0;
			return;
		}

		setBox(walkdata.curbox);
		moving &= MF_IN_LEG;
	}

	do {
		moving &= ~MF_NEW_LEG;

		if (walkbox == kInvalidBox) {
			setBox(walkdata.destbox);
			walkdata.curbox = walkdata.destbox;
			break;
		}

		if (walkbox == walkdata.destbox)
			break;

		box = _vm->getPathToDestBox(walkbox, walkdata.destbox);
		if (box < 0) {
			walkdata.destbox = walkbox;
			moving |= MF_LAST_LEG;
			return;
		}

		walkdata.curbox = box;
		
		if (findPathTowards(walkbox, box, walkdata.destbox, foundPathX, foundPathY))
			break;

		if (calcMovementFactor(foundPathX, foundPathY))
			return;

		setBox(walkdata.curbox);
	} while (1);

	moving |= MF_LAST_LEG;
	calcMovementFactor(walkdata.destx, walkdata.desty);
}

void Actor::walkActorOld() {
	ScummVM::Point gateLoc[5];	// Gate locations
	int new_dir, next_box;

	if (!moving)
		return;

	if (!(moving & MF_NEW_LEG)) {
		if (moving & MF_IN_LEG && actorWalkStep())
			return;
	
		if (moving & MF_LAST_LEG) {
			moving = 0;
			startWalkAnim(3, walkdata.destdir);
			return;
		}
	
		if (moving & MF_TURN) {
			new_dir = updateActorDirection(false);
			if (facing != new_dir)
				setDirection(new_dir);
			else
				moving = 0;
			return;
		}
	
		if (walkdata.point3x != 32000) {
			if (calcMovementFactor(walkdata.point3x, walkdata.point3y)) {
				walkdata.point3x = 32000;
				return;
			}
			walkdata.point3x = 32000;
		}
	
		walkbox = walkdata.curbox;
		moving &= MF_IN_LEG;
	}

	do {
		moving &= ~MF_NEW_LEG;

		if (walkbox == kInvalidBox) {
			walkbox = walkdata.destbox;
			walkdata.curbox = walkdata.destbox;
			break;
		}

		if (walkbox == walkdata.destbox)
			break;

		next_box = _vm->getPathToDestBox(walkbox, walkdata.destbox);
		if (next_box < 0) {
			moving |= MF_LAST_LEG;
			return;
		}

		// Can't walk through locked boxes
		int flags = _vm->getBoxFlags(next_box);
		if (flags & kBoxLocked && !(flags & kBoxPlayerOnly && !isPlayer())) {
			moving |= MF_LAST_LEG;
			return;
		}


		walkdata.curbox = next_box;

		findPathTowardsOld(walkbox, next_box, walkdata.destbox, gateLoc);
		if (gateLoc[2].x == 32000 && gateLoc[3].x == 32000) {
			break;
		}

		if (gateLoc[2].x != 32000) {
			if (calcMovementFactor(gateLoc[2].x, gateLoc[2].y)) {
				walkdata.point3x = gateLoc[3].x; 
				walkdata.point3y = gateLoc[3].y;
				return;
			}
		}

		if (calcMovementFactor(gateLoc[3].x, gateLoc[3].y))
			return;

		walkbox = walkdata.destbox;
	} while (1);

	moving |= MF_LAST_LEG;
	calcMovementFactor(walkdata.destx, walkdata.desty);
}

byte *Actor::getActorName() {
	byte *ptr = _vm->getResourceAddress(rtActorName, number);
	if (ptr == NULL) {
		warning("Failed to find name of actor %d", number);
	}
	return ptr;
}

void Actor::remapActorPalette(int r_fact, int g_fact, int b_fact, int threshold) {
	const byte *akos, *rgbs, *akpl;
	int akpl_size, i;
	int r, g, b;
	byte akpl_color;

	if (!isInCurrentRoom()) {
		warning("Remap actor %d not in current room", number);
		return;
	}

	if (costume < 1 || costume >= _vm->_numCostumes - 1) {
		warning("Remap actor %d invalid costume %d", number, costume);
		return;
	}

	akos = _vm->getResourceAddress(rtCostume, costume);
	if (!akos) {
		warning("Can't remap actor %d, costume %d not found", number, costume);
		return;
	}

	akpl = findResource(MKID('AKPL'), akos);
	if (!akpl) {
		warning("Can't remap actor %d, costume %d doesn't contain an AKPL block", number, costume);
		return;
	}

	//get num palette entries
	akpl_size = RES_SIZE(akpl) - 8;

	//skip resource header
	akpl = RES_DATA(akpl);

	rgbs = findResource(MKID('RGBS'), akos);

	if (!rgbs) {
		warning("Can't remap actor %d costume %d doesn't contain an RGB block", number, costume);
		return;
	}
	// skip resource header
	rgbs = RES_DATA(rgbs);

	for (i = 0; i < akpl_size; i++) {
		r = *rgbs++;
		g = *rgbs++;
		b = *rgbs++;

		akpl_color = *akpl++;

		// allow remap of generic palette entry?
		if (!shadow_mode || akpl_color >= 16) {
			if (r_fact != 256)
				r = (r * r_fact) >> 8;
			if (g_fact != 256)
				g = (g * g_fact) >> 8;
			if (b_fact != 256)
				b = (b * b_fact) >> 8;
			palette[i] = _vm->remapPaletteColor(r, g, b, threshold);
		}
	}
}

void Scumm::resetActorBgs() {
	int i, j;

	for (i = 0; i < gdi._numStrips; i++) {
		int strip = _screenStartStrip + i;
		for (j = 1; j < _numActors; j++) {
			if (testGfxUsageBit(strip, j) &&
				((_actors[j].top != 0xFF || _actors[j].needRedraw) || _actors[j].needBgReset)) {
				clearGfxUsageBit(strip, j);
				if ((_actors[j].bottom - _actors[j].top) >= 0)
					gdi.resetBackground(_actors[j].top, _actors[j].bottom, i);
			}
		}
	}

	for (i = 1; i < _numActors; i++) {
		_actors[i].needBgReset = false;
	}
}

void Actor::classChanged(int cls, bool value) {
	if (cls == kObjectClassAlwaysClip)
		forceClip = value;
	if (cls == kObjectClassIgnoreBoxes)
		ignoreBoxes = value;
}

bool Actor::isInClass(int cls) {
	return _vm->getClass(number, cls);
}

bool Actor::isPlayer() {
	if (_vm->_version <= 2)
		return _vm->VAR(42) <= number && number <= _vm->VAR(43);
	else
		return isInClass(kObjectClassPlayer);
}


const SaveLoadEntry *Actor::getSaveLoadEntries() {
	static const SaveLoadEntry actorEntries[] = {
		MKLINE(Actor, x, sleInt16, VER_V8),
		MKLINE(Actor, y, sleInt16, VER_V8),
		MKLINE(Actor, top, sleInt16, VER_V8),
		MKLINE(Actor, bottom, sleInt16, VER_V8),
		MKLINE(Actor, elevation, sleInt16, VER_V8),
		MKLINE(Actor, width, sleUint16, VER_V8),
		MKLINE(Actor, facing, sleUint16, VER_V8),
		MKLINE(Actor, costume, sleUint16, VER_V8),
		MKLINE(Actor, room, sleByte, VER_V8),
		MKLINE(Actor, talkColor, sleByte, VER_V8),
		MKLINE(Actor, talkFrequency, sleInt16, VER_V16),
		MKLINE(Actor, scalex, sleByte, VER_V8),
		MKLINE(Actor, scaley, sleByte, VER_V8),
		MKLINE(Actor, charset, sleByte, VER_V8),
		MKARRAY(Actor, sound[0], sleByte, 8, VER_V8),
		MKARRAY(Actor, animVariable[0], sleUint16, 8, VER_V8),
		MKLINE(Actor, newDirection, sleUint16, VER_V8),
		MKLINE(Actor, moving, sleByte, VER_V8),
		MKLINE(Actor, ignoreBoxes, sleByte, VER_V8),
		MKLINE(Actor, forceClip, sleByte, VER_V8),
		MKLINE(Actor, initFrame, sleByte, VER_V8),
		MKLINE(Actor, walkFrame, sleByte, VER_V8),
		MKLINE(Actor, standFrame, sleByte, VER_V8),
		MKLINE(Actor, talkStartFrame, sleByte, VER_V8),
		MKLINE(Actor, talkStopFrame, sleByte, VER_V8),
		MKLINE(Actor, speedx, sleUint16, VER_V8),
		MKLINE(Actor, speedy, sleUint16, VER_V8),
		MKLINE(Actor, cost.animCounter, sleUint16, VER_V8),
		MKLINE(Actor, cost.soundCounter, sleByte, VER_V8),
	
		// Actor palette grew from 64 to 256 bytes
		MKARRAY_OLD(Actor, palette[0], sleByte, 64, VER_V8, VER_V9),
		MKARRAY(Actor, palette[0], sleByte, 256, VER_V10),
	
		MK_OBSOLETE(Actor, mask, sleByte, VER_V8, VER_V9),
		MKLINE(Actor, shadow_mode, sleByte, VER_V8),
		MKLINE(Actor, visible, sleByte, VER_V8),
		MKLINE(Actor, frame, sleByte, VER_V8),
		MKLINE(Actor, animSpeed, sleByte, VER_V8),
		MKLINE(Actor, animProgress, sleByte, VER_V8),
		MKLINE(Actor, walkbox, sleByte, VER_V8),
		MKLINE(Actor, needRedraw, sleByte, VER_V8),
		MKLINE(Actor, needBgReset, sleByte, VER_V8),
		MKLINE(Actor, costumeNeedsInit, sleByte, VER_V8),
	
		MKLINE(Actor, talkPosY, sleInt16, VER_V8),
		MKLINE(Actor, talkPosX, sleInt16, VER_V8),
		MKLINE(Actor, ignoreTurns, sleByte, VER_V8),
	
		MKLINE(Actor, layer, sleByte, VER_V8),
	
		MKLINE(Actor, talkScript, sleUint16, VER_V8),
		MKLINE(Actor, walkScript, sleUint16, VER_V8),
	
		MKLINE(Actor, walkdata.destx, sleInt16, VER_V8),
		MKLINE(Actor, walkdata.desty, sleInt16, VER_V8),
		MKLINE(Actor, walkdata.destbox, sleByte, VER_V8),
		MKLINE(Actor, walkdata.destdir, sleUint16, VER_V8),
		MKLINE(Actor, walkdata.curbox, sleByte, VER_V8),
		MKLINE(Actor, walkdata.x, sleInt16, VER_V8),
		MKLINE(Actor, walkdata.y, sleInt16, VER_V8),
		MKLINE(Actor, walkdata.newx, sleInt16, VER_V8),
		MKLINE(Actor, walkdata.newy, sleInt16, VER_V8),
		MKLINE(Actor, walkdata.deltaXFactor, sleInt32, VER_V8),
		MKLINE(Actor, walkdata.deltaYFactor, sleInt32, VER_V8),
		MKLINE(Actor, walkdata.xfrac, sleUint16, VER_V8),
		MKLINE(Actor, walkdata.yfrac, sleUint16, VER_V8),
	
		MKARRAY(Actor, cost.active[0], sleByte, 16, VER_V8),
		MKLINE(Actor, cost.stopped, sleUint16, VER_V8),
		MKARRAY(Actor, cost.curpos[0], sleUint16, 16, VER_V8),
		MKARRAY(Actor, cost.start[0], sleUint16, 16, VER_V8),
		MKARRAY(Actor, cost.end[0], sleUint16, 16, VER_V8),
		MKARRAY(Actor, cost.frame[0], sleUint16, 16, VER_V8),
		MKEND()
	};
	
	return actorEntries;
}
