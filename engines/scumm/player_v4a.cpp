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


#include "engines/engine.h"
#include "scumm/player_v4a.h"
#include "scumm/scumm.h"

#include "common/file.h"

namespace Scumm {

Player_V4A::Player_V4A(ScummEngine *scumm, Audio::Mixer *mixer)
	: _vm(scumm), _mixer(mixer), _musicId(), _sfxSlots(), _tfmxPlay(0), _tfmxSfx(0), _musicHandle(), _sfxHandle() {
	init();
}

bool Player_V4A::init() {
	if (_vm->_game.id != GID_MONKEY_VGA)
		error("player_v4a - unknown game");
	
	Common::File fileMdat;
	Common::File fileSample;
	bool mdatExists = fileMdat.open("music.dat");
	bool sampleExists = fileSample.open("sample.dat");

	if (mdatExists && sampleExists) {
		Audio::Tfmx *play =  new Audio::Tfmx(_mixer->getOutputRate(), true);
		if (play->load(fileMdat, fileSample)) {
			play->setSignalPtr(_signal, ARRAYSIZE(_signal));
			_tfmxPlay = play;
		} else
			delete play;

		play = new Audio::Tfmx(_mixer->getOutputRate(), true);
		fileMdat.seek(0);
		fileSample.seek(0);
		if (play->load(fileMdat, fileSample)) {
			_tfmxSfx = play;
		} else
			delete play;
	}
	return _tfmxPlay != 0;
}

Player_V4A::~Player_V4A() {
	_mixer->stopHandle(_musicHandle);
	_mixer->stopHandle(_sfxHandle);
	delete _tfmxPlay;
	delete _tfmxSfx;
}

void Player_V4A::setMusicVolume(int vol) {
	debug(5, "player_v4a: setMusicVolume %i", vol);
}

void Player_V4A::stopAllSounds() {
	debug(5, "player_v4a: stopAllSounds");
	if (_tfmxPlay) {
		_tfmxPlay->stopSong();
		_signal[0] = 0;		
	}
	_musicId = 0;
	if (_tfmxSfx)
		_tfmxSfx->stopSong();
	clearSfxSlots();
}

void Player_V4A::stopSound(int nr) {
	debug(5, "player_v4a: stopSound %d", nr);
	if (nr == 0)
		return;
	if (nr == _musicId) {
		_musicId = 0;
		_tfmxPlay->stopSong();
		_signal[0] = 0;
	} else {
		const int chan = getSfxChan(nr);
		if (chan != -1) {
			setSfxSlot(chan, 0);
			_tfmxSfx->stopMacroEffect(chan);
		}
	}
}

void Player_V4A::startSound(int nr) {
	assert(_vm);
	const byte *ptr = _vm->getResourceAddress(rtSound, nr);
	assert(ptr);

	static const int8 monkeyCommands[52] = {
		 -1,  -2,  -3,  -4,  -5,  -6,  -7,  -8,
		 -9, -10, -11, -12, -13, -14,  18,  17,
		-17, -18, -19, -20, -21, -22, -23, -24,
		-25, -26, -27, -28, -29, -30, -31, -32,
		-33,  16, -35,   0,   1,   2,   3,   7,
		  8,  10,  11,   4,   5,  14,  15,  12,
		  6,  13,   9,  19
	};

	const int val = ptr[9];
	if (val < 0 || val >= ARRAYSIZE(monkeyCommands)) {
		warning("player_v4a: illegal Songnumber %i", val);
		return;
	}

	if (!_tfmxSfx || !_tfmxPlay)
		return;

	int index = monkeyCommands[val];
	const byte type = ptr[6];
	if (index < 0) {
		// SoundFX
		index = -index - 1;
		debug(3, "player_v4a: play %d: custom %i - %02X", nr, index, type);

		if (_tfmxSfx->getSongIndex() < 0)
			_tfmxSfx->doSong(0x18);
		const int chan = _tfmxSfx->doSfx((uint16)index);
		if (chan >= 0 && chan < ARRAYSIZE(_sfxSlots))
			setSfxSlot(chan, nr, type);
		else
			warning("player_v4a: custom %i is not of required type", index);

		if (!_mixer->isSoundHandleActive(_sfxHandle))
			_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_sfxHandle, _tfmxSfx, -1, Audio::Mixer::kMaxChannelVolume, 0, false);
	} else {
		// Song
		debug(3, "player_v4a: play %d: song %i - %02X", nr, index, type);
		if (ptr[6] != 0x7F)
			warning("player_v4a: Song has wrong type");

		_tfmxPlay->doSong(index);
		_signal[0] = 2;
		_musicId = nr;

		if (!_mixer->isSoundHandleActive(_musicHandle))
			_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_musicHandle, _tfmxPlay, -1, Audio::Mixer::kMaxChannelVolume, 0, false);
	}
}

int Player_V4A::getMusicTimer() const {
	if (_musicId) {
		// TODO: The titlesong is running with ~70 ticks per second and the scale seems to be based on that. 
		// Other songs dont and I have no clue if this scalevalue is anything close to correct for them.
		// The Amiga-Game doesnt counts the ticks of the song, but has an own timer and I hope thespeed is constant through the game
		const int magicScale = 357; // ~ 1000 * 25 * (10173 / 709379.0)
		return (_mixer->getSoundElapsedTime(_musicHandle)) / magicScale;
	}
	return 0;
}

int Player_V4A::getSoundStatus(int nr) const {
	bool a = nr == _musicId;
	bool b = _tfmxPlay->getSongIndex() >= 0;
	int c = _signal[0];

	debug(5, "player_v4a: getSoundStatus music=%d, active=%d, signal=%d", a, b, c);
	if (nr == _musicId)
		return _signal[0];
	return 0;
}

} // End of namespace Scumm
