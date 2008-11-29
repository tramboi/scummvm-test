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

#include "common/file.h"
#include "common/endian.h"

#include "gob/gob.h"
#include "gob/sound/adlib.h"

namespace Gob {

const unsigned char AdLib::_operators[] = {0, 1, 2, 8, 9, 10, 16, 17, 18};
const unsigned char AdLib::_volRegNums[] = {
	3,  4,  5,
	11, 12, 13,
	19, 20, 21
};

AdLib::AdLib(Audio::Mixer &mixer) : _mixer(&mixer) {
	_index = -1;
	_data = 0;
	_playPos = 0;
	_dataSize = 0;

	_rate = _mixer->getOutputRate();
	_opl = makeAdlibOPL(_rate);

	_first = true;
	_ended = false;
	_playing = false;
	_needFree = false;

	_repCount = -1;
	_samplesTillPoll = 0;

	for (int i = 0; i < 16; i ++)
		_pollNotes[i] = 0;
	setFreqs();

	_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_handle,
			this, -1, 255, 0, false, true);
}

AdLib::~AdLib() {
	Common::StackLock slock(_mutex);

	_mixer->stopHandle(_handle);
	OPLDestroy(_opl);
	if (_data && _needFree)
		delete[] _data;
}

int AdLib::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock slock(_mutex);
	int samples;
	int render;

	if (!_playing || (numSamples < 0)) {
		memset(buffer, 0, numSamples * sizeof(int16));
		return numSamples;
	}
	if (_first) {
		memset(buffer, 0, numSamples * sizeof(int16));
		pollMusic();
		return numSamples;
	}

	samples = numSamples;
	while (samples && _playing) {
		if (_samplesTillPoll) {
			render = (samples > _samplesTillPoll) ?  (_samplesTillPoll) : (samples);
			samples -= render;
			_samplesTillPoll -= render;
			YM3812UpdateOne(_opl, buffer, render);
			buffer += render;
		} else {
			pollMusic();
			if (_ended) {
				memset(buffer, 0, samples * sizeof(int16));
				samples = 0;
			}
		}
	}

	if (_ended) {
		_first = true;
		_ended = false;
		_playPos = _data + 3 + (_data[1] + 1) * 0x38;
		_samplesTillPoll = 0;
		if (_repCount == -1) {
			reset();
			setVoices();
		} else if (_repCount > 0) {
			_repCount--;
			reset();
			setVoices();
		}
		else
			_playing = false;
	}

	return numSamples;
}

void AdLib::writeOPL(byte reg, byte val) {
	debugC(6, kDebugSound, "writeOPL(%02X, %02X)", reg, val);
	OPLWriteReg(_opl, reg, val);
}

void AdLib::setFreqs() {
	byte lin;
	byte col;
	long val = 0;

	// Run through the 11 channels
	for (lin = 0; lin < 11; lin ++) {
		_notes[lin] = 0;
		_notCol[lin] = 0;
		_notLin[lin] = 0;
		_notOn[lin] = false;
	}

	// Run through the 25 lines
	for (lin = 0; lin < 25; lin ++) {
		// Run through the 12 columns
		for (col = 0; col < 12; col ++) {
			if (!col)
				val = (((0x2710L + lin * 0x18) * 0xCB78 / 0x3D090) << 0xE) *
					9 / 0x1B503;
			_freqs[lin][col] = (short)((val + 4) >> 3);
			val = val * 0x6A / 0x64;
		}
	}
}

void AdLib::reset() {
	_first = true;
	OPLResetChip(_opl);
	_samplesTillPoll = 0;

	setFreqs();
	// Set frequencies and octave to 0; notes off
	for (int i = 0; i < 9; i++) {
		writeOPL(0xA0 | i, 0);
		writeOPL(0xB0 | i, 0);
		writeOPL(0xE0 | _operators[i]     , 0);
		writeOPL(0xE0 |(_operators[i] + 3), 0);
	}

	// Authorize the control of the waveformes
	writeOPL(0x01, 0x20);
}

void AdLib::setVoices() {
	// Definitions of the 9 instruments
	for (int i = 0; i < 9; i++)
		setVoice(i, i, true);
}

void AdLib::setVoice(byte voice, byte instr, bool set) {
	int i;
	int j;
	uint16 strct[27];
	byte channel;
	byte *dataPtr;

	// i = 0 :  0  1  2  3  4  5  6  7  8  9 10 11 12 26
	// i = 1 : 13 14 15 16 17 18 19 20 21 22 23 24 25 27
	for (i = 0; i < 2; i++) {
		dataPtr = _data + 3 + instr * 0x38 + i * 0x1A;
		for (j = 0; j < 27; j++) {
			strct[j] = READ_LE_UINT16(dataPtr);
			dataPtr += 2;
		}
		channel = _operators[voice] + i * 3;
		writeOPL(0xBD, 0x00);
		writeOPL(0x08, 0x00);
		writeOPL(0x40 | channel, ((strct[0] & 3) << 6) | (strct[8] & 0x3F));
		if (!i)
			writeOPL(0xC0 | voice,
					((strct[2] & 7) << 1) | (1 - (strct[12] & 1)));
		writeOPL(0x60 | channel, ((strct[3] & 0xF) << 4) | (strct[6] & 0xF));
		writeOPL(0x80 | channel, ((strct[4] & 0xF) << 4) | (strct[7] & 0xF));
		writeOPL(0x20 | channel, ((strct[9] & 1) << 7) |
			((strct[10] & 1) << 6) | ((strct[5] & 1) << 5) |
			((strct[11] & 1) << 4) |  (strct[1] & 0xF));
		if (!i)
			writeOPL(0xE0 | channel, (strct[26] & 3));
		else
			writeOPL(0xE0 | channel, (strct[14] & 3));
		if (i && set)
			writeOPL(0x40 | channel, 0);
	}
}

void AdLib::setKey(byte voice, byte note, bool on, bool spec) {
	short freq = 0;
	short octa = 0;

	// Instruction AX
	if (spec) {
		// 0x7F donne 0x16B;
		//     7F
		// <<   7 =  3F80
		// + E000 = 11F80
		// & FFFF =  1F80
		// *   19 = 31380
		// / 2000 =    18 => Ligne 18h, colonne  0 => freq 16B

		// 0x3A donne 0x2AF;
		//     3A
		// <<   7 =  1D00
		// + E000 =  FD00 negatif
		// *   19 = xB500
		// / 2000 =    -2 => Ligne 17h, colonne -1

		//     2E
		// <<   7 =  1700
		// + E000 =  F700 negatif
		// *   19 = x1F00
		// / 2000 =
		short a;
		short lin;
		short col;

		a = (note << 7) + 0xE000; // Volontairement tronque
		a = (short)((long)a * 25 / 0x2000);
		if (a < 0) {
			col = - ((24 - a) / 25);
			lin = (-a % 25);
			if (lin)
				lin = 25 - lin;
		}
		else {
			col = a / 25;
			lin = a % 25;
		}

		_notCol[voice] = col;
		_notLin[voice] = lin;
		note = _notes[voice];
	}
	// Instructions 0X 9X 8X
	else {
		note -= 12;
		_notOn[voice] = on;
	}

	_notes[voice] = note;
	note += _notCol[voice];
	note = MIN((byte) 0x5F, note);
	octa = note / 12;
	freq = _freqs[_notLin[voice]][note - octa * 12];

	writeOPL(0xA0 + voice,  freq & 0xFF);
	writeOPL(0xB0 + voice, (freq >> 8) | (octa << 2) | 0x20 * on);

	if (!freq)
		warning("Voice %d, note %02X unknown\n", voice, note);
}

void AdLib::setVolume(byte voice, byte volume) {
	volume = 0x3F - (volume * 0x7E + 0x7F) / 0xFE;
	writeOPL(0x40 + _volRegNums[voice], volume);
}

void AdLib::pollMusic() {
	unsigned char instr;
	byte channel;
	byte note;
	byte volume;
	uint16 tempo;

	if ((_playPos > (_data + _dataSize)) && (_dataSize != 0xFFFFFFFF)) {
		_ended = true;
		return;
	}

	// First tempo, we'll ignore it...
	if (_first) {
		tempo = *(_playPos++);
		// Tempo on 2 bytes
		if (tempo & 0x80)
			tempo = ((tempo & 3) << 8) | *(_playPos++);
	}
	_first = false;

	// Instruction
	instr = *(_playPos++);
	channel = instr & 0x0F;

	switch (instr & 0xF0) {
		// Note on + Volume
		case 0x00:
			note = *(_playPos++);
			_pollNotes[channel] = note;
			setVolume(channel, *(_playPos++));
			setKey(channel, note, true, false);
			break;
		// Note on
		case 0x90:
			note = *(_playPos++);
			_pollNotes[channel] = note;
			setKey(channel, note, true, false);
			break;
		// Last note off
		case 0x80:
			note = _pollNotes[channel];
			setKey(channel, note, false, false);
			break;
		// Frequency on/off
		case 0xA0:
			note = *(_playPos++);
			setKey(channel, note, _notOn[channel], true);
			break;
		// Volume
		case 0xB0:
			volume = *(_playPos++);
			setVolume(channel, volume);
			break;
		// Program change
		case 0xC0:
			setVoice(channel, *(_playPos++), false);
			break;
		// Special
		case 0xF0:
			switch (instr & 0x0F) {
			case 0xF: // End instruction
				_ended = true;
				_samplesTillPoll = 0;
				return;
			default:
				warning("Unknown special command in ADL, stopping playback: %X",
						instr & 0x0F);
				_repCount = 0;
				_ended = true;
				break;
			}
			break;
		default:
			warning("Unknown command in ADL, stopping playback: %X",
					instr & 0xF0);
			_repCount = 0;
			_ended = true;
			break;
	}

	// Temporization
	tempo = *(_playPos++);
	// End tempo
	if (tempo == 0xFF) {
		_ended = true;
		return;
	}
	// Tempo on 2 bytes
	if (tempo & 0x80)
		tempo = ((tempo & 3) << 8) | *(_playPos++);
	if (!tempo)
		tempo ++;

	_samplesTillPoll = tempo * (_rate / 1000);
}

bool AdLib::load(const char *fileName) {
	Common::File song;

	unload();
	song.open(fileName);
	if (!song.isOpen())
		return false;

	_needFree = true;
	_dataSize = song.size();
	_data = new byte[_dataSize];
	song.read(_data, _dataSize);
	song.close();

	reset();
	setVoices();
	_playPos = _data + 3 + (_data[1] + 1) * 0x38;

	return true;
}

bool AdLib::load(byte *data, uint32 size, int index) {
	unload();
	_repCount = 0;

	_dataSize = size;
	_data = data;
	_index = index;

	reset();
	setVoices();
	_playPos = _data + 3 + (_data[1] + 1) * 0x38;

	return true;
}

void AdLib::unload() {
	_playing = false;
	_index = -1;

	if (_data && _needFree)
		delete[] _data;

	_needFree = false;
}

bool AdLib::isPlaying() const {
	return _playing;
}

bool AdLib::getRepeating() const {
	return _repCount != 0;
}

void AdLib::setRepeating(int32 repCount) {
	_repCount = repCount;
}

int AdLib::getIndex() const {
	return _index;
}

void AdLib::startPlay() {
	if (_data) _playing = true;
}

void AdLib::stopPlay() {
	Common::StackLock slock(_mutex);
	_playing = false;
}

} // End of namespace Gob
