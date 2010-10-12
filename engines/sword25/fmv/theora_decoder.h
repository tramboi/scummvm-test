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

#ifndef SWORD25_THEORADECODER_H
#define SWORD25_THEORADECODER_H

#include "graphics/video/video_decoder.h"

#include <theora/theoradec.h>
#include <vorbis/codec.h>

namespace Common {
	class SeekableReadStream;
}

namespace Sword25 {

/**
 *
 * Decoder for Theora videos.
 * Video decoder used in engines:
 *  - sword25
 */
class TheoraDecoder : public Graphics::FixedRateVideoDecoder {
public:
	TheoraDecoder();
	virtual ~TheoraDecoder();

	/**
	 * Load a video file
	 * @param stream  the stream to load
	 */
	bool load(Common::SeekableReadStream &stream);
	void close();

	/**
	 * Decode the next frame and return the frame's surface
	 * @note the return surface should *not* be freed
	 * @note this may return 0, in which case the last frame should be kept on screen
	 */
	Graphics::Surface *decodeNextFrame();

	bool isVideoLoaded() const { return _fileStream != 0; }
	uint16 getWidth() const { return _surface->w; }
	uint16 getHeight() const { return _surface->h; }
	uint32 getFrameCount() const { return _frameCount; }
	Graphics::PixelFormat getPixelFormat() const { return Graphics::PixelFormat(3, 8, 8, 8, 0, 0, 0, 0, 0); }

protected:
	Common::Rational getFrameRate() const { return _frameRate; }

private:
	void queuePage(ogg_page *page);
	int bufferData();

private:
	Common::SeekableReadStream *_fileStream;
	Graphics::Surface *_surface;
	Common::Rational _frameRate;
	uint32 _frameCount;

	ogg_sync_state _oggSync;
	ogg_page _oggPage;
	ogg_packet _oggPacket;
	ogg_stream_state _vorbisOut;
	ogg_stream_state _theoraOut;
	th_info _theoraInfo;
	th_comment _theoraComment;
	th_dec_ctx *_theoraDecode;
	th_setup_info *_theoraSetup;
	vorbis_info _vorbisInfo;
	vorbis_dsp_state _vorbisDSP;
	vorbis_block _vorbisBlock;
	vorbis_comment _vorbisComment;

	int _theoraPacket;
	int _vorbisPacket;
	int _stateFlag;

	int _ppLevelMax;
	int _ppLevel;
	int _ppInc;

	// single frame video buffering
	int _videobufReady;
	ogg_int64_t  _videobufGranulePos;
	double _videobufTime;

	// single audio fragment audio buffering
	int _audiobufFill;
	int _audiobufReady;
	ogg_int16_t *_audiobuf;
	ogg_int64_t  _audiobufGranulePos; // time position of last sample
};

} // End of namespace Sword25

#endif
