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

#include "common/util.h"
#include "common/stack.h"
#include "graphics/primitives.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/engine/vm.h"
#include "sci/graphics/gfx.h"
#include "sci/graphics/view.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/transitions.h"
#include "sci/graphics/animate.h"

namespace Sci {

SciGuiAnimate::SciGuiAnimate(EngineState *state, Gfx *gfx, Screen *screen, SciPalette *palette)
	: _s(state), _gfx(gfx), _screen(screen), _palette(palette) {
	init();
}

SciGuiAnimate::~SciGuiAnimate() {
	free(_listData);
	free(_lastCastData);
}

void SciGuiAnimate::init() {
	_listData = NULL;
	_listCount = 0;
	_lastCastData = NULL;
	_lastCastCount = 0;

	_ignoreFastCast = false;
	// fastCast object is not found in any SCI games prior SCI1
	if (getSciVersion() <= SCI_VERSION_01)
		_ignoreFastCast = true;
	// Also if fastCast object exists at gamestartup, we can assume that the interpreter doesnt do kAnimate aborts
	//  (found in larry 1)
	if (!_s->_segMan->findObjectByName("fastCast").isNull())
		_ignoreFastCast = true;
}

void SciGuiAnimate::disposeLastCast() {
	_lastCastCount = 0;
}

bool SciGuiAnimate::invoke(List *list, int argc, reg_t *argv) {
	reg_t curAddress = list->first;
	Node *curNode = _s->_segMan->lookupNode(curAddress);
	reg_t curObject;
	uint16 signal;

	while (curNode) {
		curObject = curNode->value;

		if (!_ignoreFastCast) {
			// Check if the game has a fastCast object set
			//  if we don't abort kAnimate processing, at least in kq5 there will be animation cels drawn into speech boxes.
			reg_t global84 = _s->script_000->_localsBlock->_locals[84];

			if (!global84.isNull()) {
				if (!strcmp(_s->_segMan->getObjectName(global84), "fastCast"))
					return false;
			}
		}

		signal = GET_SEL32V(_s->_segMan, curObject, signal);
		if (!(signal & kSignalFrozen)) {
			// Call .doit method of that object
			invoke_selector(_s, curObject, _s->_kernel->_selectorCache.doit, kContinueOnInvalidSelector, argv, argc, 0);
			// Lookup node again, since the nodetable it was in may have been reallocated
			curNode = _s->_segMan->lookupNode(curAddress);
		}
		curAddress = curNode->succ;
		curNode = _s->_segMan->lookupNode(curAddress);
	}
	return true;
}

bool sortHelper(const AnimateEntry* entry1, const AnimateEntry* entry2) {
	return (entry1->y == entry2->y) ? (entry1->z < entry2->z) : (entry1->y < entry2->y);
}

void SciGuiAnimate::makeSortedList(List *list) {
	reg_t curAddress = list->first;
	Node *curNode = _s->_segMan->lookupNode(curAddress);
	reg_t curObject;
	AnimateEntry *listEntry;
	int16 listNr, listCount = 0;

	// Count the list entries
	while (curNode) {
		listCount++;
		curAddress = curNode->succ;
		curNode = _s->_segMan->lookupNode(curAddress);
	}

	_list.clear();

	// No entries -> exit immediately
	if (listCount == 0)
		return;

	// Adjust list size, if needed
	if ((_listData == NULL) || (_listCount < listCount)) {
		free(_listData);
		_listData = (AnimateEntry *)malloc(listCount * sizeof(AnimateEntry));
		if (!_listData)
			error("Could not allocate memory for _listData");
		_listCount = listCount;

		free(_lastCastData);
		_lastCastData = (AnimateEntry *)malloc(listCount * sizeof(AnimateEntry));
		if (!_lastCastData)
			error("Could not allocate memory for _lastCastData");
		_lastCastCount = 0;
	}

	// Fill the list
	curAddress = list->first;
	curNode = _s->_segMan->lookupNode(curAddress);
	listEntry = _listData;
	for (listNr = 0; listNr < listCount; listNr++) {
		curObject = curNode->value;
		listEntry->object = curObject;

		// Get data from current object
		listEntry->viewId = GET_SEL32V(_s->_segMan, curObject, view);
		listEntry->loopNo = GET_SEL32V(_s->_segMan, curObject, loop);
		listEntry->celNo = GET_SEL32V(_s->_segMan, curObject, cel);
		listEntry->paletteNo = GET_SEL32V(_s->_segMan, curObject, palette);
		listEntry->x = GET_SEL32V(_s->_segMan, curObject, x);
		listEntry->y = GET_SEL32V(_s->_segMan, curObject, y);
		listEntry->z = GET_SEL32V(_s->_segMan, curObject, z);
		listEntry->priority = GET_SEL32V(_s->_segMan, curObject, priority);
		listEntry->signal = GET_SEL32V(_s->_segMan, curObject, signal);
		// listEntry->celRect is filled in AnimateFill()
		listEntry->showBitsFlag = false;

		_list.push_back(listEntry);

		listEntry++;
		curAddress = curNode->succ;
		curNode = _s->_segMan->lookupNode(curAddress);
	}

	// Now sort the list according y and z (descending)
	AnimateList::iterator listBegin = _list.begin();
	AnimateList::iterator listEnd = _list.end();

	Common::sort(_list.begin(), _list.end(), sortHelper);
}

void SciGuiAnimate::fill(byte &old_picNotValid) {
	reg_t curObject;
	AnimateEntry *listEntry;
	uint16 signal;
	View *view = NULL;
	AnimateList::iterator listIterator;
	AnimateList::iterator listEnd = _list.end();

	listIterator = _list.begin();
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;

		// Get the corresponding view
		view = _gfx->getView(listEntry->viewId);
		
		// adjust loop and cel, if any of those is invalid
		if (listEntry->loopNo >= view->getLoopCount()) {
			listEntry->loopNo = 0;
			PUT_SEL32V(_s->_segMan, curObject, loop, listEntry->loopNo);
		}
		if (listEntry->celNo >= view->getCelCount(listEntry->loopNo)) {
			listEntry->celNo = 0;
			PUT_SEL32V(_s->_segMan, curObject, cel, listEntry->celNo);
		}

		// Create rect according to coordinates and given cel
		view->getCelRect(listEntry->loopNo, listEntry->celNo, listEntry->x, listEntry->y, listEntry->z, &listEntry->celRect);
		PUT_SEL32V(_s->_segMan, curObject, nsLeft, listEntry->celRect.left);
		PUT_SEL32V(_s->_segMan, curObject, nsTop, listEntry->celRect.top);
		PUT_SEL32V(_s->_segMan, curObject, nsRight, listEntry->celRect.right);
		PUT_SEL32V(_s->_segMan, curObject, nsBottom, listEntry->celRect.bottom);

		signal = listEntry->signal;

		// Calculate current priority according to y-coordinate
		if (!(signal & kSignalFixedPriority)) {
			listEntry->priority = _gfx->CoordinateToPriority(listEntry->y);
			PUT_SEL32V(_s->_segMan, curObject, priority, listEntry->priority);
		}
		
		if (signal & kSignalNoUpdate) {
			if (signal & (kSignalForceUpdate | kSignalViewUpdated)
				|| (signal & kSignalHidden && !(signal & kSignalRemoveView))
				|| (!(signal & kSignalHidden) && signal & kSignalRemoveView)
				|| (signal & kSignalAlwaysUpdate))
				old_picNotValid++;
			signal &= 0xFFFF ^ kSignalStopUpdate;
		} else {
			if (signal & kSignalStopUpdate || signal & kSignalAlwaysUpdate)
				old_picNotValid++;
			signal &= 0xFFFF ^ kSignalForceUpdate;
		}
		listEntry->signal = signal;

		listIterator++;
	}
}

void SciGuiAnimate::update() {
	reg_t curObject;
	AnimateEntry *listEntry;
	uint16 signal;
	reg_t bitsHandle;
	Common::Rect rect;
	AnimateList::iterator listIterator;
	AnimateList::iterator listBegin = _list.begin();
	AnimateList::iterator listEnd = _list.end();

	// Remove all no-update cels, if requested
	listIterator = _list.reverse_begin();
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;
		signal = listEntry->signal;

		if (signal & kSignalNoUpdate) {
			if (!(signal & kSignalRemoveView)) {
				bitsHandle = GET_SEL32(_s->_segMan, curObject, underBits);
				if (_screen->_picNotValid != 1) {
					_gfx->BitsRestore(bitsHandle);
					listEntry->showBitsFlag = true;
				} else	{
					_gfx->BitsFree(bitsHandle);
				}
				PUT_SEL32V(_s->_segMan, curObject, underBits, 0);
			}
			signal &= 0xFFFF ^ kSignalForceUpdate;
			signal &= signal & kSignalViewUpdated ? 0xFFFF ^ (kSignalViewUpdated | kSignalNoUpdate) : 0xFFFF;
		} else if (signal & kSignalStopUpdate) {
			signal =  (signal & (0xFFFF ^ kSignalStopUpdate)) | kSignalNoUpdate;
		}
		listEntry->signal = signal;
		listIterator--;
	}

	// Draw always-update cels
	listIterator = listBegin;
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;
		signal = listEntry->signal;

		if (signal & kSignalAlwaysUpdate) {
			// draw corresponding cel
			_gfx->drawCel(listEntry->viewId, listEntry->loopNo, listEntry->celNo, listEntry->celRect, listEntry->priority, listEntry->paletteNo);
			listEntry->showBitsFlag = true;

			signal &= 0xFFFF ^ (kSignalStopUpdate | kSignalViewUpdated | kSignalNoUpdate | kSignalForceUpdate);
			if ((signal & kSignalIgnoreActor) == 0) {
				rect = listEntry->celRect;
				rect.top = CLIP<int16>(_gfx->PriorityToCoordinate(listEntry->priority) - 1, rect.top, rect.bottom - 1);  
				_gfx->FillRect(rect, SCI_SCREEN_MASK_CONTROL, 0, 0, 15);
			}
			listEntry->signal = signal;
		}
		listIterator++;
	}

	// Saving background for all NoUpdate-cels
	listIterator = listBegin;
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;
		signal = listEntry->signal;

		if (signal & kSignalNoUpdate) {
			if (signal & kSignalHidden) {
				signal |= kSignalRemoveView;
			} else {
				signal &= 0xFFFF ^ kSignalRemoveView;
				if (signal & kSignalIgnoreActor)
					bitsHandle = _gfx->BitsSave(listEntry->celRect, SCI_SCREEN_MASK_VISUAL|SCI_SCREEN_MASK_PRIORITY);
				else
					bitsHandle = _gfx->BitsSave(listEntry->celRect, SCI_SCREEN_MASK_ALL);
				PUT_SEL32(_s->_segMan, curObject, underBits, bitsHandle);
			}
			listEntry->signal = signal;
		}
		listIterator++;
	}

	// Draw NoUpdate cels
	listIterator = listBegin;
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;
		signal = listEntry->signal;

		if (signal & kSignalNoUpdate && !(signal & kSignalHidden)) {
			// draw corresponding cel
			_gfx->drawCel(listEntry->viewId, listEntry->loopNo, listEntry->celNo, listEntry->celRect, listEntry->priority, listEntry->paletteNo);
			listEntry->showBitsFlag = true;

			if ((signal & kSignalIgnoreActor) == 0) {
				rect = listEntry->celRect;
				rect.top = CLIP<int16>(_gfx->PriorityToCoordinate(listEntry->priority) - 1, rect.top, rect.bottom - 1);  
				_gfx->FillRect(rect, SCI_SCREEN_MASK_CONTROL, 0, 0, 15);
			}
		}
		listIterator++;
	}
}

void SciGuiAnimate::drawCels() {
	reg_t curObject;
	AnimateEntry *listEntry;
	AnimateEntry *lastCastEntry = _lastCastData;
	uint16 signal;
	reg_t bitsHandle;
	AnimateList::iterator listIterator;
	AnimateList::iterator listEnd = _list.end();
	uint16 scaleX = 128, scaleY = 128;	// no scaling
	_lastCastCount = 0;

	listIterator = _list.begin();
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;
		signal = listEntry->signal;

		if (!(signal & (kSignalNoUpdate | kSignalHidden | kSignalAlwaysUpdate))) {
			// Save background
			bitsHandle = _gfx->BitsSave(listEntry->celRect, SCI_SCREEN_MASK_ALL);
			PUT_SEL32(_s->_segMan, curObject, underBits, bitsHandle);

			if (getSciVersion() >= SCI_VERSION_1_1) {
				// View scaling
				scaleX = GET_SEL32V(_s->_segMan, curObject, scaleX);
				scaleY = GET_SEL32V(_s->_segMan, curObject, scaleY);
			}

			// draw corresponding cel
			_gfx->drawCel(listEntry->viewId, listEntry->loopNo, listEntry->celNo, listEntry->celRect, listEntry->priority, listEntry->paletteNo, -1, scaleX, scaleY);
			listEntry->showBitsFlag = true;

			if (signal & kSignalRemoveView) {
				signal &= 0xFFFF ^ kSignalRemoveView;
			}
			listEntry->signal = signal;

			// Remember that entry in lastCast
			memcpy(lastCastEntry, listEntry, sizeof(AnimateEntry));
			lastCastEntry++; _lastCastCount++;
		}
		listIterator++;
	}
}

void SciGuiAnimate::updateScreen(byte oldPicNotValid) {
	reg_t curObject;
	AnimateEntry *listEntry;
	uint16 signal;
	AnimateList::iterator listIterator;
	AnimateList::iterator listEnd = _list.end();
	Common::Rect lsRect;
	Common::Rect workerRect;

	listIterator = _list.begin();
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;
		signal = listEntry->signal;

		if (listEntry->showBitsFlag || !(signal & (kSignalRemoveView | kSignalNoUpdate) ||
										(!(signal & kSignalRemoveView) && (signal & kSignalNoUpdate) && oldPicNotValid))) {
			lsRect.left = GET_SEL32V(_s->_segMan, curObject, lsLeft);
			lsRect.top = GET_SEL32V(_s->_segMan, curObject, lsTop);
			lsRect.right = GET_SEL32V(_s->_segMan, curObject, lsRight);
			lsRect.bottom = GET_SEL32V(_s->_segMan, curObject, lsBottom);

			workerRect = lsRect;
			workerRect.clip(listEntry->celRect);

			if (!workerRect.isEmpty()) {
				workerRect = lsRect;
				workerRect.extend(listEntry->celRect);
			} else {
				_gfx->BitsShow(lsRect);
				workerRect = listEntry->celRect;
			}
			PUT_SEL32V(_s->_segMan, curObject, lsLeft, workerRect.left);
			PUT_SEL32V(_s->_segMan, curObject, lsTop, workerRect.top);
			PUT_SEL32V(_s->_segMan, curObject, lsRight, workerRect.right);
			PUT_SEL32V(_s->_segMan, curObject, lsBottom, workerRect.bottom);
			_gfx->BitsShow(workerRect);

			if (signal & kSignalHidden) {
				listEntry->signal |= kSignalRemoveView;
			}
		}

		listIterator++;
	}
	// use this for debug purposes
	// _screen->copyToScreen();
}

void SciGuiAnimate::restoreAndDelete(int argc, reg_t *argv) {
	reg_t curObject;
	AnimateEntry *listEntry;
	uint16 signal;
	AnimateList::iterator listIterator;
	AnimateList::iterator listEnd = _list.end();


	// This has to be done in a separate loop. At least in sq1 some .dispose modifies FIXEDLOOP flag in signal for
	//  another object. In that case we would overwrite the new signal with our version of the old signal
	listIterator = _list.begin();
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;
		signal = listEntry->signal;

		// Finally update signal
		PUT_SEL32V(_s->_segMan, curObject, signal, signal);
		listIterator++;
	}

	listIterator = _list.reverse_begin();
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;
		// We read out signal here again, this is not by accident but to ensure that we got an up-to-date signal
		signal = GET_SEL32V(_s->_segMan, curObject, signal);

		if ((signal & (kSignalNoUpdate | kSignalRemoveView)) == 0) {
			_gfx->BitsRestore(GET_SEL32(_s->_segMan, curObject, underBits));
			PUT_SEL32V(_s->_segMan, curObject, underBits, 0);
		}

		if (signal & kSignalDisposeMe) {
			// Call .delete_ method of that object
			invoke_selector(_s, curObject, _s->_kernel->_selectorCache.delete_, kContinueOnInvalidSelector, argv, argc, 0);
		}
		listIterator--;
	}
}

void SciGuiAnimate::reAnimate(Common::Rect rect) {
	AnimateEntry *lastCastEntry;
	uint16 lastCastCount;

	if (_lastCastCount > 0) {
		lastCastEntry = _lastCastData;
		lastCastCount = _lastCastCount;
		while (lastCastCount > 0) {
			lastCastEntry->castHandle = _gfx->BitsSave(lastCastEntry->celRect, SCI_SCREEN_MASK_VISUAL|SCI_SCREEN_MASK_PRIORITY);
			_gfx->drawCel(lastCastEntry->viewId, lastCastEntry->loopNo, lastCastEntry->celNo, lastCastEntry->celRect, lastCastEntry->priority, lastCastEntry->paletteNo);
			lastCastEntry++; lastCastCount--;
		}
		_gfx->BitsShow(rect);
		// restoring
		lastCastCount = _lastCastCount;
		while (lastCastCount > 0) {
			lastCastEntry--;
			_gfx->BitsRestore(lastCastEntry->castHandle);
			lastCastCount--;
		}
	} else {
		_gfx->BitsShow(rect);
	}

	/*
	if (!_lastCast->isEmpty()) {
		HEAPHANDLE hnode = _lastCast->getFirst();
		sciCast *pCast;
		CResView *res;
		while (hnode) {
			pCast = (sciCast *)heap2Ptr(hnode);
			res = (CResView *)ResMgr.ResLoad(SCI_RES_VIEW, pCast->view);
			pCast->hSaved = _gfx->SaveBits(pCast->rect, 3);
			res->drawCel(pCast->loop, pCast->cel, &pCast->rect, pCast->z, pCast->pal);
			hnode = pCast->node.next;
		}
		_gfx->BitsShow(rect);
		// restoring
		hnode = _lastCast->getLast();
		while (hnode) {
			pCast = (sciCast *)heap2Ptr(hnode);
			_gfx->BitsShow(pCast->hSaved);
			hnode = pCast->node.prev;
		}
	*/
}

void SciGuiAnimate::addToPicDrawCels() {
	reg_t curObject;
	AnimateEntry *listEntry;
	View *view = NULL;
	AnimateList::iterator listIterator;
	AnimateList::iterator listEnd = _list.end();

	listIterator = _list.begin();
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;

		if (listEntry->priority == -1)
			listEntry->priority = _gfx->CoordinateToPriority(listEntry->y);

		// Get the corresponding view
		view = _gfx->getView(listEntry->viewId);

		// Create rect according to coordinates and given cel
		view->getCelRect(listEntry->loopNo, listEntry->celNo, listEntry->x, listEntry->y, listEntry->z, &listEntry->celRect);

		// draw corresponding cel
		_gfx->drawCel(listEntry->viewId, listEntry->loopNo, listEntry->celNo, listEntry->celRect, listEntry->priority, listEntry->paletteNo);
		if ((listEntry->signal & kSignalIgnoreActor) == 0) {
			listEntry->celRect.top = CLIP<int16>(_gfx->PriorityToCoordinate(listEntry->priority) - 1, listEntry->celRect.top, listEntry->celRect.bottom - 1);
			_gfx->FillRect(listEntry->celRect, SCI_SCREEN_MASK_CONTROL, 0, 0, 15);
		}

		listIterator++;
	}
}

void SciGuiAnimate::addToPicDrawView(GuiResourceId viewId, LoopNo loopNo, CelNo celNo, int16 leftPos, int16 topPos, int16 priority, int16 control) {
	View *view = _gfx->getView(viewId);
	Common::Rect celRect;

	// Create rect according to coordinates and given cel
	view->getCelRect(loopNo, celNo, leftPos, topPos, priority, &celRect);
	_gfx->drawCel(view, loopNo, celNo, celRect, priority, 0);
}

} // End of namespace Sci
