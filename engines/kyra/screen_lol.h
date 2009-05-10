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

#ifdef ENABLE_LOL

#ifndef KYRA_SCREEN_LOL_H
#define KYRA_SCREEN_LOL_H

#include "kyra/screen_v2.h"

namespace Kyra {

class LoLEngine;

class Screen_LoL : public Screen_v2 {
public:
	Screen_LoL(LoLEngine *vm, OSystem *system);
	~Screen_LoL();

	bool init();

	void setScreenDim(int dim);
	const ScreenDim *getScreenDim(int dim);
	int curDimIndex() { return _curDimIndex; }
	void modifyScreenDim(int dim, int x, int y, int w, int h);

	void fprintString(const char *format, int x, int y, uint8 col1, uint8 col2, uint16 flags, ...);
	void fprintStringIntro(const char *format, int x, int y, uint8 c1, uint8 c2, uint8 c3, uint16 flags, ...);

	void drawGridBox(int x, int y, int w, int h, int col);
	void fadeClearSceneWindow(int delay);

	// smooth scrolling
	void backupSceneWindow(int srcPageNum, int dstPageNum);
	void restoreSceneWindow(int srcPageNum, int dstPageNum);
	void smoothScrollZoomStepTop(int srcPageNum, int dstPageNum, int x, int y);
	void smoothScrollZoomStepBottom(int srcPageNum, int dstPageNum, int x, int y);
	void smoothScrollHorizontalStep(int pageNum, int x, int u2, int w);
	void smoothScrollTurnStep1(int srcPage1Num, int srcPage2Num, int dstPageNum);
	void smoothScrollTurnStep2(int srcPage1Num, int srcPage2Num, int dstPageNum);
	void smoothScrollTurnStep3(int srcPage1Num, int srcPage2Num, int dstPageNum);

	// palette stuff
	void fadeToBlack(int delay=0x54, const UpdateFunctor *upFunc = 0);
	void loadSpecialColors(uint8 *destPalette);
	void copyColor(int dstColorIndex, int srcColorIndex);
	bool fadeColor(int dstColorIndex, int srcColorIndex, uint32 elapsedTime, uint32 targetTime);
	bool fadePalSpecial(uint8 *pal1, uint8 *pal2, uint32 elapsedTime, uint32 targetTime);

	void generateGrayOverlay(const uint8 *srcPal, uint8 *grayOverlay, int factor, int addR, int addG, int addB, int lastColor, bool skipSpecialColors);
	uint8 *generateLevelOverlay(const uint8 *srcPal, uint8 *ovl, int opColor, int weight);
	uint8 *getLevelOverlay(int index) { return _levelOverlays[index]; }

	void copyBlockSpecial(int page1, int x1, int y1, int page2, int x2, int y2, int w, int h, int dim, uint8 *ovl);
	void applyOverlaySpecial(int page1, int x1, int y1, int page2, int x2, int y2, int w, int h, int dim, int flag, uint8 *ovl);

	uint8 getShapePaletteSize(const uint8 *shp);

	uint8 *_paletteOverlay1;
	uint8 *_paletteOverlay2;
	uint8 *_grayOverlay;
	int _fadeFlag;

private:
	LoLEngine *_vm;

	const ScreenDim *_screenDimTable;
	static const int _screenDimTableCount;

	static const ScreenDim _screenDimTable256C[];
	static const ScreenDim _screenDimTable16C[];

	ScreenDim **_customDimTable;
	int _curDimIndex;

	uint8 *_levelOverlays[8];

	// magic atlas
	void calcMapBoundaries(int dstX, int dstY, int c, int d);

	int _internDimX;
	int _internDimY;
	int _internDimW;
	int _internDimH;
	int _internDimDstX;
	int _internBlockWidth;
	int _internDimDstY;
	int _internBlockHeight;
	int _internDimU5;
	int _internDimU6;
	int _internBlockWidth2;
	int _internDimU8;
};

} // end of namespace Kyra

#endif

#endif // ENABLE_LOL

