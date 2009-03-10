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

#ifndef GROOVIE_MUSIC_H
#define GROOVIE_MUSIC_H

#include "groovie/groovie.h"

#include "sound/mididrv.h"
#include "sound/midiparser.h"
#include "common/mutex.h"

namespace Groovie {

class MusicPlayer : public MidiDriver {
public:
	MusicPlayer(GroovieEngine *vm, const Common::String &gtlName);
	~MusicPlayer();
	void playSong(uint32 fileref);
	void setBackgroundSong(uint32 fileref);
	void playCD(uint8 track);
	void startBackground();

	// Volume
	void setUserVolume(uint16 volume);
	void setGameVolume(uint16 volume, uint16 time);

private:
	// User volume
	uint16 _userVolume;

	// Game volume
	uint16 _gameVolume;
	uint32 _fadingStartTime;
	uint16 _fadingStartVolume;
	uint16 _fadingEndVolume;
	uint16 _fadingDuration;
	void endTrack();
	void applyFading();

	// Song volumes
	byte _chanVolumes[0x10];
	void updateChanVolume(byte channel);

	// Channel banks
	byte _chanBanks[0x10];

	// Timbres
	class Timbre {
	public:
		Timbre() : data(NULL) {}
		byte patch;
		byte bank;
		uint32 size;
		byte *data;
	};
	Common::Array<Timbre> _timbres;
	void loadTimbres(const Common::String &filename);
	void clearTimbres();
	void setTimbreAD(byte channel, const Timbre &timbre);
	void setTimbreMT(byte channel, const Timbre &timbre);

public:
	// MidiDriver interface
	int open();
	void close();
	void send(uint32 b);
	void metaEvent(byte type, byte *data, uint16 length);
	void setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc);
	uint32 getBaseTempo(void);
	MidiChannel *allocateChannel();
	MidiChannel *getPercussionChannel();

private:
	GroovieEngine *_vm;
	Common::Mutex _mutex;
	byte *_data;
	MidiParser *_midiParser;
	MidiDriver *_driver;
	uint8 _musicType;
	bool _isPlaying;

	uint32 _backgroundFileRef;
	uint8 _prevCDtrack;

	static void onTimer(void *data);

	bool play(uint32 fileref, bool loop);
	bool load(uint32 fileref);
	void unload();
};

} // End of Groovie namespace

#endif // GROOVIE_MUSIC_H
