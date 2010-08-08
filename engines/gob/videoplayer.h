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

#ifndef GOB_VIDEOPLAYER_H
#define GOB_VIDEOPLAYER_H

#include "common/array.h"
#include "common/str.h"

#include "graphics/video/coktel_decoder.h"

#include "gob/util.h"

namespace Gob {

class GobEngine;
class DataStream;

class VideoPlayer {
public:
	enum Flags {
		kFlagNone = 0,
		kFlagUseBackSurfaceContent = 0x40,
		kFlagFrontSurface = 0x80,
		kFlagNoVideo = 0x100,
		kFlagOtherSurface = 0x800,
		kFlagScreenSurface = 0x400000
	};

	enum Type {
		kVideoTypeTry = -1,
		kVideoTypeIMD = 0,
		kVideoTypePreIMD = 1,
		kVideoTypeVMD = 2,
		kVideoTypeRMD = 3
	};

	VideoPlayer(GobEngine *vm);
	~VideoPlayer();

	bool primaryOpen(const char *videoFile, int16 x = -1, int16 y = -1,
			int32 flags = kFlagFrontSurface, Type which = kVideoTypeTry,
			int16 width = -1, int16 height = -1);
	bool primaryPlay(int16 startFrame = -1, int16 lastFrame = -1,
			int16 breakKey = kShortKeyEscape,
			uint16 palCmd = 8, int16 palStart = 0, int16 palEnd = 255,
			int16 palFrame = -1, int16 endFrame = -1, bool fade = false,
			int16 reverseTo = -1, bool forceSeek = false);
	void primaryClose();

	void playFrame(int16 frame, int16 breakKey = kShortKeyEscape,
			uint16 palCmd = 8, int16 palStart = 0, int16 palEnd = 255,
			int16 palFrame = -1 , int16 endFrame = -1, bool noRetrace = false);

	int slotOpen(const char *videoFile, Type which = kVideoTypeTry,
			int16 width = -1, int16 height = -1);
	void slotPlay(int slot, int16 frame = -1);
	void slotClose(int slot);
	void slotCopyFrame(int slot, byte *dest,
			uint16 left, uint16 top, uint16 width, uint16 height,
			uint16 x, uint16 y, uint16 pitch, int16 transp = -1);
	void slotCopyPalette(int slot, int16 palStart = -1, int16 palEnd = -1);
	void slotWaitEndFrame(int slot = -1, bool onlySound = false);

	void slotSetDoubleMode(int slot, bool doubleMode);

	bool slotIsOpen(int slot) const;

	const Common::String &getPrimaryFileName() const;
	uint16 getFlags(int slot = -1) const;
	int16 getFrameCount(int slot = -1) const;
	int16 getCurrentFrame(int slot = -1) const;
	int16 getWidth(int slot = -1) const;
	int16 getHeight(int slot = -1) const;
	int16 getDefaultX(int slot = -1) const;
	int16 getDefaultY(int slot = -1) const;

	void getState(int slot = -1) const;
	uint32 getFeatures(int slot = -1) const;

	bool hasExtraData(const char *fileName, int slot = -1) const;
	Common::MemoryReadStream *getExtraData(const char *fileName, int slot = -1);

	void writeVideoInfo(const char *videoFile, int16 varX, int16 varY,
			int16 varFrames, int16 varWidth, int16 varHeight);

private:
	static const int kVideoSlotCount = 32;

	static const char *_extensions[];

	GobEngine *_vm;

	Graphics::CoktelDecoder *_primaryVideo;
	Graphics::CoktelDecoder *_videoSlots[kVideoSlotCount];

	Common::String _primaryFileName;

	bool _ownSurf;
	bool _backSurf;
	bool _needBlit;
	bool _noCursorSwitch;

	bool _woodruffCohCottWorkaround;

	bool findFile(char *fileName, Type &which);

	const Graphics::CoktelDecoder *getVideoBySlot(int slot = -1) const;
	Graphics::CoktelDecoder *getVideoBySlot(int slot = -1);

	int getNextFreeSlot();

	void copyPalette(Graphics::CoktelDecoder &video, int16 palStart = -1, int16 palEnd = -1);
	bool doPlay(int16 frame, int16 breakKey,
			uint16 palCmd, int16 palStart, int16 palEnd,
			int16 palFrame, int16 endFrame, bool noRetrace = false);
	void evalBgShading(Graphics::CoktelDecoder &video);

	Graphics::CoktelDecoder *openVideo(const char *fileName, Type which, uint16 width, uint16 height);
};

} // End of namespace Gob

#endif // GOB_VIDEOPLAYER_H
