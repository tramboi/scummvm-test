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

#include "sci/resource.h"
#include "sci/engine/selector.h"
#include "sci/engine/kernel.h"
#include "sci/engine/seg_manager.h"
#include "sci/sound/audio.h"

#include "common/system.h"
#include "common/file.h"

#include "sound/audiostream.h"
#include "sound/audiocd.h"
#include "sound/decoders/raw.h"
#include "sound/decoders/wave.h"

namespace Sci {

AudioPlayer::AudioPlayer(ResourceManager *resMan) : _resMan(resMan), _audioRate(11025),
		_syncResource(NULL), _syncOffset(0), _audioCdStart(0) {

	_mixer = g_system->getMixer();
}

AudioPlayer::~AudioPlayer() {
	stopAllAudio();
}

void AudioPlayer::stopAllAudio() {
	stopSoundSync();
	stopAudio();
	if (_audioCdStart > 0)
		audioCdStop();
}

int AudioPlayer::startAudio(uint16 module, uint32 number) {
	int sampleLen;
	Audio::AudioStream *audioStream = getAudioStream(number, module, &sampleLen);

	if (audioStream) {
		_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_audioHandle, audioStream);
		return sampleLen;
	}

	return 0;
}

void AudioPlayer::stopAudio() {
	_mixer->stopHandle(_audioHandle);
}

void AudioPlayer::pauseAudio() {
	_mixer->pauseHandle(_audioHandle, true);
}

void AudioPlayer::resumeAudio() {
	_mixer->pauseHandle(_audioHandle, false);
}

int AudioPlayer::getAudioPosition() {
	if (_mixer->isSoundHandleActive(_audioHandle))
		return _mixer->getSoundElapsedTime(_audioHandle) * 6 / 100; // return elapsed time in ticks
	else
		return -1; // Sound finished
}

enum SolFlags {
	kSolFlagCompressed = 1 << 0,
	kSolFlagUnknown    = 1 << 1,
	kSolFlag16Bit      = 1 << 2,
	kSolFlagIsSigned   = 1 << 3
};

// FIXME: Move this to sound/adpcm.cpp?
// Note that the 16-bit version is also used in coktelvideo.cpp
static const uint16 tableDPCM16[128] = {
	0x0000, 0x0008, 0x0010, 0x0020, 0x0030, 0x0040, 0x0050, 0x0060, 0x0070, 0x0080,
	0x0090, 0x00A0, 0x00B0, 0x00C0, 0x00D0, 0x00E0, 0x00F0, 0x0100, 0x0110, 0x0120,
	0x0130, 0x0140, 0x0150, 0x0160, 0x0170, 0x0180, 0x0190, 0x01A0, 0x01B0, 0x01C0,
	0x01D0, 0x01E0, 0x01F0, 0x0200, 0x0208, 0x0210, 0x0218, 0x0220, 0x0228, 0x0230,
	0x0238, 0x0240, 0x0248, 0x0250, 0x0258, 0x0260, 0x0268, 0x0270, 0x0278, 0x0280,
	0x0288, 0x0290, 0x0298, 0x02A0, 0x02A8, 0x02B0, 0x02B8, 0x02C0, 0x02C8, 0x02D0,
	0x02D8, 0x02E0, 0x02E8, 0x02F0, 0x02F8, 0x0300, 0x0308, 0x0310, 0x0318, 0x0320,
	0x0328, 0x0330, 0x0338, 0x0340, 0x0348, 0x0350, 0x0358, 0x0360, 0x0368, 0x0370,
	0x0378, 0x0380, 0x0388, 0x0390, 0x0398, 0x03A0, 0x03A8, 0x03B0, 0x03B8, 0x03C0,
	0x03C8, 0x03D0, 0x03D8, 0x03E0, 0x03E8, 0x03F0, 0x03F8, 0x0400, 0x0440, 0x0480,
	0x04C0, 0x0500, 0x0540, 0x0580, 0x05C0, 0x0600, 0x0640, 0x0680, 0x06C0, 0x0700,
	0x0740, 0x0780, 0x07C0, 0x0800, 0x0900, 0x0A00, 0x0B00, 0x0C00, 0x0D00, 0x0E00,
	0x0F00, 0x1000, 0x1400, 0x1800, 0x1C00, 0x2000, 0x3000, 0x4000
};

static const byte tableDPCM8[8] = {0, 1, 2, 3, 6, 10, 15, 21};

static void deDPCM16(byte *soundBuf, Common::SeekableReadStream &audioStream, uint32 n) {
	int16 *out = (int16 *) soundBuf;

	int32 s = 0;
	for (uint32 i = 0; i < n; i++) {
		byte b = audioStream.readByte();
		if (b & 0x80)
			s -= tableDPCM16[b & 0x7f];
		else
			s += tableDPCM16[b];

		s = CLIP<int32>(s, -32768, 32767);
		*out++ = TO_LE_16(s);
	}
}

static void deDPCM8Nibble(byte *soundBuf, int32 &s, byte b) {
	if (b & 8) {
#ifdef ENABLE_SCI32
		// SCI2.1 reverses the order of the table values here
		if (getSciVersion() >= SCI_VERSION_2_1)
			s -= tableDPCM8[b & 7];
		else
#endif
			s -= tableDPCM8[7 - (b & 7)];
	} else
		s += tableDPCM8[b & 7];
	s = CLIP<int32>(s, 0, 255);
	*soundBuf = s;
}

static void deDPCM8(byte *soundBuf, Common::SeekableReadStream &audioStream, uint32 n) {
	int32 s = 0x80;

	for (uint i = 0; i < n; i++) {
		byte b = audioStream.readByte();

		deDPCM8Nibble(soundBuf++, s, b >> 4);
		deDPCM8Nibble(soundBuf++, s, b & 0xf);
	}
}

// Sierra SOL audio file reader
// Check here for more info: http://wiki.multimedia.cx/index.php?title=Sierra_Audio
static bool readSOLHeader(Common::SeekableReadStream *audioStream, int headerSize, uint32 &size, uint16 &audioRate, byte &audioFlags) {
	if (headerSize != 11 && headerSize != 12) {
		warning("SOL audio header of size %i not supported", headerSize);
		return false;
	}

	audioStream->readUint32LE();			// skip "SOL" + 0 (4 bytes)
	audioRate = audioStream->readUint16LE();
	audioFlags = audioStream->readByte();

	size = audioStream->readUint32LE();
	return true;
}

static byte *readSOLAudio(Common::SeekableReadStream *audioStream, uint32 &size, byte audioFlags, byte &flags) {
	byte *buffer;

	// Convert the SOL stream flags to our own format
	flags = 0;
	if (audioFlags & kSolFlag16Bit)
		flags |= Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN;

	if (!(audioFlags & kSolFlagIsSigned))
		flags |= Audio::FLAG_UNSIGNED;

	if (audioFlags & kSolFlagCompressed) {
		buffer = (byte *)malloc(size * 2);

		if (audioFlags & kSolFlag16Bit)
			deDPCM16(buffer, *audioStream, size);
		else
			deDPCM8(buffer, *audioStream, size);

		size *= 2;
	} else {
		// We assume that the sound data is raw PCM
		buffer = (byte *)malloc(size);
		audioStream->read(buffer, size);
	}

	return buffer;
}

Audio::RewindableAudioStream *AudioPlayer::getAudioStream(uint32 number, uint32 volume, int *sampleLen) {
	Audio::RewindableAudioStream *audioStream = 0;
	uint32 size = 0;
	byte *data = 0;
	byte flags = 0;
	Sci::Resource *audioRes;

	if (volume == 65535) {
		audioRes = _resMan->findResource(ResourceId(kResourceTypeAudio, number), false);
		if (!audioRes) {
			warning("Failed to find audio entry %i", number);
			return NULL;
		}
	} else {
		audioRes = _resMan->findResource(ResourceId(kResourceTypeAudio36, volume, number), false);
		if (!audioRes) {
			warning("Failed to find audio entry (%i, %i, %i, %i, %i)", volume, (number >> 24) & 0xff,
					(number >> 16) & 0xff, (number >> 8) & 0xff, number & 0xff);
			return NULL;
		}
	}

	byte audioFlags;

	if (audioRes->_headerSize > 0) {
		// SCI1.1
		Common::MemoryReadStream headerStream(audioRes->_header, audioRes->_headerSize, DisposeAfterUse::NO);

		if (readSOLHeader(&headerStream, audioRes->_headerSize, size, _audioRate, audioFlags)) {
			Common::MemoryReadStream dataStream(audioRes->data, audioRes->size, DisposeAfterUse::NO);
			data = readSOLAudio(&dataStream, size, audioFlags, flags);
		}
	} else {
		// SCI1 or WAVE file
		if (audioRes->size > 4) {
			if (memcmp(audioRes->data, "RIFF", 4) == 0) {
				// WAVE detected
				Common::MemoryReadStream *waveStream = new Common::MemoryReadStream(audioRes->data, audioRes->size, DisposeAfterUse::NO);
				audioStream = Audio::makeWAVStream(waveStream, DisposeAfterUse::YES);
			}
		}
		if (!audioStream) {
			// SCI1 raw audio
			size = audioRes->size;
			data = (byte *)malloc(size);
			assert(data);
			memcpy(data, audioRes->data, size);
			flags = Audio::FLAG_UNSIGNED;
		}
	}

	if (data)
		audioStream = Audio::makeRawStream(data, size, _audioRate, flags);

	if (audioStream) {
		*sampleLen = (flags & Audio::FLAG_16BITS ? size >> 1 : size) * 60 / _audioRate;
		return audioStream;
	}

	return NULL;
}

void AudioPlayer::setSoundSync(ResourceId id, reg_t syncObjAddr, SegManager *segMan) {
	_syncResource = _resMan->findResource(id, 1);
	_syncOffset = 0;

	if (_syncResource) {
		PUT_SEL32V(segMan, syncObjAddr, SELECTOR(syncCue), 0);
	} else {
		warning("setSoundSync: failed to find resource %s", id.toString().c_str());
		// Notify the scripts to stop sound sync
		PUT_SEL32V(segMan, syncObjAddr, SELECTOR(syncCue), SIGNAL_OFFSET);
	}
}

void AudioPlayer::doSoundSync(reg_t syncObjAddr, SegManager *segMan) {
	if (_syncResource && (_syncOffset < _syncResource->size - 1)) {
		int16 syncCue = -1;
		int16 syncTime = (int16)READ_LE_UINT16(_syncResource->data + _syncOffset);

		_syncOffset += 2;

		if ((syncTime != -1) && (_syncOffset < _syncResource->size - 1)) {
			syncCue = (int16)READ_LE_UINT16(_syncResource->data + _syncOffset);
			_syncOffset += 2;
		}

		PUT_SEL32V(segMan, syncObjAddr, SELECTOR(syncTime), syncTime);
		PUT_SEL32V(segMan, syncObjAddr, SELECTOR(syncCue), syncCue);
	}
}

void AudioPlayer::stopSoundSync() {
	if (_syncResource) {
		_resMan->unlockResource(_syncResource);
		_syncResource = NULL;
	}
}

int AudioPlayer::audioCdPlay(int track, int start, int duration) {
	if (getSciVersion() == SCI_VERSION_1_1) {
		// King's Quest VI CD Audio format
		_audioCdStart = g_system->getMillis();

		// Subtract one from track. KQ6 starts at track 1, while ScummVM
		// ignores the data track and considers track 2 to be track 1.
		AudioCD.play(track - 1, 1, start, duration);
		return 1;
	} else {
		// Jones in the Fast Lane CD Audio format
		uint32 length = 0;

		audioCdStop();

		Common::File audioMap;
		if(!audioMap.open("cdaudio.map"))
			error("Could not open cdaudio.map");

		while (audioMap.pos() < audioMap.size()) {
			uint16 res = audioMap.readUint16LE();
			uint32 startFrame = audioMap.readUint16LE();
			startFrame += audioMap.readByte() << 16;
			audioMap.readByte(); // Unknown, always 0x20
			length = audioMap.readUint16LE();
			length += audioMap.readByte() << 16;
			audioMap.readByte(); // Unknown, always 0x00

			// Jones uses the track as the resource value in the map
			if (res == track) {
				AudioCD.play(1, 1, startFrame, length);
				_audioCdStart = g_system->getMillis();
				break;
			}
		}

		audioMap.close();

		return length * 60 / 75; // return sample length in ticks
	}
}

void AudioPlayer::audioCdStop() {
	_audioCdStart = 0;
	AudioCD.stop();
}

void AudioPlayer::audioCdUpdate() {
	AudioCD.updateCD();
}

int AudioPlayer::audioCdPosition() {
	// Return -1 if the sample is done playing. Converting to frames to compare.
	if (((g_system->getMillis() - _audioCdStart) * 75 / 1000) >= (uint32)AudioCD.getStatus().duration)
		return -1;

	// Return the position otherwise (in ticks).
	return (g_system->getMillis() - _audioCdStart) * 60 / 1000;
}

} // End of namespace Sci
