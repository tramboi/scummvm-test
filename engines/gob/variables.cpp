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

#include "common/endian.h"

#include "gob/gob.h"
#include "gob/variables.h"

namespace Gob {

Variables::Variables(uint32 size) {
	_size = size;

	_vars = new byte[_size];
	_sizes = new byte[_size];

	clear();
}

Variables::~Variables() {
	delete[] _vars;
	delete[] _sizes;
}

uint32 Variables::getSize() const {
	return _size;
}

void Variables::clear() {
	memset(_vars, 0, _size);

	// All variables are 32 bit wide per default
	memset(_sizes, 0, _size);
	for (uint32 i = 0; i < _size; i += 4)
		_sizes[i] = kSize32;
}

void Variables::clearSize(uint32 offset) {
	uint32 inVar = offset % 4;
	uint32 varOff = (offset >> 2) << 2;

	// Clearing out the old size
	for (uint32 i = 0; i < 4; i++) {
		if (_sizes[varOff + i] == kSize32)
			_sizes[varOff + i] = kSize8;
		else if ((inVar == (i + 1)) && (_sizes[varOff + i] == kSize16))
			_sizes[varOff + i] = kSize8;
	}
}

void Variables::writeSize(uint32 offset, byte n) {
	clearSize(offset);

	_sizes[offset] = n;
	// Setting following bytes of size to 8 bit, for easy clearing out afterwards
	for (; n > 0; n--)
		_sizes[offset + n] = kSize8;
}

void Variables::writeSizeString(uint32 offset, uint32 length) {
	clearSize(offset);

	memset(_sizes + offset, kSize8, length);
}

void Variables::writeVar8(uint32 var, uint8 value) {
	writeOff8(var * 4, value);
}

void Variables::writeVar16(uint32 var, uint16 value) {
	writeOff16(var * 4, value);
}

void Variables::writeVar32(uint32 var, uint32 value) {
	writeOff32(var * 4, value);
}

void Variables::writeVarString(uint32 var, const char *value) {
	writeOffString(var * 4, value);
}

void Variables::writeOff8(uint32 offset, uint8 value) {
	write8(_vars + offset, value);
	writeSize(offset, kSize8);
}

void Variables::writeOff16(uint32 offset, uint16 value) {
	write16(_vars + offset, value);
	writeSize(offset, kSize16);
}

void Variables::writeOff32(uint32 offset, uint32 value) {
	write32(_vars + offset, value);
	writeSize(offset, kSize32);
}

void Variables::writeOffString(uint32 offset, const char *value) {
	strcpy((char *) (_vars + offset), value);
	writeSizeString(offset, strlen(value));
}

uint8 Variables::readVar8(uint32 var) const {
	return readOff8(var * 4);
}

uint16 Variables::readVar16(uint32 var) const {
	return readOff16(var * 4);
}

uint32 Variables::readVar32(uint32 var) const {
	return readOff32(var * 4);
}

void Variables::readVarString(uint32 var, char *value, uint32 length) {
	readOffString(var * 4, value, length);
}

uint8 Variables::readOff8(uint32 offset) const {
	return read8(_vars + offset);
}

uint16 Variables::readOff16(uint32 offset) const {
	return read16(_vars + offset);
}

uint32 Variables::readOff32(uint32 offset) const {
	return read32(_vars + offset);
}

void Variables::readOffString(uint32 offset, char *value, uint32 length) {
	strncpy0(value, (const char *) (_vars + offset), length - 1);
}

const uint8 *Variables::getAddressVar8(uint32 var) const {
	return getAddressOff8(var * 4);
}

uint8 *Variables::getAddressVar8(uint32 var, uint32 n) {
	return getAddressOff8(var * 4, n);
}

const uint16 *Variables::getAddressVar16(uint32 var) const {
	return getAddressOff16(var * 4);
}

uint16 *Variables::getAddressVar16(uint32 var, uint32 n) {
	return getAddressOff16(var * 4, n);
}

const uint32 *Variables::getAddressVar32(uint32 var) const {
	return getAddressOff32(var * 4);
}

uint32 *Variables::getAddressVar32(uint32 var, uint32 n) {
	return getAddressOff32(var * 4, n);
}

const char *Variables::getAddressVarString(uint32 var) const {
	return getAddressOffString(var * 4);
}

char *Variables::getAddressVarString(uint32 var, uint32 n) {
	return getAddressOffString(var * 4, n);
}

const uint8 *Variables::getAddressOff8(uint32 offset) const {
	return ((const uint8 *) (_vars + offset));
}

uint8 *Variables::getAddressOff8(uint32 offset, uint32 n) {
	for (uint32 i = 0; i < n; i++)
		writeSize(offset + i, kSize8);

	return ((uint8 *) (_vars + offset));
}

const uint16 *Variables::getAddressOff16(uint32 offset) const {
	return ((const uint16 *) (_vars + offset));
}

uint16 *Variables::getAddressOff16(uint32 offset, uint32 n) {
	for (uint32 i = 0; i < n; i++)
		writeSize(offset + i * 2, kSize16);

	return ((uint16 *) (_vars + offset));
}

const uint32 *Variables::getAddressOff32(uint32 offset) const {
	return ((const uint32 *) (_vars + offset));
}

uint32 *Variables::getAddressOff32(uint32 offset, uint32 n) {
	for (uint32 i = 0; i < n; i++)
		writeSize(offset + i * 4, kSize32);

	return ((uint32 *) (_vars + offset));
}

const char *Variables::getAddressOffString(uint32 offset) const {
	return ((const char *) (_vars + offset));
}

char *Variables::getAddressOffString(uint32 offset, uint32 n) {
	writeSizeString(offset, (n == 0xFFFFFFFF) ? strlen((char *) (_vars + offset)) : n);

	return ((char *) (_vars + offset));
}

bool Variables::copyTo(uint32 offset, byte *variables, byte *sizes, uint32 n) const {
	if ((offset + n) > _size)
		return false;

	if (variables)
		memcpy(variables, _vars + offset, n);
	if (sizes)
		memcpy(sizes, _sizes + offset, n);

	return true;
}

bool Variables::copyFrom(uint32 offset, const byte *variables, const byte *sizes, uint32 n) {
	if (((offset + n) > _size) || !variables || !sizes)
		return false;

	memcpy(_vars + offset, variables, n);
	memcpy(_sizes + offset, sizes, n);

	return true;
}


VariablesLE::VariablesLE(uint32 size) : Variables(size) {
}

VariablesLE::~VariablesLE() {
}

void VariablesLE::write8(byte *buf, uint8 data) const {
	*buf = (byte) data;
}

void VariablesLE::write16(byte *buf, uint16 data) const {
	WRITE_LE_UINT16(buf, data);
}

void VariablesLE::write32(byte *buf, uint32 data) const {
	WRITE_LE_UINT32(buf, data);
}

uint8 VariablesLE::read8(const byte *buf) const {
	return (uint8) *buf;
}

uint16 VariablesLE::read16(const byte *buf) const {
	return READ_LE_UINT16(buf);
}

uint32 VariablesLE::read32(const byte *buf) const {
	return READ_LE_UINT32(buf);
}


VariablesBE::VariablesBE(uint32 size) : Variables(size) {
}

VariablesBE::~VariablesBE() {
}

void VariablesBE::write8(byte *buf, uint8 data) const {
	*buf = (byte) data;
}

void VariablesBE::write16(byte *buf, uint16 data) const {
	WRITE_BE_UINT16(buf, data);
}

void VariablesBE::write32(byte *buf, uint32 data) const {
	WRITE_BE_UINT32(buf, data);
}

uint8 VariablesBE::read8(const byte *buf) const {
	return (uint8) *buf;
}

uint16 VariablesBE::read16(const byte *buf) const {
	return READ_BE_UINT16(buf);
}

uint32 VariablesBE::read32(const byte *buf) const {
	return READ_BE_UINT32(buf);
}

VariableReference::VariableReference() {
	_vars = 0;
	_offset = 0;
}

VariableReference::VariableReference(Variables &vars, uint32 offset, Variables::Type type) {
	set(vars, offset, type);
}

VariableReference::~VariableReference() {
}

void VariableReference::set(Variables &vars, uint32 offset, Variables::Type type) {
	_vars = &vars;
	_offset = offset;
	_type = type;
}

VariableReference &VariableReference::operator=(uint32 value) {
	if (_vars) {
		switch (_type) {
			case Variables::kVariableType8:
				_vars->writeOff8(_offset, (uint8) value);
				break;
			case Variables::kVariableType16:
				_vars->writeOff16(_offset, (uint16) value);
				break;
			case Variables::kVariableType32:
				_vars->writeOff32(_offset, value);
				break;
		}
	}
	return *this;
}

VariableReference::operator uint32() {
	if (_vars) {
		switch (_type) {
			case Variables::kVariableType8:
				return (uint32) _vars->readOff8(_offset);
			case Variables::kVariableType16:
				return (uint32) _vars->readOff16(_offset);
			case Variables::kVariableType32:
				return _vars->readOff32(_offset);
		}
	}

	return 0;
}

VariableReference &VariableReference::operator+=(uint32 value) {
	return (*this = (*this + value));
}

VariableReference &VariableReference::operator*=(uint32 value) {
	return (*this = (*this * value));
}

} // End of namespace Gob
