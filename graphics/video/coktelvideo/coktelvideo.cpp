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

#include "graphics/video/coktelvideo/coktelvideo.h"

#ifdef GRAPHICS_VIDEO_COKTELVIDEO_H

#include "common/endian.h"
#include "common/system.h"

#include "graphics/dither.h"
#include "graphics/video/coktelvideo/indeo3.h"

#include "sound/audiostream.h"
#include "sound/decoders/raw.h"

namespace Graphics {

PreImd::PreImd() {
	zeroData();

	_forcedWidth  = 0;
	_forcedHeight = 0;
}

PreImd::PreImd(int16 width, int16 height) {
	zeroData();

	_forcedWidth  = width;
	_forcedHeight = height;
}

PreImd::~PreImd() {
	deleteData();
	zeroData();
}

uint32 PreImd::getFeatures() const {
	return _features;
}

uint16 PreImd::getFlags() const {
	return _flags;
}

int16 PreImd::getX() const {
	return _x;
}

int16 PreImd::getY() const {
	return _y;
}

int16 PreImd::getWidth() const {
	return _width;
}

int16 PreImd::getHeight() const {
	return _height;
}

uint16 PreImd::getFramesCount() const {
	return _framesCount;
}

uint16 PreImd::getCurrentFrame() const {
	return _curFrame;
}

void PreImd::setFrameRate(int16 frameRate) {
	if (frameRate == 0)
		frameRate = 1;

	_frameRate   = frameRate;
	_frameLength = 1000 / _frameRate;
}

int16 PreImd::getFrameRate() const {
	return 0;
}

uint32 PreImd::getSyncLag() const {
	return 0;
}

const byte *PreImd::getPalette() const {
	return _palette;
}

bool PreImd::getFrameCoords(int16 frame,
		int16 &x, int16 &y, int16 &width, int16 &height) {

	return false;
}

bool PreImd::hasExtraData() const {
	return false;
}

bool PreImd::hasExtraData(const char *fileName) const {
	return false;
}

Common::MemoryReadStream *PreImd::getExtraData(const char *fileName) {
	return 0;
}

bool PreImd::load(Common::SeekableReadStream &stream) {
	// Since PreIMDs don't have any width and height values stored,
	// we need them to be specified in the constructor
	assert((_forcedWidth > 0) && (_forcedHeight > 0));

	unload();

	_stream = &stream;

	_stream->seek(0);

	_framesCount = _stream->readUint16LE();

	_width  = _forcedWidth;
	_height = _forcedHeight;

	_vidBufferSize = _width * _height;
	_vidBuffer = new byte[_vidBufferSize];
	memset(_vidBuffer, 0, _vidBufferSize);

	return true;
}

void PreImd::unload() {
	clear();
}

void PreImd::setXY(int16 x, int16 y) {
	_x = x;
	_y = y;
}

void PreImd::setVideoMemory(byte *vidMem, uint16 width, uint16 height) {
	deleteVidMem();

	_hasOwnVidMem = false;
	_vidMem       = vidMem;
	_vidMemWidth  = width;
	_vidMemHeight = height;
}

void PreImd::setVideoMemory() {
	deleteVidMem();

	if ((_width > 0) && (_height > 0)) {
		setXY(0, 0);
		_hasOwnVidMem = true;
		_vidMem       = new byte[_width * _height];
		_vidMemWidth  = _width;
		_vidMemHeight = _height;

		memset(_vidMem, 0, _width * _height);
	}
}

void PreImd::setDoubleMode(bool doubleMode) {
}

void PreImd::enableSound(Audio::Mixer &mixer) {
}

void PreImd::disableSound() {
}

bool PreImd::isSoundPlaying() const {
	return false;
}

void PreImd::seekFrame(int32 frame, int16 whence, bool restart) {
	if (!_stream)
		// Nothing to do
		return;

	// Find the frame to which to seek
	if (whence == SEEK_CUR)
		frame += _curFrame;
	else if (whence == SEEK_END)
		frame = _framesCount - frame - 1;
	else if (whence != SEEK_SET)
		return;

	if ((frame < 0) || (frame >= _framesCount) || (frame == _curFrame))
		// Nothing to do
		return;

	// Run through the frames
	_curFrame = 0;
	_stream->seek(2);
	while (_curFrame != frame) {
		uint16 frameSize = _stream->readUint16LE();

		_stream->skip(frameSize + 2);

		_curFrame++;
	}
}

CoktelVideo::State PreImd::nextFrame() {
	return processFrame(_curFrame);
}

uint32 PreImd::getFrameWaitTime() {
	return _frameLength;
}

void PreImd::waitEndFrame() {
	uint32 waitTime = getFrameWaitTime();
	if (waitTime > 0)
		g_system->delayMillis(waitTime);
}

void PreImd::copyCurrentFrame(byte *dest,
		uint16 left, uint16 top, uint16 width, uint16 height,
		uint16 x, uint16 y, uint16 pitch, int16 transp) {
}

void PreImd::zeroVidMem() {
	_hasOwnVidMem = false;
	_vidMem       = 0;
	_vidMemWidth  = 0;
	_vidMemHeight = 0;
}

void PreImd::deleteVidMem() {
	if (_hasOwnVidMem)
		delete[] _vidMem;

	zeroVidMem();
}

void PreImd::zeroData() {
	_stream = 0;

	_features = 0;
	_flags    = 0;

	_x      = 0;
	_y      = 0;
	_width  = 0;
	_height = 0;

	_framesCount = 0;
	_curFrame    = 0;
	_frameRate   = 12;
	_frameLength = 1000 / _frameRate;

	memset(_palette, 0, 768);

	zeroVidMem();

	_vidBufferSize = 0;
	_vidBuffer     = 0;
}

void PreImd::deleteData() {
	deleteVidMem();

	delete[] _vidBuffer;
}

void PreImd::clear() {
	deleteData();
	zeroData();
}

CoktelVideo::State PreImd::processFrame(uint16 frame) {
	assert((_width > 0) && (_height > 0));

	State state;

	if (!_stream || (frame >= _framesCount)) {
		state.flags = kStateBreak;
		return state;
	}

	if (frame != _curFrame) {
		state.flags |= kStateSeeked;
		seekFrame(frame);
	}

	if (!_vidMem)
		setVideoMemory();

	uint16 frameSize = _stream->readUint16LE();

	uint32 nextFramePos = _stream->pos() + frameSize + 2;

	byte cmd;

	cmd = _stream->readByte();
	frameSize--;

	bool hasPalette = false;
	if (cmd == 0) {
		// Palette. Ignored by Fascination, though

		hasPalette = true;

		_stream->read(_palette, 768);

		frameSize -= 769;

		cmd = _stream->readByte();
	}

	if (cmd != 2) {
		// Partial frame data

		uint32 fSize = frameSize;
		uint32 vidSize = _vidBufferSize;
		byte *vidBuffer = _vidBuffer;

		while ((fSize > 0) && (vidSize > 0)) {
			uint32 n = _stream->readByte();
			fSize--;

			if ((n & 0x80) != 0) {
				// Data

				n = MIN<uint32>((n & 0x7F) + 1, MIN(fSize, vidSize));

				_stream->read(vidBuffer, n);

				vidBuffer += n;
				vidSize -= n;
				fSize -= n;

			} else {
				// Skip

				n = MIN<uint32>(n + 1, vidSize);

				vidBuffer += n;
				vidSize -= n;
			}
		}

	} else {
		// Full direct frame

		uint32 vidSize = MIN<uint32>(_vidBufferSize, frameSize);

		_stream->read(_vidBuffer, vidSize);
	}

	renderFrame();

	_stream->seek(nextFramePos);

	_curFrame++;

	// Complete frame needs to be updated
	state.left   = _x;
	state.top    = _y;
	state.right  = _x + _width  - 1;
	state.bottom = _y + _height - 1;

	return state;
}

// Simple blit
void PreImd::renderFrame() {
	assert(_vidMem);

	uint16 w = MIN<uint16>(_vidMemWidth , _width);
	uint16 h = MIN<uint16>(_vidMemHeight, _height);

	const byte *src = _vidBuffer;
	byte *dst = _vidMem + (_y * _vidMemWidth) + _x;

	uint32 frameDataSize = _vidBufferSize;

	while (h-- > 0) {
		uint32 n = MIN<uint32>(w, frameDataSize);

		memcpy(dst, src, n);

		src += _width;
		dst += _vidMemWidth;

		frameDataSize -= n;
	}
}


Imd::Imd() {
	zeroData();
}

Imd::~Imd() {
	deleteData();
	zeroData();
}

void Imd::setFrameRate(int16 frameRate) {
	if (frameRate == 0)
		frameRate = 1;

	_frameRate   = frameRate;
	_frameLength = 1000 / _frameRate;
}

int16 Imd::getFrameRate() const {
	if (!_hasSound)
		return _frameRate;

	return 1000 / (_soundSliceLength >> 16);
}

uint32 Imd::getSyncLag() const {
	return _skipFrames;
}

bool Imd::loadCoordinates() {
	// Standard coordinates
	if (_version >= 3) {
		_stdX = _stream->readUint16LE();
		if (_stdX > 1) {
			warning("IMD: More than one standard coordinate quad found (%d)", _stdX);
			return false;
		}
		if (_stdX != 0) {
			_stdX      = _stream->readSint16LE();
			_stdY      = _stream->readSint16LE();
			_stdWidth  = _stream->readSint16LE();
			_stdHeight = _stream->readSint16LE();
			_features |= kFeaturesStdCoords;
		} else
			_stdX = -1;
	} else
		_stdX = -1;

	return true;
}

bool Imd::loadFrameTableOffsets(uint32 &framesPosPos, uint32 &framesCoordsPos) {
	framesPosPos     = 0;
	framesCoordsPos = 0;

	// Frame positions
	if (_version >= 4) {
		framesPosPos = _stream->readUint32LE();
		if (framesPosPos != 0) {
			_framesPos = new uint32[_framesCount];
			assert(_framesPos);
			_features |= kFeaturesFramesPos;
		}
	}

	// Frame coordinates
	if (_features & kFeaturesFrameCoords)
		framesCoordsPos = _stream->readUint32LE();

	return true;
}

bool Imd::assessVideoProperties() {
	// Sizes of the frame data and extra video buffer
	if (_features & kFeaturesDataSize) {
		_frameDataSize = _stream->readUint16LE();
		if (_frameDataSize == 0) {
			_frameDataSize = _stream->readUint32LE();
			_vidBufferSize = _stream->readUint32LE();
		} else
			_vidBufferSize = _stream->readUint16LE();
	} else {
		_frameDataSize = _width * _height + 500;
		if (!(_flags & 0x100) || (_flags & 0x1000))
			_vidBufferSize = _frameDataSize;
	}

	// Allocating working memory
	_frameData = new byte[_frameDataSize + 500];
	assert(_frameData);
	memset(_frameData, 0, _frameDataSize + 500);
	_vidBuffer = new byte[_vidBufferSize + 500];
	assert(_vidBuffer);
	memset(_vidBuffer, 0, _vidBufferSize + 500);

	return true;
}

bool Imd::assessAudioProperties() {
	if (_features & kFeaturesSound) {
		_soundFreq        = _stream->readSint16LE();
		_soundSliceSize   = _stream->readSint16LE();
		_soundSlicesCount = _stream->readSint16LE();

		if (_soundFreq < 0)
			_soundFreq = -_soundFreq;

		if (_soundSlicesCount < 0)
			_soundSlicesCount = -_soundSlicesCount - 1;

		if (_soundSlicesCount > 40) {
			warning("Imd::load(): More than 40 sound slices found (%d)", _soundSlicesCount);
			return false;
		}

		_soundSliceLength = (uint32) (((double) (1000 << 16)) /
				((double) _soundFreq / (double) _soundSliceSize));
		_frameLength = _soundSliceLength >> 16;

		_soundStage = 1;
		_hasSound   = true;

		_audioStream = Audio::makeQueuingAudioStream(_soundFreq, false);
	} else
		_frameLength = 1000 / _frameRate;

	return true;
}

bool Imd::loadFrameTables(uint32 framesPosPos, uint32 framesCoordsPos) {
	// Positions table
	if (_framesPos) {
		_stream->seek(framesPosPos, SEEK_SET);
		for (int i = 0; i < _framesCount; i++)
			_framesPos[i] = _stream->readUint32LE();
	}

	// Coordinates table
	if (_features & kFeaturesFrameCoords) {
		_stream->seek(framesCoordsPos, SEEK_SET);
		_frameCoords = new Coord[_framesCount];
		assert(_frameCoords);
		for (int i = 0; i < _framesCount; i++) {
			_frameCoords[i].left   = _stream->readSint16LE();
			_frameCoords[i].top    = _stream->readSint16LE();
			_frameCoords[i].right  = _stream->readSint16LE();
			_frameCoords[i].bottom = _stream->readSint16LE();
		}
	}

	return true;
}

bool Imd::load(Common::SeekableReadStream &stream) {
	unload();

	_stream = &stream;

	uint16 handle;

	handle   = _stream->readUint16LE();
	_version = _stream->readByte();

	// Version checking
	if ((handle != 0) || (_version < 2)) {
		warning("Imd::load(): Version incorrect (%d,%X)", handle, _version);
		unload();
		return false;
	}

	// Rest header
	_features      = _stream->readByte();
	_framesCount   = _stream->readUint16LE();
	_x             = _stream->readSint16LE();
	_y             = _stream->readSint16LE();
	_width         = _stream->readSint16LE();
	_height        = _stream->readSint16LE();
	_flags         = _stream->readUint16LE();
	_firstFramePos = _stream->readUint16LE();

	// IMDs always have video
	_features |= kFeaturesVideo;
	// IMDs always have palettes
	_features |= kFeaturesPalette;

	// Palette
	_stream->read((byte *) _palette, 768);

	if (!loadCoordinates()) {
		unload();
		return false;
	}

	uint32 framesPosPos, frameCoordsPos;
	if (!loadFrameTableOffsets(framesPosPos, frameCoordsPos)) {
		unload();
		return false;
	}

	if (!assessAudioProperties()) {
		unload();
		return false;
	}

	if (!assessVideoProperties()) {
		unload();
		return false;
	}

	if (!loadFrameTables(framesPosPos, frameCoordsPos)) {
		unload();
		return false;
	}

	// Seek to the first frame
	_stream->seek(_firstFramePos, SEEK_SET);

	return true;
}

void Imd::unload() {
	clear();
}

void Imd::setXY(int16 x, int16 y) {
	// Adjusting the standard coordinates
	if (_stdX != -1) {
		if (x >= 0)
			_stdX = _stdX - _x + x;
		if (y >= 0)
			_stdY = _stdY - _y + y;
	}

	// Going through the coordinate table as well
	if (_frameCoords) {
		for (int i = 0; i < _framesCount; i++) {
			if (_frameCoords[i].left != -1) {
				if (x >= 0) {
					_frameCoords[i].left  = _frameCoords[i].left  - _x + x;
					_frameCoords[i].right = _frameCoords[i].right - _x + x;
				}
				if (y >= 0) {
					_frameCoords[i].top    = _frameCoords[i].top    - _y + y;
					_frameCoords[i].bottom = _frameCoords[i].bottom - _y + y;
				}
			}
		}
	}

	if (x >= 0)
		_x = x;
	if (y >= 0)
		_y = y;
}

void Imd::enableSound(Audio::Mixer &mixer) {
	// Sanity check
	if (mixer.getOutputRate() == 0)
		return;

	// Only possible on the first frame
	if (_curFrame > 0)
		return;

	_mixer = &mixer;
	_soundEnabled = true;
}

void Imd::disableSound() {
	if (_audioStream) {

		if (_soundStage == 2) {
			_audioStream->finish();
			_mixer->stopHandle(_audioHandle);
		} else
			delete _audioStream;

		_audioStream = 0;
		_soundStage  = 0;
	}
	_soundEnabled = false;
	_mixer = 0;
}

bool Imd::isSoundPlaying() const {
	if (_audioStream && _mixer && _mixer->isSoundHandleActive(_audioHandle))
		return true;

	return false;
}

void Imd::seekFrame(int32 frame, int16 whence, bool restart) {
	if (!_stream)
		// Nothing to do
		return;

	// Find the frame to which to seek
	if (whence == SEEK_CUR)
		frame += _curFrame;
	else if (whence == SEEK_END)
		frame = _framesCount - frame - 1;
	else if (whence != SEEK_SET)
		return;

	if ((frame < 0) || (frame >= _framesCount) || (frame == _curFrame))
		// Nothing to do
		return;

	// Try every possible way to find a file offset to that frame
	uint32 framePos = 0;
	if (frame == 0) {
		framePos = _firstFramePos;
	} else if (frame == 1) {
		framePos = _firstFramePos;
		_stream->seek(framePos, SEEK_SET);
		framePos += _stream->readUint16LE() + 4;
	} else if (_framesPos) {
		framePos = _framesPos[frame];
	} else if (restart && (_soundStage == 0)) {
		for (int i = ((frame > _curFrame) ? _curFrame : 0); i <= frame; i++)
			processFrame(i);
		return;
//FIXME: This workaround is needed for Bargon Attack intro, which was broken by a fix concerning Ween in r42995.
	} else if (_soundStage == 0) {
		warning("Imd::seekFrame(): Avoiding \"Frame %d is not directly accessible\"", frame);
		_curFrame = frame;
//End of fixme
	} else
		error("Imd::seekFrame(): Frame %d is not directly accessible", frame);

	// Seek
	_stream->seek(framePos);
	_curFrame = frame;
}

CoktelVideo::State Imd::nextFrame() {
	return processFrame(_curFrame);
}

uint32 Imd::getFrameWaitTime() {
	if (_soundEnabled && _hasSound) {;
		if (_soundStage != 2)
			return 0;

		if (_skipFrames == 0) {
			int32 waitTime = (int16) (((_curFrame * _soundSliceLength) -
				(_mixer->getSoundElapsedTime(_audioHandle) << 16)) >> 16);

			if (waitTime < 0) {
				_skipFrames = -waitTime / (_soundSliceLength >> 16);
				warning("Video A/V sync broken, skipping %d frame(s)", _skipFrames + 1);
			} else if (waitTime > 0)
				return waitTime;

		} else
			_skipFrames--;
	} else
		return _frameLength;

	return 0;
}

void Imd::copyCurrentFrame(byte *dest,
		uint16 left, uint16 top, uint16 width, uint16 height,
		uint16 x, uint16 y, uint16 pitch, int16 transp) {

	if (!_vidMem)
		return;

	if (((left + width) > _width) || ((top + height) > _height))
		return;

	dest += pitch * y;
	byte *vidMem = _vidMem + _width * top;

	if (transp < 0) {
		// No transparency
		if ((x > 0) || (left > 0) || (pitch != _width) || (width != _width)) {
			// Copy row-by-row
			for (int i = 0; i < height; i++) {
				byte *d = dest + x;
				byte *s = vidMem + left;

				memcpy(d, s, width);

				dest += pitch;
				vidMem += _width;
			}
		} else
			// Dimensions fit, copy everything at once
			memcpy(dest, vidMem, width * height);

		return;
	}

	for (int i = 0; i < height; i++) {
		byte *d = dest + x;
		byte *s = vidMem + left;

		for (int j = 0; j < width; j++) {
			if (*s != transp)
				*d = *s;

			s++;
			d++;
		}

		dest += pitch;
		vidMem += _width;
	}

}

void Imd::zeroData() {
	PreImd::zeroData();

	_version  = 0;

	_stdX      = 0;
	_stdY      = 0;
	_stdWidth  = 0;
	_stdHeight = 0;

	_framesPos     = 0;
	_firstFramePos = 0;
	_frameCoords   = 0;

	_frameDataSize = 0;
	_frameData     = 0;
	_frameDataLen  = 0;

	_hasSound     = false;
	_soundEnabled = false;
	_soundStage   = 0;
	_skipFrames   = 0;

	_soundFlags       = 0;
	_soundFreq        = 0;
	_soundSliceSize   = 0;
	_soundSlicesCount = 0;
	_soundSliceLength = 0;
	_audioStream      = 0;

	_lastFrameTime = 0;
}

void Imd::deleteData() {
	PreImd::deleteData();

	delete[] _framesPos;
	delete[] _frameCoords;
	delete[] _frameData;

	disableSound();
}

void Imd::clear() {
	deleteData();
	zeroData();
}

void Imd::nextSoundSlice(bool hasNextCmd) {
	if (hasNextCmd || !_soundEnabled) {
		_stream->seek(_soundSliceSize, SEEK_CUR);
		return;
	}

	byte *soundBuf = (byte *)malloc(_soundSliceSize);

	_stream->read(soundBuf, _soundSliceSize);
	unsignedToSigned(soundBuf, _soundSliceSize);

	_audioStream->queueBuffer(soundBuf, _soundSliceSize, DisposeAfterUse::YES, 0);
}

bool Imd::initialSoundSlice(bool hasNextCmd) {
	int dataLength = _soundSliceSize * _soundSlicesCount;

	if (hasNextCmd || !_soundEnabled) {
		_stream->seek(dataLength, SEEK_CUR);
		return false;
	}

	byte *soundBuf = (byte *)malloc(dataLength);

	_stream->read(soundBuf, dataLength);
	unsignedToSigned(soundBuf, dataLength);

	_audioStream->queueBuffer(soundBuf, dataLength, DisposeAfterUse::YES, 0);

	return _soundStage == 1;
}

void Imd::emptySoundSlice(bool hasNextCmd) {
	if (hasNextCmd || !_soundEnabled)
		return;

	byte *soundBuf = (byte *)malloc(_soundSliceSize);

	memset(soundBuf, 0, _soundSliceSize);

	_audioStream->queueBuffer(soundBuf, _soundSliceSize, DisposeAfterUse::YES, 0);
}

void Imd::videoData(uint32 size, State &state) {
	_stream->read(_frameData, size);
	_frameDataLen = size;

/*
	if (_vidMemWidth <= state.right) {
		state.left   = 0;
		state.right -= state.left;
	}
	if (_vidMemWidth <= state.right)
		state.right = _vidMemWidth - 1;
	if (_vidMemHeight <= state.bottom) {
		state.top     = 0;
		state.bottom -= state.top;
	}
	if (_vidMemHeight <= state.bottom)
		state.bottom = _vidMemHeight -1;
*/

	state.flags |= renderFrame(state.left, state.top, state.right, state.bottom);
	state.flags |= _frameData[0];
}

void Imd::calcFrameCoords(uint16 frame, State &state) {
	if (_stdX != -1) {
		state.left   = _stdX;
		state.top    = _stdY;
		state.right  = _stdWidth  + state.left - 1;
		state.bottom = _stdHeight + state.top  - 1;
		state.flags |= kStateStdCoords;
	}
	if (_frameCoords &&
			(_frameCoords[frame].left != -1)) {
		state.left   = _frameCoords[frame].left;
		state.top    = _frameCoords[frame].top;
		state.right  = _frameCoords[frame].right;
		state.bottom = _frameCoords[frame].bottom;
		state.flags |= kStateFrameCoords;
	}
}

CoktelVideo::State Imd::processFrame(uint16 frame) {
	State state;
	uint32 cmd = 0;
	bool hasNextCmd = false;
	bool startSound = false;

	if (!_stream || (frame >= _framesCount)) {
		state.flags = kStateBreak;
		return state;
	}

	if (frame != _curFrame) {
		state.flags |= kStateSeeked;
		seekFrame(frame);
	}

	if (!_vidMem)
		setVideoMemory();

	state.left   = _x;
	state.top    = _y;
	state.right  = _width  + state.left - 1;
	state.bottom = _height + state.top  - 1;

	do {
		if (frame != 0)
			calcFrameCoords(frame, state);

		cmd = _stream->readUint16LE();

		if ((cmd & kCommandBreakMask) == kCommandBreak) {
			// Flow control

			if (cmd == kCommandBreak) {
				_stream->seek(2, SEEK_CUR);
				cmd = _stream->readUint16LE();
			}

			// Break
			if (cmd == kCommandBreakSkip0) {
				state.flags = kStateBreak;
				continue;
			} else if (cmd == kCommandBreakSkip16) {
				cmd = _stream->readUint16LE();
				_stream->seek(cmd, SEEK_CUR);
				state.flags = kStateBreak;
				continue;
			} else if (cmd == kCommandBreakSkip32) {
				cmd = _stream->readUint32LE();
				_stream->seek(cmd, SEEK_CUR);
				state.flags = kStateBreak;
				continue;
			}
		}

		// Audio
		if (_soundStage != 0) {
			if (cmd == kCommandNextSound) {

				nextSoundSlice(hasNextCmd);
				cmd = _stream->readUint16LE();

			} else if (cmd == kCommandStartSound) {

				startSound = initialSoundSlice(hasNextCmd);
				cmd = _stream->readUint16LE();

			} else
				emptySoundSlice(hasNextCmd);
		}

		// Set palette
		if (cmd == kCommandPalette) {
			_stream->seek(2, SEEK_CUR);
			state.flags |= kStatePalette;

			_stream->read(_palette, 768);
			cmd = _stream->readUint16LE();
		}

		hasNextCmd = false;

		if (cmd == kCommandJump) {
			// Jump to frame

			frame = _stream->readSint16LE();
			if (_framesPos) {
				_curFrame = frame;
				_stream->seek(_framesPos[frame], SEEK_SET);

				hasNextCmd = true;
				state.flags |= kStateJump;
			}

		} else if (cmd == kCommandVideoData) {
			uint32 size = _stream->readUint32LE() + 2;

			videoData(size, state);

			state.flags |= 1;

		} else if (cmd != 0) {
			uint32 size = cmd + 2;

			videoData(size, state);

		} else
			state.flags |= kStateNoVideoData;

	} while (hasNextCmd);

	if (startSound && _soundEnabled) {
		_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_audioHandle, _audioStream);
		_skipFrames = 0;
		_soundStage = 2;
	}

	_curFrame++;
	if ((_curFrame == _framesCount) && (_soundStage == 2)) {
		_audioStream->finish();
		_mixer->stopHandle(_audioHandle);
		_audioStream = 0;
		_soundStage  = 0;
	}

	_lastFrameTime = g_system->getMillis();
	return state;
}

// A whole, completely filled block
void Imd::renderBlockWhole(byte *dest, const byte *src, int16 width, int16 height,
		int16 destPitch, int16 srcPitch) {

	for (int i = 0; i < height; i++) {
		memcpy(dest, src, width);

		src  += srcPitch;
		dest += destPitch;
	}
}

// A quarter-wide whole, completely filled block
void Imd::renderBlockWhole4X(byte *dest, const byte *src, int16 width, int16 height,
		int16 destPitch, int16 srcPitch) {

	for (int i = 0; i < height; i++) {
		byte *destRow = dest;
		const byte *srcRow = src;

		for (int j = 0; j < width; j += 4, destRow += 4, srcRow++)
			memset(destRow, *srcRow, 4);

		src  += srcPitch;
		dest += destPitch;
	}
}

// A half-high whole, completely filled block
void Imd::renderBlockWhole2Y(byte *dest, const byte *src, int16 width, int16 height,
		int16 destPitch, int16 srcPitch) {

	while (height > 1) {
		memcpy(dest            , src, width);
		memcpy(dest + destPitch, src, width);

		height -= 2;
		dest   += 2 * destPitch;
		src    += srcPitch;
	}

	if (height == 1)
		memcpy(dest, src, width);
}

// A sparse block
void Imd::renderBlockSparse(byte *dest, const byte *src, int16 width, int16 height,
		int16 destPitch, int16 srcPitch) {

	for (int i = 0; i < height; i++) {
		byte *destRow = dest;
		int16 pixWritten = 0;

		while (pixWritten < srcPitch) {
			int16 pixCount = *src++;

			if (pixCount & 0x80) { // Data
				int16 copyCount;

				pixCount = MIN((pixCount & 0x7F) + 1, srcPitch - pixWritten);
				copyCount = MAX<int16>(0, MIN<int16>(pixCount, width - pixWritten));
				memcpy(destRow, src, copyCount);

				pixWritten += pixCount;
				destRow    += pixCount;
				src        += pixCount;
			} else { // "Hole"
				pixWritten += pixCount + 1;
				destRow    += pixCount + 1;
			}

		}

		dest += destPitch;
	}
}

// A half-high sparse block
void Imd::renderBlockSparse2Y(byte *dest, const byte *src, int16 width, int16 height,
		int16 destPitch, int16 srcPitch) {

	for (int i = 0; i < height; i += 2) {
		byte *destRow = dest;
		int16 pixWritten = 0;

		while (pixWritten < srcPitch) {
			int16 pixCount = *src++;

			if (pixCount & 0x80) { // Data
				int16 copyCount;

				pixCount = MIN((pixCount & 0x7F) + 1, srcPitch - pixWritten);
				copyCount = MAX<int16>(0, MIN<int16>(pixCount, width - pixWritten));
				memcpy(destRow            , src, pixCount);
				memcpy(destRow + destPitch, src, pixCount);

				pixWritten += pixCount;
				destRow    += pixCount;
				src        += pixCount;
			} else { // "Hole"
				pixWritten += pixCount + 1;
				destRow    += pixCount + 1;
			}

		}

		dest += destPitch;
	}
}

uint32 Imd::renderFrame(int16 left, int16 top, int16 right, int16 bottom) {
	if (!_frameData || !_vidMem || (_width <= 0) || (_height <= 0))
		return 0;

	uint32 retVal = 0;

	int16 width  = right  - left + 1;
	int16 height = bottom - top  + 1;
	int16 sW     = _vidMemWidth;
	int16 sH     = _vidMemHeight;

	byte *dataPtr   = _frameData;
	byte *imdVidMem = _vidMem + sW * top + left;
	byte *srcPtr;

	uint8 type = *dataPtr++;

	if (type & 0x10) { // Palette data
		// One byte index
		int index = *dataPtr++;
		// 16 entries with each 3 bytes (RGB)
		memcpy(_palette + index * 3, dataPtr, MIN((255 - index) * 3, 48));

		retVal = kStatePalette;
		dataPtr += 48;
		type ^= 0x10;
	}

	srcPtr = dataPtr;

	if (type & 0x80) {
		// Frame data is compressed

		srcPtr = _vidBuffer;
		type &= 0x7F;
		if ((type == 2) && (width == sW)) {
			// Directly uncompress onto the video surface
			deLZ77(imdVidMem, dataPtr);
			return retVal;
		} else
			deLZ77(srcPtr, dataPtr);
	}

	int16 drawWidth  = MAX<int16>(0, MIN<int16>(width , sW - left));
	int16 drawHeight = MAX<int16>(0, MIN<int16>(height, sH - top ));

	// Evaluate the block type
	if      (type == 0x01)
		renderBlockSparse  (imdVidMem, srcPtr, drawWidth, drawHeight, sW, width);
	else if (type == 0x02)
		renderBlockWhole   (imdVidMem, srcPtr, drawWidth, drawHeight, sW, width);
	else if (type == 0x42)
		renderBlockWhole4X (imdVidMem, srcPtr, drawWidth, drawHeight, sW, width);
	else if ((type & 0x0F) == 0x02)
		renderBlockWhole2Y (imdVidMem, srcPtr, drawWidth, drawHeight, sW, width);
	else
		renderBlockSparse2Y(imdVidMem, srcPtr, drawWidth, drawHeight, sW, width);

	return retVal;
}

void Imd::deLZ77(byte *dest, byte *src) {
	int i;
	byte buf[4370];
	uint16 chunkLength;
	uint32 frameLength;
	uint16 bufPos1;
	uint16 bufPos2;
	uint16 tmp;
	uint8 chunkBitField;
	uint8 chunkCount;
	bool mode;

	frameLength = READ_LE_UINT32(src);
	src += 4;

	if ((READ_LE_UINT16(src) == 0x1234) && (READ_LE_UINT16(src + 2) == 0x5678)) {
		src += 4;
		bufPos1 = 273;
		mode = 1; // 123Ch (cmp al, 12h)
	} else {
		bufPos1 = 4078;
		mode = 0; // 275h (jnz +2)
	}

	memset(buf, 32, bufPos1);
	chunkCount = 1;
	chunkBitField = 0;

	while (frameLength > 0) {
		chunkCount--;
		if (chunkCount == 0) {
			tmp = *src++;
			chunkCount = 8;
			chunkBitField = tmp;
		}
		if (chunkBitField % 2) {
			chunkBitField >>= 1;
			buf[bufPos1] = *src;
			*dest++ = *src++;
			bufPos1 = (bufPos1 + 1) % 4096;
			frameLength--;
			continue;
		}
		chunkBitField >>= 1;

		tmp = READ_LE_UINT16(src);
		src += 2;
		chunkLength = ((tmp & 0xF00) >> 8) + 3;

		if ((mode && ((chunkLength & 0xFF) == 0x12)) ||
				(!mode && (chunkLength == 0)))
			chunkLength = *src++ + 0x12;

		bufPos2 = (tmp & 0xFF) + ((tmp >> 4) & 0x0F00);
		if (((tmp + chunkLength) >= 4096) ||
				((chunkLength + bufPos1) >= 4096)) {

			for (i = 0; i < chunkLength; i++, dest++) {
				*dest = buf[bufPos2];
				buf[bufPos1] = buf[bufPos2];
				bufPos1 = (bufPos1 + 1) % 4096;
				bufPos2 = (bufPos2 + 1) % 4096;
			}

		} else if (((tmp + chunkLength) < bufPos1) ||
				((chunkLength + bufPos1) < bufPos2)) {

			memcpy(dest, buf + bufPos2, chunkLength);
			memmove(buf + bufPos1, buf + bufPos2, chunkLength);

			dest += chunkLength;
			bufPos1 += chunkLength;
			bufPos2 += chunkLength;

		} else {

			for (i = 0; i < chunkLength; i++, dest++, bufPos1++, bufPos2++) {
				*dest = buf[bufPos2];
				buf[bufPos1] = buf[bufPos2];
			}

		}
		frameLength -= chunkLength;

	}
}

inline void Imd::unsignedToSigned(byte *buffer, int length) {
	while (length-- > 0) *buffer++ ^= 0x80;
}


Vmd::ExtraData::ExtraData() {
	memset(name, 0, 16);

	offset   = 0;
	size     = 0;
	realSize = 0;
}


Vmd::Part::Part() {
	type    = kPartTypeSeparator;
	field_1 = 0;
	field_E = 0;
	size    = 0;
	left    = 0;
	top     = 0;
	right   = 0;
	bottom  = 0;
	id      = 0;
	flags   = 0;
}


Vmd::Frame::Frame() {
	parts  = 0;
	offset = 0;
}

Vmd::Frame::~Frame() {
	delete[] parts;
}


const uint16 Vmd::_tableDPCM[128] = {
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

const int32 Vmd::_tableADPCM[] = {
			7,     8,     9,    10,    11,    12,    13,    14,
		 16,    17,    19,    21,    23,    25,    28,    31,
		 34,    37,    41,    45,    50,    55,    60,    66,
		 73,    80,    88,    97,   107,   118,   130,   143,
		157,   173,   190,   209,   230,   253,   279,   307,
		337,   371,   408,   449,   494,   544,   598,   658,
		724,   796,   876,   963,  1060,  1166,  1282,  1411,
	 1552,  1707,  1878,  2066,  2272,  2499,  2749,  3024,
	 3327,  3660,  4026,  4428,  4871,  5358,  5894,  6484,
	 7132,  7845,  8630,  9493, 10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794,
	32767,     0
};

const int32 Vmd::_tableADPCMStep[] = {
	-1, -1, -1, -1, 2,  4,  6,  8,
	-1, -1, -1, -1, 2,  4,  6,  8
};

Vmd::Vmd(Graphics::PaletteLUT *palLUT) : _palLUT(palLUT) {
	zeroData();
}

Vmd::~Vmd() {
	deleteData();
	zeroData();
}

bool Vmd::assessVideoProperties() {
	if (_bytesPerPixel > 1)
		_features |= kFeaturesFullColor;
	else
		_features |= kFeaturesPalette;

	if ((_version & 2) && !(_version & 8)) {
		_externalCodec = true;
		_frameDataSize = _vidBufferSize = 0;
	} else
		_externalCodec = false;

	if (_externalCodec) {
		if (_videoCodec == MKID_BE('iv32')) {
#ifdef USE_INDEO3
			_features &= ~kFeaturesPalette;
			_features |=  kFeaturesFullColor;
			_codecIndeo3 = new Indeo3(_width, _height, _palLUT);
#else
			warning("Vmd::assessVideoProperties(): Indeo3 decoder not compiled in");
#endif
		} else {
			char *fourcc = (char *) &_videoCodec;

			warning("Vmd::assessVideoProperties(): Unknow video codec FourCC \'%c%c%c%c\'",
					fourcc[3], fourcc[2], fourcc[1], fourcc[0]);
			return false;
		}
	}

	_preScaleX  = 1;
	_postScaleX = 1;

	if (_externalCodec)
		_blitMode = 0;
	else if (_bytesPerPixel == 1)
		_blitMode = 0;
	else if ((_bytesPerPixel == 2) || (_bytesPerPixel == 3)) {
		int n = (_flags & 0x80) ? 2 : 3;

		_blitMode = n - 1;

		if (_bytesPerPixel == 2) {
			_preScaleX  = n;
			_postScaleX = 1;
		} else if (_bytesPerPixel == 3) {
			_preScaleX  = 1;
			_postScaleX = n;
		}

		_bytesPerPixel = n;
	}

	_scaleExternalX = 1;
	if (!_externalCodec && !(_flags & 0x1000))
			_scaleExternalX = _bytesPerPixel;

	if (_hasVideo) {
		if ((_frameDataSize == 0) || (_frameDataSize > 1048576))
			_frameDataSize = _width * _height + 1000;
		if ((_vidBufferSize == 0) || (_vidBufferSize > 1048576))
			_vidBufferSize = _frameDataSize;

		_frameData = new byte[_frameDataSize];
		assert(_frameData);
		memset(_frameData, 0, _frameDataSize);

		_vidBuffer = new byte[_vidBufferSize];
		assert(_vidBuffer);
		memset(_vidBuffer, 0, _vidBufferSize);

		if (_blitMode > 0) {
			_vidMemBuffer = new byte[_bytesPerPixel * (_width * _height + 1000)];
			memset(_vidMemBuffer, 0, _bytesPerPixel * (_width * _height + 1000));
		}
	}

#ifdef USE_INDEO3
	if (_externalCodec && _codecIndeo3)
		_features |= kFeaturesSupportsDouble;
#endif

	return true;
}

bool Vmd::assessAudioProperties() {
	bool supportedFormat = true;

	_features |= kFeaturesSound;

	_soundStereo = (_soundFlags & 0x8000) ? 1 : ((_soundFlags & 0x200) ? 2 : 0);

	if (_soundSliceSize < 0) {
		_soundBytesPerSample = 2;
		_soundSliceSize      = -_soundSliceSize;

		if (_soundFlags & 0x10) {
			_audioFormat     = kAudioFormat16bitADPCM;
			_soundHeaderSize = 3;
			_soundDataSize   = _soundSliceSize >> 1;

			if (_soundStereo > 0)
				supportedFormat = false;

		} else {
			_audioFormat     = kAudioFormat16bitDPCM;
			_soundHeaderSize = 1;
			_soundDataSize   = _soundSliceSize;

			if (_soundStereo == 1) {
				supportedFormat = false;
			} else if (_soundStereo == 2) {
				_soundDataSize = 2 * _soundDataSize + 2;
				_soundHeaderSize = 4;
			}

		}
	} else {
		_soundBytesPerSample = 1;
		_audioFormat         = kAudioFormat8bitDirect;
		_soundHeaderSize     = 0;
		_soundDataSize       = _soundSliceSize;

		if (_soundStereo > 0)
			supportedFormat = false;
	}

	if (!supportedFormat) {
		warning("Vmd::assessAudioProperties(): Unsupported audio format: %d bits, encoding %d, stereo %d",
				_soundBytesPerSample * 8, _audioFormat, _soundStereo);
		return false;
	}

	_soundSliceLength = (uint32) (((double) (1000 << 16)) /
			((double) _soundFreq / (double) _soundSliceSize));
	_frameLength = _soundSliceLength >> 16;

	_soundStage = 1;

	_audioStream = Audio::makeQueuingAudioStream(_soundFreq, _soundStereo != 0);

	return true;
}

void Vmd::readFrameTable(int &numExtraData) {
	numExtraData = 0;

	_stream->seek(_frameInfoOffset);
	_frames = new Frame[_framesCount];
	for (uint16 i = 0; i < _framesCount; i++) {
		_frames[i].parts = new Part[_partsPerFrame];
		_stream->skip(2); // Unknown
		_frames[i].offset = _stream->readUint32LE();
	}

	for (uint16 i = 0; i < _framesCount; i++) {
		bool separator = false;

		for (uint16 j = 0; j < _partsPerFrame; j++) {

			_frames[i].parts[j].type    = (PartType) _stream->readByte();
			_frames[i].parts[j].field_1 = _stream->readByte();
			_frames[i].parts[j].size    = _stream->readUint32LE();

			if (_frames[i].parts[j].type == kPartTypeAudio) {

				_frames[i].parts[j].flags = _stream->readByte();
				_stream->skip(9); // Unknown

			} else if (_frames[i].parts[j].type == kPartTypeVideo) {

				_frames[i].parts[j].left    = _stream->readUint16LE();
				_frames[i].parts[j].top     = _stream->readUint16LE();
				_frames[i].parts[j].right   = _stream->readUint16LE();
				_frames[i].parts[j].bottom  = _stream->readUint16LE();
				_frames[i].parts[j].field_E = _stream->readByte();
				_frames[i].parts[j].flags   = _stream->readByte();

			} else if (_frames[i].parts[j].type == kPartTypeSpeech) {
				_frames[i].parts[j].id = _stream->readUint16LE();
				// Speech text file name
				_stream->skip(8);
			} else if (_frames[i].parts[j].type == kPartTypeExtraData) {
				if (!separator)
					numExtraData++;
				_stream->skip(10);
			} else if (_frames[i].parts[j].type == kPartTypeSeparator) {
				separator = true;
				_stream->skip(10);
			} else {
				// Unknow type
				_stream->skip(10);
			}

		}
	}
}

void Vmd::readExtraData() {
	uint32 ssize = _stream->size();
	for (uint16 i = 0; i < _framesCount; i++) {
		_stream->seek(_frames[i].offset);

		for (uint16 j = 0; j < _partsPerFrame; j++) {
			if (_frames[i].parts[j].type == kPartTypeSeparator)
				break;

			if (_frames[i].parts[j].type == kPartTypeExtraData) {
				ExtraData data;

				data.offset   = _stream->pos() + 20;
				data.size     = _frames[i].parts[j].size;
				data.realSize = _stream->readUint32LE();

				_stream->read(data.name, 16);
				data.name[15] = '\0';

				_stream->skip(_frames[i].parts[j].size - 20);

				if ((((uint32) data.realSize) >= ssize) || (data.name[0] == 0))
					continue;

				_extraData.push_back(data);

			} else
				_stream->skip(_frames[i].parts[j].size);
		}
	}
}

bool Vmd::load(Common::SeekableReadStream &stream) {
	unload();

	_stream = &stream;

	uint16 headerLength;
	uint16 handle;

	headerLength = _stream->readUint16LE();
	handle       = _stream->readUint16LE();
	_version     = _stream->readUint16LE();

	bool readPalette;

	// Version checking
	if (headerLength == 50) {
		// Newer version, used in Addy 5 upwards
		warning("Vmd::load(): TODO: Addy 5 videos");
		readPalette = false;
	} else if (headerLength == 814) {
		// Old version
		readPalette = true;
	} else {
		warning("Vmd::load(): Version incorrect (%d, %d, %d)", headerLength, handle, _version);
		unload();
		return false;
	}

	_framesCount = _stream->readUint16LE();

	_x      = _stream->readSint16LE();
	_y      = _stream->readSint16LE();
	_width  = _stream->readSint16LE();
	_height = _stream->readSint16LE();

	if ((_width != 0) && (_height != 0)) {

		_hasVideo = true;
		_features |= kFeaturesVideo;

	} else
		_hasVideo = false;

	_bytesPerPixel = 1;
	if (_version & 4)
		_bytesPerPixel = handle + 1;

	if (_bytesPerPixel > 3) {
		warning("Vmd::load(): Requested %d bytes per pixel (%d, %d, %d)", _bytesPerPixel, headerLength, handle, _version);
		unload();
		return false;
	}

	_flags = _stream->readUint16LE();

	_partsPerFrame = _stream->readUint16LE();
	_firstFramePos = _stream->readUint32LE();

	_videoCodec = _stream->readUint32BE();

	if (readPalette)
		_stream->read((byte *) _palette, 768);

	_frameDataSize = _stream->readUint32LE();
	_vidBufferSize = _stream->readUint32LE();

	_doubleMode = false;

	if (_hasVideo) {
		if (!assessVideoProperties()) {
			unload();
			return false;
		}
	}

	_soundFreq        = _stream->readSint16LE();
	_soundSliceSize   = _stream->readSint16LE();
	_soundSlicesCount = _stream->readSint16LE();
	_soundFlags       = _stream->readUint16LE();

	_hasSound = (_soundFreq != 0);

	if (_hasSound) {
		if (!assessAudioProperties()) {
			unload();
			return false;
		}
	} else
		_frameLength = 1000 / _frameRate;

	_frameInfoOffset = _stream->readUint32LE();

	int numExtraData;
	readFrameTable(numExtraData);

	_stream->seek(_firstFramePos);

	if (numExtraData == 0)
		return true;

	_extraData.reserve(numExtraData);
	readExtraData();

	_stream->seek(_firstFramePos);
	return true;
}

void Vmd::unload() {
	clear();
}

int16 Vmd::getWidth() const {
	return preScaleX(_width);
}

void Vmd::setXY(int16 x, int16 y) {

	x *= _scaleExternalX;

	for (int i = 0; i < _framesCount; i++) {
		for (int j = 0; j < _partsPerFrame; j++) {

			if (_frames[i].parts[j].type == kPartTypeVideo) {
				if (x >= 0) {
					_frames[i].parts[j].left  = _frames[i].parts[j].left  - _x + x;
					_frames[i].parts[j].right = _frames[i].parts[j].right - _x + x;
				}
				if (y >= 0) {
					_frames[i].parts[j].top    = _frames[i].parts[j].top    - _y + y;
					_frames[i].parts[j].bottom = _frames[i].parts[j].bottom - _y + y;
				}
			}

		}
	}

	if (x >= 0)
		_x = x;
	if (y >= 0)
		_y = y;
}

void Vmd::setDoubleMode(bool doubleMode) {
	if (_doubleMode == doubleMode)
		return;

	if (_vidBuffer) {
		delete[] _vidBuffer;

		if (doubleMode)
			_vidBufferSize *= 4;
		else
			_vidBufferSize /= 4;

		_vidBuffer = new byte[_vidBufferSize];
		assert(_vidBuffer);
		memset(_vidBuffer, 0, _vidBufferSize);

	}

#ifdef USE_INDEO3
	if (_codecIndeo3) {
		delete _codecIndeo3;

		_codecIndeo3 = new Indeo3(_width * (doubleMode ? 2 : 1),
				_height * (doubleMode ? 2 : 1), _palLUT);
	}
#endif

	_doubleMode = doubleMode;
}

void Vmd::seekFrame(int32 frame, int16 whence, bool restart) {
	if (!_stream)
		// Nothing to do
		return;

	// Find the frame to which to seek
	if (whence == SEEK_CUR)
		frame += _curFrame;
	else if (whence == SEEK_END)
		frame = _framesCount - frame - 1;
	else if (whence != SEEK_SET)
		return;

	if ((frame < 0) || (frame >= _framesCount))
		// Nothing to do
		return;

	// Restart sound
	if (_hasSound && (frame == 0) && (_soundStage == 0) && !_audioStream) {
		_soundStage = 1;
		_audioStream = Audio::makeQueuingAudioStream(_soundFreq, _soundStereo != 0);
	}

	// Seek
	_stream->seek(_frames[frame].offset);
	_curFrame = frame;
}

CoktelVideo::State Vmd::nextFrame() {
	State state;

	state = processFrame(_curFrame);
	_curFrame++;
	return state;
}

void Vmd::zeroData() {
	Imd::zeroData();

	_hasVideo   = true;
	_videoCodec = 0;

#ifdef USE_INDEO3
	_codecIndeo3 = 0;
#endif

	_partsPerFrame = 0;
	_frames = 0;

	_extraData.clear();

	_soundBytesPerSample = 1;
	_soundStereo         = 0;
	_soundHeaderSize     = 0;
	_soundDataSize       = 0;
	_audioFormat         = kAudioFormat8bitDirect;

	_externalCodec  = false;
	_doubleMode     = false;
	_blitMode       = 0;
	_bytesPerPixel  = 1;
	_preScaleX      = 1;
	_postScaleX     = 1;
	_scaleExternalX = 1;
	_vidMemBuffer   = 0;
}

void Vmd::deleteData() {
	Imd::deleteData();

#ifdef USE_INDEO3
		delete _codecIndeo3;
#endif
		delete[] _frames;
		delete[] _vidMemBuffer;
}

void Vmd::clear() {
	deleteData();
	zeroData();
}

CoktelVideo::State Vmd::processFrame(uint16 frame) {
	State state;
	bool startSound = false;

	seekFrame(frame);

	state.flags |= kStateNoVideoData;
	state.left   = 0x7FFF;
	state.top    = 0x7FFF;
	state.right  = 0;
	state.bottom = 0;

	if (!_vidMem)
		setVideoMemory();

	for (uint16 i = 0; (i < _partsPerFrame) && (frame < _framesCount); i++) {
		uint32 pos = _stream->pos();

		Part &part = _frames[frame].parts[i];

		if (part.type == kPartTypeAudio) {
			// Next sound slice data
			if (part.flags == 1) {

				if (_soundEnabled) {
					filledSoundSlice(part.size);

					if (_soundStage == 1)
						startSound = true;

				} else
					_stream->skip(part.size);

			// Initial sound data (all slices)
			} else if (part.flags == 2) {

				if (_soundEnabled) {
					uint32 mask = _stream->readUint32LE();
					filledSoundSlices(part.size - 4, mask);

					if (_soundStage == 1)
						startSound = true;

				} else
					_stream->skip(part.size);

			// Empty sound slice
			} else if (part.flags == 3) {

				if (_soundEnabled) {
					emptySoundSlice(_soundDataSize * _soundBytesPerSample);

					if (_soundStage == 1)
						startSound = true;
				}

				_stream->skip(part.size);
			} else if (part.flags == 4) {
				warning("Vmd::processFrame(): TODO: Addy 5 sound type 4 (%d)", part.size);
				disableSound();
				_stream->skip(part.size);
			} else {
				warning("Vmd::processFrame(): Unknown sound type %d", part.flags);
				_stream->skip(part.size);
			}

			_stream->seek(pos + part.size);

		} else if ((part.type == kPartTypeVideo) && !_hasVideo) {
			warning("Vmd::processFrame(): Header claims there's no video, but video found (%d)", part.size);
			_stream->skip(part.size);
		} else if ((part.type == kPartTypeVideo) && _hasVideo) {
			state.flags &= ~kStateNoVideoData;

			uint32 size = part.size;

			// New palette
			if (part.flags & 2) {
				uint8 index = _stream->readByte();
				uint8 count = _stream->readByte();

				_stream->read(_palette + index * 3, (count + 1) * 3);
				_stream->skip((255 - count) * 3);

				state.flags |= kStatePalette;

				size -= (768 + 2);
			}

			_stream->read(_frameData, size);
			_frameDataLen = size;

			int16 l = part.left, t = part.top, r = part.right, b = part.bottom;
			if (renderFrame(l, t, r, b)) {
				if (!_externalCodec) {
					l = preScaleX(l);
					r = preScaleX(r);
				}
				// Rendering succeeded, merging areas
				state.left   = MIN(state.left,   l);
				state.top    = MIN(state.top,    t);
				state.right  = MAX(state.right,  r);
				state.bottom = MAX(state.bottom, b);
			}

		} else if (part.type == kPartTypeSeparator) {
		} else if (part.type == kPartTypeExtraData) {
			_stream->skip(part.size);
		} else if (part.type == kPartType4) {
			// Unknown
			_stream->skip(part.size);
		} else if (part.type == kPartTypeSpeech) {
			state.flags |= kStateSpeech;
			state.speechId = part.id;
			// Always triggers when speech starts
			_stream->skip(part.size);
		} else {
			warning("Vmd::processFrame(): Unknown frame part type %d, size %d (%d of %d)", part.type, part.size, i + 1, _partsPerFrame);
		}
	}

	if (startSound && _soundEnabled) {
		if (_hasSound && _audioStream) {
			_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_audioHandle, _audioStream);
			_skipFrames = 0;
			_soundStage = 2;
		} else
			_soundStage = 0;
	}

	if ((_curFrame == (_framesCount - 1)) && (_soundStage == 2)) {
		_audioStream->finish();
		_mixer->stopHandle(_audioHandle);
		_audioStream = 0;
		_soundStage  = 0;
	}

	// If these are still 0x7FFF, no video data has been processed
	if ((state.left == 0x7FFF) || (state.top == 0x7FFF))
		state.left = state.top = state.right = state.bottom = 0;

	_lastFrameTime = g_system->getMillis();
	return state;
}

void Vmd::deRLE(byte *&destPtr, const byte *&srcPtr, int16 destLen, int16 srcLen) {
	srcPtr++;

	if (srcLen & 1) {
		byte data = *srcPtr++;

		if (destLen > 0) {
			*destPtr++ = data;
			destLen--;
		}
	}

	srcLen >>= 1;

	while (srcLen > 0) {
		uint8 tmp = *srcPtr++;
		if (tmp & 0x80) { // Verbatim copy
			tmp &= 0x7F;

			int16 copyCount = MAX<int16>(0, MIN<int16>(destLen, tmp * 2));

			memcpy(destPtr, srcPtr, copyCount);

			srcPtr  += tmp * 2;
			destPtr += copyCount;
			destLen -= copyCount;
		} else { // 2 bytes tmp times
			for (int i = 0; (i < tmp) && (destLen > 0); i++) {
				for (int j = 0; j < 2; j++) {
					if (destLen <= 0)
						break;

					*destPtr++ = srcPtr[j];
					destLen--;
				}
			}
			srcPtr += 2;
		}
		srcLen -= tmp;
	}
}

// A run-length-encoded sparse block
void Vmd::renderBlockRLE(byte *dest, const byte *src, int16 width, int16 height,
		int16 destPitch, int16 srcPitch) {

	for (int i = 0; i < height; i++) {
		byte *destRow = dest;
		int16 pixWritten = 0;

		while (pixWritten < srcPitch) {
			int16 pixCount = *src++;

			if (pixCount & 0x80) {
				int16 copyCount;

				pixCount = (pixCount & 0x7F) + 1;
				copyCount = MAX<int16>(0, MIN<int16>(pixCount, width - pixWritten));

				if (*src != 0xFF) { // Normal copy

					memcpy(destRow, src, copyCount);
					destRow += copyCount;
					src     += pixCount;
				} else
					deRLE(destRow, src, copyCount, pixCount);

				pixWritten += pixCount;
			} else { // "Hole"
				int16 copyCount = MAX<int16>(0, MIN<int16>(pixCount + 1, width - pixWritten));

				destRow    += copyCount;
				pixWritten += pixCount + 1;
			}

		}

		dest += destPitch;
	}

}

uint32 Vmd::renderFrame(int16 &left, int16 &top, int16 &right, int16 &bottom) {
	if (!_frameData || !_vidMem || (_width <= 0) || (_height <= 0))
		return 0;

	int16 width  = right  - left + 1;
	int16 height = bottom - top  + 1;
	int16 sW     = _vidMemWidth;
	int16 sH     = _vidMemHeight;

	byte *dataPtr   = _frameData;
	byte *imdVidMem = _vidMem + sW * top + left;
	byte *srcPtr;

	if ((left < 0) || (top < 0) || (right < 0) || (bottom < 0))
		return 1;
	if ((width <= 0) || (height <= 0))
		return 1;

	uint8 type;
	byte *dest = imdVidMem;

#ifdef USE_INDEO3
	uint32 dataLen = _frameDataLen;

	if (Indeo3::isIndeo3(dataPtr, dataLen)) {
		if (!_codecIndeo3)
			return 0;

		if (!_codecIndeo3->decompressFrame(dataPtr, dataLen, _vidBuffer,
					width * (_doubleMode ? 2 : 1), height * (_doubleMode ? 2 : 1)))
			return 0;

		type   = 2;
		srcPtr = _vidBuffer;
		width  = _width  * (_doubleMode ? 2 : 1);
		height = _height * (_doubleMode ? 2 : 1);
		right  = left + width  - 1;
		bottom = top  + height - 1;

	} else {

		if (_externalCodec) {
			warning("Unknown external codec");
			return 0;
		}

#else

	if (_externalCodec) {
		return 0;
	} else {

#endif

		type   = *dataPtr++;
		srcPtr =  dataPtr;

		if (_blitMode > 0) {
			dest = _vidMemBuffer + postScaleX(_width) * (top - _y) + postScaleX((left - _x));
			imdVidMem = _vidMem + _vidMemWidth * top + preScaleX(left);
			sW = postScaleX(_width);
			sH = _height;
		}

		if (type & 0x80) {
			// Frame data is compressed

			srcPtr = _vidBuffer;
			type &= 0x7F;
			if ((type == 2) && (postScaleX(width) == sW)) {
				// Directly uncompress onto the video surface
				deLZ77(dest, dataPtr);
				blit(imdVidMem, dest, width, height);
				return 1;
			} else
				deLZ77(srcPtr, dataPtr);
		}

	}

	width = postScaleX(width);

	int16 drawWidth  = MAX<int16>(0, MIN<int16>(width , sW - left));
	int16 drawHeight = MAX<int16>(0, MIN<int16>(height, sH - top ));

	// Evaluate the block type
	if      (type == 0x01)
		renderBlockSparse  (dest, srcPtr, drawWidth, drawHeight, sW, width);
	else if (type == 0x02)
		renderBlockWhole   (dest, srcPtr, drawWidth, drawHeight, sW, width);
	else if (type == 0x03)
		renderBlockRLE     (dest, srcPtr, drawWidth, drawHeight, sW, width);
	else if (type == 0x42)
		renderBlockWhole4X (dest, srcPtr, drawWidth, drawHeight, sW, width);
	else if ((type & 0x0F) == 0x02)
		renderBlockWhole2Y (dest, srcPtr, drawWidth, drawHeight, sW, width);
	else
		renderBlockSparse2Y(dest, srcPtr, drawWidth, drawHeight, sW, width);

	dest = _vidMemBuffer + postScaleX(_width) * (top - _y) + postScaleX(left - _x);
	blit(imdVidMem, dest, width, height);

	return 1;
}

inline int32 Vmd::preScaleX(int32 x) const {
	return x / _preScaleX;
}

inline int32 Vmd::postScaleX(int32 x) const {
	return x * _postScaleX;
}

void Vmd::blit(byte *dest, byte *src, int16 width, int16 height) {
	if (_blitMode == 0)
		return;

	if (_blitMode == 1)
		blit16(dest, src, preScaleX(_width), preScaleX(width), height);
	else if (_blitMode == 2)
		blit24(dest, src, preScaleX(_width), preScaleX(width), height);
}

void Vmd::blit16(byte *dest, byte *src, int16 srcPitch, int16 width, int16 height) {
	assert(_palLUT);

	Graphics::SierraLight *dither =
		new Graphics::SierraLight(width, _palLUT);

	for (int i = 0; i < height; i++) {
		byte *d = dest;
		byte *s = src;

		for (int j = 0; j < width; j++, s += 2) {
			uint16 data = READ_LE_UINT16(s);
			byte r = ((data & 0x7C00) >> 10);
			byte g = ((data & 0x03E0) >>  5);
			byte b = ((data & 0x001F) >>  0);
			byte dY, dU, dV;

			Graphics::PaletteLUT::RGB2YUV(r << 3, g << 3, b << 3, dY, dU, dV);

			byte p = dither->dither(dY, dU, dV, j);

			if ((dY == 0) || ((r == 0) && (g == 0) && (b == 0)))
				*d++ = 0;
			else
				*d++ = p;
		}

		dither->nextLine();
		dest += _vidMemWidth;
		src += 2 * srcPitch;
	}

	delete dither;
}

void Vmd::blit24(byte *dest, byte *src, int16 srcPitch, int16 width, int16 height) {
	assert(_palLUT);

	Graphics::SierraLight *dither =
		new Graphics::SierraLight(width, _palLUT);

	for (int i = 0; i < height; i++) {
		byte *d = dest;
		byte *s = src;

		for (int j = 0; j < width; j++, s += 3) {
			byte r = s[2];
			byte g = s[1];
			byte b = s[0];
			byte dY, dU, dV;

			Graphics::PaletteLUT::RGB2YUV(r, g, b, dY, dU, dV);

			byte p = dither->dither(dY, dU, dV, j);

			if ((dY == 0) || ((r == 0) && (g == 0) && (b == 0)))
				*d++ = 0;
			else
				*d++ = p;
		}

		dither->nextLine();
		dest += _vidMemWidth;
		src  += 3 * srcPitch;
	}

	delete dither;
}

byte *Vmd::deDPCM(const byte *data, uint32 &size, int32 init[2]) {
	if (!data || (size == 0))
		return 0;

	int channels = (_soundStereo > 0) ? 2 : 1;

	uint32 inSize  = size;
	uint32 outSize = size + channels;

	int16 *out   = (int16 *)malloc(outSize * 2);
	byte  *sound = (byte *) out;

	int channel = 0;

	for (int i = 0; i < channels; i++) {
		*out++        = TO_BE_16(init[channel]);

		channel = (channel + 1) % channels;
	}

	while (inSize-- > 0) {
		if (*data & 0x80)
			init[channel] -= _tableDPCM[*data++ & 0x7F];
		else
			init[channel] += _tableDPCM[*data++];

		init[channel] = CLIP<int32>(init[channel], -32768, 32767);
		*out++        = TO_BE_16(init[channel]);

		channel = (channel + 1) % channels;
	}

	size = outSize * 2;
	return sound;
}

// Yet another IMA ADPCM variant
byte *Vmd::deADPCM(const byte *data, uint32 &size, int32 init, int32 index) {
	if (!data || (size == 0))
		return 0;

	uint32 outSize = size * 2;

	int16 *out   = (int16 *)malloc(outSize * 2);
	byte  *sound = (byte *) out;

	index = CLIP<int32>(index, 0, 88);

	int32 predictor = _tableADPCM[index];

	uint32 dataByte = 0;
	bool newByte = true;

	size *= 2;
	while (size -- > 0) {
		byte code = 0;

		if (newByte) {
			dataByte = *data++;
			code = (dataByte >> 4) & 0xF;
		} else
			code = dataByte & 0xF;

		newByte = !newByte;

		index += _tableADPCMStep[code];
		index  = CLIP<int32>(index, 0, 88);

		int32 value = predictor / 8;

		if (code & 4)
			value += predictor;
		if (code & 2)
			value += predictor / 2;
		if (code & 1)
			value += predictor / 4;

		if (code & 8)
			init -= value;
		else
			init += value;

		init = CLIP<int32>(init, -32768, 32767);

		predictor = _tableADPCM[index];

		*out++ = TO_BE_16(init);
	}

	size = outSize * 2;
	return sound;
}

byte *Vmd::soundEmpty(uint32 &size) {
	if (!_audioStream)
		return 0;

	byte *soundBuf = (byte *)malloc(size);
	memset(soundBuf, 0, size);

	return soundBuf;
}

byte *Vmd::sound8bitDirect(uint32 &size) {
	if (!_audioStream) {
		_stream->skip(size);
		return 0;
	}

	byte *soundBuf = (byte *)malloc(size);
	_stream->read(soundBuf, size);
	unsignedToSigned(soundBuf, size);

	return soundBuf;
}

byte *Vmd::sound16bitDPCM(uint32 &size) {
	if (!_audioStream) {
		_stream->skip(size);
		return 0;
	}

	int32 init[2];

	init[0] = _stream->readSint16LE();
	size -= 2;

	if (_soundStereo > 0) {
		init[1] = _stream->readSint16LE();
		size -= 2;
	}

	byte *data  = new byte[size];
	byte *sound = 0;

	if (_stream->read(data, size) == size)
		sound = deDPCM(data, size, init);

	delete[] data;

	return sound;
}

byte *Vmd::sound16bitADPCM(uint32 &size) {
	if (!_audioStream) {
		_stream->skip(size);
		return 0;
	}

	int32 init = _stream->readSint16LE();
	size -= 2;

	int32 index = _stream->readByte();
	size--;

	byte *data  = new byte[size];
	byte *sound = 0;

	if (_stream->read(data, size) == size)
		sound = deADPCM(data, size, init, index);

	delete[] data;

	return sound;
}

void Vmd::emptySoundSlice(uint32 size) {
	byte *sound = soundEmpty(size);

	if (sound) {
		uint32 flags = 0;
		flags |= (_soundBytesPerSample == 2) ? Audio::FLAG_16BITS : 0;
		flags |= (_soundStereo > 0) ? Audio::FLAG_STEREO : 0;

		_audioStream->queueBuffer(sound, size, DisposeAfterUse::YES, flags);
	}
}

void Vmd::filledSoundSlice(uint32 size) {
	byte *sound = 0;
	if (_audioFormat == kAudioFormat8bitDirect)
		sound = sound8bitDirect(size);
	else if (_audioFormat == kAudioFormat16bitDPCM)
		sound = sound16bitDPCM(size);
	else if (_audioFormat == kAudioFormat16bitADPCM)
		sound = sound16bitADPCM(size);

	if (sound) {
		uint32 flags = 0;
		flags |= (_soundBytesPerSample == 2) ? Audio::FLAG_16BITS : 0;
		flags |= (_soundStereo > 0) ? Audio::FLAG_STEREO : 0;

		_audioStream->queueBuffer(sound, size, DisposeAfterUse::YES, flags);
	}
}

uint8 Vmd::evaluateMask(uint32 mask, bool *fillInfo, uint8 &max) {
	max = MIN<int>(_soundSlicesCount - 1, 31);

	uint8 n = 0;
	for (int i = 0; i < max; i++) {

		if (!(mask & 1)) {
			n++;
			*fillInfo++ = true;
		} else
			*fillInfo++ = false;

		mask >>= 1;
	}

	return n;
}

void Vmd::filledSoundSlices(uint32 size, uint32 mask) {
	bool fillInfo[32];

	uint8 max;
	uint8 n = evaluateMask(mask, fillInfo, max);

	int32 extraSize;

	extraSize = size - n * _soundDataSize;

	if (_soundSlicesCount > 32)
		extraSize -= (_soundSlicesCount - 32) * _soundDataSize;

	if (n > 0)
		extraSize /= n;

	for (uint8 i = 0; i < max; i++)
		if (fillInfo[i])
			filledSoundSlice(_soundDataSize + extraSize);
		else
			emptySoundSlice(_soundDataSize * _soundBytesPerSample);

	if (_soundSlicesCount > 32)
		filledSoundSlice((_soundSlicesCount - 32) * _soundDataSize + _soundHeaderSize);
}

bool Vmd::getPartCoords(int16 frame, PartType type,
		int16 &x, int16 &y, int16 &width, int16 &height) {

	if (frame >= _framesCount)
		return false;

	Frame &f = _frames[frame];

	// Look for a part matching the requested type, stopping at a separator
	Part *part = 0;
	for (int i = 0; i < _partsPerFrame; i++) {
		Part &p = f.parts[i];

		if ((p.type == kPartTypeSeparator) || (p.type == type)) {
			part = &p;
			break;
		}
	}

	if (!part)
		return false;

	x      = part->left;
	y      = part->top;
	width  = part->right  - part->left + 1;
	height = part->bottom - part->top  + 1;

	return true;
}

bool Vmd::getFrameCoords(int16 frame,
		int16 &x, int16 &y, int16 &width, int16 &height) {

	return getPartCoords(frame, kPartTypeVideo, x, y, width, height);
}

bool Vmd::hasExtraData() const {
	return !_extraData.empty();
}

bool Vmd::hasExtraData(const char *fileName) const {
	for (uint i = 0; i < _extraData.size(); i++)
		if (!scumm_stricmp(_extraData[i].name, fileName))
			return true;

	return false;
}

Common::MemoryReadStream *Vmd::getExtraData(const char *fileName) {
	uint i = 0;

	for (i = 0; i < _extraData.size(); i++)
		if (!scumm_stricmp(_extraData[i].name, fileName))
			break;

	if (i >= _extraData.size())
		return 0;

	if ((_extraData[i].size - 20) != _extraData[i].realSize) {
		warning("Vmd::getExtraData(): Sizes for \"%s\" differ! (%d, %d)",
				fileName, (_extraData[i].size - 20), _extraData[i].realSize);
		return 0;
	}

	if (!_stream->seek(_extraData[i].offset)) {
		warning("Vmd::getExtraData(): Can't seek to offset %d to (file \"%s\")",
				_extraData[i].offset, fileName);
		return 0;
	}

	byte *data = (byte *) malloc(_extraData[i].realSize);
	if (_stream->read(data, _extraData[i].realSize) != _extraData[i].realSize) {
		free(data);
		warning("Vmd::getExtraData(): Couldn't read %d bytes (file \"%s\")",
				_extraData[i].realSize, fileName);
	}

	Common::MemoryReadStream *stream =
		new Common::MemoryReadStream(data, _extraData[i].realSize, DisposeAfterUse::YES);

	return stream;
}

} // End of namespace Graphics

#endif // GRAPHICS_VIDEO_COKTELVIDEO_H
