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

#include "kyra/gui.h"

#include "kyra/screen.h"
#include "kyra/text.h"
#include "kyra/wsamovie.h"

#include "common/savefile.h"

namespace Kyra {

GUI::GUI(KyraEngine_v1 *kyra)
	: _vm(kyra), _screen(kyra->screen()), _text(kyra->text()) {
	_menuButtonList = 0;

	_redrawButtonFunctor = BUTTON_FUNCTOR(GUI, this, &GUI::redrawButtonCallback);
	_redrawShadedButtonFunctor = BUTTON_FUNCTOR(GUI, this, &GUI::redrawShadedButtonCallback);
}

Button *GUI::addButtonToList(Button *list, Button *newButton) {
	if (!newButton)
		return list;

	newButton->nextButton = 0;

	if (list) {
		Button *cur = list;
		while (cur->nextButton)
			cur = cur->nextButton;
		cur->nextButton = newButton;
	} else {
		list = newButton;
	}

	return list;
}

void GUI::initMenuLayout(Menu &menu) {
	if (menu.x == -1)
		menu.x = (320 - menu.width) >> 1;
	if (menu.y == -1)
		menu.y = (200 - menu.height) >> 1;

	for (int i = 0; i < menu.numberOfItems; ++i) {
		if (menu.item[i].x == -1)
			menu.item[i].x = (menu.width - menu.item[i].width) >> 1;
	}
}

void GUI::initMenu(Menu &menu) {
	_menuButtonList = 0;

	_screen->hideMouse();

	int textX;
	int textY;

	int menu_x2 = menu.width  + menu.x - 1;
	int menu_y2 = menu.height + menu.y - 1;

	_screen->fillRect(menu.x + 2, menu.y + 2, menu_x2 - 2, menu_y2 - 2, menu.bkgdColor);
	_screen->drawShadedBox(menu.x, menu.y, menu_x2, menu_y2, menu.color1, menu.color2);

	if (menu.titleX != -1)
		textX = menu.titleX;
	else
		textX = _text->getCenterStringX(getMenuTitle(menu), menu.x, menu_x2);

	textY = menu.y + menu.titleY;

	_text->printText(getMenuTitle(menu), textX - 1, textY + 1, defaultColor1(), defaultColor2(), 0);
	_text->printText(getMenuTitle(menu), textX, textY, menu.textColor, 0, 0);

	int x1, y1, x2, y2;
	for (int i = 0; i < menu.numberOfItems; ++i) {
		if (!menu.item[i].enabled)
			continue;

		x1 = menu.x + menu.item[i].x;
		y1 = menu.y + menu.item[i].y;

		x2 = x1 + menu.item[i].width - 1;
		y2 = y1 + menu.item[i].height - 1;

		if (i < 7) {
			Button *menuButtonData = getButtonListData() + i;
			menuButtonData->nextButton = 0;
			menuButtonData->x = x1;
			menuButtonData->y = y1;
			menuButtonData->width  = menu.item[i].width - 1;
			menuButtonData->height = menu.item[i].height - 1;
			menuButtonData->buttonCallback = menu.item[i].callback;
			menuButtonData->keyCode = menu.item[i].unk1F;
			menuButtonData->keyCode2 = 0;

			_menuButtonList = addButtonToList(_menuButtonList, menuButtonData);
		}

		_screen->fillRect(x1, y1, x2, y2, menu.item[i].bkgdColor);
		_screen->drawShadedBox(x1, y1, x2, y2, menu.item[i].color1, menu.item[i].color2);

		if (getMenuItemTitle(menu.item[i])) {
			if (menu.item[i].titleX != -1)
				textX = x1 + menu.item[i].titleX + 3;
			else
				textX = _text->getCenterStringX(getMenuItemTitle(menu.item[i]), x1, x2);

			textY = y1 + 2;
			_text->printText(getMenuItemTitle(menu.item[i]), textX - 1, textY + 1, defaultColor1(), 0, 0);

			if (i == menu.highlightedItem)
				_text->printText(getMenuItemTitle(menu.item[i]), textX, textY, menu.item[i].highlightColor, 0, 0);
			else
				_text->printText(getMenuItemTitle(menu.item[i]), textX, textY, menu.item[i].textColor, 0, 0);
		}
	}

	for (int i = 0; i < menu.numberOfItems; ++i) {
		if (getMenuItemLabel(menu.item[i])) {
			_text->printText(getMenuItemLabel(menu.item[i]), menu.x + menu.item[i].labelX - 1, menu.y + menu.item[i].labelY + 1, defaultColor1(), 0, 0);
			_text->printText(getMenuItemLabel(menu.item[i]), menu.x + menu.item[i].labelX, menu.y + menu.item[i].labelY, menu.item[i].textColor, 0, 0);
		}
	}

	if (menu.scrollUpButtonX != -1) {
		Button *scrollUpButton = getScrollUpButton();
		scrollUpButton->x = menu.scrollUpButtonX + menu.x;
		scrollUpButton->y = menu.scrollUpButtonY + menu.y;
		scrollUpButton->buttonCallback = getScrollUpButtonHandler();
		scrollUpButton->nextButton = 0;
		scrollUpButton->mouseWheel = -1;

		_menuButtonList = addButtonToList(_menuButtonList, scrollUpButton);
		updateMenuButton(scrollUpButton);

		Button *scrollDownButton = getScrollDownButton();
		scrollDownButton->x = menu.scrollDownButtonX + menu.x;
		scrollDownButton->y = menu.scrollDownButtonY + menu.y;
		scrollDownButton->buttonCallback = getScrollDownButtonHandler();
		scrollDownButton->nextButton = 0;
		scrollDownButton->mouseWheel = 1;

		_menuButtonList = addButtonToList(_menuButtonList, scrollDownButton);
		updateMenuButton(scrollDownButton);
	}

	_screen->showMouse();
	_screen->updateScreen();
}

void GUI::processHighlights(Menu &menu, int mouseX, int mouseY) {
	int x1, y1, x2, y2;

	for (int i = 0; i < menu.numberOfItems; ++i) {
		if (!menu.item[i].enabled)
			continue;

		x1 = menu.x + menu.item[i].x;
		y1 = menu.y + menu.item[i].y;

		x2 = x1 + menu.item[i].width;
		y2 = y1 + menu.item[i].height;

		if (mouseX > x1 && mouseX < x2 &&
			mouseY > y1 && mouseY < y2) {

			if (menu.highlightedItem != i) {
				if (menu.item[menu.highlightedItem].enabled)
					redrawText(menu);

				menu.highlightedItem = i;
				redrawHighlight(menu);
				_screen->updateScreen();
			}
		}
	}
}

void GUI::redrawText(const Menu &menu) {
	int textX;
	int i = menu.highlightedItem;

	int x1 = menu.x + menu.item[i].x;
	int y1 = menu.y + menu.item[i].y;

	int x2 = x1 + menu.item[i].width - 1;

	if (menu.item[i].titleX >= 0)
		textX = x1 + menu.item[i].titleX + 3;
	else
		textX = _text->getCenterStringX(getMenuItemTitle(menu.item[i]), x1, x2);

	int textY = y1 + 2;
	_text->printText(getMenuItemTitle(menu.item[i]), textX - 1, textY + 1, defaultColor1(), 0, 0);
	_text->printText(getMenuItemTitle(menu.item[i]), textX, textY, menu.item[i].textColor, 0, 0);
}

void GUI::redrawHighlight(const Menu &menu) {
	int textX;
	int i = menu.highlightedItem;

	int x1 = menu.x + menu.item[i].x;
	int y1 = menu.y + menu.item[i].y;

	int x2 = x1 + menu.item[i].width - 1;

	if (menu.item[i].titleX != -1)
		textX = x1 + menu.item[i].titleX + 3;
	else
		textX = _text->getCenterStringX(getMenuItemTitle(menu.item[i]), x1, x2);

	int textY = y1 + 2;
	_text->printText(getMenuItemTitle(menu.item[i]), textX - 1, textY + 1, defaultColor1(), 0, 0);
	_text->printText(getMenuItemTitle(menu.item[i]), textX, textY, menu.item[i].highlightColor, 0, 0);
}

void GUI::updateAllMenuButtons() {
	for (Button *cur = _menuButtonList; cur; cur = cur->nextButton)
		updateMenuButton(cur);
}

void GUI::updateMenuButton(Button *button) {
	if (!_displayMenu)
		return;

	_screen->hideMouse();
	updateButton(button);
	_screen->updateScreen();
	_screen->showMouse();
}

void GUI::updateButton(Button *button) {
	if (!button || (button->flags & 8))
		return;

	if (button->flags2 & 1)
		button->flags2 &= 0xFFF7;
	else
		button->flags2 |= 8;

	button->flags2 &= 0xFFFC;

	if (button->flags2 & 4)
		button->flags2 |= 0x10;
	else
		button->flags2 &= 0xEEEF;

	button->flags2 &= 0xFFFB;

	processButton(button);
}

int GUI::redrawButtonCallback(Button *button) {
	if (!_displayMenu)
		return 0;

	_screen->hideMouse();
	_screen->drawBox(button->x + 1, button->y + 1, button->x + button->width - 1, button->y + button->height - 1, 0xF8);
	_screen->showMouse();

	return 0;
}

int GUI::redrawShadedButtonCallback(Button *button) {
	if (!_displayMenu)
		return 0;

	_screen->hideMouse();
	_screen->drawShadedBox(button->x, button->y, button->x + button->width, button->y + button->height, 0xF9, 0xFA);
	_screen->showMouse();

	return 0;
}

void GUI::updateSaveList() {
	Common::String pattern = _vm->_targetName + ".???";
	Common::StringList saveFileList = _vm->_saveFileMan->listSavefiles(pattern.c_str());
	_saveSlots.clear();

	for (Common::StringList::const_iterator i = saveFileList.begin(); i != saveFileList.end(); ++i) {
		char s1 = 0, s2 = 0, s3 = 0;
		s1 = (*i)[i->size()-3];
		s2 = (*i)[i->size()-2];
		s3 = (*i)[i->size()-1];
		if (!isdigit(s1) || !isdigit(s2) || !isdigit(s3))
			continue;
		s1 -= '0';
		s2 -= '0';
		s3 -= '0';
		_saveSlots.push_back(s1*100+s2*10+s3);
	}

	if (_saveSlots.begin() == _saveSlots.end())
		return;

	Common::sort(_saveSlots.begin(), _saveSlots.end(), Common::Less<int>());
	if (_saveSlots.size() > 2)
		Common::sort(_saveSlots.begin()+1, _saveSlots.end(), Common::Greater<int>());
}

int GUI::getNextSavegameSlot() {
	Common::InSaveFile *in;

	for (int i = 1; i < 990; i++) {
		if ((in = _vm->_saveFileMan->openForLoading(_vm->getSavegameFilename(i))))
			delete in;
		else
			return i;
	}
	warning("Didn't save: Ran out of saveGame filenames");
	return 0;
}

void GUI::checkTextfieldInput() {
	Common::Event event;

	uint32 now = _vm->_system->getMillis();

	bool running = true;
	int keys = 0;
	while (_vm->_eventMan->pollEvent(event) && running) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == 'q' && event.kbd.flags == Common::KBD_CTRL)
				_vm->quitGame();
			else
				_keyPressed = event.kbd;
			running = false;
			break;

		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_LBUTTONUP: {
			Common::Point pos = _vm->getMousePos();
			_vm->_mouseX = pos.x;
			_vm->_mouseY = pos.y;
			keys = event.type == Common::EVENT_LBUTTONDOWN ? 199 : (200 | 0x800);
			running = false;
			} break;

		case Common::EVENT_MOUSEMOVE: {
			Common::Point pos = _vm->getMousePos();
			_vm->_mouseX = pos.x;
			_vm->_mouseY = pos.y;
			_screen->updateScreen();
			_lastScreenUpdate = now;
			} break;

		default:
			break;
		}
	}

	if (now - _lastScreenUpdate > 50) {
		_vm->_system->updateScreen();
		_lastScreenUpdate = now;
	}

	processButtonList(_menuButtonList, keys | 0x8000, 0);
	_vm->_system->delayMillis(3);
}

#pragma mark -

MainMenu::MainMenu(KyraEngine_v1 *vm) : _vm(vm), _screen(0) {
	_screen = _vm->screen();
	_nextUpdate = 0;
	_system = g_system;
}

void MainMenu::init(StaticData data, Animation anim) {
	_static = data;
	_anim = anim;
	_animIntern.curFrame = _anim.startFrame;
	_animIntern.direction = 1;
}

void MainMenu::updateAnimation() {
	if (_anim.anim) {
		uint32 now = _system->getMillis();
		if (now > _nextUpdate) {
			_nextUpdate = now + _anim.delay * _vm->tickLength();

			_anim.anim->displayFrame(_animIntern.curFrame, 0, 0, 0);
			_animIntern.curFrame += _animIntern.direction ;
			if (_animIntern.curFrame < _anim.startFrame) {
				_animIntern.curFrame = _anim.startFrame;
				_animIntern.direction = 1;
			} else if (_animIntern.curFrame > _anim.endFrame) {
				_animIntern.curFrame = _anim.endFrame;
				_animIntern.direction = -1;
			}
		}
	}

	_screen->updateScreen();
}

bool MainMenu::getInput() {
	Common::Event event;

	while (_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_LBUTTONUP:
			return true;
		default:
			break;
		}
	}
	return false;
}

int MainMenu::handle(int dim) {
	debugC(9, kDebugLevelMain, "MainMenu::handle(%d)", dim);
	int command = -1;

	uint8 colorMap[16];
	memset(colorMap, 0, sizeof(colorMap));
	_screen->setTextColorMap(colorMap);

	Screen::FontId oldFont = _screen->setFont(_static.font);
	int charWidthBackUp = _screen->_charWidth;

	if (_vm->game() != GI_LOL)
		_screen->_charWidth = -2;
	_screen->setScreenDim(dim);

	int backUpX = _screen->_curDim->sx;
	int backUpY = _screen->_curDim->sy;
	int backUpWidth = _screen->_curDim->w;
	int backUpHeight = _screen->_curDim->h;
	_screen->copyRegion(backUpX, backUpY, backUpX, backUpY, backUpWidth, backUpHeight, 0, 3);

	int x = _screen->_curDim->sx << 3;
	int y = _screen->_curDim->sy;
	int width = _screen->_curDim->w << 3;
	int height =  _screen->_curDim->h;

	drawBox(x, y, width, height, 1);
	drawBox(x + 1, y + 1, width - 2, height - 2, 0);

	int selected = 0;

	draw(selected);

	while (!_screen->isMouseVisible())
		_screen->showMouse();

	int fh = _screen->getFontHeight();
	int textPos = ((_screen->_curDim->w >> 1) + _screen->_curDim->sx) << 3;

	Common::Rect menuRect(x + 16, y + 4, x + width - 16, y + 4 + fh * _static.menuTable[3]);

	while (!_vm->shouldQuit()) {
		updateAnimation();
		bool mousePressed = getInput();

		Common::Point mouse = _vm->getMousePos();
		if (menuRect.contains(mouse)) {
			int item = (mouse.y - menuRect.top) / fh;

			if (item != selected) {
				printString(_static.strings[selected], textPos, menuRect.top + selected * fh, _static.menuTable[5], 0, 5);
				printString(_static.strings[item], textPos, menuRect.top + item * fh, _static.menuTable[6], 0, 5);

				selected = item;
			}

			if (mousePressed) {
				for (int i = 0; i < 3; i++) {
					printString(_static.strings[selected], textPos, menuRect.top + selected * fh, _static.menuTable[5], 0, 5);
					_screen->updateScreen();
					_system->delayMillis(50);
					printString(_static.strings[selected], textPos, menuRect.top + selected * fh, _static.menuTable[6], 0, 5);
					_screen->updateScreen();
					_system->delayMillis(50);
				}
				command = item;
				break;
			}
		}
		_system->delayMillis(10);
	}

	if (_vm->shouldQuit())
		command = -1;

	_screen->copyRegion(backUpX, backUpY, backUpX, backUpY, backUpWidth, backUpHeight, 3, 0);
	_screen->_charWidth = charWidthBackUp;
	_screen->setFont(oldFont);

	return command;
}

void MainMenu::draw(int select) {
	debugC(9, kDebugLevelMain, "MainMenu::draw(%d)", select);
	int top = _screen->_curDim->sy;
	top += _static.menuTable[1];

	for (int i = 0; i < _static.menuTable[3]; ++i) {
		int curY = top + i * _screen->getFontHeight();
		int color = (i == select) ? _static.menuTable[6] : _static.menuTable[5];
		printString(_static.strings[i], ((_screen->_curDim->w >> 1) + _screen->_curDim->sx) << 3, curY, color, 0, 5);
	}
}

void MainMenu::drawBox(int x, int y, int w, int h, int fill) {
	debugC(9, kDebugLevelMain, "MainMenu::drawBox(%d, %d, %d, %d, %d)", x, y, w, h, fill);
	--w; --h;

	if (fill)
		_screen->fillRect(x, y, x+w, y+h, _static.colorTable[0]);

	_screen->drawClippedLine(x, y+h, x+w, y+h, _static.colorTable[1]);
	_screen->drawClippedLine(x+w, y, x+w, y+h, _static.colorTable[1]);
	_screen->drawClippedLine(x, y, x+w, y, _static.colorTable[2]);
	_screen->drawClippedLine(x, y, x, y+h, _static.colorTable[2]);

	_screen->setPagePixel(_screen->_curPage, x, y+h, _static.colorTable[3]);
	_screen->setPagePixel(_screen->_curPage, x+w, y, _static.colorTable[3]);
}

void MainMenu::printString(const char *format, int x, int y, int col1, int col2, int flags, ...) {
	debugC(9, kDebugLevelMain, "MainMenu::printString('%s', %d, %d, %d, %d, %d, ...)", format, x, y, col1, col2, flags);
	if (!format)
		return;

	char string[512];
	va_list vaList;
	va_start(vaList, flags);
	vsprintf(string, format, vaList);
	va_end(vaList);

	if (flags & 1)
		x -= _screen->getTextWidth(string) >> 1;

	if (flags & 2)
		x -= _screen->getTextWidth(string);

	if (flags & 4) {
		_screen->printText(string, x - 1, y, _static.altColor, col2);
		_screen->printText(string, x, y + 1, _static.altColor, col2);
	}

	if (flags & 8) {
		_screen->printText(string, x - 1, y, 227, col2);
		_screen->printText(string, x, y + 1, 227, col2);
	}

	_screen->printText(string, x, y, col1, col2);
}

} // end of namespace Kyra

