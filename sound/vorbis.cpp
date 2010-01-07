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

#include "sound/vorbis.h"

#ifdef USE_VORBIS

#include "common/debug.h"
#include "common/stream.h"
#include "common/util.h"

#include "sound/audiostream.h"
#include "sound/audiocd.h"

#ifdef USE_TREMOR
#ifdef __GP32__ // GP32 uses custom libtremor
#include <ivorbisfile.h>
#else
#include <tremor/ivorbisfile.h>
#endif
#else
#include <vorbis/vorbisfile.h>
#endif


namespace Audio {

// These are wrapper functions to allow using a SeekableReadStream object to
// provide data to the OggVorbis_File object.

static size_t read_stream_wrap(void *ptr, size_t size, size_t nmemb, void *datasource) {
	Common::SeekableReadStream *stream = (Common::SeekableReadStream *)datasource;

	uint32 result = stream->read(ptr, size * nmemb);

	return result / size;
}

static int seek_stream_wrap(void *datasource, ogg_int64_t offset, int whence) {
	Common::SeekableReadStream *stream = (Common::SeekableReadStream *)datasource;
	stream->seek((int32)offset, whence);
	return stream->pos();
}

static int close_stream_wrap(void *datasource) {
	// Do nothing -- we leave it up to the VorbisInputStream to free memory as appropriate.
	return 0;
}

static long tell_stream_wrap(void *datasource) {
	Common::SeekableReadStream *stream = (Common::SeekableReadStream *)datasource;
	return stream->pos();
}

static ov_callbacks g_stream_wrap = {
	read_stream_wrap, seek_stream_wrap, close_stream_wrap, tell_stream_wrap
};



#pragma mark -
#pragma mark --- Ogg Vorbis stream ---
#pragma mark -


class VorbisInputStream : public SeekableAudioStream {
protected:
	Common::SeekableReadStream *_inStream;
	bool _disposeAfterUse;

	bool _isStereo;
	int _rate;

	Timestamp _length;

	OggVorbis_File _ovFile;

	int16 _buffer[4096];
	const int16 *_bufferEnd;
	const int16 *_pos;

public:
	// startTime / duration are in milliseconds
	VorbisInputStream(Common::SeekableReadStream *inStream, bool dispose);
	~VorbisInputStream();

	int readBuffer(int16 *buffer, const int numSamples);

	bool endOfData() const		{ return _pos >= _bufferEnd; }
	bool isStereo() const		{ return _isStereo; }
	int getRate() const			{ return _rate; }

	bool seek(const Timestamp &where);
	Timestamp getLength() const { return _length; }
protected:
	bool refill();
};

VorbisInputStream::VorbisInputStream(Common::SeekableReadStream *inStream, bool dispose) :
	_inStream(inStream),
	_disposeAfterUse(dispose),
	_length(0, 1000),
	_bufferEnd(_buffer + ARRAYSIZE(_buffer)) {

	int res = ov_open_callbacks(inStream, &_ovFile, NULL, 0, g_stream_wrap);
	if (res < 0) {
		warning("Could not create Vorbis stream (%d)", res);
		_pos = _bufferEnd;
		return;
	}

#ifdef USE_TREMOR
	_length = Timestamp(ov_time_total(&_ovFile, -1), getRate());
#else
	_length = Timestamp(uint32(ov_time_total(&_ovFile, -1) * 1000.0), getRate());
#endif

	// Read in initial data
	if (!refill())
		return;

	// Setup some header information
	_isStereo = ov_info(&_ovFile, -1)->channels >= 2;
	_rate = ov_info(&_ovFile, -1)->rate;
}

VorbisInputStream::~VorbisInputStream() {
	ov_clear(&_ovFile);
	if (_disposeAfterUse)
		delete _inStream;
}

int VorbisInputStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples = 0;
	while (samples < numSamples && _pos < _bufferEnd) {
		const int len = MIN(numSamples - samples, (int)(_bufferEnd - _pos));
		memcpy(buffer, _pos, len * 2);
		buffer += len;
		_pos += len;
		samples += len;
		if (_pos >= _bufferEnd) {
			if (!refill())
				break;
		}
	}
	return samples;
}

bool VorbisInputStream::seek(const Timestamp &where) {
	int res = ov_pcm_seek(&_ovFile, calculateSampleOffset(where, getRate()));
	if (res) {
		warning("Error seeking in Vorbis stream (%d)", res);
		_pos = _bufferEnd;
		return false;
	}

	return refill();
}

bool VorbisInputStream::refill() {
	// Read the samples
	uint len_left = sizeof(_buffer);
	char *read_pos = (char *)_buffer;

	while (len_left > 0) {
		long result;

#ifdef USE_TREMOR
		// Tremor ov_read() always returns data as signed 16 bit interleaved PCM
		// in host byte order. As such, it does not take arguments to request
		// specific signedness, byte order or bit depth as in Vorbisfile.
		result = ov_read(&_ovFile, read_pos, len_left,
						NULL);
#else
#ifdef SCUMM_BIG_ENDIAN
		result = ov_read(&_ovFile, read_pos, len_left,
						1,
						2,	// 16 bit
						1,	// signed
						NULL);
#else
		result = ov_read(&_ovFile, read_pos, len_left,
						0,
						2,	// 16 bit
						1,	// signed
						NULL);
#endif
#endif
		if (result == OV_HOLE) {
			// Possibly recoverable, just warn about it
			warning("Corrupted data in Vorbis file");
		} else if (result == 0) {
			//warning("End of file while reading from Vorbis file");
			//_pos = _bufferEnd;
			//return false;
			break;
		} else if (result < 0) {
			warning("Error reading from Vorbis stream (%d)", int(result));
			_pos = _bufferEnd;
			// Don't delete it yet, that causes problems in
			// the CD player emulation code.
			return false;
		} else {
			len_left -= result;
			read_pos += result;
		}
	}

	_pos = _buffer;
	_bufferEnd = (int16 *)read_pos;

	return true;
}


#pragma mark -
#pragma mark --- Ogg Vorbis factory functions ---
#pragma mark -


AudioStream *makeVorbisStream(
	Common::SeekableReadStream *stream,
	bool disposeAfterUse,
	uint32 startTime,
	uint32 duration,
	uint numLoops) {

	SeekableAudioStream *input = new VorbisInputStream(stream, disposeAfterUse);
	assert(input);

	if (startTime || duration) {
		Timestamp start(startTime, 1000), end(startTime + duration, 1000);

		if (!duration)
			end = input->getLength();

		input = new SubSeekableAudioStream(input, start, end);
		assert(input);
	}

	if (numLoops)
		return new LoopingAudioStream(input, numLoops);
	else
		return input;
}

SeekableAudioStream *makeVorbisStream(
	Common::SeekableReadStream *stream,
	bool disposeAfterUse) {
	return new VorbisInputStream(stream, disposeAfterUse);
}

} // End of namespace Audio

#endif // #ifdef USE_VORBIS
