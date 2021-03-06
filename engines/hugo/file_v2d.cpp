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

#include "hugo/hugo.h"
#include "hugo/file.h"
#include "hugo/schedule.h"
#include "hugo/display.h"
#include "hugo/text.h"
#include "hugo/util.h"

namespace Hugo {
FileManager_v2d::FileManager_v2d(HugoEngine *vm) : FileManager_v1d(vm) {
}

FileManager_v2d::~FileManager_v2d() {
}

/**
* Open "database" file (packed files)
*/
void FileManager_v2d::openDatabaseFiles() {
	debugC(1, kDebugFile, "openDatabaseFiles");

	if (!_stringArchive.open(getStringFilename()))
		error("File not found: %s", getStringFilename());
	if (!_sceneryArchive1.open(getSceneryFilename()))
		error("File not found: %s", getSceneryFilename());
	if (!_objectsArchive.open(getObjectFilename()))
		error("File not found: %s", getObjectFilename());
}

/**
* Close "Database" files
*/
void FileManager_v2d::closeDatabaseFiles() {
	debugC(1, kDebugFile, "closeDatabaseFiles");

	_stringArchive.close();
	_sceneryArchive1.close();
	_objectsArchive.close();
}

/**
* Read a PCX image into dib_a
*/
void FileManager_v2d::readBackground(int screenIndex) {
	debugC(1, kDebugFile, "readBackground(%d)", screenIndex);

	_sceneryArchive1.seek((uint32) screenIndex * sizeof(sceneBlock_t), SEEK_SET);

	sceneBlock_t sceneBlock;                        // Read a database header entry
	sceneBlock.scene_off = _sceneryArchive1.readUint32LE();
	sceneBlock.scene_len = _sceneryArchive1.readUint32LE();
	sceneBlock.b_off = _sceneryArchive1.readUint32LE();
	sceneBlock.b_len = _sceneryArchive1.readUint32LE();
	sceneBlock.o_off = _sceneryArchive1.readUint32LE();
	sceneBlock.o_len = _sceneryArchive1.readUint32LE();
	sceneBlock.ob_off = _sceneryArchive1.readUint32LE();
	sceneBlock.ob_len = _sceneryArchive1.readUint32LE();

	_sceneryArchive1.seek(sceneBlock.scene_off, SEEK_SET);

	// Read the image into dummy seq and static dib_a
	seq_t *dummySeq;                                // Image sequence structure for Read_pcx
	dummySeq = readPCX(_sceneryArchive1, 0, _vm->_screen->getFrontBuffer(), true, _vm->_text->getScreenNames(screenIndex));
	free(dummySeq);
}

/**
* Open and read in an overlay file, close file
*/
void FileManager_v2d::readOverlay(int screenNum, image_pt image, ovl_t overlayType) {
	debugC(1, kDebugFile, "readOverlay(%d, ...)", screenNum);

	image_pt tmpImage = image;                  // temp ptr to overlay file
	_sceneryArchive1.seek((uint32)screenNum * sizeof(sceneBlock_t), SEEK_SET);

	sceneBlock_t sceneBlock;                        // Database header entry
	sceneBlock.scene_off = _sceneryArchive1.readUint32LE();
	sceneBlock.scene_len = _sceneryArchive1.readUint32LE();
	sceneBlock.b_off = _sceneryArchive1.readUint32LE();
	sceneBlock.b_len = _sceneryArchive1.readUint32LE();
	sceneBlock.o_off = _sceneryArchive1.readUint32LE();
	sceneBlock.o_len = _sceneryArchive1.readUint32LE();
	sceneBlock.ob_off = _sceneryArchive1.readUint32LE();
	sceneBlock.ob_len = _sceneryArchive1.readUint32LE();

	uint32 i = 0;
	switch (overlayType) {
	case kOvlBoundary:
		_sceneryArchive1.seek(sceneBlock.b_off, SEEK_SET);
		i = sceneBlock.b_len;
		break;
	case kOvlOverlay:
		_sceneryArchive1.seek(sceneBlock.o_off, SEEK_SET);
		i = sceneBlock.o_len;
		break;
	case kOvlBase:
		_sceneryArchive1.seek(sceneBlock.ob_off, SEEK_SET);
		i = sceneBlock.ob_len;
		break;
	default:
		error("Bad overlayType: %d", overlayType);
		break;
	}
	if (i == 0) {
		for (int idx = 0; idx < kOvlSize; idx++)
			image[idx] = 0;
		return;
	}

	// Read in the overlay file using MAC Packbits.  (We're not proud!)
	int16 k = 0;                                    // byte count
	do {
		int8 data = _sceneryArchive1.readByte();    // Read a code byte
		if ((byte)data == 0x80)                     // Noop
			;
		else if (data >= 0) {                       // Copy next data+1 literally
			for (i = 0; i <= (byte)data; i++, k++)
				*tmpImage++ = _sceneryArchive1.readByte();
		} else {                                    // Repeat next byte -data+1 times
			int16 j = _sceneryArchive1.readByte();

			for (i = 0; i < (byte)(-data + 1); i++, k++)
				*tmpImage++ = j;
		}
	} while (k < kOvlSize);
}

/**
* Fetch string from file, decode and return ptr to string in memory
*/
char *FileManager_v2d::fetchString(int index) {
	debugC(1, kDebugFile, "fetchString(%d)", index);
	static char buffer[kMaxBoxChar];

	// Get offset to string[index] (and next for length calculation)
	_stringArchive.seek((uint32)index * sizeof(uint32), SEEK_SET);
	uint32 off1, off2;
	if (_stringArchive.read((char *)&off1, sizeof(uint32)) == 0)
		error("An error has occurred: bad String offset");
	if (_stringArchive.read((char *)&off2, sizeof(uint32)) == 0)
		error("An error has occurred: bad String offset");

	// Check size of string
	if ((off2 - off1) >= (uint32) kMaxBoxChar)
		error("Fetched string too long!");

	// Position to string and read it into gen purpose _textBoxBuffer
	_stringArchive.seek(off1, SEEK_SET);
	if (_stringArchive.read(buffer, (uint16)(off2 - off1)) == 0)
		error("An error has occurred: fetchString");

	// Null terminate, decode and return it
	buffer[off2-off1] = '\0';
	_vm->_scheduler->decodeString(buffer);
	return buffer;
}
} // End of namespace Hugo

