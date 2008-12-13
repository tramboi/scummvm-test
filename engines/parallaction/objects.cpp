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

#include "parallaction/parallaction.h"
#include "parallaction/objects.h"
#include "parallaction/parser.h"

namespace Parallaction {


ZonePtr nullZonePtr;
AnimationPtr nullAnimationPtr;
InstructionPtr nullInstructionPtr;

Command::Command() {
	_id = 0;
	_flagsOn = 0;
	_flagsOff = 0;
}

Command::~Command() {

}


Animation::Animation() {
	gfxobj = NULL;
	_scriptName = 0;
	_frame = 0;
	_z = 0;
}

Animation::~Animation() {
	free(_scriptName);
	gfxobj->release();
}

uint16 Animation::width() const {
	if (!gfxobj) return 0;
	Common::Rect r;
	gfxobj->getRect(_frame, r);
	return r.width();
}

uint16 Animation::height() const {
	if (!gfxobj) return 0;
	Common::Rect r;
	gfxobj->getRect(_frame, r);
	return r.height();
}

int16 Animation::getFrameX() const {
	if (!gfxobj) return _left;
	Common::Rect r;
	gfxobj->getRect(_frame, r);
	return r.left + _left;
}

int16 Animation::getFrameY() const {
	if (!gfxobj) return _top;
	Common::Rect r;
	gfxobj->getRect(_frame, r);
	return r.top + _top;
}

uint16 Animation::getFrameNum() const {
	if (!gfxobj) return 0;
	return gfxobj->getNum();
}

byte* Animation::getFrameData(uint32 index) const {
	if (!gfxobj) return NULL;
	return gfxobj->getData(index);
}

void Animation::validateScriptVars() {
	// this is used to clip values of _frame, _left and _top
	// which can be screwed up by buggy scripts.

	_frame = CLIP(_frame, (int16)0, (int16)(getFrameNum() - 1));
}

#define NUM_LOCALS	10
char	_localNames[NUM_LOCALS][10];

Program::Program() {
	_loopCounter = 0;
	_locals = new LocalVariable[NUM_LOCALS];
	_numLocals = 0;
	_status = kProgramIdle;
}

Program::~Program() {
	delete[] _locals;
}

int16 Program::findLocal(const char* name) {
	for (uint16 _si = 0; _si < NUM_LOCALS; _si++) {
		if (!scumm_stricmp(name, _localNames[_si]))
			return _si;
	}

	return -1;
}

int16 Program::addLocal(const char *name, int16 value, int16 min, int16 max) {
	assert(_numLocals < NUM_LOCALS);

	strcpy(_localNames[_numLocals], name);
	_locals[_numLocals].setRange(min, max);
	_locals[_numLocals].setValue(value);

	return _numLocals++;
}

void LocalVariable::setValue(int16 value) {
	if (value >= _max)
		value = _min;
	if (value < _min)
		value = _max - 1;

	_value = value;
}

void LocalVariable::setRange(int16 min, int16 max) {
	_max = max;
	_min = min;
}

int16 LocalVariable::getValue() const {
	return _value;
}


Zone::Zone() {
	_left = _top = _right = _bottom = 0;

	_type = 0;

	_flags = kFlagsNoName;
	_label = 0;

	// BRA specific
	_index = 0;
	_linkedName = 0;
}

Zone::~Zone() {
//	printf("~Zone(%s)\n", _name);

	switch (_type & 0xFFFF) {
	case kZoneExamine:
		free(u.examine->_filename);
		u.examine->_description.clear();
		delete u.examine->_cnv;
		delete u.examine;
		break;

	case kZoneDoor:
		free(u.door->_location);
		u.door->gfxobj->release();
		delete u.door;
		break;

	case kZoneSpeak:
		delete u.speak->_dialogue;
		delete u.speak;
		break;

	case kZoneGet:
		u.get->gfxobj->release();
		delete u.get;
		break;

	case kZoneHear:
		delete u.hear;
		break;

	case kZoneMerge:
		delete u.merge;
		break;

	case kZonePath:
		delete u.path;
		break;

	default:
		break;
	}


	free(_linkedName);
}

void Zone::translate(int16 x, int16 y) {
	_left += x;
	_right += x;
	_top += y;
	_bottom += y;
}

uint16 Zone::width() const {
	return _right - _left;
}

uint16 Zone::height() const {
	return _bottom - _top;
}

Dialogue::Dialogue() {
	memset(_questions, 0, sizeof(_questions));
}

Dialogue::~Dialogue() {
	for (int i = 0; i < NUM_QUESTIONS; i++) {
		delete _questions[i];
	}
}

Answer::Answer() {
	_mood = 0;
	_followingQuestion =  NULL;
	_noFlags = 0;
	_yesFlags = 0;
}

Question::Question() {
	_mood = 0;

	for (uint32 i = 0; i < NUM_ANSWERS; i++)
		_answers[i] = NULL;

}

Question::~Question() {
	for (uint32 i = 0; i < NUM_ANSWERS; i++) {
		delete _answers[i];
	}
}

Instruction::Instruction() {
	_index = 0;
	_flags = 0;

	// common
	_immediate = 0;

	// BRA specific
	_text = 0;
	_text2 = 0;
	_y = 0;
}

Instruction::~Instruction() {
	free(_text);
	free(_text2);
}

int16 ScriptVar::getValue() {

	if (_flags & kParaImmediate) {
		return _value;
	}

	if (_flags & kParaLocal) {
		return _local->getValue();
	}

	if (_flags & kParaField) {
		return _field->getValue();
	}

	if (_flags & kParaRandom) {
		return (_vm->_rnd.getRandomNumber(65536) * _value) >> 16;
	}

	error("Parameter is not an r-value");

	return 0;
}

void ScriptVar::setValue(int16 value) {
	if ((_flags & kParaLValue) == 0) {
		error("Only l-value can be set");
	}

	if (_flags & kParaLocal) {
		_local->setValue(value);
	}

	if (_flags & kParaField) {
		_field->setValue(value);
	}

}

void ScriptVar::setLocal(LocalVariable *local) {
	_local = local;
	_flags |= (kParaLocal | kParaLValue);
}

void ScriptVar::setField(Animation *anim, AnimationField::AccessorFunc accessor, AnimationField::MutatorFunc mutator) {
	_field = new AnimationField(anim, accessor, mutator);
	_flags |= (kParaField | kParaLValue);
}

void ScriptVar::setField(Animation *anim, AnimationField::AccessorFunc accessor) {
	_field = new AnimationField(anim, accessor);
	_flags |= kParaField;
}

void ScriptVar::setImmediate(int16 value) {
	_value = value;
	_flags |= kParaImmediate;
}

void ScriptVar::setRandom(int16 seed) {
	_value = seed;
	_flags |= kParaRandom;
}


ScriptVar::ScriptVar() {
	_flags = 0;
	_local = 0;
	_value = 0;
	_field = 0;
}

ScriptVar::~ScriptVar() {
	delete _field;
}


Table::Table(uint32 size) : _size(size), _used(0), _disposeMemory(true) {
	_data = (char**)calloc(size, sizeof(char*));
}

Table::Table(uint32 size, const char **data) : _size(size), _used(size), _disposeMemory(false) {
	_data = const_cast<char**>(data);
}

Table::~Table() {

	if (!_disposeMemory) return;

	clear();

	free(_data);

}

void Table::addData(const char* s) {

	if (!(_used < _size))
		error("Table overflow");

	_data[_used++] = strdup(s);

}

uint16 Table::lookup(const char* s) {

	for (uint16 i = 0; i < _used; i++) {
		if (!scumm_stricmp(_data[i], s)) return i + 1;
	}

	return notFound;
}

void Table::clear() {
	for (uint32 i = 0; i < _used; i++)
		free(_data[i]);

	_used = 0;
}

const char *Table::item(uint index) const {
	assert(index < _used);
	return _data[index];
}


FixedTable::FixedTable(uint32 size, uint32 fixed) : Table(size), _numFixed(fixed) {
}

void FixedTable::clear() {
	uint32 deleted = 0;
	for (uint32 i = _numFixed; i < _used; i++) {
		free(_data[i]);
		_data[i] = 0;
		deleted++;
	}

	_used -= deleted;
}

Table* createTableFromStream(uint32 size, Common::SeekableReadStream &stream) {

	Table *t = new Table(size);

	Script s(&stream, false);

	s.readLineToken();
	while (scumm_stricmp(_tokens[0], "ENDTABLE")) {
		t->addData(_tokens[0]);
		s.readLineToken();
	}

	return t;
}


} // namespace Parallaction
