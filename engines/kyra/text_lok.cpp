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

#include "kyra/kyra_lok.h"
#include "kyra/screen_lok.h"
#include "kyra/text.h"
#include "kyra/animator_lok.h"
#include "kyra/sprites.h"
#include "kyra/timer.h"

namespace Kyra {

void KyraEngine_LoK::waitForChatToFinish(int vocFile, int16 chatDuration, const char *chatStr, uint8 charNum) {
	debugC(9, kDebugLevelMain, "KyraEngine_LoK::waitForChatToFinish(%i, %s, %i)", chatDuration, chatStr, charNum);
	bool hasUpdatedNPCs = false;
	bool runLoop = true;
	bool drawText = textEnabled();
	uint8 currPage;
	Common::Event event;

	//while (towns_isEscKeyPressed() )
		//towns_getKey();

	uint32 timeToEnd = strlen(chatStr) * 8 * _tickLength + _system->getMillis();

	if (_configVoice == 0 && chatDuration != -1) {
		switch (_configTextspeed) {
		case 0:
			chatDuration *= 2;
			break;
		case 2:
			chatDuration /= 4;
			break;
		case 3:
			chatDuration = -1;
			break;
		}
	}

	if (chatDuration != -1)
		chatDuration *= _tickLength;

	if (vocFile != -1)
		snd_playVoiceFile(vocFile);

	_timer->disable(14);
	_timer->disable(18);
	_timer->disable(19);

	uint32 timeAtStart = _system->getMillis();
	uint32 loopStart;
	while (runLoop) {
		loopStart = _system->getMillis();
		if (_currentCharacter->sceneId == 210)
			if (seq_playEnd())
				break;

		if (_system->getMillis() > timeToEnd && !hasUpdatedNPCs) {
			hasUpdatedNPCs = true;
			_timer->disable(15);
			_currHeadShape = 4;
			_animator->animRefreshNPC(0);
			_animator->animRefreshNPC(_talkingCharNum);

			if (_charSayUnk2 != -1) {
				_animator->sprites()[_charSayUnk2].active = 0;
				_sprites->_anims[_charSayUnk2].play = false;
				_charSayUnk2 = -1;
			}
		}

		_timer->update();
		_sprites->updateSceneAnims();
		_animator->restoreAllObjectBackgrounds();
		_animator->preserveAnyChangedBackgrounds();
		_animator->prepDrawAllObjects();

		if (drawText) {
			currPage = _screen->_curPage;
			_screen->_curPage = 2;
			_text->printCharacterText(chatStr, charNum, _characterList[charNum].x1);
			_animator->_updateScreen = true;
			_screen->_curPage = currPage;
		}

		_animator->copyChangedObjectsForward(0);
		updateTextFade();

		if (((chatDuration < (int16)(_system->getMillis() - timeAtStart)) && chatDuration != -1 && drawText) || (!drawText && !snd_voiceIsPlaying()))
			break;

		uint32 nextTime = loopStart + _tickLength;

		while (_system->getMillis() < nextTime) {
			while (_eventMan->pollEvent(event)) {
				switch (event.type) {
				case Common::EVENT_KEYDOWN:
					if (event.kbd.keycode == '.')
						_skipFlag = true;
					break;
				case Common::EVENT_RTL:
				case Common::EVENT_QUIT:
					runLoop = false;
					break;
				case Common::EVENT_LBUTTONDOWN:
					runLoop = false;
					break;
				default:
					break;
				}
			}

			if (nextTime - _system->getMillis() >= 10) {
				_system->delayMillis(10);
				_system->updateScreen();
			}
		}

		if (_skipFlag)
			runLoop = false;
	}

	if (_skipFlag)
		snd_stopVoice();

	_timer->enable(14);
	_timer->enable(15);
	_timer->enable(18);
	_timer->enable(19);
	//clearKyrandiaButtonIO();
}

void KyraEngine_LoK::endCharacterChat(int8 charNum, int16 convoInitialized) {
	_charSayUnk3 = -1;

	if (charNum > 4 && charNum < 11) {
		//TODO: weird _game_inventory stuff here
		//warning("STUB: endCharacterChat() for high charnums");
	}

	if (convoInitialized != 0) {
		_talkingCharNum = -1;
		if (_currentCharacter->currentAnimFrame != 88)
			_currentCharacter->currentAnimFrame = 7;
		_animator->animRefreshNPC(0);
		_animator->updateAllObjectShapes();
	}
}

void KyraEngine_LoK::restoreChatPartnerAnimFrame(int8 charNum) {
	_talkingCharNum = -1;

	if (charNum > 0 && charNum < 5) {
		_characterList[charNum].currentAnimFrame = _currentChatPartnerBackupFrame;
		_animator->animRefreshNPC(charNum);
	}

	if (_currentCharacter->currentAnimFrame != 88)
		_currentCharacter->currentAnimFrame = 7;

	_animator->animRefreshNPC(0);
	_animator->updateAllObjectShapes();
}

void KyraEngine_LoK::backupChatPartnerAnimFrame(int8 charNum) {
	_talkingCharNum = 0;

	if (charNum < 5 && charNum > 0)
		_currentChatPartnerBackupFrame = _characterList[charNum].currentAnimFrame;

	if (_currentCharacter->currentAnimFrame != 88) {
		_currentCharacter->currentAnimFrame = 16;
		if (_scaleMode != 0)
			_currentCharacter->currentAnimFrame = 7;
	}

	_animator->animRefreshNPC(0);
	_animator->updateAllObjectShapes();
}

int8 KyraEngine_LoK::getChatPartnerNum() {
	uint8 sceneTable[] = {0x2, 0x5, 0x2D, 0x7, 0x1B, 0x8, 0x22, 0x9, 0x30, 0x0A};
	int pos = 0;
	int partner = -1;

	for (int i = 1; i < 6; i++) {
		if (_currentCharacter->sceneId == sceneTable[pos]) {
			partner = sceneTable[pos+1];
			break;
		}
		pos += 2;
	}

	for (int i = 1; i < 5; i++) {
		if (_characterList[i].sceneId == _currentCharacter->sceneId) {
			partner = i;
			break;
		}
	}
	return partner;
}

int KyraEngine_LoK::initCharacterChat(int8 charNum) {
	int returnValue = 0;

	if (_talkingCharNum == -1) {
		returnValue = 1;
		_talkingCharNum = 0;

		if (_currentCharacter->currentAnimFrame != 88) {
			_currentCharacter->currentAnimFrame = 16;
			if (_scaleMode != 0)
				_currentCharacter->currentAnimFrame = 7;
		}

		_animator->animRefreshNPC(0);
		_animator->updateAllObjectShapes();
	}

	_charSayUnk2 = -1;
	_animator->flagAllObjectsForBkgdChange();
	_animator->restoreAllObjectBackgrounds();

	if (charNum > 4 && charNum < 11) {
		// TODO: Fill in weird _game_inventory stuff here
		//warning("STUB: initCharacterChat() for high charnums");
	}

	_animator->flagAllObjectsForRefresh();
	_animator->flagAllObjectsForBkgdChange();
	_animator->preserveAnyChangedBackgrounds();
	_charSayUnk3 = charNum;

	return returnValue;
}

void KyraEngine_LoK::characterSays(int vocFile, const char *chatStr, int8 charNum, int8 chatDuration) {
	debugC(9, kDebugLevelMain, "KyraEngine_LoK::characterSays('%s', %i, %d)", chatStr, charNum, chatDuration);
	uint8 startAnimFrames[] =  { 0x10, 0x32, 0x56, 0x0, 0x0, 0x0 };

	uint16 chatTicks;
	int16 convoInitialized;
	int8 chatPartnerNum;

	if (_currentCharacter->sceneId == 210)
		return;

	snd_voiceWaitForFinish(true);

	convoInitialized = initCharacterChat(charNum);
	chatPartnerNum = getChatPartnerNum();

	if (chatPartnerNum >= 0 && chatPartnerNum < 5)
		backupChatPartnerAnimFrame(chatPartnerNum);

	if (charNum < 5) {
		_characterList[charNum].currentAnimFrame = startAnimFrames[charNum];
		_charSayUnk3 = charNum;
		_talkingCharNum = charNum;
		_animator->animRefreshNPC(charNum);
	}

	char *processedString = _text->preprocessString(chatStr);
	int lineNum = _text->buildMessageSubstrings(processedString);

	int16 yPos = _characterList[charNum].y1;
	yPos -= ((_scaleTable[yPos] * _characterList[charNum].height) >> 8);
	yPos -= 8;
	yPos -= lineNum * 10;

	if (yPos < 11)
		yPos = 11;

	if (yPos > 100)
		yPos = 100;

	_text->_talkMessageY = yPos;
	_text->_talkMessageH = lineNum * 10;

	if (textEnabled()) {
		_animator->restoreAllObjectBackgrounds();

		_screen->copyRegion(12, _text->_talkMessageY, 12, 136, 296, _text->_talkMessageH, 2, 2);
		_screen->hideMouse();

		_text->printCharacterText(processedString, charNum, _characterList[charNum].x1);
		_screen->showMouse();
	}

	if (chatDuration == -2)
		chatTicks = strlen(processedString) * 9;
	else
		chatTicks = chatDuration;

	if (!speechEnabled())
		vocFile = -1;
	waitForChatToFinish(vocFile, chatTicks, chatStr, charNum);

	if (textEnabled()) {
		_animator->restoreAllObjectBackgrounds();

		_screen->copyRegion(12, 136, 12, _text->_talkMessageY, 296, _text->_talkMessageH, 2, 2);
		_animator->preserveAllBackgrounds();
		_animator->prepDrawAllObjects();
		_screen->hideMouse();

		_screen->copyRegion(12, _text->_talkMessageY, 12, _text->_talkMessageY, 296, _text->_talkMessageH, 2, 0);
		_screen->showMouse();
		_animator->flagAllObjectsForRefresh();
		_animator->copyChangedObjectsForward(0);
	}

	if (chatPartnerNum != -1 && chatPartnerNum < 5)
		restoreChatPartnerAnimFrame(chatPartnerNum);

	endCharacterChat(charNum, convoInitialized);
}

void KyraEngine_LoK::drawSentenceCommand(const char *sentence, int color) {
	debugC(9, kDebugLevelMain, "KyraEngine_LoK::drawSentenceCommand('%s', %i)", sentence, color);
	_screen->hideMouse();
	_screen->fillRect(8, 143, 311, 152, 12);

	if (_startSentencePalIndex != color || _fadeText != false) {
		_currSentenceColor[0] = _screen->_currentPalette[765] = _screen->_currentPalette[color*3];
		_currSentenceColor[1] = _screen->_currentPalette[766] = _screen->_currentPalette[color*3+1];
		_currSentenceColor[2] = _screen->_currentPalette[767] = _screen->_currentPalette[color*3+2];

		_screen->setScreenPalette(_screen->_currentPalette);
		_startSentencePalIndex = 0;
	}

	_text->printText(sentence, 8, 143, 0xFF, 12, 0);
	_screen->showMouse();
	setTextFadeTimerCountdown(15);
	_fadeText = false;
}

void KyraEngine_LoK::updateSentenceCommand(const char *str1, const char *str2, int color) {
	debugC(9, kDebugLevelMain, "KyraEngine_LoK::updateSentenceCommand('%s', '%s', %i)", str1, str2, color);
	char sentenceCommand[500];
	strncpy(sentenceCommand, str1, 500);
	if (str2)
		strncat(sentenceCommand, str2, 500 - strlen(sentenceCommand));

	drawSentenceCommand(sentenceCommand, color);
	_screen->updateScreen();
}

void KyraEngine_LoK::updateTextFade() {
	debugC(9, kDebugLevelMain, "KyraEngine_LoK::updateTextFade()");
	if (!_fadeText)
		return;

	bool finished = false;
	for (int i = 0; i < 3; i++) {
		if (_currSentenceColor[i] > 4)
			_currSentenceColor[i] -= 4;
		else
			if (_currSentenceColor[i]) {
				_currSentenceColor[i] = 0;
				finished = true;
			}
	}

	_screen->_currentPalette[765] = _currSentenceColor[0];
	_screen->_currentPalette[766] = _currSentenceColor[1];
	_screen->_currentPalette[767] = _currSentenceColor[2];
	_screen->setScreenPalette(_screen->_currentPalette);

	if (finished) {
		_fadeText = false;
		_startSentencePalIndex = -1;
	}
}

} // end of namespace Kyra
