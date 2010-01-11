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

#include "graphics/cursorman.h"
#include "graphics/video/avi_decoder.h"
#include "graphics/surface.h"

#include "sci/sci.h"
#include "sci/debug.h"	// for g_debug_sleeptime_factor
#include "sci/resource.h"
#include "sci/video/seq_decoder.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/graphics/gui.h"
#include "sci/graphics/animate.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/view.h"

namespace Sci {

void _k_dirloop(reg_t object, uint16 angle, EngineState *s, int argc, reg_t *argv) {
	GuiResourceId viewId = GET_SEL32V(s->_segMan, object, view);
	uint16 signal = GET_SEL32V(s->_segMan, object, signal);
	int16 loopNo;
	int16 maxLoops;
	bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);

	if (signal & kSignalDoesntTurn)
		return;

	angle %= 360;

	if (!oldScriptHeader) {
		if (angle < 45)
			loopNo = 3;
		else if (angle < 136)
			loopNo = 0;
		else if (angle < 225)
			loopNo = 2;
		else if (angle < 316)
			loopNo = 1;
		else
			loopNo = 3;
	} else {
		if (angle >= 330 || angle <= 30)
			loopNo = 3;
		else if (angle <= 150)
			loopNo = 0;
		else if (angle <= 210)
			loopNo = 2;
		else if (angle < 330)
			loopNo = 1;
		else loopNo = -1;
	}

	maxLoops = s->_gui->getLoopCount(viewId);

	if ((loopNo > 1) && (maxLoops < 4))
		return;

	PUT_SEL32V(s->_segMan, object, loop, loopNo);
}

static reg_t kSetCursorSci0(EngineState *s, int argc, reg_t *argv) {
	Common::Point pos;
	GuiResourceId cursorId = argv[0].toSint16();

	// Set pointer position, if requested
	if (argc >= 4) {
		pos.y = argv[3].toSint16();
		pos.x = argv[2].toSint16();
		s->_gui->setCursorPos(pos);
	}

	if ((argc >= 2) && (argv[1].toSint16() == 0)) {
		cursorId = -1;
	}

	s->_gui->setCursorShape(cursorId);
	return s->r_acc;
}

static reg_t kSetCursorSci11(EngineState *s, int argc, reg_t *argv) {
	Common::Point pos;
	Common::Point *hotspot = NULL;

	switch (argc) {
	case 1:
		switch (argv[0].toSint16()) {
		case 0:
			s->_gui->hideCursor();
			break;
		case -1:
			// TODO: Special case at least in kq6, check disassembly
			break;
		case -2:
			// TODO: Special case at least in kq6, check disassembly
			break;
		default:
			s->_gui->showCursor();
			break;
		}
	case 2:
		pos.y = argv[1].toSint16();
		pos.x = argv[0].toSint16();
		s->_gui->setCursorPos(pos);
		break;
	case 4: {
		int16 top = argv[0].toSint16();
		int16 left = argv[1].toSint16();
		int16 bottom = argv[2].toSint16();
		int16 right = argv[3].toSint16();

		if ((right >= left) && (bottom >= top)) {
			Common::Rect rect = Common::Rect(left, top, right, bottom);
			s->_gui->setCursorZone(rect);
		} else {
			warning("kSetCursor: Ignoring invalid mouse zone (%i, %i)-(%i, %i)", left, top, right, bottom);
		}
		break;
	}
	case 5:
	case 9:
		hotspot = new Common::Point(argv[3].toSint16(), argv[4].toSint16());
		// Fallthrough
	case 3:
		s->_gui->setCursorView(argv[0].toUint16(), argv[1].toUint16(), argv[2].toUint16(), hotspot);
		break;
	default :
		warning("kSetCursor: Unhandled case: %d arguments given", argc);
		break;
	}
	return s->r_acc;
}

reg_t kSetCursor(EngineState *s, int argc, reg_t *argv) {
	switch (s->detectSetCursorType()) {
	case SCI_VERSION_0_EARLY:
		return kSetCursorSci0(s, argc, argv);
	case SCI_VERSION_1_1:
		return kSetCursorSci11(s, argc, argv);
	default:
		warning("Unknown SetCursor type");
		return NULL_REG;
	}
}

reg_t kMoveCursor(EngineState *s, int argc, reg_t *argv) {
	Common::Point pos;
	if (argc == 2) {
		pos.y = argv[1].toSint16();
		pos.x = argv[0].toSint16();
		s->_gui->setCursorPos(pos);
	}
	return s->r_acc;
}

reg_t kPicNotValid(EngineState *s, int argc, reg_t *argv) {
	int16 newPicNotValid = (argc > 0) ? argv[0].toUint16() : -1;

	return make_reg(0, s->_gui->picNotValid(newPicNotValid));
}

void kGraphCreateRect(int16 x, int16 y, int16 x1, int16 y1, Common::Rect *destRect) {
	if (x > x1) SWAP(x, x1);
	if (y > y1) SWAP(y, y1);
	*destRect = Common::Rect(x, y, x1, y1);
}

// Graph subfunctions
enum {
	K_GRAPH_GET_COLORS_NR = 2,
	// 3 - SET PALETTE VIA RESOURCE
	K_GRAPH_DRAW_LINE = 4,
	// 5 - NOP
	// 6 - DRAW PATTERN
	K_GRAPH_SAVE_BOX = 7,
	K_GRAPH_RESTORE_BOX = 8,
	K_GRAPH_FILL_BOX_BACKGROUND = 9,
	K_GRAPH_FILL_BOX_FOREGROUND = 10,
	K_GRAPH_FILL_BOX_ANY = 11,
	K_GRAPH_UPDATE_BOX = 12,
	K_GRAPH_REDRAW_BOX = 13,
	K_GRAPH_ADJUST_PRIORITY = 14,
	K_GRAPH_SAVE_UPSCALEDHIRES_BOX = 15	// KQ6CD Windows version
};

reg_t kGraph(EngineState *s, int argc, reg_t *argv) {
	int16 x = 0, y = 0, x1 = 0, y1 = 0;
	uint16 screenMask;
	int16 priority, control, color, colorMask;
	Common::Rect rect;

	if (argc >= 5) {
		x = argv[2].toSint16();
		y = argv[1].toSint16();
		x1 = argv[4].toSint16();
		y1 = argv[3].toSint16();
	}

	switch (argv[0].toSint16()) {
	case K_GRAPH_GET_COLORS_NR:
		return make_reg(0, !s->resMan->isVGA() ? 0x10 : 0x100);
		break;

	case K_GRAPH_DRAW_LINE:
		priority = (argc > 6) ? argv[6].toSint16() : -1;
		control = (argc > 7) ? argv[7].toSint16() : -1;
		color = argv[5].toSint16();

		s->_gui->graphDrawLine(Common::Point(x, y), Common::Point(x1, y1), color, priority, control);
		break;

	case K_GRAPH_SAVE_BOX:
		kGraphCreateRect(x, y, x1, y1, &rect);
		screenMask = (argc > 5) ? argv[5].toUint16() : 0;
		return s->_gui->graphSaveBox(rect, screenMask);

	case K_GRAPH_RESTORE_BOX:
		// This may be called with a memoryhandle from SAVE_BOX or SAVE_UPSCALEDHIRES_BOX
		s->_gui->graphRestoreBox(argv[1]);
		break;

	case K_GRAPH_FILL_BOX_BACKGROUND:
		kGraphCreateRect(x, y, x1, y1, &rect);
		s->_gui->graphFillBoxBackground(rect);
		break;

	case K_GRAPH_FILL_BOX_FOREGROUND:
		kGraphCreateRect(x, y, x1, y1, &rect);
		s->_gui->graphFillBoxForeground(rect);
		break;

	case K_GRAPH_FILL_BOX_ANY:
		priority = (argc > 7) ? argv[7].toSint16() : -1;
		control = (argc > 8) ? argv[8].toSint16() : -1;
		color = argv[6].toSint16();
		colorMask = argv[5].toUint16();

		rect = Common::Rect(x, y, x1, y1);
		s->_gui->graphFillBox(rect, colorMask, color, priority, control);
		break;

	case K_GRAPH_UPDATE_BOX: {
		kGraphCreateRect(x, y, x1, y1, &rect);
		bool hiresMode = (argc > 6) ? true : false;
		// argc == 7 on upscaled hires
		s->_gui->graphUpdateBox(rect, hiresMode);
		break;
	}

	case K_GRAPH_REDRAW_BOX:
		kGraphCreateRect(x, y, x1, y1, &rect);
		s->_gui->graphRedrawBox(rect);
		break;

	case K_GRAPH_ADJUST_PRIORITY:
		// Seems to be only implemented for SCI0/SCI01 games
		debugC(2, kDebugLevelGraphics, "adjust_priority(%d, %d)\n", argv[1].toUint16(), argv[2].toUint16());
		s->_gui->graphAdjustPriority(argv[1].toUint16(), argv[2].toUint16());
		break;

	case K_GRAPH_SAVE_UPSCALEDHIRES_BOX:
		kGraphCreateRect(x, y, x1, y1, &rect);
		return s->_gui->graphSaveUpscaledHiresBox(rect);

	default:
		warning("Unsupported kGraph() operation %04x", argv[0].toSint16());
	}

	return s->r_acc;
}

reg_t kTextSize(EngineState *s, int argc, reg_t *argv) {
	int16 textWidth, textHeight;
	Common::String text = s->_segMan->getString(argv[1]);
	reg_t *dest = s->_segMan->derefRegPtr(argv[0], 4);
	int maxwidth = (argc > 3) ? argv[3].toUint16() : 0;
	int font_nr = argv[2].toUint16();

	Common::String sep_str;
	const char *sep = NULL;
	if ((argc > 4) && (argv[4].segment)) {
		sep_str = s->_segMan->getString(argv[4]);
		sep = sep_str.c_str();
	}

	dest[0] = dest[1] = NULL_REG;

	if (text.empty() || !dest) { // Empty text
		dest[2] = dest[3] = make_reg(0, 0);
		debugC(2, kDebugLevelStrings, "GetTextSize: Empty string\n");
		return s->r_acc;
	}

	textWidth = dest[3].toUint16(); textHeight = dest[2].toUint16();
	s->_gui->textSize(s->strSplit(text.c_str(), sep).c_str(), font_nr, maxwidth, &textWidth, &textHeight);
	debugC(2, kDebugLevelStrings, "GetTextSize '%s' -> %dx%d\n", text.c_str(), textWidth, textHeight);

	dest[2] = make_reg(0, textHeight);
	dest[3] = make_reg(0, textWidth);
	return s->r_acc;
}

reg_t kWait(EngineState *s, int argc, reg_t *argv) {
	int sleep_time = argv[0].toUint16();
#if 0
	uint32 time;

	time = g_system->getMillis();
	s->r_acc = make_reg(0, ((long)time - (long)s->last_wait_time) * 60 / 1000);
	s->last_wait_time = time;

	sleep_time *= g_debug_sleeptime_factor;
	gfxop_sleep(s->gfx_state, sleep_time * 1000 / 60);

#endif

	// FIXME: we should not be asking from the GUI to wait. The kernel sounds
	// like a better place
	s->_gui->wait(sleep_time);

	return s->r_acc;
}

reg_t kCoordPri(EngineState *s, int argc, reg_t *argv) {
	int16 y = argv[0].toSint16();

	if ((argc < 2) || (y != 1)) {
		return make_reg(0, s->_gui->coordinateToPriority(y));
	} else {
		int16 priority = argv[1].toSint16();
		return make_reg(0, s->_gui->priorityToCoordinate(priority));
	}
}

reg_t kPriCoord(EngineState *s, int argc, reg_t *argv) {
	int16 priority = argv[0].toSint16();

	return make_reg(0, s->_gui->priorityToCoordinate(priority));
}

reg_t kDirLoop(EngineState *s, int argc, reg_t *argv) {
	_k_dirloop(argv[0], argv[1].toUint16(), s, argc, argv);

	return s->r_acc;
}

reg_t kCanBeHere(EngineState *s, int argc, reg_t *argv) {
	reg_t curObject = argv[0];
	reg_t listReference = (argc > 1) ? argv[1] : NULL_REG;
	bool canBeHere = s->_gui->canBeHere(curObject, listReference);
	return make_reg(0, canBeHere);
}

// kCantBeHere does the same thing as kCanBeHere, except that it returns the opposite result.
reg_t kCantBeHere(EngineState *s, int argc, reg_t *argv) {
	reg_t curObject = argv[0];
	reg_t listReference = (argc > 1) ? argv[1] : NULL_REG;
	bool canBeHere = s->_gui->canBeHere(curObject, listReference);
	return make_reg(0, !canBeHere);
}

reg_t kIsItSkip(EngineState *s, int argc, reg_t *argv) {
	GuiResourceId viewId = argv[0].toSint16();
	int16 loopNo = argv[1].toSint16();
	int16 celNo = argv[2].toSint16();
	Common::Point position(argv[4].toUint16(), argv[3].toUint16());

	bool result = s->_gui->isItSkip(viewId, loopNo, celNo, position);
	return make_reg(0, result);
}

reg_t kCelHigh(EngineState *s, int argc, reg_t *argv) {
	GuiResourceId viewId = argv[0].toSint16();
	if (viewId == -1)	// Happens in SCI32
		return NULL_REG;
	int16 loopNo = argv[1].toSint16();
	int16 celNo = (argc >= 3) ? argv[2].toSint16() : 0;

	return make_reg(0, s->_gui->getCelHeight(viewId, loopNo, celNo));
}

reg_t kCelWide(EngineState *s, int argc, reg_t *argv) {
	GuiResourceId viewId = argv[0].toSint16();
	if (viewId == -1)	// Happens in SCI32
		return NULL_REG;
	int16 loopNo = argv[1].toSint16();
	int16 celNo = (argc >= 3) ? argv[2].toSint16() : 0;

	return make_reg(0, s->_gui->getCelWidth(viewId, loopNo, celNo));
}

reg_t kNumLoops(EngineState *s, int argc, reg_t *argv) {
	reg_t object = argv[0];
	GuiResourceId viewId = GET_SEL32V(s->_segMan, object, view);
	int16 loopCount = s->_gui->getLoopCount(viewId);

	debugC(2, kDebugLevelGraphics, "NumLoops(view.%d) = %d\n", viewId, loopCount);

	return make_reg(0, loopCount);
}

reg_t kNumCels(EngineState *s, int argc, reg_t *argv) {
	reg_t object = argv[0];
	GuiResourceId viewId = GET_SEL32V(s->_segMan, object, view);
	int16 loopNo = GET_SEL32V(s->_segMan, object, loop);
	int16 celCount = s->_gui->getCelCount(viewId, loopNo);

	debugC(2, kDebugLevelGraphics, "NumCels(view.%d, %d) = %d\n", viewId, loopNo, celCount);

	return make_reg(0, celCount);
}

reg_t kOnControl(EngineState *s, int argc, reg_t *argv) {
	Common::Rect rect;
	byte screenMask;
	int argBase = 0;

	if ((argc == 2) || (argc == 4)) {
		screenMask = SCI_SCREEN_MASK_CONTROL;
	} else {
		screenMask = argv[0].toUint16();
		argBase = 1;
	}
	rect.left = argv[argBase].toSint16();
	rect.top = argv[argBase + 1].toSint16();
	if (argc > 3) {
		rect.right = argv[argBase + 2].toSint16();
		rect.bottom = argv[argBase + 3].toSint16();
	} else {
		rect.right = rect.left + 1;
		rect.bottom = rect.top + 1;
	}
	uint16 result = s->_gui->onControl(screenMask, rect);
	return make_reg(0, result);
}

#define K_DRAWPIC_FLAGS_MIRRORED			(1 << 14)
#define K_DRAWPIC_FLAGS_ANIMATIONBLACKOUT	(1 << 15)

reg_t kDrawPic(EngineState *s, int argc, reg_t *argv) {
	GuiResourceId pictureId = argv[0].toUint16();
	uint16 flags = 0;
	int16 animationNr = -1;
	bool animationBlackoutFlag = false;
	bool mirroredFlag = false;
	bool addToFlag = false;
	int16 EGApaletteNo = 0; // default needs to be 0

	if (argc >= 2) {
		flags = argv[1].toUint16();
		if (flags & K_DRAWPIC_FLAGS_ANIMATIONBLACKOUT)
			animationBlackoutFlag = true;
		animationNr = flags & 0xFF;
		if (flags & K_DRAWPIC_FLAGS_MIRRORED)
			mirroredFlag = true;
	}
	if (argc >= 3) {
		if (!argv[2].isNull())
			addToFlag = true;
		if (!s->usesOldGfxFunctions())
			addToFlag = !addToFlag;
	}
	if (argc >= 4)
		EGApaletteNo = argv[3].toUint16();

	s->_gui->drawPicture(pictureId, animationNr, animationBlackoutFlag, mirroredFlag, addToFlag, EGApaletteNo);

	return s->r_acc;
}

reg_t kBaseSetter(EngineState *s, int argc, reg_t *argv) {
	reg_t object = argv[0];

	s->_gui->baseSetter(object);

	// WORKAROUND for a problem in LSL1VGA. This allows the casino door to be opened,
	// till the actual problem is found
	if (s->_gameId == "lsl1sci" && s->currentRoomNumber() == 300) {
		int top = GET_SEL32V(s->_segMan, object, brTop);
		PUT_SEL32V(s->_segMan, object, brTop, top + 2);
	}

	return s->r_acc;
}

reg_t kSetNowSeen(EngineState *s, int argc, reg_t *argv) {
	s->_gui->setNowSeen(argv[0]);
	return s->r_acc;
}

reg_t kPalette(EngineState *s, int argc, reg_t *argv) {
	switch (argv[0].toUint16()) {
	case 1: // Set resource palette
		if (argc==3) {
			GuiResourceId resourceId = argv[1].toUint16();
			uint16 flags = argv[2].toUint16();
			s->_gui->paletteSet(resourceId, flags);
		}
		break;
	case 2: { // Set palette-flag(s)
		uint16 fromColor = CLIP<uint16>(argv[1].toUint16(), 1, 255);
		uint16 toColor = CLIP<uint16>(argv[2].toUint16(), 1, 255);
		uint16 flags = argv[3].toUint16();
		s->_gui->paletteSetFlag(fromColor, toColor, flags);
		break;
	}
	case 3:	{ // Remove palette-flag(s)
		uint16 fromColor = CLIP<uint16>(argv[1].toUint16(), 1, 255);
		uint16 toColor = CLIP<uint16>(argv[2].toUint16(), 1, 255);
		uint16 flags = argv[3].toUint16();
		s->_gui->paletteUnsetFlag(fromColor, toColor, flags);
		break;
	}
	case 4:	{ // Set palette intensity
		switch (argc) {
		case 4:
		case 5: {
			uint16 fromColor = CLIP<uint16>(argv[1].toUint16(), 1, 255);
			uint16 toColor = CLIP<uint16>(argv[2].toUint16(), 1, 255);
			uint16 intensity = argv[3].toUint16();
			bool setPalette = (argc < 5) ? true : (argv[4].isNull()) ? true : false;

			s->_gui->paletteSetIntensity(fromColor, toColor, intensity, setPalette);
			break;
		}
		default:
			warning("kPalette(4) called with %d parameters", argc);
		}
		break;
	}
	case 5: { // Find closest color
		uint16 r = argv[1].toUint16();
		uint16 g = argv[2].toUint16();
		uint16 b = argv[3].toUint16();

		return make_reg(0, s->_gui->paletteFind(r, g, b));
	}
	case 6: { // Animate
		int16 argNr;
		bool paletteChanged = false;
		for (argNr = 1; argNr < argc; argNr += 3) {
			uint16 fromColor = argv[argNr].toUint16();
			uint16 toColor = argv[argNr + 1].toUint16();
			int16 speed = argv[argNr + 2].toSint16();
			if (s->_gui->paletteAnimate(fromColor, toColor, speed))
				paletteChanged = true;
		}
		if (paletteChanged)
			s->_gui->paletteAnimateSet();
		break;
	}
	case 7: { // Save palette to heap
		warning("kPalette(7), save palette to heap STUB");
		break;
	}
	case 8: { // Restore palette from heap
		warning("kPalette(8), set stored palette STUB");
		break;
	}
	default:
		warning("kPalette(%d), not implemented", argv[0].toUint16());
	}

	return s->r_acc;
}

// This here is needed to make Pharkas work
reg_t kPalVary(EngineState *s, int argc, reg_t *argv) {
	uint16 operation = argv[0].toUint16();

	switch (operation) {
	case 0: { // Init
		GuiResourceId paletteId;
		uint16 time;
		if (argc == 3) {
			paletteId = argv[1].toUint16();
			time = argv[2].toUint16();
			// forward call to SciGui
		} else {
			warning("kPalVary(init) called with unsupported argc %d", argc);
		}
		break;
	}
	case 3: { // DeInit
		if (argc == 1) {
			// forward call to SciGui
		} else {
			warning("kPalVary(deinit) called with unsupported argc %d", argc);
		}
		break;
	}
	case 6: { // Pause
		bool pauseState;
		if (argc == 2) {
			pauseState = argv[1].isNull() ? false : true;
			// this call is actually counting states, so calling this 3 times with true will require calling it later
			//  3 times with false to actually remove pause
			// forward call to SciGui
		} else {
			warning("kPalVary(pause) called with unsupported argc %d", argc);
		}
		break;
	}
	default:
		warning("kPalVary(%d), not implemented (argc = %d)", operation, argc);
	}
	return NULL_REG;
}

reg_t kAssertPalette(EngineState *s, int argc, reg_t *argv) {
	GuiResourceId viewId = argv[1].toUint16();
	// TODO: implement this

	warning("kAssertPalette() called with viewId = %d", viewId);
	return s->r_acc;
}

// Used to show hires character portraits in the Windows CD version of KQ6
reg_t kPortrait(EngineState *s, int argc, reg_t *argv) {
	uint16 operation = argv[0].toUint16();

	switch (operation) {
	case 0: { // load
		if (argc == 2) {
			Common::String resourceName = s->_segMan->getString(argv[1]);
			return s->_gui->portraitLoad(resourceName);
		} else {
			warning("kPortrait(loadResource) called with unsupported argc %d", argc);
		}
		break;
	}
	case 1: { // show
		if (argc == 10) {
			Common::String resourceName = s->_segMan->getString(argv[1]);
			Common::Point position = Common::Point(argv[2].toUint16(), argv[3].toUint16());
			uint resourceNum = argv[4].toUint16();
			uint noun = argv[5].toUint16() & 0xff;
			uint verb = argv[6].toUint16() & 0xff;
			uint cond = argv[7].toUint16() & 0xff;
			uint seq = argv[8].toUint16() & 0xff;
			// argv[9] is usually 0??!!

			s->_gui->portraitShow(resourceName, position, resourceNum, noun, verb, cond, seq);
			return SIGNAL_REG;
		} else {
			warning("kPortrait(show) called with unsupported argc %d", argc);
		}
		break;
	}
	case 2: { // unload
		if (argc == 2) {
			uint16 portraitId = argv[1].toUint16();
			s->_gui->portraitUnload(portraitId);
		} else {
			warning("kPortrait(unload) called with unsupported argc %d", argc);
		}
		break;
	}
	default:
		warning("kPortrait(%d), not implemented (argc = %d)", operation, argc);
	}

	return s->r_acc;
}

void _k_GenericDrawControl(EngineState *s, reg_t controlObject, bool hilite) {
	int16 type = GET_SEL32V(s->_segMan, controlObject, type);
	int16 style = GET_SEL32V(s->_segMan, controlObject, state);
	int16 x = GET_SEL32V(s->_segMan, controlObject, nsLeft);
	int16 y = GET_SEL32V(s->_segMan, controlObject, nsTop);
	GuiResourceId fontId = GET_SEL32V(s->_segMan, controlObject, font);
	reg_t textReference = GET_SEL32(s->_segMan, controlObject, text);
	Common::String text;
	Common::Rect rect;
	TextAlignment alignment;
	int16 mode, maxChars, cursorPos, upperPos, listCount, i;
	int16 upperOffset, cursorOffset;
	GuiResourceId viewId;
	int16 loopNo;
	int16 celNo;
	reg_t listSeeker;
	Common::String *listStrings = NULL;
	const char **listEntries = NULL;
	bool isAlias = false;

	kGraphCreateRect(x, y, GET_SEL32V(s->_segMan, controlObject, nsRight), GET_SEL32V(s->_segMan, controlObject, nsBottom), &rect);

	if (!textReference.isNull())
		text = s->_segMan->getString(textReference);

	switch (type) {
	case SCI_CONTROLS_TYPE_BUTTON:
		debugC(2, kDebugLevelGraphics, "drawing button %04x:%04x to %d,%d\n", PRINT_REG(controlObject), x, y);
		s->_gui->drawControlButton(rect, controlObject, s->strSplit(text.c_str(), NULL).c_str(), fontId, style, hilite);
		return;

	case SCI_CONTROLS_TYPE_TEXT:
		alignment = GET_SEL32V(s->_segMan, controlObject, mode);
		debugC(2, kDebugLevelGraphics, "drawing text %04x:%04x ('%s') to %d,%d, mode=%d\n", PRINT_REG(controlObject), text.c_str(), x, y, alignment);
		s->_gui->drawControlText(rect, controlObject, s->strSplit(text.c_str()).c_str(), fontId, alignment, style, hilite);
		return;

	case SCI_CONTROLS_TYPE_TEXTEDIT:
		mode = GET_SEL32V(s->_segMan, controlObject, mode);
		maxChars = GET_SEL32V(s->_segMan, controlObject, max);
		cursorPos = GET_SEL32V(s->_segMan, controlObject, cursor);
		debugC(2, kDebugLevelGraphics, "drawing edit control %04x:%04x (text %04x:%04x, '%s') to %d,%d\n", PRINT_REG(controlObject), PRINT_REG(textReference), text.c_str(), x, y);
		s->_gui->drawControlTextEdit(rect, controlObject, s->strSplit(text.c_str(), NULL).c_str(), fontId, mode, style, cursorPos, maxChars, hilite);
		return;

	case SCI_CONTROLS_TYPE_ICON:
		viewId = GET_SEL32V(s->_segMan, controlObject, view);
		{
			int l = GET_SEL32V(s->_segMan, controlObject, loop);
			loopNo = (l & 0x80) ? l - 256 : l;
			int c = GET_SEL32V(s->_segMan, controlObject, cel);
			celNo = (c & 0x80) ? c - 256 : c;
		}
		debugC(2, kDebugLevelGraphics, "drawing icon control %04x:%04x to %d,%d\n", PRINT_REG(controlObject), x, y - 1);
		s->_gui->drawControlIcon(rect, controlObject, viewId, loopNo, celNo, style, hilite);
		return;

	case SCI_CONTROLS_TYPE_LIST:
	case SCI_CONTROLS_TYPE_LIST_ALIAS:
		if (type == SCI_CONTROLS_TYPE_LIST_ALIAS)
			isAlias = true;

		maxChars = GET_SEL32V(s->_segMan, controlObject, x); // max chars per entry
		// NOTE: most types of pointer dereferencing don't like odd offsets
		if (maxChars & 1) {
			warning("List control with odd maxChars %d. This is not yet implemented for all types of segments", maxChars);
		}
		cursorOffset = GET_SEL32V(s->_segMan, controlObject, cursor);
		if (s->_kernel->_selectorCache.topString != -1) {
			// Games from early SCI1 onwards use topString
			upperOffset = GET_SEL32V(s->_segMan, controlObject, topString);
		} else {
			// Earlier games use lsTop
			upperOffset = GET_SEL32V(s->_segMan, controlObject, lsTop);
		}

		// Count string entries in NULL terminated string list
		listCount = 0; listSeeker = textReference;
		while (s->_segMan->strlen(listSeeker) > 0) {
			listCount++;
			listSeeker.offset += maxChars;
		}

		// TODO: This is rather convoluted... It would be a lot cleaner
		// if sciw_new_list_control would take a list of Common::String
		cursorPos = 0; upperPos = 0;
		if (listCount) {
			// We create a pointer-list to the different strings, we also find out whats upper and cursor position
			listSeeker = textReference;
			listEntries = (const char**)malloc(sizeof(char *) * listCount);
			listStrings = new Common::String[listCount];
			for (i = 0; i < listCount; i++) {
				listStrings[i] = s->_segMan->getString(listSeeker);
				listEntries[i] = listStrings[i].c_str();
				if (listSeeker.offset == upperOffset)
					upperPos = i;
				if (listSeeker.offset == cursorOffset)
					cursorPos = i;
				listSeeker.offset += maxChars;
			}
		}

		debugC(2, kDebugLevelGraphics, "drawing list control %04x:%04x to %d,%d, diff %d\n", PRINT_REG(controlObject), x, y, SCI_MAX_SAVENAME_LENGTH);
		s->_gui->drawControlList(rect, controlObject, maxChars, listCount, listEntries, fontId, style, upperPos, cursorPos, isAlias, hilite);
		free(listEntries);
		delete[] listStrings;
		return;

	case SCI_CONTROLS_TYPE_DUMMY:
		// Actually this here does nothing at all, its required by at least QfG1/EGA that we accept this type
		return;

	default:
		error("unsupported control type %d", type);
	}
}

reg_t kDrawControl(EngineState *s, int argc, reg_t *argv) {
	reg_t controlObject = argv[0];
	Common::String objName = s->_segMan->getObjectName(controlObject);

	// Disable the "Change Directory" button, as we don't allow the game engine to
	// change the directory where saved games are placed
	if (objName == "changeDirI") {
		int state = GET_SEL32V(s->_segMan, controlObject, state);
		PUT_SEL32V(s->_segMan, controlObject, state, (state | kControlStateDisabled) & ~kControlStateEnabled);
	}

	_k_GenericDrawControl(s, controlObject, false);
	return NULL_REG;
}

reg_t kHiliteControl(EngineState *s, int argc, reg_t *argv) {
	reg_t controlObject = argv[0];

	_k_GenericDrawControl(s, controlObject, true);
	return s->r_acc;
}

reg_t kEditControl(EngineState *s, int argc, reg_t *argv) {
	reg_t controlObject = argv[0];
	reg_t eventObject = argv[1];

	if (!controlObject.isNull())
		s->_gui->editControl(controlObject, eventObject);
	return s->r_acc;
}

reg_t kAddToPic(EngineState *s, int argc, reg_t *argv) {
	GuiResourceId viewId;
	int16 loopNo;
	int16 celNo;
	int16 leftPos, topPos, priority, control;

	switch (argc) {
	// Is this ever really gets called with 0 parameters, we need to set _picNotValid!!
	//case 0:
	//	break;
	case 1:
		if (argv[0].isNull())
			return s->r_acc;
		s->_gui->addToPicList(argv[0], argc, argv);
		break;
	case 7:
		viewId = argv[0].toUint16();
		loopNo = argv[1].toSint16();
		celNo = argv[2].toSint16();
		leftPos = argv[3].toSint16();
		topPos = argv[4].toSint16();
		priority = argv[5].toSint16();
		control = argv[6].toSint16();
		s->_gui->addToPicView(viewId, loopNo, celNo, leftPos, topPos, priority, control);
		break;
	default:
		error("kAddToPic with unsupported parameter count %d", argc);
	}
	return s->r_acc;
}

reg_t kGetPort(EngineState *s, int argc, reg_t *argv) {
	return s->_gui->getPort();
}

reg_t kSetPort(EngineState *s, int argc, reg_t *argv) {
	uint16 portPtr;
	Common::Rect picRect;
	int16 picTop, picLeft;
	bool initPriorityBandsFlag = false;

	switch (argc) {
	case 1:
		portPtr = argv[0].toSint16();
		s->_gui->setPort(portPtr);
		break;

	case 7:
		initPriorityBandsFlag = true;
	case 4:
	case 6:
		picRect.top = argv[0].toSint16();
		picRect.left = argv[1].toSint16();
		picRect.bottom = argv[2].toSint16();
		picRect.right = argv[3].toSint16();
		picTop = (argc >= 6) ? argv[4].toSint16() : 0;
		picLeft = (argc >= 6) ? argv[5].toSint16() : 0;
		s->_gui->setPortPic(picRect, picTop, picLeft, initPriorityBandsFlag);
		break;

	default:
		error("SetPort was called with %d parameters", argc);
		break;
	}
	return NULL_REG;
}

reg_t kDrawCel(EngineState *s, int argc, reg_t *argv) {
	GuiResourceId viewId = argv[0].toSint16();
	int16 loopNo = argv[1].toSint16();
	int16 celNo = argv[2].toSint16();
	uint16 x = argv[3].toUint16();
	uint16 y = argv[4].toUint16();
	int16 priority = (argc > 5) ? argv[5].toSint16()  : -1;
	uint16 paletteNo = (argc > 6) ? argv[6].toUint16() : 0;
	bool hiresMode = (argc > 7) ? true : false;
	reg_t upscaledHiresHandle = (argc > 7) ? argv[7] : NULL_REG;

	s->_gui->drawCel(viewId, loopNo, celNo, x, y, priority, paletteNo, hiresMode, upscaledHiresHandle);

	return s->r_acc;
}

reg_t kDisposeWindow(EngineState *s, int argc, reg_t *argv) {
	int goner_nr = argv[0].toSint16();
	bool reanimate = true;
	if ((argc == 2) && (argv[1].isNull()))
		reanimate = false;

	s->_gui->disposeWindow(goner_nr, reanimate);
	return s->r_acc;
}

reg_t kNewWindow(EngineState *s, int argc, reg_t *argv) {
	Common::Rect rect1 (argv[1].toSint16(), argv[0].toSint16(), argv[3].toSint16(), argv[2].toSint16());
	Common::Rect rect2;
	int argextra = argc == 13 ? 4 : 0; // Triggers in PQ3 and SCI1.1 games
	int	style = argv[5 + argextra].toSint16();
	int	priority = (argc > 6 + argextra) ? argv[6 + argextra].toSint16() : -1;
	int colorPen = (argc > 7 + argextra) ? argv[7 + argextra].toSint16() : 0;
	int colorBack = (argc > 8 + argextra) ? argv[8 + argextra].toSint16() : 255;

	//	const char *title = argv[4 + argextra].segment ? kernel_dereference_char_pointer(s, argv[4 + argextra], 0) : NULL;
	if (argc==13) {
		rect2 = Common::Rect (argv[5].toSint16(), argv[4].toSint16(), argv[7].toSint16(), argv[6].toSint16());
	}

	Common::String title;
	if (argv[4 + argextra].segment) {
		title = s->_segMan->getString(argv[4 + argextra]);
		title = s->strSplit(title.c_str(), NULL);
	}

	return s->_gui->newWindow(rect1, rect2, style, priority, colorPen, colorBack, title.c_str());
}

reg_t kAnimate(EngineState *s, int argc, reg_t *argv) {
	reg_t castListReference = (argc > 0) ? argv[0] : NULL_REG;
	bool cycle = (argc > 1) ? ((argv[1].toUint16()) ? true : false) : false;

#ifdef USE_OLD_MUSIC_FUNCTIONS
	// Take care of incoming events (kAnimate is called semi-regularly)
	process_sound_events(s);
#endif
	s->_gui->animate(castListReference, cycle, argc, argv);

	return s->r_acc;
}

reg_t kShakeScreen(EngineState *s, int argc, reg_t *argv) {
	int16 shakeCount = (argc > 0) ? argv[0].toUint16() : 1;
	int16 directions = (argc > 1) ? argv[1].toUint16() : 1;

	s->_gui->shakeScreen(shakeCount, directions);
	return s->r_acc;
}

reg_t kDisplay(EngineState *s, int argc, reg_t *argv) {
	reg_t textp = argv[0];
	int index = (argc > 1) ? argv[1].toUint16() : 0;

	Common::String text;

	if (textp.segment) {
		argc--; argv++;
		text = s->_segMan->getString(textp);
	} else {
		argc--; argc--; argv++; argv++;
		text = kernel_lookup_text(s, textp, index);
	}

	s->_gui->display(s->strSplit(text.c_str()).c_str(), argc, argv);
	return s->r_acc;
}

reg_t kShowMovie(EngineState *s, int argc, reg_t *argv) {
	bool playedVideo = false;

	// KQ6 Windows calls this with one argument. It doesn't seem
	// to have a purpose...
	if (argc == 1)
		return NULL_REG;

	s->_gui->hideCursor();

	// The Windows and DOS versions use different video format as well
	// as a different argument set.
	if (argv[0].toUint16() == 0) {
		// Windows

		Common::String filename = s->_segMan->getString(argv[1]);

		Graphics::AviDecoder *aviDecoder = new Graphics::AviDecoder(g_system->getMixer());
		Graphics::VideoPlayer *player = new Graphics::VideoPlayer(aviDecoder);
		if (aviDecoder->loadFile(filename.c_str())) {
			player->playVideo();
			playedVideo = true;
		} else {
			warning("Failed to open movie file %s", filename.c_str());
		}
		aviDecoder->closeFile();
		delete player;
		delete aviDecoder;
	} else {
		// DOS

		Common::String filename = s->_segMan->getString(argv[0]);
		int delay = argv[1].toUint16(); // Time between frames in ticks

		SeqDecoder *seqDecoder = new SeqDecoder();
		Graphics::VideoPlayer *player = new Graphics::VideoPlayer(seqDecoder);
		if (seqDecoder->loadFile(filename.c_str(), delay)) {
			player->playVideo();
			playedVideo = true;
		} else {
			warning("Failed to open movie file %s", filename.c_str());
		}
		seqDecoder->closeFile();
		delete player;
		delete seqDecoder;
	}

	if (playedVideo)
		s->_gui->syncWithFramebuffer();

	s->_gui->showCursor();

	return s->r_acc;
}

reg_t kSetVideoMode(EngineState *s, int argc, reg_t *argv) {
	// This call is used for KQ6's intro. It has one parameter, which is
	// 1 when the intro begins, and 0 when it ends. It is suspected that
	// this is actually a flag to enable video planar memory access, as
	// the video decoder in KQ6 is specifically written for the planar
	// memory model. Planar memory mode access was used for VGA "Mode X"
	// (320x240 resolution, although the intro in KQ6 is 320x200).
	// Refer to http://en.wikipedia.org/wiki/Mode_X

	//warning("STUB: SetVideoMode %d", argv[0].toUint16());
	return s->r_acc;
}

// New calls for SCI11. Using those is only needed when using text-codes so that one is able to change
//  font and/or color multiple times during kDisplay and kDrawControl
reg_t kTextFonts(EngineState *s, int argc, reg_t *argv) {
	s->_gui->textFonts(argc, argv);
	return s->r_acc;
}

reg_t kTextColors(EngineState *s, int argc, reg_t *argv) {
	s->_gui->textColors(argc, argv);
	return s->r_acc;
}

} // End of namespace Sci
