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

#include "sci/sci.h"
#include "sci/engine/script.h"
#include "sci/engine/state.h"

#include "common/util.h"

namespace Sci {

#define PATCH_END             0xFFFF
#define PATCH_COMMANDMASK     0xF000
#define PATCH_VALUEMASK       0x0FFF
#define PATCH_ADDTOOFFSET     0xE000
#define PATCH_GETORIGINALBYTE 0xD000
#define PATCH_ADJUSTWORD      0xC000
#define PATCH_ADJUSTWORD_NEG  0xB000
#define PATCH_MAGICDWORD(a, b, c, d) CONSTANT_LE_32(a | (b << 8) | (c << 16) | (d << 24))
#define PATCH_VALUELIMIT      4096

struct SciScriptSignature {
	uint16 scriptNr;
	const char *description;
	int16 applyCount;
	uint32 magicDWord;
	int magicOffset;
	const byte *data;
	const uint16 *patch;
};

#define SCI_SIGNATUREENTRY_TERMINATOR { 0, NULL, 0, 0, 0, NULL, NULL }

// signatures are built like this:
//  - first a counter of the bytes that follow
//  - then the actual bytes that need to get matched
//  - then another counter of bytes (0 for EOS)
//  - if not EOS, an adjust offset and the actual bytes
//  - rinse and repeat

// ===========================================================================
// Castle of Dr. Brain
// cipher::init (script 391) is called on room 380 init. This resets the word
//  cipher puzzle. The puzzle sadly operates on some hep strings, which aren't
//  saved in our sci. So saving/restoring in this room will break the puzzle
//  Because of this issue, we just init the puzzle each time it's accessed.
//  this is not 100% sierra behaviour, in fact we will actually reset the puzzle
//  during each access which makes it impossible to cheat.
const byte castlebrainSignatureCipherPuzzle[] = {
	22,
	0x35, 0x00,        // ldi 00
	0xa3, 0x26,        // sal local[26]
	0xa3, 0x25,        // sal local[25]
	0x35, 0x00,        // ldi 00
	0xa3, 0x2a,        // sal local[2a] (local is not used)
	0xa3, 0x29,        // sal local[29] (local is not used)
	0x35, 0xff,        // ldi ff
	0xa3, 0x2c,        // sal local[2c]
	0xa3, 0x2b,        // sal local[2b]
	0x35, 0x00,        // ldi 00
	0x65, 0x16,        // aTop highlightedIcon
	0
};

const uint16 castlebrainPatchCipherPuzzle[] = {
	0x39, 0x6b,        // pushi 6b (selector init)
	0x76,              // push0
	0x55, 0x04,        // self 04
	0x35, 0x00,        // ldi 00
	0xa3, 0x25,        // sal local[25]
	0xa3, 0x26,        // sal local[26]
	0xa3, 0x29,        // sal local[29]
	0x65, 0x16,        // aTop highlightedIcon
	0x34, 0xff, 0xff,  // ldi ffff
	0xa3, 0x2b,        // sal local[2b]
	0xa3, 0x2c,        // sal local[2c]
	PATCH_END
};

//    script, description,                                      magic DWORD,                             adjust
const SciScriptSignature castlebrainSignatures[] = {
	{    391, "cipher puzzle save/restore break",            1, PATCH_MAGICDWORD(0xa3, 0x26, 0xa3, 0x25),    -2, castlebrainSignatureCipherPuzzle, castlebrainPatchCipherPuzzle },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// stayAndHelp::changeState (0) is called when ego swims to the left or right
//  boundaries of room 660. Normally a textbox is supposed to get on screen
//  but the call is wrong, so not only do we get an error message the script
//  is also hanging because the cue won't get sent out
//  This also happens in sierra sci - ffs. bug #3038387
const byte ecoquest1SignatureStayAndHelp[] = {
	40,
	0x3f, 0x01,        // link 01
	0x87, 0x01,        // lap param[1]
	0x65, 0x14,        // aTop state
	0x36,              // push
	0x3c,              // dup
	0x35, 0x00,        // ldi 00
	0x1a,              // eq?
	0x31, 0x1c,        // bnt [next state]
	0x76,              // push0
	0x45, 0x01, 0x00,  // callb export1 from script 0 (switching control off)
	0x38, 0x22, 0x01,  // pushi 0122
	0x78,              // push1
	0x76,              // push0
	0x81, 0x00,        // lag global[0]
	0x4a, 0x06,        // send 06 - call ego::setMotion(0)
	0x39, 0x6e,        // pushi 6e (selector init)
	0x39, 0x04,        // pushi 04
	0x76,              // push0
	0x76,              // push0
	0x39, 0x17,        // pushi 17
	0x7c,              // pushSelf
	0x51, 0x82,        // class EcoNarrator
	0x4a, 0x0c,        // send 0c - call EcoNarrator::init(0, 0, 23, self) (BADLY BROKEN!)
	0x33,              // jmp [end]
	0
};

const uint16 ecoquest1PatchStayAndHelp[] = {
	0x87, 0x01,        // lap param[1]
	0x65, 0x14,        // aTop state
	0x36,              // push
	0x2f, 0x22,        // bt [next state] (this optimization saves 6 bytes)
	0x39, 0x00,        // pushi 0 (wasting 1 byte here)
	0x45, 0x01, 0x00,  // callb export1 from script 0 (switching control off)
	0x38, 0x22, 0x01,  // pushi 0122
	0x78,              // push1
	0x76,              // push0
	0x81, 0x00,        // lag global[0]
	0x4a, 0x06,        // send 06 - call ego::setMotion(0)
	0x39, 0x6e,        // pushi 6e (selector init)
	0x39, 0x06,        // pushi 06
	0x39, 0x02,        // pushi 02 (additional 2 bytes)
	0x76,              // push0
	0x76,              // push0
	0x39, 0x17,        // pushi 17
	0x7c,              // pushSelf
	0x38, 0x80, 0x02,  // pushi 280 (additional 3 bytes)
	0x51, 0x82,        // class EcoNarrator
	0x4a, 0x10,        // send 10 - call EcoNarrator::init(2, 0, 0, 23, self, 640)
	PATCH_END
};

//    script, description,                                      magic DWORD,                                 adjust
const SciScriptSignature ecoquest1Signatures[] = {
	{    660, "CD: bad messagebox and freeze",               1, PATCH_MAGICDWORD(0x38, 0x22, 0x01, 0x78),   -17, ecoquest1SignatureStayAndHelp, ecoquest1PatchStayAndHelp },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// doMyThing::changeState (2) is supposed to remove the initial text on the
//  ecorder. This is done by reusing temp-space, that was filled on state 1.
//  this worked in sierra sci just by accident. In our sci, the temp space
//  is resetted every time, which means the previous text isn't available
//  anymore. We have to patch the code because of that ffs. bug #3035386
const byte ecoquest2SignatureEcorder[] = {
	35,
	0x31, 0x22,        // bnt [next state]
	0x39, 0x0a,        // pushi 0a
	0x5b, 0x04, 0x1e,  // lea temp[1e]
	0x36,              // push
	0x39, 0x64,        // pushi 64
	0x39, 0x7d,        // pushi 7d
	0x39, 0x32,        // pushi 32
	0x39, 0x66,        // pushi 66
	0x39, 0x17,        // pushi 17
	0x39, 0x69,        // pushi 69
	0x38, 0x31, 0x26,  // pushi 2631
	0x39, 0x6a,        // pushi 6a
	0x39, 0x64,        // pushi 64
	0x43, 0x1b, 0x14,  // call kDisplay
	0x35, 0x0a,        // ldi 0a
	0x65, 0x20,        // aTop ticks
	0x33,              // jmp [end]
	+1, 5,             // [skip 1 byte]
	0x3c,              // dup
	0x35, 0x03,        // ldi 03
	0x1a,              // eq?
	0x31,              // bnt [end]
	0
};

const uint16 ecoquest2PatchEcorder[] = {
	0x2f, 0x02,        // bt [to pushi 07]
	0x3a,              // toss
	0x48,              // ret
	0x38, 0x07, 0x00,  // pushi 07 (parameter count) (waste 1 byte)
	0x39, 0x0b,        // push (FillBoxAny)
	0x39, 0x1d,        // pushi 29d
	0x39, 0x73,        // pushi 115d
	0x39, 0x5e,        // pushi 94d
	0x38, 0xd7, 0x00,  // pushi 215d
	0x78,              // push1 (visual screen)
	0x38, 0x17, 0x00,  // pushi 17 (color) (waste 1 byte)
	0x43, 0x6c, 0x0e,  // call kGraph
	0x38, 0x05, 0x00,  // pushi 05 (parameter count) (waste 1 byte)
	0x39, 0x0c,        // pushi 12d (UpdateBox)
	0x39, 0x1d,        // pushi 29d
	0x39, 0x73,        // pushi 115d
	0x39, 0x5e,        // pushi 94d
	0x38, 0xd7, 0x00,  // pushi 215d
	0x43, 0x6c, 0x0a,  // call kGraph
	PATCH_END
};

//    script, description,                                      magic DWORD,                                 adjust
const SciScriptSignature ecoquest2Signatures[] = {
	{     50, "initial text not removed on ecorder",         1, PATCH_MAGICDWORD(0x39, 0x64, 0x39, 0x7d),    -8, ecoquest2SignatureEcorder, ecoquest2PatchEcorder },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
//  script 0 of freddy pharkas/CD PointsSound::check waits for a signal and if
//   no signal received will call kDoSound(0xD) which is a dummy in sierra sci
//   and ScummVM and will use acc (which is not set by the dummy) to trigger
//   sound disposal. This somewhat worked in sierra sci, because the sample
//   was already playing in the sound driver. In our case we would also stop
//   the sample from playing, so we patch it out
//   The "score" code is already buggy and sets volume to 0 when playing
const byte freddypharkasSignatureScoreDisposal[] = {
	10,
	0x67, 0x32,       // pTos 32 (selector theAudCount)
	0x78,             // push1
	0x39, 0x0d,       // pushi 0d
	0x43, 0x75, 0x02, // call kDoAudio
	0x1c,             // ne?
	0x31,             // bnt (-> to skip disposal)
	0
};

const uint16 freddypharkasPatchScoreDisposal[] = {
	0x34, 0x00, 0x00, // ldi 0000
	0x34, 0x00, 0x00, // ldi 0000
	0x34, 0x00, 0x00, // ldi 0000
	PATCH_END
};

//  script 235 of freddy pharkas rm235::init and sEnterFrom500::changeState
//   disable icon 7+8 of iconbar (CD only). When picking up the cannister after
//   placing it down, the scripts will disable all the other icons. This results
//   in IconBar::disable doing endless loops even in sierra sci, because there
//   is no enabled icon left. We remove disabling of icon 8 (which is help),
//   this fixes the issue.
const byte freddypharkasSignatureCannisterHang[] = {
	12,
	0x38, 0xf1, 0x00, // pushi f1 (selector disable)
	0x7a,             // push2
	0x39, 0x07,       // pushi 07
	0x39, 0x08,       // pushi 08
	0x81, 0x45,       // lag 45
	0x4a, 0x08,       // send 08 - call IconBar::disable(7, 8)
	0
};

const uint16 freddypharkasPatchCannisterHang[] = {
	PATCH_ADDTOOFFSET | +3,
	0x78,             // push1
	PATCH_ADDTOOFFSET | +2,
	0x33, 0x00,       // ldi 00 (waste 2 bytes)
	PATCH_ADDTOOFFSET | +3,
	0x06,             // send 06 - call IconBar::disable(7)
	PATCH_END
};

//  script 215 of freddy pharkas lowerLadder::doit and highLadder::doit actually
//   process keyboard-presses when the ladder is on the screen in that room.
//   They strangely also call kGetEvent. Because the main User::doit also calls
//   kGetEvent, it's pure luck, where the event will hit. It's the same issue
//   as in QfG1VGA and if you turn dos-box to max cycles, and click around for
//   ego, sometimes clicks also won't get registered. Strangely it's not nearly
//   as bad as in our sci, but these differences may be caused by timing.
//   We just reuse the active event, thus removing the duplicate kGetEvent call.
const byte freddypharkasSignatureLadderEvent[] = {
	21,
	0x39, 0x6d,       // pushi 6d (selector new)
	0x76,             // push0
	0x38, 0xf5, 0x00, // pushi f5 (selector curEvent)
	0x76,             // push0
	0x81, 0x50,       // lag global[50]
	0x4a, 0x04,       // send 04 - read User::curEvent
	0x4a, 0x04,       // send 04 - call curEvent::new
	0xa5, 0x00,       // sat temp[0]
	0x38, 0x94, 0x00, // pushi 94 (selector localize)
	0x76,             // push0
	0x4a, 0x04,       // send 04 - call curEvent::localize
	0
};

const uint16 freddypharkasPatchLadderEvent[] = {
	0x34, 0x00, 0x00, // ldi 0000 (waste 3 bytes, overwrites first 2 pushes)
	PATCH_ADDTOOFFSET | +8,
	0xa5, 0x00,       // sat temp[0] (waste 2 bytes, overwrites 2nd send)
	PATCH_ADDTOOFFSET | +2,
	0x34, 0x00, 0x00, // ldi 0000
	0x34, 0x00, 0x00, // ldi 0000 (waste 6 bytes, overwrites last 3 opcodes)
	PATCH_END
};

//    script, description,                                      magic DWORD,                                  adjust
const SciScriptSignature freddypharkasSignatures[] = {
	{      0, "CD: score early disposal",                    1, PATCH_MAGICDWORD(0x39, 0x0d, 0x43, 0x75),    -3, freddypharkasSignatureScoreDisposal, freddypharkasPatchScoreDisposal },
	{    235, "CD: cannister pickup hang",                   3, PATCH_MAGICDWORD(0x39, 0x07, 0x39, 0x08),    -4, freddypharkasSignatureCannisterHang, freddypharkasPatchCannisterHang },
	{    320, "ladder event issue",                          2, PATCH_MAGICDWORD(0x6d, 0x76, 0x38, 0xf5),    -1, freddypharkasSignatureLadderEvent,   freddypharkasPatchLadderEvent },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// daySixBeignet::changeState (4) is called when the cop goes out and sets cycles to 220.
//  this is not enough time to get to the door, so we patch that to 23 seconds
const byte gk1SignatureDay6PoliceBeignet[] = {
	4,
	0x35, 0x04,        // ldi 04
	0x1a,              // eq?
	0x30,              // bnt [next state check]
	+2, 5,             // [skip 2 bytes, offset of bnt]
	0x38, 0x93, 0x00,  // pushi 93 (selector dispose)
	0x76,              // push0
	0x72,              // lofsa deskSarg
	+2, 9,             // [skip 2 bytes, offset of lofsa]
	0x4a, 0x04, 0x00,  // send 04
	0x34, 0xdc, 0x00,  // ldi 220
	0x65, 0x1a,        // aTop cycles
	0x32,              // jmp [end]
	0
};

const uint16 gk1PatchDay6PoliceBeignet[] = {
	PATCH_ADDTOOFFSET | +16,
	0x34, 0x17, 0x00,  // ldi 23
	0x65, 0x1c,        // aTop seconds
	PATCH_END
};

// sargSleeping::changeState (8) is called when the cop falls asleep and sets cycles to 220.
//  this is not enough time to get to the door, so we patch it to 42 seconds
const byte gk1SignatureDay6PoliceSleep[] = {
	4,
	0x35, 0x08,        // ldi 08
	0x1a,              // eq?
	0x31,              // bnt [next state check]
	+1, 6,             // [skip 1 byte, offset of bnt]
	0x34, 0xdc, 0x00,  // ldi 220
	0x65, 0x1a,        // aTop cycles
	0x32,              // jmp [end]
	0
};

const uint16 gk1PatchDay6PoliceSleep[] = {
	PATCH_ADDTOOFFSET | +5,
	0x34, 0x2a, 0x00,  // ldi 42
	0x65, 0x1c,        // aTop seconds
	PATCH_END
};

// startOfDay5::changeState (20h) - when gabriel goes to the phone the script will hang
const byte gk1SignatureDay5PhoneFreeze[] = {
	5,
	0x35, 0x03,        // ldi 03
	0x65, 0x1a,        // aTop cycles
	0x32,              // jmp [end]
	+2, 3,             // [skip 2 bytes, offset of jmp]
	0x3c,              // dup
	0x35, 0x21,        // ldi 21
	0
};

const uint16 gk1PatchDay5PhoneFreeze[] = {
	0x35, 0x06,        // ldi 06
	0x65, 0x20,        // aTop ticks
	PATCH_END
};

//    script, description,                                      magic DWORD,                                 adjust
const SciScriptSignature gk1Signatures[] = {
	{    212, "day 5 phone freeze",                          1, PATCH_MAGICDWORD(0x35, 0x03, 0x65, 0x1a),     0, gk1SignatureDay5PhoneFreeze, gk1PatchDay5PhoneFreeze },
	{    230, "day 6 police beignet timer issue",            1, PATCH_MAGICDWORD(0x34, 0xdc, 0x00, 0x65),   -16, gk1SignatureDay6PoliceBeignet, gk1PatchDay6PoliceBeignet },
	{    230, "day 6 police sleep timer issue",              1, PATCH_MAGICDWORD(0x34, 0xdc, 0x00, 0x65),    -5, gk1SignatureDay6PoliceSleep, gk1PatchDay6PoliceSleep },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// this here gets called on entry and when going out of game windows
//  uEvt::port will not get changed after kDisposeWindow but a bit later, so
//  we would get an invalid port handle to a kSetPort call. We just patch in
//  resetting of the port selector. We destroy the stop/fade code in there,
//  it seems it isn't used at all in the game.
//const byte hoyle4SignaturePortFix[] = {
//	28,
//	0x39, 0x09,        // pushi 09
//	0x89, 0x0b,        // lsg 0b
//	0x39, 0x64,        // pushi 64
//	0x38, 0xc8, 0x00,  // pushi 00c8
//	0x38, 0x2c, 0x01,  // pushi 012c
//	0x38, 0x90, 0x01,  // pushi 0190
//	0x38, 0xf4, 0x01,  // pushi 01f4
//	0x38, 0x58, 0x02,  // pushi 0258
//	0x38, 0xbc, 0x02,  // pushi 02bc
//	0x38, 0x20, 0x03,  // pushi 0320
//	0x46,              // calle [xxxx] [xxxx] [xx]
//	+5, 43,            // [skip 5 bytes]
//	0x30, 0x27, 0x00,  // bnt 0027 -> end of routine
//	0x87, 0x00,        // lap 00
//	0x30, 0x19, 0x00,  // bnt 0019 -> fade out
//	0x87, 0x01,        // lap 01
//	0x30, 0x14, 0x00,  // bnt 0014 -> fade out
//	0x38, 0xa7, 0x00,  // pushi 00a7
//	0x76,              // push0
//	0x80, 0x29, 0x01,  // lag 0129
//	0x4a, 0x04,        // send 04 - call song::stop
//	0x39, 0x27,        // pushi 27
//	0x78,              // push1
//	0x8f, 0x01,        // lsp 01
//	0x51, 0x54,        // class 54
//	0x4a, 0x06,        // send 06 - call PlaySong::play
//	0x33, 0x09,        // jmp 09 -> end of routine
//	0x38, 0xaa, 0x00,  // pushi 00aa
//	0x76,              // push0
//	0x80, 0x29, 0x01,  // lag 0129
//	0x4a, 0x04,        // send 04
//	0x48,              // ret
//	0
//};

//const uint16 hoyle4PatchPortFix[] = {
//	PATCH_ADDTOOFFSET | +33,
//	0x38, 0x31, 0x01,  // pushi 0131 (selector curEvent)
//	0x76,              // push0
//	0x80, 0x50, 0x00,  // lag 0050 (global var 80h, "User")
//	0x4a, 0x04,        // send 04 - read User::curEvent
//
//	0x38, 0x93, 0x00,  // pushi 0093 (selector port)
//	0x78,              // push1
//	0x76,              // push0
//	0x4a, 0x06,        // send 06 - write 0 to that object::port
//	0x48,              // ret
//	PATCH_END
//};

//    script, description,                                   magic DWORD,                                 adjust
//const SciScriptSignature hoyle4Signatures[] = {
//    {      0, "port fix when disposing windows",             PATCH_MAGICDWORD(0x64, 0x38, 0xC8, 0x00),    -5, hoyle4SignaturePortFix,   hoyle4PatchPortFix },
//    {      0, NULL,                                          0,                                            0, NULL,                     NULL }
//};

// ===========================================================================
// at least during harpy scene export 29 of script 0 is called in kq5cd and
//  has an issue for those calls, where temp 3 won't get inititialized, but
//  is later used to set master volume. This issue makes sierra sci set
//  the volume to max. We fix the export, so volume won't get modified in
//  those cases.
const byte kq5SignatureCdHarpyVolume[] = {
	34,
	0x80, 0x91, 0x01,  // lag global[191h]
	0x18,              // not
	0x30, 0x2c, 0x00,  // bnt [jump further] (jumping, if global 191h is 1)
	0x35, 0x01,        // ldi 01
	0xa0, 0x91, 0x01,  // sag global[191h] (setting global 191h to 1)
	0x38, 0x7b, 0x01,  // pushi 017b
	0x76,              // push0
	0x81, 0x01,        // lag global[1]
	0x4a, 0x04,        // send 04 - read KQ5::masterVolume
	0xa5, 0x03,        // sat temp[3] (store volume in temp 3)
	0x38, 0x7b, 0x01,  // pushi 017b
	0x76,              // push0
	0x81, 0x01,        // lag global[1]
	0x4a, 0x04,        // send 04 - read KQ5::masterVolume
	0x36,              // push
	0x35, 0x04,        // ldi 04
	0x20,              // ge? (followed by bnt)
	0
};

const uint16 kq5PatchCdHarpyVolume[] = {
	0x38, 0x2f, 0x02,  // pushi 022f (selector theVol) (3 new bytes)
	0x76,              // push0 (1 new byte)
	0x51, 0x88,        // class SpeakTimer (2 new bytes)
	0x4a, 0x04,        // send 04 (2 new bytes) -> read SpeakTimer::theVol
	0xa5, 0x03,        // sat temp[3] (2 new bytes) -> write to temp 3
	0x80, 0x91, 0x01,  // lag global[191h]
	// saving 1 byte due optimization
	0x2e, 0x23, 0x00,  // bt [jump further] (jumping, if global 191h is 1)
	0x35, 0x01,        // ldi 01
	0xa0, 0x91, 0x01,  // sag global[191h] (setting global 191h to 1)
	0x38, 0x7b, 0x01,  // pushi 017b
	0x76,              // push0
	0x81, 0x01,        // lag global[1]
	0x4a, 0x04,        // send 04 - read KQ5::masterVolume
	0xa5, 0x03,        // sat temp[3] (store volume in temp 3)
	// saving 8 bytes due removing of duplicate code
	0x39, 0x04,        // pushi 04 (saving 1 byte due swapping)
	0x22,              // lt? (because we switched values)
	PATCH_END
};

//    script, description,                                      magic DWORD,                                 adjust
const SciScriptSignature kq5Signatures[] = {
	{      0, "CD: harpy volume change",                     1, PATCH_MAGICDWORD(0x80, 0x91, 0x01, 0x18),     0, kq5SignatureCdHarpyVolume, kq5PatchCdHarpyVolume },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// this is called on every death dialog. Problem is at least the german
//  version of lsl6 gets title text that is far too long for the
//  available temp space resulting in temp space corruption
//  This patch moves the title text around, so this overflow
//  doesn't happen anymore. We would otherwise get a crash
//  calling for invalid views (this happens of course also
//  in sierra sci)
const byte larry6SignatureDeathDialog[] = {
	7,
	0x3e, 0x33, 0x01,             // link 0133 (offset 0x20)
	0x35, 0xff,                   // ldi ff
	0xa3, 0x00,                   // sal 00
	+255, 0,
	+255, 0,
	+170, 12,                     // [skip 680 bytes]
	0x8f, 0x01,                   // lsp 01 (offset 0x2cf)
	0x7a,                         // push2
	0x5a, 0x04, 0x00, 0x0e, 0x01, // lea 0004 010e
	0x36,                         // push
	0x43, 0x7c, 0x0e,             // kMessage[7c] 0e
	+90, 10,                      // [skip 90 bytes]
	0x38, 0xd6, 0x00,             // pushi 00d6 (offset 0x335)
	0x78,                         // push1
	0x5a, 0x04, 0x00, 0x0e, 0x01, // lea 0004 010e
	0x36,                         // push
	+76, 11,                      // [skip 76 bytes]
	0x38, 0xcd, 0x00,             // pushi 00cd (offset 0x38b)
	0x39, 0x03,                   // pushi 03
	0x5a, 0x04, 0x00, 0x0e, 0x01, // lea 0004 010e
	0x36,
	0
};

const uint16 larry6PatchDeathDialog[] = {
	0x3e, 0x00, 0x02,             // link 0200
	PATCH_ADDTOOFFSET | +687,
	0x5a, 0x04, 0x00, 0x40, 0x01, // lea 0004 0140
	PATCH_ADDTOOFFSET | +98,
	0x5a, 0x04, 0x00, 0x40, 0x01, // lea 0004 0140
	PATCH_ADDTOOFFSET | +82,
	0x5a, 0x04, 0x00, 0x40, 0x01, // lea 0004 0140
	PATCH_END
};

//    script, description,                                      magic DWORD,                                  adjust
const SciScriptSignature larry6Signatures[] = {
	{     82, "death dialog memory corruption",              1, PATCH_MAGICDWORD(0x3e, 0x33, 0x01, 0x35),     0, larry6SignatureDeathDialog, larry6PatchDeathDialog },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// rm560::doit was supposed to close the painting, when heimlich enters the
//  room. The code is buggy, so it actually closes the painting, when heimlich
//  is not in the room. We fix that.
const byte laurabow2SignaturePaintingClosing[] = {
	17,
	0x4a, 0x04,       // send 04 - read aHeimlich::room
	0x36,             // push
	0x81, 0x0b,       // lag global[11d] -> current room
	0x1c,             // ne?
	0x31, 0x0e,       // bnt [don't close]
	0x35, 0x00,       // ldi 00
	0xa3, 0x00,       // sal local[0]
	0x38, 0x92, 0x00, // pushi 0092
	0x78,             // push1
	0x72,             // lofsa sDumpSafe
	0
};

const uint16 laurabow2PatchPaintingClosing[] = {
	PATCH_ADDTOOFFSET | +6,
	0x2f, 0x0e,       // bt [don't close]
	PATCH_END
};

//    script, description,                                      magic DWORD,                                  adjust
const SciScriptSignature laurabow2Signatures[] = {
	{    560, "painting closing immediately",                1, PATCH_MAGICDWORD(0x36, 0x81, 0x0b, 0x1c),    -2, laurabow2SignaturePaintingClosing, laurabow2PatchPaintingClosing },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// Mother Goose SCI1/SCI1.1
// MG::replay somewhat calculates the savedgame-id used when saving again	
//  this doesn't work right and we remove the code completely.
//  We set the savedgame-id directly right after restoring in kRestoreGame.
const byte mothergoose256SignatureReplay[] = {
	6,
	0x36,             // push
	0x35, 0x20,       // ldi 20
	0x04,             // sub
	0xa1, 0xb3,       // sag global[b3]
	0
};

const uint16 mothergoose256PatchReplay[] = {
	0x34, 0x00, 0x00, // ldi 0000 (dummy)
	0x34, 0x00, 0x00, // ldi 0000 (dummy)
	PATCH_END
};

// when saving, it also checks if the savegame-id is below 13.
//  we change this to check if below 113 instead
const byte mothergoose256SignatureSaveLimit[] = {
	5,
	0x89, 0xb3,       // lsg global[b3]
	0x35, 0x0d,       // ldi 0d
	0x20,             // ge?
	0
};

const uint16 mothergoose256PatchSaveLimit[] = {
	PATCH_ADDTOOFFSET | +2,
	0x35, 0x0d + SAVEGAMEID_OFFICIALRANGE_START, // ldi 113d
	PATCH_END
};

//    script, description,                                      magic DWORD,                                  adjust
const SciScriptSignature mothergoose256Signatures[] = {
	{      0, "replay save issue",                           1, PATCH_MAGICDWORD(0x20, 0x04, 0xa1, 0xb3),    -2, mothergoose256SignatureReplay,    mothergoose256PatchReplay },
	{      0, "save limit dialog (SCI1.1)",                  1, PATCH_MAGICDWORD(0xb3, 0x35, 0x0d, 0x20),    -1, mothergoose256SignatureSaveLimit, mothergoose256PatchSaveLimit },
	{    994, "save limit dialog (SCI1)",                    1, PATCH_MAGICDWORD(0xb3, 0x35, 0x0d, 0x20),    -1, mothergoose256SignatureSaveLimit, mothergoose256PatchSaveLimit },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
//  script 215 of qfg1vga pointBox::doit actually processes button-presses
//   during fighting with monsters. It strangely also calls kGetEvent. Because
//   the main User::doit also calls kGetEvent it's pure luck, where the event
//   will hit. It's the same issue as in freddy pharkas and if you turn dos-box
//   to max cycles, sometimes clicks also won't get registered. Strangely it's
//   not nearly as bad as in our sci, but these differences may be caused by
//   timing.
//   We just reuse the active event, thus removing the duplicate kGetEvent call.
const byte qfg1vgaSignatureFightEvents[] = {
	25,
	0x39, 0x6d,       // pushi 6d (selector new)
	0x76,             // push0
	0x51, 0x07,       // class Event
	0x4a, 0x04,       // send 04 - call Event::new
	0xa5, 0x00,       // sat temp[0]
	0x78,             // push1
	0x76,             // push0
	0x4a, 0x04,       // send 04 - read Event::x
	0xa5, 0x03,       // sat temp[3]
	0x76,             // push0 (selector y)
	0x76,             // push0
	0x85, 0x00,       // lat temp[0]
	0x4a, 0x04,       // send 04 - read Event::y
	0x36,             // push
	0x35, 0x0a,       // ldi 0a
	0x04,             // sub (poor mans localization) ;-)
	0
};

const uint16 qfg1vgaPatchFightEvents[] = {
	0x38, 0x5a, 0x01, // pushi 15a (selector curEvent)
	0x76,             // push0
	0x81, 0x50,       // lag global[50]
	0x4a, 0x04,       // send 04 - read User::curEvent -> needs one byte more than previous code
	0xa5, 0x00,       // sat temp[0]
	0x78,             // push1
	0x76,             // push0
	0x4a, 0x04,       // send 04 - read Event::x
	0xa5, 0x03,       // sat temp[3]
	0x76,             // push0 (selector y)
	0x76,             // push0
	0x85, 0x00,       // lat temp[0]
	0x4a, 0x04,       // send 04 - read Event::y
	0x39, 0x00,       // pushi 00
	0x02,             // add (waste 3 bytes) - we don't need localization, User::doit has already done it
	PATCH_END
};

//    script, description,                                      magic DWORD,                                  adjust
const SciScriptSignature qfg1vgaSignatures[] = {
	{    215, "fight event issue",                           1, PATCH_MAGICDWORD(0x6d, 0x76, 0x51, 0x07),    -1, qfg1vgaSignatureFightEvents, qfg1vgaPatchFightEvents },
	{    216, "weapon master event issue",                   1, PATCH_MAGICDWORD(0x6d, 0x76, 0x51, 0x07),    -1, qfg1vgaSignatureFightEvents, qfg1vgaPatchFightEvents },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
//  script 298 of sq4/floppy has an issue. object "nest" uses another property
//   which isn't included in property count. We return 0 in that case, the game
//   adds it to nest::x. The problem is that the script also checks if x exceeds
//   we never reach that of course, so the pterodactyl-flight will go endlessly
//   we could either calculate property count differently somehow fixing this
//   but I think just patching it out is cleaner (ffs. bug #3037938)
const byte sq4FloppySignatureEndlessFlight[] = {
	8,
	0x39, 0x04,       // pushi 04 (selector x)
	0x78,             // push1
	0x67, 0x08,       // pTos 08 (property x)
	0x63, 0x44,       // pToa 44 (invalid property)
	0x02,             // add
	0
};

const uint16 sq4FloppyPatchEndlessFlight[] = {
	PATCH_ADDTOOFFSET | +5,
	0x35, 0x03,       // ldi 03 (which would be the content of the property)
	PATCH_END
};

//    script, description,                                      magic DWORD,                                  adjust
const SciScriptSignature sq4Signatures[] = {
	{    298, "Floppy: endless flight",                      1, PATCH_MAGICDWORD(0x67, 0x08, 0x63, 0x44),    -3, sq4FloppySignatureEndlessFlight, sq4FloppyPatchEndlessFlight },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// It seems to scripts warp ego outside the screen somehow (or maybe kDoBresen?)
//  ego::mover is set to 0 and rm119::doit will crash in that case. This here
//  fixes part of the problem and actually checks ego::mover to be 0 and skips
//  TODO: this should get further investigated by waltervn and maybe properly
//   patched. For now ego will shortly disappear and reappear a bit after
//   this isn't good, but sierra sci also "crashed" (endless looped) so this
//   is at least better than the original code
const byte sq5SignatureScrubbing[] = {
	19,
	0x18,             // not
	0x31, 0x37,       // bnt 37
	0x78,             // push1 (selector x)
	0x76,             // push0
	0x39, 0x38,       // pushi 38 (selector mover)
	0x76,             // push0
	0x81, 0x00,       // lag 00
	0x4a, 0x04,       // send 04 - read ego::mover
	0x4a, 0x04,       // send 04 - read ego::mover::x
	0x36,             // push
	0x34, 0xa0, 0x00, // ldi 00a0
	0x1c,             // ne?
	0
};

const uint16 sq5PatchScrubbing[] = {
	0x18,             // not
	0x31, 0x37,       // bnt 37
//	0x2f, 0x38,       // bt 37 (would save another byte, isn't needed
	0x39, 0x38,       // pushi 38 (selector mover)
	0x76,             // push0
	0x81, 0x00,       // lag 00
	0x4a, 0x04,       // send 04 - read ego::mover
	0x31, 0x2e,       // bnt 2e (jump if ego::mover is 0)
	0x78,             // push1 (selector x)
	0x76,             // push0
	0x4a, 0x04,       // send 04 - read ego::mover::x
	0x39, 0xa0,       // pushi a0 (saving 2 bytes)
	0x1c,             // ne?
	PATCH_END
};

//    script, description,                                      magic DWORD,                                  adjust
const SciScriptSignature sq5Signatures[] = {
	{    119, "scrubbing send crash",                        1, PATCH_MAGICDWORD(0x18, 0x31, 0x37, 0x78),     0, sq5SignatureScrubbing, sq5PatchScrubbing },
	SCI_SIGNATUREENTRY_TERMINATOR
};


// will actually patch previously found signature area
void Script::applyPatch(const uint16 *patch, byte *scriptData, const uint32 scriptSize, int32 signatureOffset) {
	byte orgData[PATCH_VALUELIMIT];
	int32 offset = signatureOffset;
	uint16 patchWord = *patch;

	// Copy over original bytes from script
	uint32 orgDataSize = scriptSize - offset;
	if (orgDataSize > PATCH_VALUELIMIT)
		orgDataSize = PATCH_VALUELIMIT;
	memcpy(&orgData, &scriptData[offset], orgDataSize);

	while (patchWord != PATCH_END) {
		uint16 patchValue = patchWord & PATCH_VALUEMASK;
		switch (patchWord & PATCH_COMMANDMASK) {
		case PATCH_ADDTOOFFSET:
			// add value to offset
			offset += patchValue & ~PATCH_ADDTOOFFSET;
			break;
		case PATCH_GETORIGINALBYTE:
			// get original byte from script
			if (patchValue >= orgDataSize)
				error("patching: can not get requested original byte from script");
			scriptData[offset] = orgData[patchValue];
			offset++;
			break;
		case PATCH_ADJUSTWORD: {
			// Adjust word right before current position
			byte *adjustPtr = &scriptData[offset - 2];
			uint16 adjustWord = READ_LE_UINT16(adjustPtr);
			adjustWord += patchValue;
			WRITE_LE_UINT16(adjustPtr, adjustWord);
			break;
		}
		case PATCH_ADJUSTWORD_NEG: {
			// Adjust word right before current position (negative way)
			byte *adjustPtr = &scriptData[offset - 2];
			uint16 adjustWord = READ_LE_UINT16(adjustPtr);
			adjustWord -= patchValue;
			WRITE_LE_UINT16(adjustPtr, adjustWord);
			break;
		}
		default:
			scriptData[offset] = patchValue & 0xFF;
			offset++;
		}
		patch++;
		patchWord = *patch;
	}	
}

// will return -1 if no match was found, otherwise an offset to the start of the signature match
int32 Script::findSignature(const SciScriptSignature *signature, const byte *scriptData, const uint32 scriptSize) {
	if (scriptSize < 4) // we need to find a DWORD, so less than 4 bytes is not okay
		return -1;

	const uint32 magicDWord = signature->magicDWord; // is platform-specific BE/LE form, so that the later match will work
	const uint32 searchLimit = scriptSize - 3;
	uint32 DWordOffset = 0;
	// first search for the magic DWORD
	while (DWordOffset < searchLimit) {
		if (magicDWord == READ_UINT32(scriptData + DWordOffset)) {
			// magic DWORD found, check if actual signature matches
			uint32 offset = DWordOffset + signature->magicOffset;
			uint32 byteOffset = offset;
			const byte *signatureData = signature->data;
			byte matchAdjust = 1;
			while (matchAdjust) {
				byte matchBytesCount = *signatureData++;
				if ((byteOffset + matchBytesCount) > scriptSize) // Out-Of-Bounds?
					break;
				if (memcmp(signatureData, &scriptData[byteOffset], matchBytesCount)) // Byte-Mismatch?
					break;
				// those bytes matched, adjust offsets accordingly
				signatureData += matchBytesCount;
				byteOffset += matchBytesCount;
				// get offset...
				matchAdjust = *signatureData++;
				byteOffset += matchAdjust;
			}
			if (!matchAdjust) // all matches worked?
				return offset;
		}
		DWordOffset++;
	}
	// nothing found
	return -1;
}

void Script::matchSignatureAndPatch(uint16 scriptNr, byte *scriptData, const uint32 scriptSize) {
	const SciScriptSignature *signatureTable = NULL;
	switch (g_sci->getGameId()) {
	case GID_CASTLEBRAIN:
		signatureTable = castlebrainSignatures;
		break;
	case GID_ECOQUEST:
		signatureTable = ecoquest1Signatures;
		break;
	case GID_ECOQUEST2:
		signatureTable = ecoquest2Signatures;
		break;
	case GID_FREDDYPHARKAS:
		signatureTable = freddypharkasSignatures;
		break;
	case GID_GK1:
		signatureTable = gk1Signatures;
		break;
	// hoyle4 now works due to workaround inside GfxPorts
	//case GID_HOYLE4:
	//	signatureTable = hoyle4Signatures;
	//	break;
	case GID_KQ5:
		signatureTable = kq5Signatures;
		break;
	case GID_LAURABOW2:
		signatureTable = laurabow2Signatures;
		break;
	case GID_LSL6:
		signatureTable = larry6Signatures;
		break;
	case GID_MOTHERGOOSE256:
		signatureTable = mothergoose256Signatures;
		break;
	case GID_QFG1VGA:
		signatureTable = qfg1vgaSignatures;
		break;
	case GID_SQ4:
		signatureTable = sq4Signatures;
		break;
	case GID_SQ5:
		signatureTable = sq5Signatures;
		break;
	default:
		break;
	}

	if (signatureTable) {
		while (signatureTable->data) {
			if (scriptNr == signatureTable->scriptNr) {
				int32 foundOffset = 0;
				int16 applyCount = signatureTable->applyCount;
				do {
					foundOffset = findSignature(signatureTable, scriptData, scriptSize);
					if (foundOffset != -1) {
						// found, so apply the patch
						warning("matched and patched %s on script %d offset %d", signatureTable->description, scriptNr, foundOffset);
						applyPatch(signatureTable->patch, scriptData, scriptSize, foundOffset);
					}
					applyCount--;
				} while ((foundOffset != -1) && (applyCount));
			}
			signatureTable++;
		}
	}
}

} // End of namespace Sci
