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

 // Console module header file

#ifndef SCI_CONSOLE_H
#define SCI_CONSOLE_H

#include "gui/debugger.h"

namespace Sci {

class SciEngine;

// Refer to the "addresses" command on how to pass address parameters
int parse_reg_t(EngineState *s, const char *str, reg_t *dest);
int printObject(EngineState *s, reg_t pos);

class Console : public GUI::Debugger {
public:
	Console(SciEngine *vm);
	virtual ~Console();

private:
	bool cmdGetVersion(int argc, const char **argv);
//	bool cmdClasses(int argc, const char **argv);	// TODO
	bool cmdOpcodes(int argc, const char **argv);
	bool cmdSelectors(int argc, const char **argv);
	bool cmdKernelNames(int argc, const char **argv);
	bool cmdSuffixes(int argc, const char **argv);
	bool cmdHexDump(int argc, const char **argv);
	bool cmdResourceId(int argc, const char **argv);
	bool cmdDissectScript(int argc, const char **argv);
	bool cmdWeakValidations(int argc, const char **argv);
	bool cmdRoomNumber(int argc, const char **argv);
	bool cmdResourceSize(int argc, const char **argv);
	bool cmdResourceTypes(int argc, const char **argv);
	bool cmdSci0Palette(int argc, const char **argv);
	bool cmdHexgrep(int argc, const char **argv);
	bool cmdList(int argc, const char **argv);
	bool cmdClearScreen(int argc, const char **argv);
	bool cmdRedrawScreen(int argc, const char **argv);
	bool cmdSaveGame(int argc, const char **argv);
	bool cmdRestoreGame(int argc, const char **argv);
	bool cmdRestartGame(int argc, const char **argv);
	bool cmdClassTable(int argc, const char **argv);
	bool cmdSentenceFragments(int argc, const char **argv);
	bool cmdParserNodes(int argc, const char **argv);
	bool cmdParserWords(int argc, const char **argv);
	bool cmdRegisters(int argc, const char **argv);
	bool cmdDrawPic(int argc, const char **argv);
	bool cmdDrawRect(int argc, const char **argv);
	bool cmdDrawCel(int argc, const char **argv);
	bool cmdViewInfo(int argc, const char **argv);
	bool cmdUpdateZone(int argc, const char **argv);
	bool cmdPropagateZone(int argc, const char **argv);
	bool cmdFillScreen(int argc, const char **argv);
	bool cmdCurrentPort(int argc, const char **argv);
	bool cmdPrintPort(int argc, const char **argv);
	bool cmdParseGrammar(int argc, const char **argv);
	bool cmdVisualState(int argc, const char **argv);
	bool cmdFlushPorts(int argc, const char **argv);
	bool cmdDynamicViews(int argc, const char **argv);
	bool cmdDroppedViews(int argc, const char **argv);
	bool cmdPriorityBands(int argc, const char **argv);
	bool cmdStatusBarColors(int argc, const char **argv);
	bool cmdSimulateKey(int argc, const char **argv);
	bool cmdTrackMouse(int argc, const char **argv);
	bool cmdPrintSegmentTable(int argc, const char **argv);
	bool cmdSegmentInfo(int argc, const char **argv);
	bool cmdKillSegment(int argc, const char **argv);
	bool cmdShowMap(int argc, const char **argv);
	bool cmdSongLib(int argc, const char **argv);
	bool cmdGCInvoke(int argc, const char **argv);
	bool cmdGCObjects(int argc, const char **argv);
	bool cmdGCInterval(int argc, const char **argv);
	bool cmdGCShowReachable(int argc, const char **argv);
	bool cmdGCShowFreeable(int argc, const char **argv);
	bool cmdGCNormalize(int argc, const char **argv);
	bool cmdVMVarlist(int argc, const char **argv);
	bool cmdStack(int argc, const char **argv);
	bool cmdViewListNode(int argc, const char **argv);
	bool cmdValueType(int argc, const char **argv);
	bool cmdViewObject(int argc, const char **argv);
	bool cmdViewActiveObject(int argc, const char **argv);
	bool cmdViewAccumulatorObject(int argc, const char **argv);
	bool cmdSleepFactor(int argc, const char **argv);
	bool cmdIsSample(int argc, const char **argv);
	bool cmdSfx01Header(int argc, const char **argv);
	bool cmdSfx01Track(int argc, const char **argv);
	bool cmdScriptSteps(int argc, const char **argv);
	bool cmdSetAccumulator(int argc, const char **argv);
	bool cmdBreakpointList(int argc, const char **argv);
	bool cmdBreakpointDelete(int argc, const char **argv);
	bool cmdExit(int argc, const char **argv);
	bool cmdAddresses(int argc, const char **argv);
	bool cmdStopSfx(int argc, const char **argv);

	bool segmentInfo(int nr);
	void printList(List *l);
	int printNode(reg_t addr);

private:
	SciEngine *_vm;
};

} // End of namespace Sci

#endif
