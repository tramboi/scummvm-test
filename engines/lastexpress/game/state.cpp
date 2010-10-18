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

#include "lastexpress/game/state.h"

#include "lastexpress/game/inventory.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

State::State(LastExpressEngine *engine) : _engine(engine), _timer(0) {
	_inventory = new Inventory(engine);
	_objects = new Objects(engine);
	_savepoints = new SavePoints(engine);
	_state = new GameState();
	_flags = new Flags();
}

State::~State() {
	delete _inventory;
	delete _objects;
	delete _savepoints;
	delete _state;
	delete _flags;

	// Zero passed pointers
	_engine = NULL;
}

bool State::isNightTime() const {
	return (_state->progress.chapter == kChapter1
		 || _state->progress.chapter == kChapter4
		 || (_state->progress.chapter == kChapter5 && !_state->progress.isNightTime));
}

uint32 State::getPowerOfTwo(uint32 x) {
	if (!x || (x & 1))
		return 0;

	uint32 num = 0;
	do {
		x >>= 1;
		num++;
	} while ((x & 1) == 0);

	return num;
}

} // End of namespace LastExpress
