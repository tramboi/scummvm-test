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

#include "common/stream.h"

#include "draci/draci.h"
#include "draci/game.h"
#include "draci/barchive.h"
#include "draci/script.h"
#include "draci/animation.h"

#include <cmath>

namespace Draci {

static const Common::String dialoguePath("ROZH");

static double real_to_double(byte real[6]);

Game::Game(DraciEngine *vm) : _vm(vm) {
	uint i;

	BArchive *initArchive = _vm->_initArchive;
	const BAFile *file;

	// Read in persons
	file = initArchive->getFile(5);
	Common::MemoryReadStream personData(file->_data, file->_length);

	uint numPersons = file->_length / personSize;
	_persons = new Person[numPersons];

	for (i = 0; i < numPersons; ++i) {
		_persons[i]._x = personData.readUint16LE();
		_persons[i]._y = personData.readUint16LE();
		_persons[i]._fontColour = personData.readByte();
	}

	// Read in dialogue offsets
	file = initArchive->getFile(4);
	Common::MemoryReadStream dialogueData(file->_data, file->_length);

	uint numDialogues = file->_length / sizeof(uint16);
	_dialogueOffsets = new uint[numDialogues];

	uint curOffset;
	for (i = 0, curOffset = 0; i < numDialogues; ++i) {
		_dialogueOffsets[i] = curOffset;
		curOffset += dialogueData.readUint16LE();
	}

	_dialogueVars = new int[curOffset];
	memset(_dialogueVars, 0, sizeof (int) * curOffset);

	// Read in game info
	file = initArchive->getFile(3);
	Common::MemoryReadStream gameData(file->_data, file->_length);

	_info._startRoom = gameData.readByte() - 1;
	_info._mapRoom = gameData.readByte() - 1;
	_info._numObjects = gameData.readUint16LE();
	_info._numItems = gameData.readUint16LE();
	_info._numVariables = gameData.readByte();
	_info._numPersons = gameData.readByte();
	_info._numDialogues = gameData.readByte();
	_info._maxItemWidth = gameData.readUint16LE();
	_info._maxItemHeight = gameData.readUint16LE();
	_info._musicLength = gameData.readUint16LE();
	_info._crc[0] = gameData.readUint16LE();
	_info._crc[1] = gameData.readUint16LE();
	_info._crc[2] = gameData.readUint16LE();
	_info._crc[3] = gameData.readUint16LE();

	_info._numDialogueBlocks = curOffset;

	// Read in variables
	file = initArchive->getFile(2);
	uint numVariables = file->_length / sizeof (int16);

	_variables = new int[numVariables];
	Common::MemoryReadStream variableData(file->_data, file->_length);

	for (i = 0; i < numVariables; ++i) {
		_variables[i] = variableData.readUint16LE();
	}

	// Read in item icon status
	file = initArchive->getFile(1);
	uint numItems = file->_length;
	_itemStatus = new byte[numItems];
	memcpy(_itemStatus, file->_data, numItems);
	_items = new GameItem[numItems];

	// Read in object status
	file = initArchive->getFile(0);
	uint numObjects = file->_length;

	_objects = new GameObject[numObjects];
	Common::MemoryReadStream objStatus(file->_data, file->_length);

	for (i = 0; i < numObjects; ++i) {
		byte tmp = objStatus.readByte();

		// Set object visibility
		_objects[i]._visible = tmp & (1 << 7);

		// Set object location
		_objects[i]._location = (~(1 << 7) & tmp) - 1;
	}

	assert(numDialogues == _info._numDialogues);
	assert(numPersons == _info._numPersons);
	assert(numVariables == _info._numVariables);
	assert(numObjects == _info._numObjects);
	assert(numItems == _info._numItems);

	// Deallocate all cached files, because we have copied them into our own data structures.
	initArchive->clearCache();
}

void Game::start() {
	while (!shouldQuit()) {
		debugC(1, kDraciGeneralDebugLevel, "Game::start()");

		// Whenever the top-level loop is entered, it should not finish unless
		// the exit is triggered by a script
		const bool force_reload = shouldExitLoop() > 1;
		setExitLoop(false);
		_vm->_script->endCurrentProgram(false);

		enterNewRoom(force_reload);

		if (_vm->_script->shouldEndProgram()) {
			// Room changed during the initialization (intro or Escape pressed).
			continue;
		}

		loop();
	}
}

void Game::init() {
	setQuit(false);
	setExitLoop(false);
	_scheduledPalette = 0;
	_fadePhases = _fadePhase = 0;
	setEnableQuickHero(true);
	setWantQuickHero(false);
	setEnableSpeedText(true);
	setLoopStatus(kStatusGate);
	setLoopSubstatus(kSubstatusOrdinary);

	_animUnderCursor = kOverlayImage;

	_currentItem = kNoItem;
	_itemUnderCursor = kNoItem;

	_vm->_mouse->setCursorType(kHighlightedCursor);	// anything different from kNormalCursor

	_oldObjUnderCursor = _objUnderCursor = kOverlayImage;
        
	// Set the inventory to empty initially
	memset(_inventory, kNoItem, kInventorySlots * sizeof(int));

	// Initialize animation for object / room titles
	Animation *titleAnim = _vm->_anims->addText(kTitleText, true);
	Text *title = new Text("", _vm->_smallFont, kTitleColour, 0, 0, 0);
	titleAnim->addFrame(title, NULL);

	// Initialize animation for speech text
	Animation *speechAnim = _vm->_anims->addText(kSpeechText, true);
	Text *speech = new Text("", _vm->_bigFont, kFontColour1, 0, 0, 0);
	speechAnim->addFrame(speech, NULL);

	// Initialize inventory animation
	const BAFile *f = _vm->_iconsArchive->getFile(13);
	Animation *inventoryAnim = _vm->_anims->addAnimation(kInventorySprite, 255, false);
	Sprite *inventorySprite = new Sprite(f->_data, f->_length, 0, 0, true);
	inventoryAnim->addFrame(inventorySprite, NULL);
	inventoryAnim->setRelative((kScreenWidth - inventorySprite->getWidth()) / 2,
	                           (kScreenHeight - inventorySprite->getHeight()) / 2);

	for (uint i = 0; i < kDialogueLines; ++i) {
		_dialogueAnims[i] = _vm->_anims->addText(kDialogueLinesID - i, true);
		Text *dialogueLine = new Text("", _vm->_smallFont, kLineInactiveColour, 0, 0, 0);
		_dialogueAnims[i]->addFrame(dialogueLine, NULL);

		_dialogueAnims[i]->setZ(254);
		_dialogueAnims[i]->setRelative(1,
		                      kScreenHeight - (i + 1) * _vm->_smallFont->getFontHeight());

		Text *text = reinterpret_cast<Text *>(_dialogueAnims[i]->getCurrentFrame());
		text->setText("");
	}

	for (uint i = 0; i < _info._numItems; ++i) {
		loadItem(i);
	}

	loadObject(kDragonObject);

	const GameObject *dragon = getObject(kDragonObject);
	debugC(4, kDraciLogicDebugLevel, "Running init program for the dragon object...");
	_vm->_script->run(dragon->_program, dragon->_init);

	// Make sure we enter the right room in start().
	setRoomNum(kNoEscRoom);
	rememberRoomNumAsPrevious();
	scheduleEnteringRoomUsingGate(_info._startRoom, 0);
	_pushedNewRoom = _pushedNewGate = -1;
}

void Game::loop() {
	Surface *surface = _vm->_screen->getSurface();

	do {
	debugC(4, kDraciLogicDebugLevel, "loopstatus: %d, loopsubstatus: %d",
		_loopStatus, _loopSubstatus);

		_vm->handleEvents();
		if (shouldExitLoop() > 1)	// after loading
			break;

		if (_fadePhase > 0 && (_vm->_system->getMillis() - _fadeTick) >= kFadingTimeUnit) {
			_fadeTick = _vm->_system->getMillis();
			--_fadePhase;
			const byte *startPal = _currentRoom._palette >= 0 ? _vm->_paletteArchive->getFile(_currentRoom._palette)->_data : NULL;
			const byte *endPal = getScheduledPalette() >= 0 ? _vm->_paletteArchive->getFile(getScheduledPalette())->_data : NULL;
			_vm->_screen->interpolatePalettes(startPal, endPal, 0, kNumColours, _fadePhases - _fadePhase, _fadePhases);
			if (_loopSubstatus == kSubstatusFade && _fadePhase == 0) {
				setExitLoop(true);
				// Rewrite the palette index of the current
				// room.  This is necessary when two fadings
				// are called after each other, such as in the
				// intro.
				_currentRoom._palette = getScheduledPalette();
			}
		}

		// Fetch mouse coordinates
		int x = _vm->_mouse->getPosX();
		int y = _vm->_mouse->getPosY();

		if (_loopStatus == kStatusDialogue && _loopSubstatus == kSubstatusOrdinary) {
			Text *text;
			for (int i = 0; i < kDialogueLines; ++i) {
				text = reinterpret_cast<Text *>(_dialogueAnims[i]->getCurrentFrame());

				if (_animUnderCursor == _dialogueAnims[i]->getID()) {
					text->setColour(kLineActiveColour);
				} else {
					text->setColour(kLineInactiveColour);
				}
			}

			if (_vm->_mouse->lButtonPressed() || _vm->_mouse->rButtonPressed()) {
				setExitLoop(true);
				_vm->_mouse->lButtonSet(false);
				_vm->_mouse->rButtonSet(false);
			}
		}

		if (_vm->_mouse->isCursorOn()) {
			// Fetch the dedicated objects' title animation / current frame
			Animation *titleAnim = _vm->_anims->getAnimation(kTitleText);
			Text *title = reinterpret_cast<Text *>(titleAnim->getCurrentFrame());

			updateCursor();
			updateTitle();

			if (_loopStatus == kStatusOrdinary && _loopSubstatus == kSubstatusOrdinary) {
				if (_vm->_mouse->lButtonPressed()) {
					_vm->_mouse->lButtonSet(false);

					if (_currentItem != kNoItem) {
						putItem(_currentItem, 0);
						_currentItem = kNoItem;
						updateCursor();
					} else {
						if (_objUnderCursor != kObjectNotFound) {
							const GameObject *obj = &_objects[_objUnderCursor];

							_vm->_mouse->cursorOff();
							titleAnim->markDirtyRect(surface);
							title->setText("");
							_objUnderCursor = kObjectNotFound;

							if (!obj->_imLook) {
								if (obj->_lookDir == kDirectionLast) {
									walkHero(x, y, obj->_lookDir);
								} else {
									walkHero(obj->_lookX, obj->_lookY, obj->_lookDir);
								}
							}

							_vm->_script->run(obj->_program, obj->_look);
							_vm->_mouse->cursorOn();
						} else {
							walkHero(x, y, kDirectionLast);
						}
					}
				}

				if (_vm->_mouse->rButtonPressed()) {
					_vm->_mouse->rButtonSet(false);

					if (_objUnderCursor != kObjectNotFound) {
						const GameObject *obj = &_objects[_objUnderCursor];

						if (_vm->_script->testExpression(obj->_program, obj->_canUse)) {
							_vm->_mouse->cursorOff();
							titleAnim->markDirtyRect(surface);
							title->setText("");
							_objUnderCursor = kObjectNotFound;

							if (!obj->_imUse) {
								if (obj->_useDir == kDirectionLast) {
									walkHero(x, y, obj->_useDir);
								} else {
									walkHero(obj->_useX, obj->_useY, obj->_useDir);
								}
							}

							_vm->_script->run(obj->_program, obj->_use);
							_vm->_mouse->cursorOn();
						} else {
							walkHero(x, y, kDirectionLast);
						}
					} else {
						if (_vm->_script->testExpression(_currentRoom._program, _currentRoom._canUse)) {
							_vm->_mouse->cursorOff();
							titleAnim->markDirtyRect(surface);
							title->setText("");


							_vm->_script->run(_currentRoom._program, _currentRoom._use);
							_vm->_mouse->cursorOn();
						} else {
							walkHero(x, y, kDirectionLast);
						}
					}
				}
			}

			if (_loopStatus == kStatusInventory && _loopSubstatus == kSubstatusOrdinary) {
				if (_inventoryExit) {
					inventoryDone();
				}

				// If we are in inventory mode, all the animations except game items'
				// images will necessarily be paused so we can safely assume that any
				// animation under the cursor (a value returned by
				// AnimationManager::getTopAnimationID()) will be an item animation or
				// an overlay, for which we check. Item animations have their IDs
				// calculated by offseting their itemID from the ID of the last "special"
				// animation ID. In this way, we obtain its itemID.
				if (_animUnderCursor != kOverlayImage && _animUnderCursor != kInventorySprite) {
					_itemUnderCursor = kInventoryItemsID - _animUnderCursor;
				} else {
					_itemUnderCursor = kNoItem;
				}

				// If the user pressed the left mouse button
				if (_vm->_mouse->lButtonPressed()) {
					_vm->_mouse->lButtonSet(false);

					// If there is an inventory item under the cursor and we aren't
					// holding any item, run its look GPL program
					if (_itemUnderCursor != kNoItem && _currentItem == kNoItem) {
						const GameItem *item = &_items[_itemUnderCursor];

						_vm->_script->run(item->_program, item->_look);
					// Otherwise, if we are holding an item, try to place it inside the
					// inventory
					} else if (_currentItem != kNoItem) {
						// FIXME: This should place the item in the nearest inventory slot,
						// not the first one available
						putItem(_currentItem, 0);

						// Remove it from our hands
						_currentItem = kNoItem;
					}
				} else if (_vm->_mouse->rButtonPressed()) {
					_vm->_mouse->rButtonSet(false);

					// If we right-clicked outside the inventory, close it
					if (_animUnderCursor != kInventorySprite && _itemUnderCursor == kNoItem) {
						inventoryDone();

					// If there is an inventory item under our cursor
					} else if (_itemUnderCursor != kNoItem) {
						// Again, we have two possibilities:

						// The first is that there is no item in our hands.
						// In that case, just take the inventory item from the inventory.
						if (_currentItem == kNoItem) {
							_currentItem = _itemUnderCursor;
							removeItem(_itemUnderCursor);

						// The second is that there *is* an item in our hands.
						// In that case, run the canUse script for the inventory item
						// which will check if the two items are combinable and, finally,
						// run the use script for the item.
						} else {
							const GameItem *item = &_items[_itemUnderCursor];

							if (_vm->_script->testExpression(item->_program, item->_canUse)) {
								_vm->_script->run(item->_program, item->_use);
							}
						}
						updateCursor();
					}
				}
			}
		}

		debugC(5, kDraciLogicDebugLevel, "Anim under cursor: %d", _animUnderCursor);

		// Handle character talking (if there is any)
		if (_loopSubstatus == kSubstatusTalk) {
			// If the current speech text has expired or the user clicked a mouse button,
			// advance to the next line of text
			if ((getEnableSpeedText() && (_vm->_mouse->lButtonPressed() || _vm->_mouse->rButtonPressed())) ||
				(_vm->_system->getMillis() - _speechTick) >= _speechDuration) {

				setExitLoop(true);
			}
			_vm->_mouse->lButtonSet(false);
			_vm->_mouse->rButtonSet(false);
		}

		// This returns true if we got a signal to quit the game
		if (shouldQuit())
			return;

		// Advance animations and redraw screen
		_vm->_anims->drawScene(surface);
		_vm->_screen->copyToScreen();
		_vm->_system->delayMillis(20);

		// HACK: Won't be needed once the game loop is implemented properly
		setExitLoop(shouldExitLoop() || (_newRoom != getRoomNum() &&
		                  (_loopStatus == kStatusOrdinary || _loopStatus == kStatusGate)));

	} while (!shouldExitLoop());
}

void Game::updateCursor() {
	// Fetch mouse coordinates
	int x = _vm->_mouse->getPosX();
	int y = _vm->_mouse->getPosY();

	// Find animation under cursor
	_animUnderCursor = _vm->_anims->getTopAnimationID(x, y);

	// If we are inside a dialogue, all we need is to update the ID of the current
	// animation under the cursor. This enables us to update the currently selected
	// dialogue line (by recolouring it) but still leave the cursor unupdated when
	// over background objects.
	if (_loopStatus == kStatusDialogue)
		return;

	bool mouseChanged = false;

	// If we are in inventory mode, we do a different kind of updating that handles
	// inventory items and return early
	if (_loopStatus == kStatusInventory && _loopSubstatus == kSubstatusOrdinary) {
		if (_itemUnderCursor != kNoItem) {
			const GameItem *item = &_items[_itemUnderCursor];

			if (_vm->_script->testExpression(item->_program, item->_canUse)) {
				if (_currentItem == kNoItem) {
					_vm->_mouse->setCursorType(kHighlightedCursor);
				} else {
					_vm->_mouse->loadItemCursor(_currentItem, true);
				}
				mouseChanged = true;
			}
		}
		if (!mouseChanged) {
			if (_currentItem == kNoItem) {
				_vm->_mouse->setCursorType(kNormalCursor);
			} else {
				_vm->_mouse->loadItemCursor(_currentItem, false);
			}
		}

		return;
	}

	// Find the game object under the cursor
	// (to be more precise, one that corresponds to the animation under the cursor)
	int curObject = getObjectWithAnimation(_animUnderCursor);

	// Update the game object under the cursor
	_objUnderCursor = curObject;
	if (_objUnderCursor != _oldObjUnderCursor) {
		_oldObjUnderCursor = _objUnderCursor;
	}

	// TODO: Handle main menu

	// If there is no game object under the cursor, try using the room itself
	if (_objUnderCursor == kObjectNotFound) {
		if (_vm->_script->testExpression(_currentRoom._program, _currentRoom._canUse)) {
			if (_currentItem == kNoItem) {
				_vm->_mouse->setCursorType(kHighlightedCursor);
			} else {
				_vm->_mouse->loadItemCursor(_currentItem, true);
			}
			mouseChanged = true;
		}
	// If there *is* a game object under the cursor, update the cursor image
	} else {
		const GameObject *obj = &_objects[_objUnderCursor];

		// If there is no walking direction set on the object (i.e. the object
		// is not a gate / exit), test whether it can be used and, if so,
		// update the cursor image (highlight it).
		if (obj->_walkDir == 0) {
			if (_vm->_script->testExpression(obj->_program, obj->_canUse)) {
				if (_currentItem == kNoItem) {
					_vm->_mouse->setCursorType(kHighlightedCursor);
				} else {
					_vm->_mouse->loadItemCursor(_currentItem, true);
				}
				mouseChanged = true;
			}
		// If the walking direction *is* set, the game object is a gate, so update
		// the cursor image to the appropriate arrow.
		} else {
			_vm->_mouse->setCursorType((CursorType)obj->_walkDir);
			mouseChanged = true;
		}
	}
	// Load the appropriate cursor (item image if an item is held or ordinary cursor
	// if not)
	if (!mouseChanged) {
		if (_currentItem == kNoItem) {
			_vm->_mouse->setCursorType(kNormalCursor);
		} else {
			_vm->_mouse->loadItemCursor(_currentItem, false);
		}
	}
}

void Game::updateTitle() {
	// If we are inside a dialogue, don't update titles
	if (_loopStatus == kStatusDialogue)
		return;

	// Fetch current surface and height of the small font (used for titles)
	Surface *surface = _vm->_screen->getSurface();
	const int smallFontHeight = _vm->_smallFont->getFontHeight();

	// Fetch mouse coordinates
	int x = _vm->_mouse->getPosX();
	int y = _vm->_mouse->getPosY();

	// Fetch the dedicated objects' title animation / current frame
	Animation *titleAnim = _vm->_anims->getAnimation(kTitleText);
	Text *title = reinterpret_cast<Text *>(titleAnim->getCurrentFrame());

	// Mark dirty rectangle to delete the previous text
	titleAnim->markDirtyRect(surface);

	if (_loopStatus == kStatusInventory) {
		// If there is no item under the cursor, delete the title.
		// Otherwise, show the item's title.
		if (_itemUnderCursor == kNoItem) {
			title->setText("");
		} else {
			const GameItem *item = &_items[_itemUnderCursor];
			title->setText(item->_title);
		}
	} else {
		// If there is no object under the cursor, delete the title.
		// Otherwise, show the object's title.
		if (_objUnderCursor == kObjectNotFound) {
			title->setText("");
		} else {
			const GameObject *obj = &_objects[_objUnderCursor];
			title->setText(obj->_title);
		}
	}

	// Move the title to the correct place (just above the cursor)
	int newX = surface->centerOnX(x, title->getWidth());
	int newY = surface->putAboveY(y - smallFontHeight / 2, title->getHeight());
	titleAnim->setRelative(newX, newY);

	// If we are currently playing the title, mark it dirty so it gets updated.
	// Otherwise, start playing the title animation.
	if (titleAnim->isPlaying()) {
		titleAnim->markDirtyRect(surface);
	} else {
		_vm->_anims->play(titleAnim->getID());
	}
}

int Game::getObjectWithAnimation(int animID) const {
	for (uint i = 0; i < _info._numObjects; ++i) {
		GameObject *obj = &_objects[i];

		for (uint j = 0; j < obj->_anim.size(); ++j) {
			if (obj->_anim[j] == animID) {
				return i;
			}
		}
	}

	return kObjectNotFound;
}

void Game::removeItem(int itemID) {
	for (uint i = 0; i < kInventorySlots; ++i) {
		if (_inventory[i] == itemID) {
			_inventory[i] = kNoItem;
			_vm->_anims->stop(kInventoryItemsID - itemID);
			break;
		}
	}
}

void Game::putItem(int itemID, int position) {
	if (itemID == kNoItem)
		return;

	if (position >= 0 &&
		position < kInventoryLines * kInventoryColumns &&
		(_inventory[position] == kNoItem || _inventory[position] == itemID)) {
		_inventory[position] = itemID;
	} else {
		for (position = 0; position < kInventorySlots; ++position) {
			if (_inventory[position] == kNoItem) {
				_inventory[position] = itemID;
				break;
			}
		}
	}

	const int line = position / kInventoryColumns + 1;
	const int column = position % kInventoryColumns + 1;

	const int anim_id = kInventoryItemsID - itemID;
	Animation *anim = _vm->_anims->getAnimation(anim_id);
	if (!anim) {
		anim = _vm->_anims->addItem(anim_id, false);
		const BAFile *img = _vm->_itemImagesArchive->getFile(2 * itemID);
		Sprite *sp = new Sprite(img->_data, img->_length, 0, 0, true);
		anim->addFrame(sp, NULL);
	}
	Drawable *frame = anim->getCurrentFrame();

	const int x = kInventoryX +
	              (column * kInventoryItemWidth) -
	              (kInventoryItemWidth / 2) -
	              (frame->getWidth() / 2);

	const int y = kInventoryY +
	              (line * kInventoryItemHeight) -
	              (kInventoryItemHeight / 2) -
	              (frame->getHeight() / 2);

	debug(2, "itemID: %d position: %d line: %d column: %d x: %d y: %d", itemID, position, line, column, x, y);

	anim->setRelative(x, y);

	// If we are in inventory mode, we need to play the item animation, immediately
	// upon returning it to its slot but *not* in other modes because it should be
	// invisible then (along with the inventory)
	if (_loopStatus == kStatusInventory && _loopSubstatus == kSubstatusOrdinary) {
		_vm->_anims->play(anim_id);
	}
}

void Game::inventoryInit() {
	// Pause all "background" animations
	_vm->_anims->pauseAnimations();

	// Draw the inventory and the current items
	inventoryDraw();

	// Turn cursor on if it is off
	_vm->_mouse->cursorOn();

	// Set the appropriate loop status
	setLoopStatus(kStatusInventory);

	// TODO: This will be used for exiting the inventory automatically when the mouse
	// is outside it for some time
	_inventoryExit = false;
}

void Game::inventoryDone() {
	_vm->_mouse->cursorOn();
	setLoopStatus(kStatusOrdinary);

	_vm->_anims->unpauseAnimations();

	_vm->_anims->stop(kInventorySprite);

	for (uint i = 0; i < kInventorySlots; ++i) {
		if (_inventory[i] != kNoItem) {
			_vm->_anims->stop(kInventoryItemsID - _inventory[i]);
		}
	}

	// Reset item under cursor
	_itemUnderCursor = kNoItem;

	// TODO: Handle main menu
}

void Game::inventoryDraw() {
	_vm->_anims->play(kInventorySprite);

	for (uint i = 0; i < kInventorySlots; ++i) {
		if (_inventory[i] != kNoItem) {
			_vm->_anims->play(kInventoryItemsID - _inventory[i]);
		}
	}
}

void Game::inventoryReload() {
	// Make sure all items are loaded into memory (e.g., after loading a
	// savegame) by re-putting them on the same spot in the inventory.
	for (uint i = 0; i < kInventorySlots; ++i) {
		putItem(_inventory[i], i);
	}
}

void Game::dialogueMenu(int dialogueID) {
	int oldLines, hit;

	char tmp[5];
	sprintf(tmp, "%d", dialogueID+1);
	Common::String ext(tmp);
	_dialogueArchive = new BArchive(dialoguePath + ext + ".dfw");

	debugC(4, kDraciLogicDebugLevel, "Starting dialogue (ID: %d, Archive: %s)",
	    dialogueID, (dialoguePath + ext + ".dfw").c_str());

	_currentDialogue = dialogueID;
	oldLines = 255;
	dialogueInit(dialogueID);

	do {
		_dialogueExit = false;
		hit = dialogueDraw();

		debugC(7, kDraciLogicDebugLevel,
			"hit: %d, _lines[hit]: %d, lastblock: %d, dialogueLines: %d, dialogueExit: %d",
			hit, _lines[hit], _lastBlock, _dialogueLinesNum, _dialogueExit);

		if ((!_dialogueExit) && (hit != -1) && (_lines[hit] != -1)) {
			if ((oldLines == 1) && (_dialogueLinesNum == 1) && (_lines[hit] == _lastBlock)) {
				break;
			}
			_currentBlock = _lines[hit];
			runDialogueProg(_dialogueBlocks[_lines[hit]]._program, 1);
		} else {
			break;
		}
		_lastBlock = _lines[hit];
		_dialogueVars[_dialogueOffsets[dialogueID] + _lastBlock] += 1;
		_dialogueBegin = false;
		oldLines = _dialogueLinesNum;

	} while (!_dialogueExit);

	dialogueDone();
	_currentDialogue = kNoDialogue;
}

int Game::dialogueDraw() {
	_dialogueLinesNum = 0;
	int i = 0;
	int ret = 0;

	Animation *anim;
	Text *dialogueLine;

	while ((_dialogueLinesNum < 4) && (i < _blockNum)) {
		GPL2Program blockTest;
		blockTest._bytecode = _dialogueBlocks[i]._canBlock;
		blockTest._length = _dialogueBlocks[i]._canLen;
		debugC(3, kDraciLogicDebugLevel, "Testing dialogue block %d", i);
		if (_vm->_script->testExpression(blockTest, 1)) {
			anim = _dialogueAnims[_dialogueLinesNum];
			dialogueLine = reinterpret_cast<Text *>(anim->getCurrentFrame());
			dialogueLine->setText(_dialogueBlocks[i]._title);

			dialogueLine->setColour(kLineInactiveColour);
			_lines[_dialogueLinesNum] = i;
			_dialogueLinesNum++;
		}
		++i;
	}

	for (i = _dialogueLinesNum; i < kDialogueLines; ++i) {
		_lines[i] = -1;
		anim = _dialogueAnims[i];
		dialogueLine = reinterpret_cast<Text *>(anim->getCurrentFrame());
		dialogueLine->setText("");
	}

	_oldObjUnderCursor = kObjectNotFound;

	if (_dialogueLinesNum > 1) {
		_vm->_mouse->cursorOn();
		setExitLoop(false);
		loop();
		_vm->_mouse->cursorOff();

		bool notDialogueAnim = true;
		for (uint j = 0; j < kDialogueLines; ++j) {
			if (_dialogueAnims[j]->getID() == _animUnderCursor) {
				notDialogueAnim = false;
				break;
			}
		}

		if (notDialogueAnim) {
			ret = -1;
		} else {
			ret = _dialogueAnims[0]->getID() - _animUnderCursor;
		}
	} else {
		ret = _dialogueLinesNum - 1;
	}

	for (i = 0; i < kDialogueLines; ++i) {
		dialogueLine = reinterpret_cast<Text *>(_dialogueAnims[i]->getCurrentFrame());
		_dialogueAnims[i]->markDirtyRect(_vm->_screen->getSurface());
		dialogueLine->setText("");
	}

	return ret;
}

void Game::dialogueInit(int dialogID) {
	_vm->_mouse->setCursorType(kDialogueCursor);

	_blockNum = _dialogueArchive->size() / 3;
	_dialogueBlocks = new Dialogue[_blockNum];

	const BAFile *f;

	for (uint i = 0; i < kDialogueLines; ++i) {
		_lines[i] = 0;
	}

	for (int i = 0; i < _blockNum; ++i) {
		f = _dialogueArchive->getFile(i * 3);
		_dialogueBlocks[i]._canLen = f->_length;
		_dialogueBlocks[i]._canBlock = f->_data;

		f = _dialogueArchive->getFile(i * 3 + 1);

		// The first byte of the file is the length of the string (without the length)
		assert(f->_length - 1 == f->_data[0]);

		_dialogueBlocks[i]._title = Common::String((char *)(f->_data+1), f->_length-1);

		f = _dialogueArchive->getFile(i * 3 + 2);
		_dialogueBlocks[i]._program._bytecode = f->_data;
		_dialogueBlocks[i]._program._length = f->_length;
	}

	for (uint i = 0; i < kDialogueLines; ++i) {
		_vm->_anims->play(_dialogueAnims[i]->getID());
	}

	setLoopStatus(kStatusDialogue);
	_lastBlock = -1;
	_dialogueBegin = true;
}

void Game::dialogueDone() {
	for (uint i = 0; i < kDialogueLines; ++i) {
		_vm->_anims->stop(_dialogueAnims[i]->getID());
	}

	delete _dialogueArchive;
	delete[] _dialogueBlocks;

	setLoopStatus(kStatusOrdinary);
	_vm->_mouse->setCursorType(kNormalCursor);
}

void Game::runDialogueProg(GPL2Program prog, int offset) {
	// Mark last animation
	int lastAnimIndex = _vm->_anims->getLastIndex();

	// Run the dialogue program
	_vm->_script->run(prog, offset);

	deleteAnimationsAfterIndex(lastAnimIndex);
}

bool Game::isDialogueBegin() const {
	return _dialogueBegin;
}

bool Game::shouldExitDialogue() const {
	return _dialogueExit;
}

void Game::setDialogueExit(bool exit) {
	_dialogueExit = exit;
}

int Game::getDialogueBlockNum() const {
	return _blockNum;
}

int Game::getDialogueVar(int dialogueID) const {
	return _dialogueVars[dialogueID];
}

void Game::setDialogueVar(int dialogueID, int value) {
	_dialogueVars[dialogueID] = value;
}

int Game::getCurrentDialogue() const {
	return _currentDialogue;
}

int Game::getDialogueLastBlock() const {
	return _lastBlock;
}

int Game::getDialogueLinesNum() const {
	return _dialogueLinesNum;
}

int Game::getDialogueCurrentBlock() const {
	return _currentBlock;
}

int Game::getCurrentDialogueOffset() const {
	return _dialogueOffsets[_currentDialogue];
}

void Game::playHeroAnimation(int anim_index) {
	const GameObject *dragon = getObject(kDragonObject);
	const int animID = dragon->_anim[anim_index];
	Animation *anim = _vm->_anims->getAnimation(animID);
	stopObjectAnimations(dragon);
	positionAnimAsHero(anim);
	_vm->_anims->play(animID);
}

void Game::walkHero(int x, int y, SightDirection dir) {
	// Needed for the map room with empty walking map.  For some reason,
	// findNearestWalkable() takes several seconds with 100% CPU to finish
	// (correctly).
	if (!_currentRoom._heroOn)
		return;

	Surface *surface = _vm->_screen->getSurface();
	_hero = _currentRoom._walkingMap.findNearestWalkable(x, y, surface->getDimensions());
	debugC(3, kDraciLogicDebugLevel, "Walk to x: %d y: %d", _hero.x, _hero.y);
	// FIXME: Need to add proper walking (this only warps the dragon to position)

	Movement movement = kStopRight;
	switch (dir) {
	case kDirectionLeft:
		movement = kStopLeft;
		break;
	case kDirectionRight:
		movement = kStopRight;
		break;
	default: {
		const GameObject *dragon = getObject(kDragonObject);
		const int anim_index = playingObjectAnimation(dragon);
		if (anim_index >= 0) {
			movement = static_cast<Movement> (anim_index);
		}
		break;
	}
	}
	playHeroAnimation(movement);
}

void Game::loadItem(int itemID) {
	const BAFile *f = _vm->_itemsArchive->getFile(itemID * 3);
	Common::MemoryReadStream itemReader(f->_data, f->_length);

	GameItem *item = _items + itemID;

	item->_init = itemReader.readSint16LE();
	item->_look = itemReader.readSint16LE();
	item->_use = itemReader.readSint16LE();
	item->_canUse = itemReader.readSint16LE();
	item->_imInit = itemReader.readByte();
	item->_imLook = itemReader.readByte();
	item->_imUse = itemReader.readByte();

	f = _vm->_itemsArchive->getFile(itemID * 3 + 1);

	// The first byte is the length of the string
	item->_title = Common::String((const char *)f->_data + 1, f->_length - 1);
	assert(f->_data[0] == item->_title.size());

	f = _vm->_itemsArchive->getFile(itemID * 3 + 2);

	item->_program._bytecode = f->_data;
	item->_program._length = f->_length;
}

void Game::loadRoom(int roomNum) {
	const BAFile *f;
	f = _vm->_roomsArchive->getFile(roomNum * 4);
	Common::MemoryReadStream roomReader(f->_data, f->_length);

	roomReader.readUint32LE(); // Pointer to room program, not used
	roomReader.readUint16LE(); // Program length, not used
	roomReader.readUint32LE(); // Pointer to room title, not used

	// Music will be played by the GPL2 command startMusic when needed.
	setMusicTrack(roomReader.readByte());

	_currentRoom._mapID = roomReader.readByte() - 1;
	_currentRoom._palette = roomReader.readByte() - 1;
	_currentRoom._numOverlays = roomReader.readSint16LE();
	_currentRoom._init = roomReader.readSint16LE();
	_currentRoom._look = roomReader.readSint16LE();
	_currentRoom._use = roomReader.readSint16LE();
	_currentRoom._canUse = roomReader.readSint16LE();
	_currentRoom._imInit = roomReader.readByte();
	_currentRoom._imLook = roomReader.readByte();
	_currentRoom._imUse = roomReader.readByte();
	_currentRoom._mouseOn = roomReader.readByte();
	_currentRoom._heroOn = roomReader.readByte();

	// Read in pers0 and persStep (stored as 6-byte Pascal reals)
	byte real[6];

	for (int i = 5; i >= 0; --i) {
		real[i] = roomReader.readByte();
	}

	_currentRoom._pers0 = real_to_double(real);

	for (int i = 5; i >= 0; --i) {
		real[i] = roomReader.readByte();
	}

	_currentRoom._persStep = real_to_double(real);

	_currentRoom._escRoom = roomReader.readByte() - 1;
	_currentRoom._numGates = roomReader.readByte();

	debugC(4, kDraciLogicDebugLevel, "Music: %d", getMusicTrack());
	debugC(4, kDraciLogicDebugLevel, "Map: %d", _currentRoom._mapID);
	debugC(4, kDraciLogicDebugLevel, "Palette: %d", _currentRoom._palette);
	debugC(4, kDraciLogicDebugLevel, "Overlays: %d", _currentRoom._numOverlays);
	debugC(4, kDraciLogicDebugLevel, "Init: %d", _currentRoom._init);
	debugC(4, kDraciLogicDebugLevel, "Look: %d", _currentRoom._look);
	debugC(4, kDraciLogicDebugLevel, "Use: %d", _currentRoom._use);
	debugC(4, kDraciLogicDebugLevel, "CanUse: %d", _currentRoom._canUse);
	debugC(4, kDraciLogicDebugLevel, "ImInit: %d", _currentRoom._imInit);
	debugC(4, kDraciLogicDebugLevel, "ImLook: %d", _currentRoom._imLook);
	debugC(4, kDraciLogicDebugLevel, "ImUse: %d", _currentRoom._imUse);
	debugC(4, kDraciLogicDebugLevel, "MouseOn: %d", _currentRoom._mouseOn);
	debugC(4, kDraciLogicDebugLevel, "HeroOn: %d", _currentRoom._heroOn);
	debugC(4, kDraciLogicDebugLevel, "Pers0: %f", _currentRoom._pers0);
	debugC(4, kDraciLogicDebugLevel, "PersStep: %f", _currentRoom._persStep);
	debugC(4, kDraciLogicDebugLevel, "EscRoom: %d", _currentRoom._escRoom);
	debugC(4, kDraciLogicDebugLevel, "Gates: %d", _currentRoom._numGates);

	// Read in the gates' numbers
	_currentRoom._gates.clear();
	for (uint i = 0; i < _currentRoom._numGates; ++i) {
		_currentRoom._gates.push_back(roomReader.readSint16LE());
	}

	// Load the walking map
	loadWalkingMap(_currentRoom._mapID);

	// Load the room's objects
	for (uint i = 0; i < _info._numObjects; ++i) {
		debugC(7, kDraciLogicDebugLevel,
			"Checking if object %d (%d) is at the current location (%d)", i,
			_objects[i]._location, roomNum);

		if (_objects[i]._location == roomNum) {
			debugC(6, kDraciLogicDebugLevel, "Loading object %d from room %d", i, roomNum);
			loadObject(i);
		}
	}

	// Run the init scripts for room objects
	// We can't do this in the above loop because some objects' scripts reference
	// other objects that may not yet be loaded
	for (uint i = 0; i < _info._numObjects; ++i) {
		if (_objects[i]._location == roomNum) {
			const GameObject *obj = getObject(i);
			debugC(6, kDraciLogicDebugLevel,
				"Running init program for object %d (offset %d)", i, obj->_init);
			_vm->_script->run(obj->_program, obj->_init);
		}
	}

	// Load the room's GPL program and run the init part
	f = _vm->_roomsArchive->getFile(roomNum * 4 + 3);
	_currentRoom._program._bytecode = f->_data;
	_currentRoom._program._length = f->_length;

	debugC(4, kDraciLogicDebugLevel, "Running room init program...");
	_vm->_script->run(_currentRoom._program, _currentRoom._init);

	// Set room palette
	f = _vm->_paletteArchive->getFile(_currentRoom._palette);
	_vm->_screen->setPalette(f->_data, 0, kNumColours);

	// HACK: Create a visible overlay from the walking map so we can test it
	byte *wlk = new byte[kScreenWidth * kScreenHeight];
	memset(wlk, 255, kScreenWidth * kScreenHeight);

	for (uint i = 0; i < kScreenWidth; ++i) {
		for (uint j = 0; j < kScreenHeight; ++j) {
			if (_currentRoom._walkingMap.isWalkable(i, j)) {
				wlk[j * kScreenWidth + i] = 2;
			}
		}
	}

	Sprite *ov = new Sprite(wlk, kScreenWidth, kScreenHeight, 0, 0, false);
        delete[] wlk;

	Animation *map = _vm->_anims->addAnimation(kWalkingMapOverlay, 255, false);
	map->addFrame(ov, NULL);
}

int Game::loadAnimation(uint animNum, uint z) {
	const BAFile *animFile = _vm->_animationsArchive->getFile(animNum);
	Common::MemoryReadStream animationReader(animFile->_data, animFile->_length);

	uint numFrames = animationReader.readByte();

	// FIXME: handle these properly
	animationReader.readByte(); // Memory logic field, not used
	animationReader.readByte(); // Disable erasing field, not used

	bool cyclic = animationReader.readByte();

	animationReader.readByte(); // Relative field, not used

	Animation *anim = _vm->_anims->addAnimation(animNum, z, false);

	anim->setLooping(cyclic);

	for (uint i = 0; i < numFrames; ++i) {
		uint spriteNum = animationReader.readUint16LE() - 1;
		int x = animationReader.readSint16LE();
		int y = animationReader.readSint16LE();
		uint scaledWidth = animationReader.readUint16LE();
		uint scaledHeight = animationReader.readUint16LE();
		byte mirror = animationReader.readByte();
		int sample = animationReader.readUint16LE() - 1;
		uint freq = animationReader.readUint16LE();
		uint delay = animationReader.readUint16LE();

		const BAFile *spriteFile = _vm->_spritesArchive->getFile(spriteNum);

		Sprite *sp = new Sprite(spriteFile->_data, spriteFile->_length, x, y, true);

		// Some frames set the scaled dimensions to 0 even though other frames
		// from the same animations have them set to normal values
		// We work around this by assuming it means no scaling is necessary
		if (scaledWidth == 0) {
			scaledWidth = sp->getWidth();
		}

		if (scaledHeight == 0) {
			scaledHeight = sp->getHeight();
		}

		sp->setScaled(scaledWidth, scaledHeight);

		if (mirror)
			sp->setMirrorOn();

		sp->setDelay(delay * 10);

		const SoundSample *sam = _vm->_soundsArchive->getSample(sample, freq);

		anim->addFrame(sp, sam);
	}

	return animNum;
}

void Game::loadObject(uint objNum) {
	const BAFile *file;

	file = _vm->_objectsArchive->getFile(objNum * 3);
	Common::MemoryReadStream objReader(file->_data, file->_length);

	GameObject *obj = _objects + objNum;

	obj->_init = objReader.readUint16LE();
	obj->_look = objReader.readUint16LE();
	obj->_use = objReader.readUint16LE();
	obj->_canUse = objReader.readUint16LE();
	obj->_imInit = objReader.readByte();
	obj->_imLook = objReader.readByte();
	obj->_imUse = objReader.readByte();
	obj->_walkDir = objReader.readByte() - 1;
	obj->_z = objReader.readByte();
	objReader.readUint16LE(); // idxSeq field, not used
	objReader.readUint16LE(); // numSeq field, not used
	obj->_lookX = objReader.readUint16LE();
	obj->_lookY = objReader.readUint16LE();
	obj->_useX = objReader.readUint16LE();
	obj->_useY = objReader.readUint16LE();
	obj->_lookDir = static_cast<SightDirection> (objReader.readByte());
	obj->_useDir = static_cast<SightDirection> (objReader.readByte());

	obj->_absNum = objNum;

	file = _vm->_objectsArchive->getFile(objNum * 3 + 1);

	// The first byte of the file is the length of the string (without the length)
	assert(file->_length - 1 == file->_data[0]);

	obj->_title = Common::String((char *)(file->_data+1), file->_length-1);

	file = _vm->_objectsArchive->getFile(objNum * 3 + 2);
	obj->_program._bytecode = file->_data;
	obj->_program._length = file->_length;
}

void Game::loadWalkingMap(int mapID) {
	if (mapID < 0) {
		mapID = _currentRoom._mapID;
	}
	const BAFile *f;
	f = _vm->_walkingMapsArchive->getFile(mapID);
	_currentRoom._walkingMap.load(f->_data, f->_length);
}

GameObject *Game::getObject(uint objNum) {
	return _objects + objNum;
}

uint Game::getNumObjects() const {
	return _info._numObjects;
}

void Game::loadOverlays() {
	uint x, y, z, num;

	const BAFile *overlayHeader;

	overlayHeader = _vm->_roomsArchive->getFile(getRoomNum() * 4 + 2);
	Common::MemoryReadStream overlayReader(overlayHeader->_data, overlayHeader->_length);

	for (int i = 0; i < _currentRoom._numOverlays; i++) {
		num = overlayReader.readUint16LE() - 1;
		x = overlayReader.readUint16LE();
		y = overlayReader.readUint16LE();
		z = overlayReader.readByte();

		const BAFile *overlayFile;
		overlayFile = _vm->_overlaysArchive->getFile(num);
		Sprite *sp = new Sprite(overlayFile->_data, overlayFile->_length, x, y, true);

		_vm->_anims->addOverlay(sp, z);
	}

	_vm->_overlaysArchive->clearCache();

	_vm->_screen->getSurface()->markDirty();
}

void Game::deleteObjectAnimations() {
	for (uint i = 0; i < _info._numObjects; ++i) {
		GameObject *obj = &_objects[i];

		if (i != 0 && (obj->_location == getPreviousRoomNum())) {
			for (uint j = 0; j < obj->_anim.size(); ++j) {
					_vm->_anims->deleteAnimation(obj->_anim[j]);
			}
			obj->_anim.clear();
		}
	}
}

int Game::playingObjectAnimation(const GameObject *obj) const {
	for (uint i = 0; i < obj->_anim.size(); ++i) {
		const int animID = obj->_anim[i];
		const Animation *anim = _vm->_anims->getAnimation(animID);
		if (anim && anim->isPlaying()) {
			return i;
		}
	}
	return -1;
}

void Game::enterNewRoom(bool force_reload) {
	if (_newRoom == getRoomNum() && !force_reload) {
		return;
	}
	debugC(1, kDraciLogicDebugLevel, "Entering room %d using gate %d", _newRoom, _newGate);

	// TODO: maybe wait till all sounds end instead of stopping them.
	// In any case, make sure all sounds are stopped before we deallocate
	// their memory by clearing the cache.
	_vm->_sound->stopAll();

	// Clear archives
	_vm->_roomsArchive->clearCache();
	_vm->_spritesArchive->clearCache();
	_vm->_paletteArchive->clearCache();
	_vm->_animationsArchive->clearCache();
	_vm->_walkingMapsArchive->clearCache();
	_vm->_soundsArchive->clearCache();
	_vm->_dubbingArchive->clearCache();

	_vm->_screen->clearScreen();

	_vm->_anims->deleteOverlays();

	// Delete walking map testing overlay
	_vm->_anims->deleteAnimation(kWalkingMapOverlay);

	// TODO: Make objects capable of stopping their own animations
	const GameObject *dragon = getObject(kDragonObject);
	stopObjectAnimations(dragon);

	// Remember the previous room for returning back from the map.
	rememberRoomNumAsPrevious();
	deleteObjectAnimations();

	// Set the current room to the new value
	_currentRoom._roomNum = _newRoom;

	// Before setting these variables we have to convert the values to 1-based indexing
	// because this is how everything is stored in the data files
	_variables[0] = _newGate + 1;
	_variables[1] = _newRoom + 1;

	// If the new room is the map room, set the appropriate coordinates
	// for the dragon in the persons array
	if (_newRoom == _info._mapRoom) {
		_persons[kDragonObject]._x = 160;
	  	_persons[kDragonObject]._y = 0;
	}

	// Set the appropriate loop statu before loading the room
	setLoopStatus(kStatusGate);
	setLoopSubstatus(kSubstatusOrdinary);

	loadRoom(_newRoom);
	loadOverlays();

	// Run the program for the gate the dragon came through
	runGateProgram(_newGate);

	// Set cursor state
	// Need to do this after we set the palette since the cursors use it
	if (_currentRoom._mouseOn) {
		debugC(6, kDraciLogicDebugLevel, "Mouse: ON");
		_vm->_mouse->cursorOn();
	} else {
		debugC(6, kDraciLogicDebugLevel, "Mouse: OFF");
		_vm->_mouse->cursorOff();
	}

	// Reset the loop status.
	setLoopStatus(kStatusOrdinary);

	_vm->_mouse->setCursorType(kNormalCursor);
}

void Game::runGateProgram(int gate) {
	debugC(6, kDraciLogicDebugLevel, "Running program for gate %d", gate);

	// Mark last animation
	int lastAnimIndex = _vm->_anims->getLastIndex();

	// Run gate program
	_vm->_script->run(_currentRoom._program, _currentRoom._gates[gate]);

	deleteAnimationsAfterIndex(lastAnimIndex);

	setExitLoop(false);
}

void Game::positionAnimAsHero(Animation *anim) {
	// Calculate scaling factors
	const double scale = getPers0() + getPersStep() * _hero.y;

	// Set the Z coordinate for the dragon's animation
	anim->setZ(_hero.y + 1);

	// Fetch current frame
	Drawable *frame = anim->getCurrentFrame();

	// Fetch base dimensions of the frame
	uint height = frame->getHeight();
	uint width = frame->getWidth();

	// We naturally want the dragon to position its feet to the location of the
	// click but sprites are drawn from their top-left corner so we subtract
	// the current height of the dragon's sprite
	Common::Point p = _hero;
	p.x -= (int)(scale * width) / 2;
	p.y -= (int)(scale * height);

	// Since _persons[] is used for placing talking text, we use the non-adjusted x value
	// so the text remains centered over the dragon.
	_persons[kDragonObject]._x = _hero.x;
	_persons[kDragonObject]._y = p.y;

	// Set the per-animation scaling factor
	anim->setScaleFactors(scale, scale);

	anim->setRelative(p.x, p.y);
}

int Game::getHeroX() const {
	return _hero.x;
}

int Game::getHeroY() const {
	return _hero.y;
}

double Game::getPers0() const {
	return _currentRoom._pers0;
}

double Game::getPersStep() const {
	return _currentRoom._persStep;
}

int Game::getMusicTrack() const {
	return _currentRoom._music;
}

void Game::setMusicTrack(int num) {
	_currentRoom._music = num;
}

int Game::getRoomNum() const {
	return _currentRoom._roomNum;
}

void Game::setRoomNum(int num) {
	_currentRoom._roomNum = num;
}

int Game::getPreviousRoomNum() const {
	return _previousRoom;
}

void Game::rememberRoomNumAsPrevious() {
	_previousRoom = getRoomNum();
}

void Game::scheduleEnteringRoomUsingGate(int room, int gate) {
	_newRoom = room;
	_newGate = gate;
}

void Game::pushNewRoom() {
	_pushedNewRoom = _newRoom;
	_pushedNewGate = _newGate;
}

void Game::popNewRoom() {
	if (_loopStatus != kStatusInventory && _pushedNewRoom >= 0) {
		scheduleEnteringRoomUsingGate(_pushedNewRoom, _pushedNewGate);
		_pushedNewRoom = _pushedNewGate = -1;
	}
}

void Game::setLoopStatus(LoopStatus status) {
	_loopStatus = status;
}

void Game::setLoopSubstatus(LoopSubstatus status) {
	_loopSubstatus = status;
}

LoopStatus Game::getLoopStatus() const {
	return _loopStatus;
}

LoopSubstatus Game::getLoopSubstatus() const {
	return _loopSubstatus;
}

int Game::getVariable(int numVar) const {
	return _variables[numVar];
}

void Game::setVariable(int numVar, int value) {
	_variables[numVar] = value;
}

int Game::getItemStatus(int itemID) const {
	return _itemStatus[itemID];
}

void Game::setItemStatus(int itemID, int status) {
	_itemStatus[itemID] = status;
}

int Game::getCurrentItem() const {
	return _currentItem;
}

void Game::setCurrentItem(int itemID) {
	_currentItem = itemID;
}

const Person *Game::getPerson(int personID) const {
	return &_persons[personID];
}

void Game::setSpeechTiming(uint tick, uint duration) {
	_speechTick = tick;
	_speechDuration = duration;
}

void Game::shiftSpeechAndFadeTick(int delta) {
	_speechTick += delta;
	_fadeTick += delta;
}

int Game::getEscRoom() const {
	return _currentRoom._escRoom;
}

int Game::getMapRoom() const {
	return _info._mapRoom;
}

void Game::schedulePalette(int paletteID) {
	_scheduledPalette = paletteID;
}

int Game::getScheduledPalette() const {
	return _scheduledPalette;
}

void Game::initializeFading(int phases) {
	_fadePhases = _fadePhase = phases;
	_fadeTick = _vm->_system->getMillis();
}

void Game::setEnableQuickHero(bool value) {
	_enableQuickHero = value;
}

void Game::setWantQuickHero(bool value) {
	_wantQuickHero = value;
	// TODO: after proper walking is implemented, do super-fast animation when walking
}

void Game::setEnableSpeedText(bool value) {
	_enableSpeedText = value;
}

/**
 * The GPL command Mark sets the animation index (which specifies the order in which
 * animations were loaded in) which is then used by the Release command to delete
 * all animations that have an index greater than the one marked.
 */
int Game::getMarkedAnimationIndex() const {
	return _markedAnimationIndex;
}

void Game::deleteAnimationsAfterIndex(int lastAnimIndex) {
	// Delete all animations loaded after the marked one
	// (from objects and from the AnimationManager)
	for (uint i = 0; i < getNumObjects(); ++i) {
		GameObject *obj = &_objects[i];

		for (uint j = 0; j < obj->_anim.size(); ++j) {
			Animation *anim;

			anim = _vm->_anims->getAnimation(obj->_anim[j]);
			if (anim != NULL && anim->getIndex() > lastAnimIndex)
				obj->_anim.remove_at(j--);
		}
	}

	_vm->_anims->deleteAfterIndex(lastAnimIndex);
}

void Game::stopObjectAnimations(const GameObject *obj) {
	for (uint i = 0; i < obj->_anim.size(); ++i) {
		_vm->_anims->stop(obj->_anim[i]);
	}
}

/**
 * See Game::getMarkedAnimationIndex().
 */

void Game::setMarkedAnimationIndex(int index) {
	_markedAnimationIndex = index;
}

Game::~Game() {
	delete[] _persons;
	delete[] _variables;
	delete[] _dialogueOffsets;
	delete[] _dialogueVars;
	delete[] _objects;
	delete[] _itemStatus;
	delete[] _items;
}

void Game::DoSync(Common::Serializer &s) {
	s.syncAsUint16LE(_currentRoom._roomNum);

	for (uint i = 0; i < _info._numObjects; ++i) {
		GameObject& obj = _objects[i];
		s.syncAsSint16LE(obj._location);
		s.syncAsByte(obj._visible);
	}

	for (uint i = 0; i < _info._numItems; ++i) {
		s.syncAsByte(_itemStatus[i]);
	}

	for (int i = 0; i < kInventorySlots; ++i) {
		s.syncAsSint16LE(_inventory[i]);
	}

	for (int i = 0; i < _info._numVariables; ++i) {
		s.syncAsSint16LE(_variables[i]);
	}
	for (uint i = 0; i < _info._numDialogueBlocks; ++i) {
		s.syncAsSint16LE(_dialogueVars[i]);
	}

}

static double real_to_double(byte real[6]) {
	// Extract sign bit
	int sign = real[0] & (1 << 7);

	// Extract exponent and adjust for bias
	int exp = real[5] - 129;

	double mantissa;
	double tmp = 0.0;

	if (real[5] == 0) {
		mantissa = 0.0;
	} else {
		// Process the first four least significant bytes
		for (int i = 4; i >= 1; --i) {
			tmp += real[i];
			tmp /= 1 << 8;
		}

		// Process the most significant byte (remove the sign bit)
		tmp += real[0] & ((1 << 7) - 1);
		tmp /= 1 << 8;

		// Calculate mantissa
		mantissa = 1.0;
		mantissa += 2.0 * tmp;
	}

	// Flip sign if necessary
	if (sign) {
		mantissa = -mantissa;
	}

	// Calculate final value
	return ldexp(mantissa, exp);
}

}
