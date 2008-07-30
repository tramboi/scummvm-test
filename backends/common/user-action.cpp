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
*
*/

#include "backends/common/user-action.h"
#include "backends/common/keymap.h"

namespace Common {

UserAction::UserAction(String des, UserActionCategory cat, UserActionType ty, 
					   int pr, int gr, int fl) {
	description = des;
	category = cat;
	type = ty;
	priority = pr;
	group = gr;
	flags = fl;
	_hwKey = 0;
	_parent = 0;
}

void UserAction::setParent(Keymap *parent) {
	_parent = parent;
}

void UserAction::mapKey(const HardwareKey *key) {
	assert(_parent);
	if (_hwKey) _parent->unregisterMapping(this);
	_hwKey = key;
	if (_hwKey) _parent->registerMapping(this, key);
}

const HardwareKey *UserAction::getMappedKey() const {
	return _hwKey;
}

} // end of namespace Common
