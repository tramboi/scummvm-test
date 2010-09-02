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
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#ifndef SWORD25_INPUTPERSISTENCEBLOCK_H
#define SWORD25_INPUTPERSISTENCEBLOCK_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "common/array.h"
#include "sword25/kernel/common.h"
#include "sword25/kernel/persistenceblock.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Class declaration
// -----------------------------------------------------------------------------

class InputPersistenceBlock : public PersistenceBlock {
public:
	enum ErrorState {
		NONE,
		END_OF_DATA,
		OUT_OF_SYNC
	};

	InputPersistenceBlock(const void *Data, unsigned int DataLength);
	virtual ~InputPersistenceBlock();

	void Read(int16 &Value);
	void Read(signed int &Value);
	void Read(unsigned int &Value);
	void Read(float &Value);
	void Read(bool &Value);
	void Read(Common::String &Value);
	void Read(Common::Array<byte> &Value);

	bool IsGood() const {
		return m_ErrorState == NONE;
	}
	ErrorState GetErrorState() const {
		return m_ErrorState;
	}

private:
	bool CheckMarker(byte Marker);
	bool CheckBlockSize(int Size);
	void RawRead(void *DestPtr, size_t Size);

	Common::Array<byte> m_Data;
	Common::Array<byte>::const_iterator m_Iter;
	ErrorState m_ErrorState;
};

} // End of namespace Sword25

#endif
