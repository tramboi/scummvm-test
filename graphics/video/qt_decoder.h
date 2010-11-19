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

//
// Heavily based on ffmpeg code.
//
// Copyright (c) 2001 Fabrice Bellard.
// First version by Francois Revol revol@free.fr
// Seek function by Gael Chardon gael.dev@4now.net
//

#ifndef GRAPHICS_QT_DECODER_H
#define GRAPHICS_QT_DECODER_H

#include "common/scummsys.h"
#include "common/queue.h"
#include "common/rational.h"

#include "graphics/video/video_decoder.h"
#include "graphics/video/codecs/codec.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace Common {
	class File;
	class MacResManager;
}

namespace Graphics {


class QuickTimeDecoder : public RewindableVideoDecoder {
public:
	QuickTimeDecoder();
	virtual ~QuickTimeDecoder();

	/**
	 * Returns the width of the video
	 * @return the width of the video
	 */
	uint16 getWidth() const;

	/**
	 * Returns the height of the video
	 * @return the height of the video
	 */
	uint16 getHeight() const;

	/**
	 * Returns the amount of frames in the video
	 * @return the amount of frames in the video
	 */
	uint32 getFrameCount() const;

	/**
	 * Load a video file
	 * @param filename	the filename to load
	 */
	bool loadFile(const Common::String &filename);

	/**
	 * Load a QuickTime video file from a SeekableReadStream
	 * @param stream	the stream to load
	 */
	bool load(Common::SeekableReadStream *stream);

	/**
	 * Close a QuickTime encoded video file
	 */
	void close();

	/**
	 * Returns the palette of the video
	 * @return the palette of the video
	 */
	byte *getPalette() { _dirtyPalette = false; return _palette; }
	bool hasDirtyPalette() const { return _dirtyPalette; }

	/**
	 * Set the beginning offset of the video so we can modify the offsets in the stco
	 * atom of videos inside the Mohawk archives
	 * @param the beginning offset of the video
	 */
	void setChunkBeginOffset(uint32 offset) { _beginOffset = offset; }

	bool isVideoLoaded() const { return _fd != 0; }
	Surface *decodeNextFrame();
	bool endOfVideo() const;
	uint32 getElapsedTime() const;
	uint32 getTimeToNextFrame() const;
	PixelFormat getPixelFormat() const;

	// RewindableVideoDecoder API
	void rewind();

	// TODO: This audio function need to be removed from the public and/or added to
	// the VideoDecoder API directly. I plan on replacing this function with something
	// that can figure out how much audio is needed instead of constantly keeping two
	// chunks in memory.
	void updateAudioBuffer();

protected:
	// This is the file handle from which data is read from. It can be the actual file handle or a decompressed stream.
	Common::SeekableReadStream *_fd;

	struct MOVatom {
		uint32 type;
		uint32 offset;
		uint32 size;
	};

	struct ParseTable {
		int (QuickTimeDecoder::*func)(MOVatom atom);
		uint32 type;
	};

	struct MOVstts {
		int count;
		int duration;
	};

	struct MOVstsc {
		uint32 first;
		uint32 count;
		uint32 id;
	};

	enum CodecType {
		CODEC_TYPE_MOV_OTHER,
		CODEC_TYPE_VIDEO,
		CODEC_TYPE_AUDIO
	};

	struct MOVStreamContext {
		MOVStreamContext();
		~MOVStreamContext();

		int ffindex; /* the ffmpeg stream id */
		int is_ff_stream; /* Is this stream presented to ffmpeg ? i.e. is this an audio or video stream ? */
		uint32 next_chunk;
		uint32 chunk_count;
		uint32 *chunk_offsets;
		int stts_count;
		MOVstts *stts_data;
		int ctts_count;
		MOVstts *ctts_data;
		int edit_count; /* number of 'edit' (elst atom) */
		uint32 sample_to_chunk_sz;
		MOVstsc *sample_to_chunk;
		int32 sample_to_chunk_index;
		int sample_to_time_index;
		uint32 sample_to_time_sample;
		uint32 sample_to_time_time;
		int sample_to_ctime_index;
		int sample_to_ctime_sample;
		uint32 sample_size;
		uint32 sample_count;
		uint32 *sample_sizes;
		uint32 keyframe_count;
		uint32 *keyframes;
		int32 time_scale;
		int time_rate;
		uint32 current_sample;
		uint32 left_in_chunk; /* how many samples before next chunk */

		uint16 width;
		uint16 height;
		int codec_type;
		uint32 codec_tag;
		char codec_name[32];
		uint16 bits_per_sample;
		uint16 color_table_id;
		bool palettized;
		Common::SeekableReadStream *extradata;

		uint16 stsd_version;
		uint16 channels;
		uint16 sample_rate;
		uint32 samples_per_frame;
		uint32 bytes_per_frame;

		uint32 nb_frames;
		uint32 duration;
		uint32 start_time;
		Common::Rational scaleFactorX;
		Common::Rational scaleFactorY;
	};

	const ParseTable *_parseTable;
	bool _foundMOOV;
	bool _foundMDAT;
	uint32 _timeScale;
	uint32 _duration;
	uint32 _mdatOffset;
	uint32 _mdatSize;
	MOVStreamContext *_partial;
	uint32 _numStreams;
	int _ni;
	Common::Rational _scaleFactorX;
	Common::Rational _scaleFactorY;
	MOVStreamContext *_streams[20];
	byte _palette[256 * 3];
	bool _dirtyPalette;
	uint32 _beginOffset;
	Common::MacResManager *_resFork;

	void initParseTable();
	Audio::AudioStream *createAudioStream(Common::SeekableReadStream *stream);
	bool checkAudioCodecSupport(uint32 tag);
	Common::SeekableReadStream *getNextFramePacket();
	uint32 getFrameDuration();
	uint32 getCodecTag();
	byte getBitsPerPixel();
	void init();

	Audio::QueuingAudioStream *_audStream;
	void startAudio();
	void stopAudio();
	int8 _audioStreamIndex;
	uint _curAudioChunk;
	Audio::SoundHandle _audHandle;

	Codec *createCodec(uint32 codecTag, byte bitsPerPixel);
	Codec *_videoCodec;
	uint32 _nextFrameStartTime;
	int8 _videoStreamIndex;

	Surface *_scaledSurface;
	Surface *scaleSurface(Surface *frame);
	Common::Rational getScaleFactorX() const;
	Common::Rational getScaleFactorY() const;

	void pauseVideoIntern(bool pause);

	int readDefault(MOVatom atom);
	int readLeaf(MOVatom atom);
	int readELST(MOVatom atom);
	int readHDLR(MOVatom atom);
	int readMDAT(MOVatom atom);
	int readMDHD(MOVatom atom);
	int readMOOV(MOVatom atom);
	int readMVHD(MOVatom atom);
	int readTKHD(MOVatom atom);
	int readTRAK(MOVatom atom);
	int readSTCO(MOVatom atom);
	int readSTSC(MOVatom atom);
	int readSTSD(MOVatom atom);
	int readSTSS(MOVatom atom);
	int readSTSZ(MOVatom atom);
	int readSTTS(MOVatom atom);
	int readCMOV(MOVatom atom);
	int readWAVE(MOVatom atom);
};

} // End of namespace Graphics

#endif
