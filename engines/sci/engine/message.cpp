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

#include "sci/engine/message.h"
#include "sci/tools.h"

namespace Sci {

void MessageState::initIndexRecordCursor() {
	_engineCursor.resource_beginning = _currentResource->data;
	_engineCursor.index_record = _indexRecords;
	_engineCursor.index = 1;
}

void MessageState::setVersion(int version) {
	_version = version;

	if (version == 2101) {
		_headerSize = 6;
		_indexRecordSize = 4;
	} else {
		_headerSize = 10;
		_indexRecordSize = 11;
	}
}

void MessageState::parse(IndexRecordCursor *cursor, MessageTuple *t) {
	t->noun = *(cursor->index_record + 0);
	t->verb = *(cursor->index_record + 1);
	if (_version == 2101) {
		t->cond = 0;
		t->seq = 0;
	} else {
		t->cond = *(cursor->index_record + 2);
		t->seq = *(cursor->index_record + 3);
	}
}

int MessageState::getSpecific(MessageTuple *t) {
	MessageTuple looking_at;
	int found = 0;

	initIndexRecordCursor();

	do {
		parse(&_engineCursor, &looking_at);
		if (t->noun == looking_at.noun && 
			t->verb == looking_at.verb && 
			t->cond == looking_at.cond && 
			t->seq == looking_at.seq)
			found = 1;
	} while (!found && getNext());

	// FIXME: Recursion not handled yet

	return found;
}

int MessageState::getNext() {
	if (_engineCursor.index == _recordCount)
		return 0;
	_engineCursor.index_record += _indexRecordSize;
	_engineCursor.index ++;
	return 1;
}

int MessageState::getTalker() {
	if (_version == 2101)
		return -1;
	else
		return *(_engineCursor.index_record + 4);
}

int MessageState::getText(char *buffer, int length) {
	int offset;
	if (_version == 2101)
		offset = READ_LE_UINT16(_engineCursor.index_record + 2);
	else
		offset = READ_LE_UINT16(_engineCursor.index_record + 5);

	char *stringptr = (char *)_engineCursor.resource_beginning + offset;
	strncpy(buffer, stringptr, length);

	return strlen(buffer);
}

int MessageState::getLength() {
	char buffer[500];
	return getText(buffer, sizeof(buffer));
}

int MessageState::loadRes(int module) {
	if (_module == module)
		return 1;

	// Unlock old resource
	if (_module != -1)
		_resmgr->unlockResource(_currentResource, _module, kResourceTypeMessage);

	_module = module;
	_currentResource = _resmgr->findResource(kResourceTypeMessage, module, 1);

	if (_currentResource == NULL || _currentResource->data == NULL) {
		sciprintf("Message subsystem: Failed to load %d.MSG\n", module);
		_module = -1;
		return 0;
	}

	if (_version == 2101)
		_recordCount = READ_LE_UINT16(_currentResource->data + 4);
	else
		_recordCount = READ_LE_UINT16(_currentResource->data + 8);

	_indexRecords = _currentResource->data + _headerSize;

	initIndexRecordCursor();
	return 1;
}

void MessageState::initialize(ResourceManager *resmgr) {
	_module = -1;
	_resmgr = resmgr;
	_currentResource = NULL;
	_recordCount = 0;
	_initialized = 1;
}

void message_state_initialize(ResourceManager *resmgr, MessageState *state) {
	Resource *tester = resmgr->findResource(kResourceTypeMessage, 0, 0);
	int version;

	if (tester == NULL)
		return;

	version = READ_LE_UINT16(tester->data);
	state->initialize(resmgr);
	state->setVersion(version);
}

} // End of namespace Sci
