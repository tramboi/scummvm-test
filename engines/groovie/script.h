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

#ifndef GROOVIE_SCRIPT_H
#define GROOVIE_SCRIPT_H

#include "common/file.h"
#include "common/rect.h"

#include "groovie/font.h"

namespace Groovie {

class GroovieEngine;

class Script {
	friend class Debugger;

public:
	Script(GroovieEngine *vm);
	~Script();

	void setDebugger(Debugger *debugger);

	bool loadScript(Common::String scriptfile);
	void directGameLoad(int slot);
	void step();

	void setMouseClick();
	void setKbdChar(uint8 c);

	Common::String &getContext();

private:
	GroovieEngine *_vm;

	Common::RandomSource _random;

	bool _firstbit;

	// Script filename (for debugging purposes)
	Common::String _scriptFile;
	Common::String _savedScriptFile;

	// Code
	byte *_code;
	uint16 _currentInstruction;
	byte *_savedCode;
	uint16 _savedInstruction;

	// Variables
	byte _variables[0x400];
	byte _savedVariables[0x180];

	// Stack
	uint16 _stack[0x20];
	uint8 _stacktop;
	uint8 _savedStacktop;

	// Input
	bool _mouseClicked;
	bool _eventMouseClicked;
	uint8 _kbdChar;
	uint8 _eventKbdChar;
	uint16 _inputLoopAddress;
	int16 _inputAction;
	uint8 _newCursorStyle;
	uint16 _hotspotTopAction;
	uint16 _hotspotTopCursor;
	uint16 _hotspotBottomAction;
	uint16 _hotspotBottomCursor;
	uint16 _hotspotRightAction;
	uint16 _hotspotLeftAction;
	uint16 _hotspotSlot;

	// Video
	Font *_font;
	Common::SeekableReadStream *_videoFile;
	uint16 _videoRef;
	uint16 _bitflags;

	// Debugging
	Debugger *_debugger;
	Common::String _debugString;

	// Helper functions
	uint8 readScript8bits();
	uint16 readScript16bits();
	uint32 readScript32bits();
	uint16 readScript8or16bits();
	uint8 readScriptChar(bool allow7C, bool limitVal, bool limitVar);
	uint8 readScriptVar();
	uint16 getVideoRefString();

	bool hotspot(Common::Rect rect, uint16 addr, uint8 cursor);

	void loadgame(uint slot);
	void savegame(uint slot);
	bool playvideofromref(uint16 fileref);

	// Opcodes
	typedef void (Script::*OpcodeFunc)();
	static OpcodeFunc _opcodes[];

	void o_invalid();

	void o_nop();
	void o_nop8();
	void o_nop16();
	void o_nop32();
	void o_nop8or16();

	void o_playsong();
	void o_bf9on();
	void o_palfadeout();
	void o_bf8on();
	void o_bf6on();
	void o_bf7on();
	void o_setbackgroundsong();
	void o_videofromref();
	void o_bf5on();
	void o_inputloopstart();
	void o_keyboardaction();
	void o_hotspot_rect();
	void o_hotspot_left();
	void o_hotspot_right();
	void o_hotspot_center();
	void o_hotspot_current();
	void o_inputloopend();
	void o_random();
	void o_jmp();
	void o_loadstring();
	void o_ret();
	void o_call();
	void o_sleep();
	void o_strcmpnejmp_var();
	void o_copybgtofg();
	void o_strcmpnejmp();
	void o_xor_obfuscate();
	void o_vdxtransition();
	void o_swap();
	void o_inc();
	void o_dec();
	void o_strcmpeqjmp();
	void o_mov();
	void o_add();
	void o_videofromstring1();
	void o_videofromstring2();
	void o_stopmidi();
	void o_endscript();
	void o_sethotspottop();
	void o_sethotspotbottom();
	void o_loadgame();
	void o_savegame();
	void o_hotspotbottom_4();
	void o_midivolume();
	void o_jne();
	void o_loadstringvar();
	void o_chargreatjmp();
	void o_bf7off();
	void o_charlessjmp();
	void o_copyrecttobg();
	void o_restorestkpnt();
	void o_obscureswap();
	void o_printstring();
	void o_hotspot_slot();
	void o_checkvalidsaves();
	void o_resetvars();
	void o_mod();
	void o_loadscript();
	void o_setvideoorigin();
	void o_sub();
	void o_othello();
	void o_returnscript();
	void o_sethotspotright();
	void o_sethotspotleft();
	void o_getcd();
	void o_opcode4D();
	void o_hotspot_outrect();
	void o_stub56();
	void o_stub59();
};

} // End of Groovie namespace

#endif // GROOVIE_SCRIPT_H
