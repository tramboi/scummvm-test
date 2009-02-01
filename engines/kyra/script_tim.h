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

#ifndef KYRA_SCRIPT_TIM_H
#define KYRA_SCRIPT_TIM_H

#include "kyra/kyra_v1.h"

#include "common/array.h"
#include "common/func.h"
#include "common/str.h"

namespace Kyra {

class WSAMovie_v2;
class Screen_v2;
struct TIM;
typedef Common::Functor2<const TIM*, const uint16*, int> TIMOpcode;

struct TIM {
	char filename[13];

	uint16 clickedButton;
	int16 dlgFunc;

	int16 procFunc;
	uint16 procParam;

	enum {
		kCountFuncs = 10
	};

	struct Function {
		uint16 *ip;

		uint32 lastTime;
		uint32 nextTime;

		uint16 *loopIp;

		uint16 *avtl;
	} func[kCountFuncs];

	enum {
		kWSASlots = 10
	};

	struct WSASlot {
		void *anim;

		int16 x, y;
		uint16 wsaFlags;
		uint16 offscreen;
	} wsa[kWSASlots];

	uint16 *avtl;
	uint8 *text;

	const Common::Array<const TIMOpcode*> *opcodes;
};

class TIMInterpreter {
public:
	struct Animation {
		WSAMovie_v2 *wsa;
		int16 x, y;
		uint16 wsaCopyParams;
	};

	TIMInterpreter(KyraEngine_v1 *vm, Screen_v2 *screen, OSystem *system);
	~TIMInterpreter();

	TIM *load(const char *filename, const Common::Array<const TIMOpcode*> *opcodes);
	void unload(TIM *&tim) const;

	Animation *initAnimStructIntern(int index, const char *filename, int x, int y, uint16 copyPara, uint16 wsaFlags);
	int freeAnimStruct(int index);

	void setLangData(const char *filename);
	void clearLangData() { delete[] _langData; _langData = 0; }

	void toggleDialogueSpeech(bool enable) { _dlgSpeechEnabled = enable; }
	void toggleRefresh(bool enable) { _refresh = enable; }
	void setWsaDrawPage2(int pageNum) { _drawPage2 = pageNum; }
	void setDialogueCompleteFlag(int val) { _dialogueComplete = val; }
	void setActiveSpeechFile(const char *filename) { _activeVoiceFile = filename; }

	const char *getCTableEntry(uint idx) const;

	void resetFinishedFlag() { _finished = false; }
	bool finished() const { return _finished; }

	int exec(TIM *tim, bool loop);
	void stopCurFunc() { if (_currentTim) cmd_stopCurFunc(0); }
	void stopAllFuncs(TIM *tim);

	void refreshTimersAfterPause(uint32 elapsedTime);

	void displayText(uint16 textId, int16 flags);
	void setupTextPalette(uint index, int fadePalette);

	int _palDelayInc, _palDiff, _palDelayAcc;

private:
	KyraEngine_v1 *_vm;
	Screen_v2 *_screen;
	OSystem *_system;

	TIM *_currentTim;
	int _currentFunc;

	bool _finished;

	Common::String _vocFiles[120];

	Animation *initAnimStruct(int index, const char *filename, int x, int y, int, int offscreenBuffer, uint16 wsaFlags);
	Animation _animations[TIM::kWSASlots];

	char _audioFilename[32];

	uint8 *_langData;
	char *getTableEntry(uint idx);
	bool _textDisplayed;
	uint8 *_textAreaBuffer;

	bool _dlgSpeechEnabled;
	bool _refresh;
	int _drawPage2;

	int _dialogueComplete;
	const char *_activeVoiceFile;

	void advanceToOpcode(int del);
	int execCommand(int cmd, const uint16 *param);

	typedef int (TIMInterpreter::*CommandProc)(const uint16 *);
	struct CommandEntry {
		CommandProc proc;
		const char *desc;
	};

	const CommandEntry *_commands;
	int _commandsSize;

	int cmd_initFunc0(const uint16 *param);
	int cmd_stopCurFunc(const uint16 *param);
	int cmd_initWSA(const uint16 *param);
	int cmd_uninitWSA(const uint16 *param);
	int cmd_initFunc(const uint16 *param);
	int cmd_stopFunc(const uint16 *param);
	int cmd_wsaDisplayFrame(const uint16 *param);
	int cmd_displayText(const uint16 *param);
	int cmd_loadVocFile(const uint16 *param);
	int cmd_unloadVocFile(const uint16 *param);
	int cmd_playVocFile(const uint16 *param);
	int cmd_loadSoundFile(const uint16 *param);
	int cmd_playMusicTrack(const uint16 *param);
	int cmd_setLoopIp(const uint16 *param);
	int cmd_continueLoop(const uint16 *param);
	int cmd_resetLoopIp(const uint16 *param);
	int cmd_resetAllRuntimes(const uint16 *param);
	int cmd_execOpcode(const uint16 *param);
	int cmd_initFuncNow(const uint16 *param);
	int cmd_stopFuncNow(const uint16 *param);

	int cmd_stopAllFuncs(const uint16 *param);
	int cmd_processDialogue(const uint16 *param);
	int cmd_dialogueBox(const uint16 *param);

#define cmd_return(n, v) \
	int cmd_return_##n(const uint16 *) { return v; }

	cmd_return( 1,  1);
	cmd_return(n1, -1);
#undef cmd_return
};

} // end of namespace Kyra

#endif

