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

#include "kyra/sound.h"
#include "kyra/lol.h"
#include "kyra/resource.h"

namespace Kyra {

#define LOL_VOICE_HANDLE "LoL_VOICE"

bool LoLEngine::snd_playCharacterSpeech(int id, int8 speaker, int) {
	if (!_speechFlag)
		return false;

	if (speaker < 65) {
		if (_characters[speaker].flags & 1)
			speaker = (int) _characters[speaker].name[0];
		else
			speaker = 0;
	}

	if (_lastSpeechId == id && speaker == _lastSpeaker)
		return true;

	_lastSpeechId = id;
	_lastSpeaker = speaker;
	_nextSpeechId = _nextSpeaker = -1;
	
	char pattern1[8];
	char pattern2[5];
	char file1[13];
	char file2[13];
	char file3[13];
	file3[0] = 0;

	Audio::AudioStream *as = 0;
	Common::List<Audio::AudioStream *> newSpeechList;

	snprintf(pattern2, sizeof(pattern2), "%02d", id & 0x4000 ? 0 : _curTlkFile);

	if (id & 0x4000) {
		snprintf(pattern1, sizeof(pattern1), "%03X", id & 0x3fff);
	} else if (id < 1000) {
		snprintf(pattern1, sizeof(pattern1), "%03d", id);
	} else {
		snprintf(file3, sizeof(file3), "@%04d%c.%s", id - 1000, (char)speaker, pattern2);
		if ((as = _sound->getVoiceStream(file3)) != 0)
			newSpeechList.push_back(as);
	}

	if (!file3[0]) {
		for (char i = 0; ; i++) {
			char symbol = '0' + i;
			snprintf(file1, sizeof(file1), "%s%c%c.%s", pattern1, (char)speaker, symbol, pattern2);
			snprintf(file2, sizeof(file2), "%s%c%c.%s", pattern1, '_', symbol, pattern2);
			if ((as = _sound->getVoiceStream(file1)) != 0)
				newSpeechList.push_back(as);
			else if ((as = _sound->getVoiceStream(file2)) != 0)
				newSpeechList.push_back(as);
			else
				break;
		}
	}

	if (newSpeechList.empty())
		return false;

	while (_sound->voiceIsPlaying(LOL_VOICE_HANDLE))
		delay(_tickLength, true, false);

	while (_sound->allVoiceChannelsPlaying())
		delay(_tickLength, false, true);

	_activeVoiceFileTotalTime = 0;
	for (Common::List<Audio::AudioStream *>::iterator i = _speechList.begin(); i != _speechList.end(); ++i)
		delete *i;
	_speechList.clear();
	_speechList = newSpeechList;

	for (Common::List<Audio::AudioStream *>::const_iterator i = _speechList.begin(); i != _speechList.end(); ++i)
		_activeVoiceFileTotalTime += (*i)->getTotalPlayTime();

	//_activeVoiceFileTotalTime = _sound->voicePlay(_activeVoiceFile, 255, false, false);
	_sound->playVoiceStream(*_speechList.begin(), LOL_VOICE_HANDLE);
	_speechList.pop_front();

	//for (Common::List<const char*>::iterator i = _speechPlayList.begin(); i != _speechPlayList.end(); ++i)
	//	_activeVoiceFileTotalTime += _sound->getTotalPlayTime(*i, false);
	/*if (_speechPlayList.begin() != _speechPlayList.end())
		_activeVoiceFileTotalTime |= 0x80000000;*/

	if (!_activeVoiceFileTotalTime)
		return false;

	_tim->_abortFlag = 0;

	return true;
}

int LoLEngine::snd_updateCharacterSpeech() {
	if (_sound->voiceIsPlaying(LOL_VOICE_HANDLE))
		return 2;

	if (_speechList.begin() != _speechList.end()) {
		_sound->playVoiceStream(*_speechList.begin(), LOL_VOICE_HANDLE);
		_speechList.pop_front();
		return 2;

	} else if (_nextSpeechId != -1) {
		_lastSpeechId = _lastSpeaker = -1;
		_activeVoiceFileTotalTime = 0;
		if (snd_playCharacterSpeech(_nextSpeechId, _nextSpeaker, 0))
			return 2;
	}

	_lastSpeechId = _lastSpeaker = -1;
	_activeVoiceFileTotalTime = 0;

	return 0;
}

void LoLEngine::snd_stopSpeech(bool setFlag) {
	if (!_sound->voiceIsPlaying(LOL_VOICE_HANDLE))
		return;

	//_dlgTimer = 0;
	_sound->voiceStop(LOL_VOICE_HANDLE);
	_activeVoiceFileTotalTime = 0;
	_nextSpeechId = _nextSpeaker = -1;

	for (Common::List<Audio::AudioStream *>::iterator i = _speechList.begin(); i != _speechList.end(); ++i)
		delete *i;
	_speechList.clear();

	if (setFlag)
		_tim->_abortFlag = 1;
}

void LoLEngine::snd_playSoundEffect(int track, int volume) {
	if (track == 1 && (_lastSfxTrack == -1 || _lastSfxTrack == 1))
		return;

	_lastSfxTrack = track;
	if (track == -1)
		return;

	int16 volIndex = (int16)READ_LE_UINT16(&_ingameSoundIndex[track * 2 + 1]);

	if (volIndex > 0)
		volume = (volIndex * volume) >> 8;
	else if (volIndex < 0)
		volume = -volIndex;

	// volume TODO
	volume = 254 - volume;

	int16 vocIndex = (int16)READ_LE_UINT16(&_ingameSoundIndex[track * 2]);

	bool hasVocFile = false;
	if (vocIndex != -1) {
		if (scumm_stricmp(_ingameSoundList[vocIndex], "EMPTY"))
			hasVocFile = true;
	}

	if (hasVocFile) {
		_sound->voicePlay(_ingameSoundList[vocIndex], volume & 0xff, true);
	} else if (_flags.platform == Common::kPlatformPC) {
		if (_sound->getSfxType() == Sound::kMidiMT32)
			track = track < _ingameMT32SoundIndexSize ? _ingameMT32SoundIndex[track] - 1 : -1;
		else if (_sound->getSfxType() == Sound::kMidiGM)
			track = track < _ingameGMSoundIndexSize ? _ingameGMSoundIndex[track] - 1: -1;

		if (track == 168)
			track = 167;

		if (track != -1)
			KyraEngine_v1::snd_playSoundEffect(track, volume);
	}
}

void LoLEngine::snd_processEnvironmentalSoundEffect(int soundId, int block) {
	if (!_sound->sfxEnabled())
		return;

	if (_environmentSfx)
		snd_playSoundEffect(_environmentSfx, _environmentSfxVol);

	int dist = 0;
	if (block) {
		dist = getMonsterDistance(_currentBlock, block);
		if (dist > _envSfxDistThreshold) {
			_environmentSfx = 0;
			return;
		}
	}

	_environmentSfx = soundId;
	_environmentSfxVol = (15 - ((block || dist < 2) ? dist : 0)) << 4;

	if (block != _currentBlock) {
		static const int8 blockShiftTable[] = { -32, -31, 1, 33, 32, 31, -1, -33 };
		uint16 cbl = _currentBlock;

		for (int i = 3; i > 0; i--) {
			int dir = calcMonsterDirection(cbl & 0x1f, cbl >> 5, block & 0x1f, block >> 5);
			cbl += blockShiftTable[dir];
			if (cbl != block) {
				if (testWallFlag(cbl, 0, 1))
					_environmentSfxVol >>= 1;
			}
		}
	}

	if (!soundId || _sceneUpdateRequired)
		return;

	snd_processEnvironmentalSoundEffect(0, 0);
}

void LoLEngine::snd_queueEnvironmentalSoundEffect(int soundId, int block) {
	if (_envSfxUseQueue && _envSfxNumTracksInQueue < 10) {
		_envSfxQueuedTracks[_envSfxNumTracksInQueue] = soundId;
		_envSfxQueuedBlocks[_envSfxNumTracksInQueue] = block;
		_envSfxNumTracksInQueue++;
	} else {
		snd_processEnvironmentalSoundEffect(soundId, block);
	}
}

void LoLEngine::snd_playQueuedEffects() {
	for (int i = 0; i < _envSfxNumTracksInQueue; i++)
		snd_processEnvironmentalSoundEffect(_envSfxQueuedTracks[i], _envSfxQueuedBlocks[i]);
	_envSfxNumTracksInQueue = 0;
}

void LoLEngine::snd_loadSoundFile(int track) {
	if (_sound->musicEnabled()) {
		char filename[13];
		int t = (track - 250) * 3;

		if (_curMusicFileIndex != _musicTrackMap[t] || _curMusicFileExt != (char)_musicTrackMap[t + 1]) {
			snd_stopMusic();
			snprintf(filename, sizeof(filename), "LORE%02d%c", _musicTrackMap[t], (char)_musicTrackMap[t + 1]);
			_sound->loadSoundFile(filename);
			_curMusicFileIndex = _musicTrackMap[t];
			_curMusicFileExt = (char)_musicTrackMap[t + 1];
		} else {
			snd_stopMusic();
		}
	} else {
		//XXX
	}
}

int LoLEngine::snd_playTrack(int track) {
	if (track == -1)
		return _lastMusicTrack;

	int res = _lastMusicTrack;
	_lastMusicTrack = track;

	if (_sound->musicEnabled()) {
		snd_loadSoundFile(track);
		int t = (track - 250) * 3;
		_sound->playTrack(_musicTrackMap[t + 2]);
	}

	return res;
}

int LoLEngine::snd_stopMusic() {
	if (_sound->musicEnabled()) {
		if (_sound->isPlaying()) {
			_sound->beginFadeOut();
			_system->delayMillis(3 * _tickLength);
		}

		_sound->haltTrack();
	}
	return snd_playTrack(-1);
}

} // end of namespace Kyra
