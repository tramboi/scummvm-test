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

#include "common/system.h"
#include "common/mutex.h"
#include "kyra/resource.h"
#include "kyra/sound_intern.h"

#include "sound/mixer.h"
#include "sound/mods/maxtrax.h"
#include "sound/audiostream.h"

namespace {
struct EffectEntry {
	uint16	duration;
	uint8	note;
	uint8	patch;
	int8	volume;
	int8	pan;
};
extern const EffectEntry tableEffectsIntro[40];
extern const EffectEntry tableEffectsGame[120];
}

namespace Kyra {

SoundAmiga::SoundAmiga(KyraEngine_v1 *vm, Audio::Mixer *mixer)
	: Sound(vm, mixer),
	  _driver(0),
	  _musicHandle(),
	  _fileLoaded(kFileNone) {
}

SoundAmiga::~SoundAmiga() {
	_mixer->stopHandle(_musicHandle);
	delete _driver;
}

bool SoundAmiga::init() {
	_driver = new Audio::MaxTrax(_mixer->getOutputRate(), true);
	return _driver != 0;
}

void SoundAmiga::loadSoundFile(uint file) {
	static const char *const tableFilenames[3][2] = {
		{ "introscr.mx", "introinst.mx" },
		{ "kyramusic.mx", 0 },
		{ "finalescr.mx", 0 }
	};
	assert(file < ARRAYSIZE(tableFilenames));
	if (_fileLoaded == (FileType)file)
		return;
	const char* scoreName = tableFilenames[file][0];
	const char* sampleName = tableFilenames[file][1];
	bool loaded = false;

	Common::SeekableReadStream *scoreIn = _vm->resource()->createReadStream(scoreName);
	if (sampleName) {
		Common::SeekableReadStream *sampleIn = _vm->resource()->createReadStream(sampleName);
		if (scoreIn && sampleIn) {
			_fileLoaded = kFileNone;
			loaded = _driver->load(*scoreIn, true, false);
			loaded = loaded && _driver->load(*sampleIn, false, true);
		} else
			warning("SoundAmiga: missing atleast one of those music files: %s, %s", scoreName, sampleName);
		delete sampleIn;
	} else {
		if (scoreIn) {
			_fileLoaded = kFileNone;
			loaded = _driver->load(*scoreIn);
		} else
			warning("SoundAmiga: missing music file: %s", scoreName);
	}
	delete scoreIn;

	if (loaded)
		_fileLoaded = (FileType)file;
}

void SoundAmiga::playTrack(uint8 track) {
	static const byte tempoIntro[6] = { 0x46, 0x55, 0x3C, 0x41, 0x78, 0x50 };
	static const byte tempoIngame[23] = {
		0x64, 0x64, 0x64, 0x64, 0x64, 0x73, 0x4B, 0x64,
		0x64, 0x64, 0x55, 0x9C, 0x6E, 0x91, 0x78, 0x84,
		0x32, 0x64, 0x64, 0x6E, 0x3C, 0xD8, 0xAF
	};
	static const byte loopIngame[23] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01,
		0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00
	};

	switch (_fileLoaded) {
	case kFileIntro:
		if (track >= 2) {
			track -= 2;
			if (_driver->playSong(track)) {
				_driver->setVolume(0x40);
				_driver->setTempo(tempoIntro[track] << 4);
				if (!_mixer->isSoundHandleActive(_musicHandle))
					_mixer->playInputStream(Audio::Mixer::kPlainSoundType, &_musicHandle, _driver, -1, Audio::Mixer::kMaxChannelVolume, 0, false);
			}
		} else if (track == 0){
			_driver->stopMusic();
		} else { // track == 1
			beginFadeOut();
		}
		break;

	case kFileGame:
		if (track < 0x80 && track != 3) {
			if (track >= 2) {
				track -= 0xB;
				if (_driver->playSong(track, loopIngame[track] != 0)) {
					_driver->setVolume(0x40);
				
					_driver->setTempo(tempoIngame[track] << 4);
					if (!_mixer->isSoundHandleActive(_musicHandle))
						_mixer->playInputStream(Audio::Mixer::kPlainSoundType, &_musicHandle, _driver, -1, Audio::Mixer::kMaxChannelVolume, 0, false);
				}
			} else if (track == 0){
				_driver->stopMusic();
			} else { // track == 1
				beginFadeOut();
			}
		}
		break;
	}
}

void SoundAmiga::haltTrack() {
	_driver->stopMusic();
}

void SoundAmiga::beginFadeOut() {
	for (int i = 0x3F; i >= 0; --i) {
		_driver->setVolume((byte)i);
		_vm->delay(_vm->tickLength());
	}

	_driver->stopMusic();
	_vm->delay(_vm->tickLength());
	_driver->setVolume(0x40);
}

void SoundAmiga::playSoundEffect(uint8 track) {
	debug("play sfx %d", track);

	switch (_fileLoaded) {
	case kFileIntro: {
		assert(track < ARRAYSIZE(tableEffectsIntro));
		const EffectEntry &entry = tableEffectsIntro[track];
		bool success = _driver->playNote(entry.note, entry.patch, entry.duration, entry.volume, entry.pan != 0) >= 0;
		if (!_mixer->isSoundHandleActive(_musicHandle))
			_mixer->playInputStream(Audio::Mixer::kPlainSoundType, &_musicHandle, _driver, -1, Audio::Mixer::kMaxChannelVolume, 0, false);
		}
		break;


	case kFileGame: {
		uint16 extVar = 1; // maybe indicates music playing or enabled
		uint16 extVar2 = 1; // sound loaded ?
		if (0x61 <= track && track <= 0x63 && extVar)
			playTrack(track - 0x4F);

		assert(track < ARRAYSIZE(tableEffectsGame));
		const EffectEntry &entry = tableEffectsGame[track];
		if (extVar2 && entry.note) {
			byte pan = (entry.pan == 2) ? 0 : entry.pan;
			_driver->playNote(entry.note, entry.patch, entry.duration, entry.volume, pan != 0);
			if (!_mixer->isSoundHandleActive(_musicHandle))
				_mixer->playInputStream(Audio::Mixer::kPlainSoundType, &_musicHandle, _driver, -1, Audio::Mixer::kMaxChannelVolume, 0, false);
		}
		}
		break;
	}
}

} // end of namespace Kyra

namespace {

const EffectEntry tableEffectsIntro[40] = {
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x252C, 0x3C, 0x19, 110,  0 },
	{ 0x252C, 0x3C, 0x19, 110,  0 },
	{ 0x252C, 0x3C, 0x19, 110,  0 },
	{ 0x1B91, 0x3C, 0x13, 110,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x2677, 0x3C, 0x16, 110,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x1198, 0x3C, 0x17, 110,  0 },
	{ 0x252C, 0x3C, 0x19, 110,  0 },
	{ 0x22D1, 0x3C, 0x18, 110,  0 },
	{ 0x252C, 0x3C, 0x19, 110,  0 },
	{ 0x0224, 0x45, 0x03, 110,  0 },
	{ 0x2677, 0x3C, 0x16, 110,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 }
};

const EffectEntry tableEffectsGame[120] = {
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x01, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0156, 0x3C, 0x13, 120,  2 },
	{ 0x272C, 0x3C, 0x14, 120,  2 },
	{ 0x1B91, 0x3C, 0x15, 120,  2 },
	{ 0x1E97, 0x3C, 0x16, 120,  2 },
	{ 0x122B, 0x3C, 0x17, 120,  2 },
	{ 0x1E97, 0x3C, 0x16, 120,  2 },
	{ 0x0224, 0x45, 0x03, 120,  2 },
	{ 0x1E97, 0x3C, 0x16, 120,  2 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x252C, 0x3C, 0x19, 120,  2 },
	{ 0x0910, 0x2C, 0x04, 120,  2 },
	{ 0x252C, 0x3C, 0x19, 120,  2 },
	{ 0x3AEB, 0x3C, 0x1A, 120,  2 },
	{ 0x138B, 0x25, 0x1B, 120,  2 },
	{ 0x0F52, 0x18, 0x03, 120,  2 },
	{ 0x0622, 0x3E, 0x1C, 120,  2 },
	{ 0x0754, 0x3B, 0x1C, 120,  2 },
	{ 0x206F, 0x16, 0x03, 120,  2 },
	{ 0x252C, 0x3C, 0x19, 120,  2 },
	{ 0x09EA, 0x3C, 0x1D, 120,  2 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x272C, 0x3C, 0x14, 120,  2 },
	{ 0x036E, 0x3C, 0x1E, 120,  2 },
	{ 0x122B, 0x3C, 0x17, 120,  2 },
	{ 0x0991, 0x4E, 0x0B, 120,  2 },
	{ 0x02BC, 0x47, 0x1B, 120,  2 },
	{ 0x0211, 0x4C, 0x1B, 120,  2 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0156, 0x3C, 0x13, 120,  2 },
	{ 0x0156, 0x3C, 0x13, 120,  2 },
	{ 0x0E9E, 0x3C, 0x1F, 120,  2 },
	{ 0x010C, 0x3C, 0x20, 120,  2 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x252C, 0x3C, 0x19, 120,  2 },
	{ 0x0F7C, 0x3C, 0x21, 120,  2 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x4C47, 0x2A, 0x0B, 120,  2 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0528, 0x3C, 0x1B, 120,  2 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0910, 0x2C, 0x04, 120,  2 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0AEE, 0x3C, 0x22, 120,  2 },
	{ 0x1E97, 0x3C, 0x16, 120,  2 },
	{ 0x1B91, 0x3C, 0x15, 120,  2 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x272C, 0x3C, 0x14, 120,  2 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0AEE, 0x3C, 0x22, 120,  2 },
	{ 0x272C, 0x3C, 0x14, 120,  2 },
	{ 0x1419, 0x32, 0x23, -100,  2 },
	{ 0x171C, 0x3C, 0x19, 120,  2 },
	{ 0x272C, 0x3C, 0x14, 120,  2 },
	{ 0x0622, 0x3E, 0x1C, 120,  2 },
	{ 0x0201, 0x43, 0x13, 120,  2 },
	{ 0x1243, 0x3C, 0x24,  90,  2 },
	{ 0x00EE, 0x3E, 0x20, 120,  2 },
	{ 0x252C, 0x3C, 0x19, 120,  2 },
	{ 0x19EA, 0x29, 0x04, 120,  2 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x010C, 0x3C, 0x20, 120,  2 },
	{ 0x30B6, 0x3C, 0x25, 120,  2 },
	{ 0x252C, 0x3C, 0x19, 120,  2 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x1E97, 0x3C, 0x16, 120,  2 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x3AEB, 0x3C, 0x1A, 120,  2 },
	{ 0x39F3, 0x1B, 0x04, 120,  2 },
	{ 0x1699, 0x30, 0x23,  80,  2 },
	{ 0x1B91, 0x3C, 0x15, 120,  2 },
	{ 0x19EA, 0x29, 0x06,  80,  2 },
	{ 0x252C, 0x3C, 0x19, 120,  2 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x3AEB, 0x3C, 0x1A, 120,  2 },
	{ 0x252C, 0x3C, 0x19, 120,  2 },
	{ 0x0713, 0x3C, 0x26, 120,  2 },
	{ 0x0713, 0x3C, 0x26, 120,  2 },
	{ 0x272C, 0x3C, 0x14, 120,  2 },
	{ 0x1699, 0x30, 0x23,  80,  2 },
	{ 0x1699, 0x30, 0x23,  80,  2 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0156, 0x3C, 0x13, 120,  2 }
};

} // end of namespace