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

#include "sound/audiostream.h"
#include "common/config-manager.h"

#include "sci/sci.h"
#include "sci/console.h"
#include "sci/resource.h"
#include "sci/engine/kernel.h"
#include "sci/engine/state.h"
#include "sci/sfx/midiparser.h"
#include "sci/sfx/music.h"
#include "sci/sfx/softseq/pcjr.h"

namespace Sci {

SciMusic::SciMusic(ResourceManager *resMan, SegManager *segMan, SciVersion soundVersion)
	: _soundVersion(soundVersion), _soundOn(true) {

	// Reserve some space in the playlist, to avoid expensive insertion
	// operations
	_playList.reserve(10);
}

SciMusic::~SciMusic() {
	if (_pMidiDrv) {
		_pMidiDrv->close();
		delete _pMidiDrv;
	}
}

void SciMusic::init() {
	// system init
	_pMixer = g_system->getMixer();
	_pMixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt(
			"sfx_volume"));
	_pMixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType,
			ConfMan.getInt("speech_volume"));
	_pMixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType,
			ConfMan.getInt("music_volume"));
	// SCI sound init
	_dwTempo = 0;

	_midiType = MidiDriver::detectMusicDriver(MDT_MIDI | MDT_ADLIB | MDT_PCSPK);

	// Sanity check
	if (_midiType != MD_ADLIB && _midiType != MD_PCJR && _midiType != MD_PCSPK) {
		warning("Unhandled MIDI type, switching to Adlib");
		_midiType = MD_ADLIB;
	}

	switch (_midiType) {
	case MD_ADLIB:
		// FIXME: There's no Amiga sound option, so we hook it up to Adlib
		if (((SciEngine *)g_engine)->getPlatform() == Common::kPlatformAmiga)
			_pMidiDrv = MidiPlayer_Amiga_create();
		else
			_pMidiDrv = MidiPlayer_Adlib_create();
		break;
	case MD_PCJR:
		_pMidiDrv = new MidiPlayer_PCJr();
		break;
	case MD_PCSPK:
		_pMidiDrv = new MidiPlayer_PCSpeaker();
		break;
	default:
		break;
	}

	if (_pMidiDrv) {
		_pMidiDrv->open();
		_pMidiDrv->setTimerCallback(this, &miditimerCallback);
		_dwTempo = _pMidiDrv->getBaseTempo();
	} else
		warning("Can't initialise music driver");
	_bMultiMidi = ConfMan.getBool("multi_midi");
}

void SciMusic::clearPlayList() {
	_pMixer->stopAll();

	_mutex.lock();
	while (!_playList.empty()) {
		soundStop(_playList[0]);
		soundKill(_playList[0]);
	}
	_mutex.unlock();
}

void SciMusic::miditimerCallback(void *p) {
	SciMusic *aud = (SciMusic *)p;

	Common::StackLock lock(aud->_mutex);
	aud->onTimer();
}

void SciMusic::soundSetSoundOn(bool soundOnFlag) {
	Common::StackLock lock(_mutex);

	_soundOn = soundOnFlag;
	_pMidiDrv->playSwitch(soundOnFlag);
}

uint16 SciMusic::soundGetVoices() {
	switch (_midiType) {
	case MD_PCSPK:
		return 1;
	case MD_PCJR:
		return 3;
	case MD_ADLIB:
		return 8;
	case MD_MT32:
		return 16;
	default:
		return 1;
	}
}

MusicEntry *SciMusic::getSlot(reg_t obj) {
	Common::StackLock lock(_mutex);

	const MusicList::iterator end = _playList.end();
	for (MusicList::iterator i = _playList.begin(); i != end; ++i) {
		if ((*i)->soundObj == obj)
			return *i;
	}

	return NULL;
}

void SciMusic::setReverb(byte reverb) {
	_reverb = reverb;

	// TODO: actually set reverb for MT-32

	// A good test case for this are the first two rooms in Longbow:
	// reverb is set for the first room (the cave) and is subsequently
	// cleared when Robin exits the cave
}

void SciMusic::resetDriver() {
	Common::StackLock lock(_mutex);

	_pMidiDrv->close();
	_pMidiDrv->open();
	_pMidiDrv->setTimerCallback(this, &miditimerCallback);
}

static int f_compare(const void *arg1, const void *arg2) {
	return ((const MusicEntry *)arg2)->prio - ((const MusicEntry *)arg1)->prio;
}

void SciMusic::sortPlayList() {
	MusicEntry ** pData = _playList.begin();
	qsort(pData, _playList.size(), sizeof(MusicEntry *), &f_compare);
}

#if 0
void SciMusic::patchSysEx(byte * addr, byte *pdata, int len) {
	byte *buff = new byte[7 + len + 1];
	uint16 chk = 0;
	int i;

	buff[0] = 0x41;
	buff[1] = 0x10;
	buff[2] = 0x16;
	buff[3] = 0x12;
	buff[4] = addr[0];
	buff[5] = addr[1];
	buff[6] = addr[2];
	for (i = 0; i < len; i++) {
		buff[7 + i] = pdata[i];
		chk += pdata[i];
	}
	chk += addr[0] + addr[1] + addr[2];
	buff[7 + i] = 128 - chk % 128;
	_pMidiDrv->sysEx(buff, len + 8);
	delete[] buff;
}

void SciMusic::patchUpdateAddr(byte *addr, int len) {
	addr[2] += len;
	if (addr[2] >= 0x7F) {
		addr[1]++;
		addr[2] -= 0x80;
	}
}
#endif

// FIXME: This should be done at the driver level
#if 0
void SciMusic::loadPatch() {
	if (_midiType == MD_MT32)
		loadPatchMT32();
}
#endif

#if 0
// currently loads patch 1.pat for Roland/MT-32 device
void SciMusic::loadPatchMT32() {
	//byte sysText[] = { 0x20, 0, 0 };
	byte sysMem[] = { 0x5, 0, 0 }; // patch memory
	byte sysRhytm[] = { 0x3, 0x1, 0x10 }; // rhytm
	byte sysMsg3[15] = { 0x41, 0x10, 0x16, 0x12, 0x52, 0, 0xA, 0x16, 0x16,
			0x16, 0x16, 0x16, 0x16, 0x20, 0x80 };
	byte sysTimbre[] = { 0x8, 0, 0 }; // timbre memory
	byte sysSystem[] = { 0x10, 0, 4 }; // partial reserve & midi channel
	byte arr[3][11];

	Resource *res = ((SciEngine *)g_engine)->getResourceManager()->findResource(ResourceId(kResourceTypePatch, 1), 0);

	if (res) {
		byte *pData = res->data, *p;
		// welcome message
		//patchSysEx(sysText, pData + 20, 20);
		// reading reverb mode, time and level
		p = pData + 74;
		for (int i = 0; i < 11; i++) {
			arr[0][i] = *p++;
			arr[1][i] = *p++;
			arr[2][i] = *p++;
		}
		// sub_657 - patch memory
		for (int i = 0; i < 48; i++) {
			patchSysEx(sysMem, p, 8);
			patchUpdateAddr(sysMem, 8);
			p += 8;
		}
		// sub_696 - timbre
		byte dl = *p++, cl = 0;
		while (dl--) {
			patchSysEx(sysTimbre, p, 14); // common area
			patchUpdateAddr(sysTimbre, 14);
			patchSysEx(sysTimbre, p + 14, 58);// partial 1
			patchUpdateAddr(sysTimbre, 58);
			patchSysEx(sysTimbre, p + 72, 58);// partial 2
			patchUpdateAddr(sysTimbre, 58);
			patchSysEx(sysTimbre, p + 130, 58);// partial 3
			patchUpdateAddr(sysTimbre, 58);
			patchSysEx(sysTimbre, p + 188, 58);// partial 4
			patchUpdateAddr(sysTimbre, 58);
			p += 246;
			cl += 2;
			sysTimbre[1] = cl;
			sysTimbre[2] = 0;
		}
		// patch memory or rhytm
		uint16 flag = READ_BE_UINT16(p);
		p += 2;
		if (flag == 0xABCD) {
			// sub_657
			for (int i = 0; i < 48; i++) {
				patchSysEx(sysMem, p, 8);
				patchUpdateAddr(sysMem, 8);
				p += 8;
			}
		} else if (flag == 0xDCBA) {
			// sub_756
			for (int i = 0; i < 64; i++) {
				patchSysEx(sysRhytm, p, 4);
				patchUpdateAddr(sysRhytm, 4);
				p += 4;
			}
			patchSysEx(sysSystem, p, 18);
		}
		// after-init text message
		//patchSysEx(sysText, pData, 20);
		// some final sysex
		_pMidiDrv->sysEx(sysMsg3, 15);
		// releasing patch resource
		//g_sci->ResMgr.ResUnload(SCI_RES_PATCH, 1);
	debug("MT-32 patch loaded");
	}
}
#endif


void SciMusic::soundInitSnd(MusicEntry *pSnd) {
	SoundResource::Track *track = NULL;
	int channelFilterMask = 0;

	switch (_midiType) {
	case MD_PCSPK:
		track = pSnd->soundRes->getTrackByType(SoundResource::TRACKTYPE_SPEAKER);
		break;
	case MD_PCJR:
		track = pSnd->soundRes->getTrackByType(SoundResource::TRACKTYPE_TANDY);
		break;
	case MD_ADLIB:
		track = pSnd->soundRes->getTrackByType(SoundResource::TRACKTYPE_ADLIB);
		break;
	case MD_MT32:
		track = pSnd->soundRes->getTrackByType(SoundResource::TRACKTYPE_MT32);
		break;
	default:
		break;
	}

	// attempting to select default MT-32/Roland track
	if (!track)
		track = pSnd->soundRes->getTrackByType(SoundResource::TRACKTYPE_MT32);
	if (track) {
		// if MIDI device is selected but there is no digital track in sound resource
		// try to use adlib's digital sample if possible
		if (_midiType <= MD_MT32 && track->digitalChannelNr == -1 && _bMultiMidi) {
			if (pSnd->soundRes->getTrackByType(SoundResource::TRACKTYPE_ADLIB)->digitalChannelNr != -1)
				track = pSnd->soundRes->getTrackByType(SoundResource::TRACKTYPE_ADLIB);
		}
		// play digital sample
		if (track->digitalChannelNr != -1) {
			byte *channelData = track->channels[track->digitalChannelNr].data;
			if (pSnd->pStreamAud)
				delete pSnd->pStreamAud;
			pSnd->pStreamAud = Audio::makeLinearInputStream(channelData, track->digitalSampleSize, track->digitalSampleRate,
					Audio::Mixer::FLAG_UNSIGNED, 0, 0);
			pSnd->soundType = Audio::Mixer::kSFXSoundType;
			pSnd->hCurrentAud = Audio::SoundHandle();
		} else {
			// play MIDI track
			_mutex.lock();
			pSnd->soundType = Audio::Mixer::kMusicSoundType;
			if (pSnd->pMidiParser == NULL) {
				pSnd->pMidiParser = new MidiParser_SCI();
				pSnd->pMidiParser->setMidiDriver(_pMidiDrv);
				pSnd->pMidiParser->setTimerRate(_dwTempo);
				pSnd->pMidiParser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);
			}

			pSnd->pauseCounter = 0;

			// Find out what channels to filter for SCI0
			channelFilterMask = pSnd->soundRes->getChannelFilterMask(_pMidiDrv->getPlayMask(_soundVersion));
			pSnd->pMidiParser->loadMusic(track, pSnd, channelFilterMask, _soundVersion);

			// Fast forward to the last position and perform associated events when loading
			pSnd->pMidiParser->jumpToTick(pSnd->ticker, true);
			_mutex.unlock();
		}
	}
}

void SciMusic::onTimer() {
	const MusicList::iterator end = _playList.end();
	for (MusicList::iterator i = _playList.begin(); i != end; ++i)
		(*i)->onTimer(_soundVersion);
}

void SciMusic::soundPlay(MusicEntry *pSnd) {
	_mutex.lock();

	uint sz = _playList.size(), i;
	// searching if sound is already in _playList
	for (i = 0; i < sz && _playList[i] != pSnd; i++)
		;
	if (i == sz) {// not found
		_playList.push_back(pSnd);
		sortPlayList();
	}
	
	_mutex.unlock();	// unlock to perform mixer-related calls

	if (pSnd->pStreamAud && !_pMixer->isSoundHandleActive(pSnd->hCurrentAud)) {
		SegManager *segMan = ((SciEngine *)g_engine)->getEngineState()->_segMan;	// HACK
		uint16 loop = GET_SEL32V(segMan, pSnd->soundObj, loop);
		// Are we supposed to loop the stream?
		if (loop > 1)
			pSnd->pStreamAud->setNumLoops(loop);
		else
			pSnd->pStreamAud->setNumLoops(1);
		_pMixer->playInputStream(pSnd->soundType, &pSnd->hCurrentAud,
				pSnd->pStreamAud, -1, pSnd->volume, 0, false);
	} else {
		_mutex.lock();
		if (pSnd->pMidiParser) {
			pSnd->pMidiParser->setVolume(pSnd->volume);
			if (pSnd->status == kSoundStopped)
				pSnd->pMidiParser->jumpToTick(0);
		}
		_mutex.unlock();
	}

	pSnd->status = kSoundPlaying;
}

void SciMusic::soundStop(MusicEntry *pSnd) {
	pSnd->status = kSoundStopped;
	if (pSnd->pStreamAud)
		_pMixer->stopHandle(pSnd->hCurrentAud);

	_mutex.lock();
	if (pSnd->pMidiParser)
		pSnd->pMidiParser->stop();
	_mutex.unlock();
}

void SciMusic::soundSetVolume(MusicEntry *pSnd, byte volume) {
	assert(volume <= MUSIC_VOLUME_MAX);
	if (pSnd->pStreamAud) {
		_pMixer->setChannelVolume(pSnd->hCurrentAud, volume * 2); // Mixer is 0-255, SCI is 0-127
	} else if (pSnd->pMidiParser) {
		_mutex.lock();
		pSnd->pMidiParser->setVolume(volume);
		_mutex.unlock();
	}
}

void SciMusic::soundSetPriority(MusicEntry *pSnd, byte prio) {
	Common::StackLock lock(_mutex);

	pSnd->prio = prio;
	sortPlayList();
}

void SciMusic::soundKill(MusicEntry *pSnd) {
	pSnd->status = kSoundStopped;

	_mutex.lock();
	if (pSnd->pMidiParser) {
		pSnd->pMidiParser->unloadMusic();
		delete pSnd->pMidiParser;
		pSnd->pMidiParser = NULL;
	}
	_mutex.unlock();

	if (pSnd->pStreamAud) {
		_pMixer->stopHandle(pSnd->hCurrentAud);
		pSnd->pStreamAud = NULL;
	}

	_mutex.lock();
	uint sz = _playList.size(), i;
	// Remove sound from playlist
	for (i = 0; i < sz; i++) {
		if (_playList[i] == pSnd) {
			delete _playList[i]->soundRes;
			delete _playList[i];
			_playList.remove_at(i);
			break;
		}
	}
	_mutex.unlock();
}

void SciMusic::soundPause(MusicEntry *pSnd) {
	pSnd->pauseCounter++;
	if (pSnd->status != kSoundPlaying)
		return;
	pSnd->status = kSoundPaused;
	if (pSnd->pStreamAud) {
		_pMixer->pauseHandle(pSnd->hCurrentAud, true);
	} else {
		_mutex.lock();
		if (pSnd->pMidiParser)
			pSnd->pMidiParser->pause();
		_mutex.unlock();
	}
}

void SciMusic::soundResume(MusicEntry *pSnd) {
	if (pSnd->pauseCounter > 0)
		pSnd->pauseCounter--;
	if (pSnd->pauseCounter != 0)
		return;
	if (pSnd->status != kSoundPaused)
		return;
	soundPlay(pSnd);
}

uint16 SciMusic::soundGetMasterVolume() {
	return (_pMixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) + 8) * 0xF / Audio::Mixer::kMaxMixerVolume;
}

void SciMusic::soundSetMasterVolume(uint16 vol) {
	vol = vol & 0xF; // 0..15
	vol = vol * Audio::Mixer::kMaxMixerVolume / 0xF;
	// "master volume" is music and SFX only, speech (audio resources) are supposed to be unaffected
	_pMixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, vol);
	_pMixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, vol);
}

void SciMusic::printPlayList(Console *con) {
	Common::StackLock lock(_mutex);

	const char *musicStatus[] = { "Stopped", "Initialized", "Paused", "Playing" };

	const MusicList::iterator end = _playList.end();
	for (MusicList::iterator i = _playList.begin(); i != end; ++i) {
		con->DebugPrintf("%d: %04x:%04x, priority: %d, status: %s\n", i, 
						PRINT_REG((*i)->soundObj), (*i)->prio,
						musicStatus[(*i)->status]);
	}
}

MusicEntry::MusicEntry() {
	soundObj = NULL_REG;

	soundRes = 0;
	resnum = 0;

	dataInc = 0;
	ticker = 0;
	signal = 0;
	prio = 0;
	loop = 0;
	volume = MUSIC_VOLUME_DEFAULT;
	hold = 0;

	pauseCounter = 0;
	sampleLoopCounter = 0;

	fadeTo = 0;
	fadeStep = 0;
	fadeTicker = 0;
	fadeTickerStep = 0;
	fadeSetVolume = false;
	fadeCompleted = false;

	status = kSoundStopped;

	soundType = Audio::Mixer::kMusicSoundType;

	pStreamAud = 0;
	pMidiParser = 0;
}

MusicEntry::~MusicEntry() {
}

void MusicEntry::onTimer(SciVersion soundVersion) {
	if (status != kSoundPlaying)
		return;

	// Fade MIDI and digital sound effects
	if (fadeStep)
		doFade();

	// Only process MIDI streams in this thread, not digital sound effects
	if (pMidiParser) {
		pMidiParser->onTimer();
		ticker = (uint16)pMidiParser->getTick();
	}
}

void MusicEntry::doFade() {
	if (fadeTicker)
		fadeTicker--;
	else {
		int16 fadeVolume = volume;
		fadeTicker = fadeTickerStep;
		fadeVolume += fadeStep;
		if (((fadeStep > 0) && (fadeVolume >= fadeTo)) || ((fadeStep < 0) && (fadeVolume <= fadeTo))) {
			fadeVolume = fadeTo;
			fadeStep = 0;
			fadeCompleted = true;
		}
		volume = fadeVolume;

		// Only process MIDI streams in this thread, not digital sound effects
		if (pMidiParser)
			pMidiParser->setVolume(volume);
		fadeSetVolume = true; // set flag so that SoundCommandParser::cmdUpdateCues will set the volume of the stream
	}
}

} // End of namespace Sci
