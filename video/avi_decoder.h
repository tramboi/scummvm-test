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

#ifndef GRAPHICS_AVI_PLAYER_H
#define GRAPHICS_AVI_PLAYER_H

#include "graphics/video/video_decoder.h"
#include "graphics/video/codecs/codec.h"
#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace Graphics {

#define UNKNOWN_HEADER(a) error("Unknown header found -- \'%s\'", tag2str(a))

// IDs used throughout the AVI files
// that will be handled by this player
#define ID_RIFF MKID_BE('RIFF')
#define ID_AVI  MKID_BE('AVI ')
#define ID_LIST MKID_BE('LIST')
#define ID_HDRL MKID_BE('hdrl')
#define ID_AVIH MKID_BE('avih')
#define ID_STRL MKID_BE('strl')
#define ID_STRH MKID_BE('strh')
#define ID_VIDS MKID_BE('vids')
#define ID_AUDS MKID_BE('auds')
#define ID_MIDS MKID_BE('mids')
#define ID_TXTS MKID_BE('txts')
#define ID_JUNK MKID_BE('JUNK')
#define ID_STRF MKID_BE('strf')
#define ID_MOVI MKID_BE('movi')
#define ID_REC  MKID_BE('rec ')
#define ID_VEDT MKID_BE('vedt')
#define ID_IDX1 MKID_BE('idx1')
#define ID_STRD MKID_BE('strd')
#define ID_00AM MKID_BE('00AM')
//#define ID_INFO MKID_BE('INFO')

// Codec tags
#define ID_RLE  MKID_BE('RLE ')
#define ID_CRAM MKID_BE('CRAM')
#define ID_MSVC MKID_BE('msvc')
#define ID_WHAM MKID_BE('WHAM')
#define ID_CVID MKID_BE('cvid')
#define ID_IV32 MKID_BE('iv32')
#define ID_DUCK MKID_BE('DUCK')

struct BITMAPINFOHEADER {
	uint32 size;
	uint32 width;
	uint32 height;
	uint16 planes;
	uint16 bitCount;
	uint32 compression;
	uint32 sizeImage;
	uint32 xPelsPerMeter;
	uint32 yPelsPerMeter;
	uint32 clrUsed;
	uint32 clrImportant;
};

struct WAVEFORMAT {
	uint16 tag;
	uint16 channels;
	uint32 samplesPerSec;
	uint32 avgBytesPerSec;
	uint16 blockAlign;
};

struct PCMWAVEFORMAT : public WAVEFORMAT {
	uint16 size;
};

struct WAVEFORMATEX : public WAVEFORMAT {
	uint16 bitsPerSample;
	uint16 size;
};

struct AVIOLDINDEX {
	uint32 size;
	struct Index {
		uint32 id;
		uint32 flags;
		uint32 offset;
		uint32 size;
	} *indices;
};

// Index Flags
enum IndexFlags {
	AVIIF_INDEX = 0x10
};

// Audio Codecs
enum {
	kWaveFormatNone = 0,
	kWaveFormatPCM = 1,
	kWaveFormatDK3 = 98
};

struct AVIHeader {
	uint32 size;
	uint32 microSecondsPerFrame;
	uint32 maxBytesPerSecond;
	uint32 padding;
	uint32 flags;
	uint32 totalFrames;
	uint32 initialFrames;
	uint32 streams;
	uint32 bufferSize;
	uint32 width;
	uint32 height;
};

// Flags from the AVIHeader
enum AviFlags {
	AVIF_HASINDEX = 0x00000010,
	AVIF_MUSTUSEINDEX = 0x00000020,
	AVIF_ISINTERLEAVED = 0x00000100,
	AVIF_TRUSTCKTYPE = 0x00000800,
	AVIF_WASCAPTUREFILE = 0x00010000,
	AVIF_WASCOPYRIGHTED = 0x00020000
};

struct AVIStreamHeader {
	uint32 size;
	uint32 streamType;
	uint32 streamHandler;
	uint32 flags;
	uint16 priority;
	uint16 language;
	uint32 initialFrames;
	uint32 scale;
	uint32 rate;
	uint32 start;
	uint32 length;
	uint32 bufferSize;
	uint32 quality;
	uint32 sampleSize;
	Common::Rect frame;
};

/**
 * Decoder for AVI videos.
 *
 * Video decoder used in engines:
 *  - sci
 */
class AviDecoder : public FixedRateVideoDecoder {
public:
	AviDecoder(Audio::Mixer *mixer,
			Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType);
	virtual ~AviDecoder();

	bool load(Common::SeekableReadStream *stream);
	void close();

	bool isVideoLoaded() const { return _fileStream != 0; }
	uint16 getWidth() const { return _header.width; }
	uint16 getHeight() const { return _header.height; }
	uint32 getFrameCount() const { return _header.totalFrames; }
	uint32 getElapsedTime() const;
	const Surface *decodeNextFrame();
	PixelFormat getPixelFormat() const;
	const byte *getPalette() { _dirtyPalette = false; return _palette; }
	bool hasDirtyPalette() const { return _dirtyPalette; }

protected:
	Common::Rational getFrameRate() const { return Common::Rational(_vidsHeader.rate, _vidsHeader.scale); }

private:
	Audio::Mixer *_mixer;
	BITMAPINFOHEADER _bmInfo;
	PCMWAVEFORMAT _wvInfo;
	AVIOLDINDEX _ixInfo;
	AVIHeader _header;
	AVIStreamHeader _vidsHeader;
	AVIStreamHeader _audsHeader;
	byte _palette[3 * 256];
	bool _dirtyPalette;

	Common::SeekableReadStream *_fileStream;
	bool _decodedHeader;

	Codec *_videoCodec;
	Codec *createCodec();

	Audio::Mixer::SoundType _soundType;

	void runHandle(uint32 tag);
	void handleList();
	void handleStreamHeader();
	void handlePalChange();

	Audio::SoundHandle *_audHandle;
	Audio::QueuingAudioStream *_audStream;
	Audio::QueuingAudioStream *createAudioStream();
	void queueAudioBuffer(uint32 chunkSize);
};

} // End of namespace Graphics

#endif
