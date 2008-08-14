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

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"

namespace Common {

Action::Action(Keymap *boss, int32 i,	String des, ActionCategory cat, 
			   ActionType typ, int pri, int grp, int flg)
	: _boss(boss), id(i), description(des), category(cat), type(typ), 
	priority(pri), group(grp), flags(flg), _hwKey(0) {
	assert(_boss);
	_boss->addAction(this);
}

void Action::mapKey(const HardwareKey *key) {
	if (_hwKey) _boss->unregisterMapping(this);
	_hwKey = key;
	if (_hwKey) _boss->registerMapping(this, _hwKey);
}

const HardwareKey *Action::getMappedKey() const {
	return _hwKey;
}

} // end of namespace Common
