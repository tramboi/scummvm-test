/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef CONTROL_H
#define CONTROL_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "sky/struc.h"
#include "common/engine.h"
#include "sky/screen.h"
#include "sky/disk.h"
#include "sky/mouse.h"
#include "sky/logic.h"

class SkyScreen;
class SkyLogic;
class SkyMouse;

#define MAX_SAVE_GAMES 999
#define MAX_TEXT_LEN 80
#define PAN_LINE_WIDTH 184
#define PAN_CHAR_HEIGHT 12
#define STATUS_WIDTH 146
#define MPNL_X 60  // Main Panel
#define MPNL_Y 10

#define SPNL_X 20  // Save Panel
#define SPNL_Y 20
#define SP_HEIGHT 149
#define SP_TOP_GAP 12
#define SP_BOT_GAP 27
#define CROSS_SZ_X 27
#define CROSS_SZ_Y 22

#define TEXT_FLAG_MASK (SF_ALLOW_SPEECH | SF_ALLOW_TEXT)

#define GAME_NAME_X (SPNL_X + 18)				// x coordinate of game names
#define GAME_NAME_Y (SPNL_Y + SP_TOP_GAP)		// start y coord of game names
#define MAX_ON_SCREEN ((SP_HEIGHT - SP_TOP_GAP - SP_BOT_GAP) / PAN_CHAR_HEIGHT) // no of save games on screen
#define CP_PANEL 60400 // main panel sprite

#define MAINPANEL 0
#define SAVEPANEL 1

#define NO_MASK false
#define WITH_MASK true

// resource's onClick routines
#define DO_NOTHING		0
#define REST_GAME_PANEL	1
#define SAVE_GAME_PANEL	2
#define SAVE_A_GAME		3
#define RESTORE_A_GAME	4
#define SP_CANCEL		5
#define SHIFT_DOWN_FAST	6
#define SHIFT_DOWN_SLOW	7
#define SHIFT_UP_FAST	8
#define SHIFT_UP_SLOW	9
#define SPEED_SLIDE		10
#define MUSIC_SLIDE		11
#define TOGGLE_FX		12
#define TOGGLE_MS		13
#define TOGGLE_TEXT		14
#define EXIT			15
#define RESTART			16
#define QUIT_TO_DOS		17
#define RESTORE_AUTO	18

// onClick return codes
#define CANCEL_PRESSED	100
#define NAME_TOO_SHORT	101
#define GAME_SAVED		102
#define SHIFTED			103
#define TOGGLED			104
#define RESTARTED		105
#define GAME_RESTORED	106
#define RESTORE_FAILED	107
#define NO_DISK_SPACE	108
#define SPEED_CHANGED	109
#define QUIT_PANEL		110

#define SLOW 0
#define FAST 1

#define SPEED_MULTIPLY 8

//-
#define SAVE_EXT	 1
#define SAVE_MEGA0	 2
#define SAVE_MEGA1	 4
#define SAVE_MEGA2	 8
#define SAVE_MEGA3	16
#define SAVE_GRAFX	32
#define SAVE_TURNP	64
#define EVIL_GRAFX	128

#define SAVE_FILE_REVISION 4

struct AllocedMem {
	uint16 *mem;
	AllocedMem *next;
};

class SkyConResource {
public:
	SkyConResource(void *pSpData, uint32 pNSprites, uint32 pCurSprite, uint16 pX, uint16 pY, uint32 pText, uint8 pOnClick, OSystem *system, uint8 *screen);
	virtual ~SkyConResource(void) {};
	void setSprite(void *pSpData) { _spriteData = (dataFileHeader*)pSpData; };
	void setText(uint32 pText) { if (pText) _text = pText + 0x7000; else _text = 0; };
	void setXY(uint16 x, uint16 y) { _x = x; _y = y; };
	bool isMouseOver(uint32 mouseX, uint32 mouseY);
	virtual void drawToScreen(bool doMask);

	dataFileHeader *_spriteData;
	uint32 _numSprites, _curSprite;
	uint16 _x, _y;
	uint32 _text;
	uint8 _onClick;
	OSystem *_system;
	uint8 *_screen;
private:
};

class SkyTextResource : public SkyConResource {
public:
	SkyTextResource(void *pSpData, uint32 pNSprites, uint32 pCurSprite, uint16 pX, uint16 pY, uint32 pText, uint8 pOnClick, OSystem *system, uint8 *screen);
	virtual ~SkyTextResource(void);
	virtual void drawToScreen(bool doMask);
	void flushForRedraw(void);
private:
	uint16 _oldX, _oldY;
	uint8 *_oldScreen;
};

class SkyControlStatus {
public:
	SkyControlStatus(SkyText *skyText, OSystem *system, uint8 *scrBuf);
	~SkyControlStatus(void);
	void setToText(const char *newText);
	void setToText(uint16 textNum);
	void drawToScreen(void);
private:
	SkyTextResource *_statusText;
	dataFileHeader *_textData;
	SkyText *_skyText;
	OSystem *_system;
	uint8 *_screenBuf;
};

class SkyControl {
public:
	SkyControl(SkyScreen *screen, SkyDisk *disk, SkyMouse *mouse, SkyText *text, SkyMusicBase *music, SkyLogic *logic, SkySound *sound, OSystem *system, const char *savePath);
	void doControlPanel(void);
	void doLoadSavePanel(void);
	void restartGame(void);
	void showGameQuitMsg(bool useScreen = true);
	void doAutoSave(void);
	uint16 quickXRestore(uint16 slot);
	bool loadSaveAllowed(void);
    
private:
	void initPanel(void);
	void removePanel(void);

	void drawMainPanel(void);

	void delay(unsigned int amount);
	
    void animClick(SkyConResource *pButton);
	bool getYesNo(char *text);
	void buttonControl(SkyConResource *pButton);
	uint16 handleClick(SkyConResource *pButton);
	uint16 doMusicSlide(void);
	uint16 doSpeedSlide(void);
	uint16 toggleFx(SkyConResource *pButton);
	uint16 toggleText(void);
	void toggleMusic(void);
	uint16 shiftDown(uint8 speed);
	uint16 shiftUp(uint8 speed);
	void drawTextCross(uint32 flags);
	void drawCross(uint16 x, uint16 y);

	uint16 saveRestorePanel(bool allowSave);
	void loadDescriptions(uint8 *destBuf);
	void saveDescriptions(uint8 *srcBuf);
	void setUpGameSprites(uint8 *nameBuf, dataFileHeader **nameSprites, uint16 firstNum, uint16 selectedGame);
	void showSprites(dataFileHeader **nameSprites, bool allowSave);
	bool checkKeyList(uint8 key);
	void handleKeyPress(uint8 key, uint8 *textBuf);

	uint16 _selectedGame;
	uint16 saveGameToFile(void);
	bool canSaveV5(void);
	void stosMegaSet(uint8 **destPos, MegaSet *mega);
	void stosCompact(uint8 **destPos, Compact *cpt);
	void stosGrafStr(uint8 **destPos, Compact *cpt);
	void stosStr(uint8 **destPos, Compact *cpt, uint16 type);
	uint32 prepareSaveData(uint8 *destBuf);

	bool autoSaveExists(void);
	uint16 restoreGameFromFile(bool autoSave);
	void lodsMegaSet(uint8 **srcPos, MegaSet *mega);
	void lodsCompact(uint8 **srcPos, Compact *cpt, uint32 saveRev);
	void lodsStr(uint8 **srcPos, uint16 *src);
	uint16 parseSaveData(uint8 *srcBuf);

	const char *_savePath;
	uint16 *lz77decode(uint16 *data);
	void applyDiff(uint16 *data, uint16 *diffData, uint16 len);
	static Compact *_saveLoadCpts[833]; //-----------------
	static uint16 *_saveLoadARs[19];
	static uint8 _resetData288[0x39B8];
	static uint8 _resetDiff303[824];    // moved to sky/compacts/savedata.cpp
	static uint8 _resetDiff331[824];
	static uint8 _resetDiff348[824];
	static uint8 _resetDiffCd[856];  //-----------------

	AllocedMem *_memListRoot;
	void appendMemList(uint16 *pMem);
	void freeMemList(void);

	SkyScreen *_skyScreen;
	SkyDisk *_skyDisk;
	SkyMouse *_skyMouse;
	SkyText *_skyText;
	SkyMusicBase *_skyMusic;
	SkyLogic *_skyLogic;
	SkySound *_skySound;
	OSystem *_system;
	int _mouseX, _mouseY;
	bool _mouseClicked;
	byte _keyPressed;

	struct {
		uint8 *controlPanel;
		uint8 *button;
		uint8 *buttonDown;
		uint8 *savePanel;
		uint8 *yesNo;
		uint8 *slide;
		uint8 *slode;
		uint8 *slode2;
		uint8 *slide2;
		uint8 *musicBodge;
	} _sprites;

	uint8 *_screenBuf;
	int _lastButton;
	uint32 _curButtonText;
	uint16 _firstText;
	uint16 _savedMouse;
	uint32 _savedCharSet;
	uint16 _enteredTextWidth;
    
	SkyConResource *createResource(void *pSpData, uint32 pNSprites, uint32 pCurSprite, int16 pX, int16 pY, uint32 pText, uint8 pOnClick, uint8 panelType);

	dataFileHeader *_textSprite;
	SkyTextResource *_text;

	SkyConResource *_controlPanel, *_exitButton, *_slide, *_slide2, *_slode;
	SkyConResource *_restorePanButton, *_savePanButton, *_dosPanButton, *_restartPanButton, *_fxPanButton, *_musicPanButton;
	SkyConResource *_bodge, *_yesNo;
	SkyConResource *_controlPanLookList[9];

	//- Save/restore panel
	SkyConResource *_savePanel;
	SkyConResource *_saveButton, *_downFastButton, *_downSlowButton;
	SkyConResource *_upFastButton, *_upSlowButton, *_quitButton, *_restoreButton;
	SkyConResource *_autoSaveButton;

	SkyConResource *_savePanLookList[6], *_restorePanLookList[7];

	SkyControlStatus *_statusBar;

	static char _quitTexts[16][35];
	static uint8 _crossImg[594];
};

#endif // CONTROL_H
