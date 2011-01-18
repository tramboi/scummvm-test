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

#include "gob/save/saveload.h"
#include "gob/save/saveconverter.h"
#include "gob/global.h"
#include "gob/inter.h"

namespace Gob {

SaveLoad_Inca2::SaveFile SaveLoad_Inca2::_saveFiles[] = {
	{"speak.inf", kSaveModeExists, 0, 0}, // Exists = speech enabled
	{"voice.inf", kSaveModeSave  , 0, 0}  // Contains the language of the voices
};


SaveLoad_Inca2::VoiceHandler::VoiceHandler(GobEngine *vm) : SaveHandler(vm) {
}

SaveLoad_Inca2::VoiceHandler::~VoiceHandler() {
}

int32 SaveLoad_Inca2::VoiceHandler::getSize() {
	return 1;
}

bool SaveLoad_Inca2::VoiceHandler::load(int16 dataVar, int32 size, int32 offset) {
	if ((size != 1) || (offset != 0)) {
		warning("Invalid voice language loading?!? (%d, %d, %d)", dataVar, size, offset);
		return false;
	}

	WRITE_VARO_UINT8(dataVar, _vm->_global->_language);
	return true;
}

bool SaveLoad_Inca2::VoiceHandler::save(int16 dataVar, int32 size, int32 offset) {
	return false;
}


SaveLoad_Inca2::SaveLoad_Inca2(GobEngine *vm, const char *targetName) : SaveLoad(vm) {
	_voiceHandler = new VoiceHandler(vm);

	_saveFiles[1].handler = _voiceHandler;
}

SaveLoad_Inca2::~SaveLoad_Inca2() {
	delete _voiceHandler;
}

const SaveLoad_Inca2::SaveFile *SaveLoad_Inca2::getSaveFile(const char *fileName) const {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return 0;
}

SaveLoad_Inca2::SaveFile *SaveLoad_Inca2::getSaveFile(const char *fileName) {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return 0;
}

SaveHandler *SaveLoad_Inca2::getHandler(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->handler;

	return 0;
}

const char *SaveLoad_Inca2::getDescription(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->description;

	return 0;
}

SaveLoad::SaveMode SaveLoad_Inca2::getSaveMode(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->mode;

	return kSaveModeNone;
}

} // End of namespace Gob
