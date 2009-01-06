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

// Based on http://wiki.multimedia.cx/index.php?title=Smacker
// and the FFmpeg Smacker decoder (libavcodec/smacker.c), revision 16143
// http://svn.ffmpeg.org/ffmpeg/trunk/libavcodec/smacker.c?revision=16143&view=markup

#ifndef GRAPHICS_VIDEO_SMK_PLAYER_H
#define GRAPHICS_VIDEO_SMK_PLAYER_H

#include "common/scummsys.h"
#include "common/stream.h"
#include "sound/mixer.h"
#include "sound/audiostream.h"

namespace Graphics {

class BigHuffmanTree;

/**
 * Implementation of a Smacker v2/v4 video decoder
 */
class SMKPlayer {
public:
	SMKPlayer(Audio::Mixer *mixer);
	virtual ~SMKPlayer();

	/**
	 * Returns the width of the video
	 * @return the width of the video
	 */
	int getWidth();

	/**
	 * Returns the height of the video
	 * @return the height of the video
	 */
	int getHeight();

	/**
	 * Returns the current frame number of the video
	 * @return the current frame number of the video
	 */
	int32 getCurFrame();

	/**
	 * Returns the amount of frames in the video
	 * @return the amount of frames in the video
	 */
	int32 getFrameCount();

	/**
	 * Returns the frame rate of the video
	 * @return the frame rate of the video
	 */
	int32 getFrameRate();

	/**
	 * Returns the time to wait for each frame in 1/100 ms
	 * @return the time to wait for each frame in 1/100 ms
	 */
	int32 getFrameDelay();

	/**
	 * Returns the current A/V lag in 1/100 ms
	 * If > 0, audio lags behind
	 * If < 0, video lags behind
	 * @return the current A/V lag in 1/100 ms
	 */
	int32 getAudioLag();

	/**
	 * Returns the time to wait until the next frame in ms, minding any lag
	 * @return the time to wait until the next frame in ms
	 */
	uint32 getFrameWaitTime();

	/**
	 * Load an SMK encoded video file
	 * @param filename	the filename to load
	 */
	bool loadFile(const char *filename);

	/**
	 * Close an SMK encoded video file
	 */
	void closeFile();

protected:
	/**
	 * Set RGB palette, based on current frame
	 * @param pal		the RGB palette data
	 */
	virtual void setPalette(byte *pal) = 0;

	/**
	 * Copy current frame into the specified position of the destination
	 * buffer.
	 * @param dst		the buffer
	 * @param x		the x position of the buffer
	 * @param y		the y position of the buffer
	 * @param pitch		the pitch of buffer
	 */
	void copyFrameToBuffer(byte *dst, uint x, uint y, uint pitch);

	/**
	 * Decode the next frame
	 */
	bool decodeNextFrame();

	Common::SeekableReadStream *_fileStream;

	byte *_image;

private:
	void unpackPalette();
	// Possible runs of blocks
	uint getBlockRun(int index) { return (index <= 58) ? index + 1 : 128 << (index - 59); }
	void queueCompressedBuffer(byte *buffer, uint32 bufferSize, uint32 unpackedSize, int streamNum);

	struct AudioInfo {
		bool isCompressed;
		bool hasAudio;
		bool is16Bits;
		bool isStereo;
		bool hasV2Compression;
		uint32 sampleRate;
	};

	struct {
		uint32 signature;
		uint32 width;
		uint32 height;
		uint32 frames;
		int32 frameRate;
		uint32 flags;
		uint32 audioSize[7];
		uint32 treesSize;
		uint32 mMapSize;
		uint32 mClrSize;
		uint32 fullSize;
		uint32 typeSize;
		AudioInfo audioInfo[7];
		uint32 dummy;
	} _header;

	uint32 *_frameSizes;
	// The FrameTypes section of a Smacker file contains an array of bytes, where
	// the 8 bits of each byte describe the contents of the corresponding frame.
	// The highest 7 bits correspond to audio frames (bit 7 is track 6, bit 6 track 5
	// and so on), so there can be up to 7 different audio tracks. When the lowest bit
	// (bit 0) is set, it denotes a frame that contains a palette record
	byte *_frameTypes;
	byte *_frameData;
	byte *_palette;

	Audio::Mixer *_mixer;
	bool _audioStarted;
	Audio::AppendableAudioStream *_audioStream;
	Audio::SoundHandle _audioHandle;

	uint32 _currentSMKFrame;
	uint32 _startTime;

	BigHuffmanTree *_MMapTree;
	BigHuffmanTree *_MClrTree;
	BigHuffmanTree *_FullTree;
	BigHuffmanTree *_TypeTree;
};

} // End of namespace Graphics

#endif
