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

// Script debugger functionality. Absolutely not threadsafe.

#include "sci/sci.h"
#include "sci/console.h"
#include "sci/debug.h"
#include "sci/engine/state.h"

namespace Sci {

int g_debugstate_valid = 0; // Set to 1 while script_debug is running
int g_debug_step_running = 0; // Set to >0 to allow multiple stepping
extern int g_debug_seek_special;

extern const char *selector_name(EngineState *s, int selector);

DebugState debugState;

int propertyOffsetToId(EngineState *s, int prop_ofs, reg_t objp) {
	Object *obj = obj_get(s, objp);
	byte *selectoroffset;
	int selectors;

	if (!obj) {
		sciprintf("Applied propertyOffsetToId on non-object at %04x:%04x\n", PRINT_REG(objp));
		return -1;
	}

	selectors = obj->_variables.size();

	if (s->_version < SCI_VERSION_1_1)
		selectoroffset = ((byte *)(obj->base_obj)) + SCRIPT_SELECTOR_OFFSET + selectors * 2;
	else {
		if (!(obj->_variables[SCRIPT_INFO_SELECTOR].offset & SCRIPT_INFO_CLASS)) {
			obj = obj_get(s, obj->_variables[SCRIPT_SUPERCLASS_SELECTOR]);
			selectoroffset = (byte *)obj->base_vars;
		} else
			selectoroffset = (byte *)obj->base_vars;
	}

	if (prop_ofs < 0 || (prop_ofs >> 1) >= selectors) {
		sciprintf("Applied propertyOffsetToId to invalid property offset %x (property #%d not in [0..%d]) on object at %04x:%04x\n",
		          prop_ofs, prop_ofs >> 1, selectors - 1, PRINT_REG(objp));
		return -1;
	}

	return READ_LE_UINT16(selectoroffset + prop_ofs);
}

reg_t disassemble(EngineState *s, reg_t pos, int print_bw_tag, int print_bytecode) {
// Disassembles one command from the heap, returns address of next command or 0 if a ret was encountered.
	MemObject *mobj = GET_SEGMENT(*s->seg_manager, pos.segment, MEM_OBJ_SCRIPT);
	Script *script_entity = NULL;
	byte *scr;
	int scr_size;
	reg_t retval = make_reg(pos.segment, pos.offset + 1);
	uint16 param_value;
	int opsize;
	uint opcode;
	int bytecount = 1;
	int i = 0;

	if (!mobj) {
		sciprintf("Disassembly failed: Segment %04x non-existant or not a script\n", pos.segment);
		return retval;
	} else
		script_entity = (Script *)mobj;

	scr = script_entity->buf;
	scr_size = script_entity->buf_size;

	if (pos.offset >= scr_size) {
		sciprintf("Trying to disassemble beyond end of script\n");
		return pos;
	}

	opsize = scr[pos.offset];
	opcode = opsize >> 1;

	if (!g_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return retval;
	}

	opsize &= 1; // byte if true, word if false

	sciprintf("%04x:%04x: ", PRINT_REG(pos));

	if (print_bytecode) {
		while (g_opcode_formats[opcode][i]) {
			switch (g_opcode_formats[opcode][i++]) {

			case Script_SByte:
			case Script_Byte:
				bytecount++;
				break;

			case Script_Word:
			case Script_SWord:
				bytecount += 2;
				break;

			case Script_SVariable:
			case Script_Variable:
			case Script_Property:
			case Script_Global:
			case Script_Local:
			case Script_Temp:
			case Script_Param:
			case Script_SRelative:
				if (opsize)
					bytecount ++;
				else
					bytecount += 2;
				break;

			default:
				break;
			}
		}

		if (pos.offset + bytecount > scr_size) {
			sciprintf("Operation arguments extend beyond end of script\n");
			return retval;
		}

		for (i = 0; i < bytecount; i++)
			sciprintf("%02x ", scr[pos.offset + i]);

		for (i = bytecount; i < 5; i++)
			sciprintf("   ");
	}

	if (print_bw_tag)
		sciprintf("[%c] ", opsize ? 'B' : 'W');
	sciprintf("%s", s->_kernel->getOpcode(opcode).name.c_str());

	i = 0;
	while (g_opcode_formats[opcode][i]) {
		switch (g_opcode_formats[opcode][i++]) {
		case Script_Invalid:
			sciprintf("-Invalid operation-");
			break;

		case Script_SByte:
		case Script_Byte:
			sciprintf(" %02x", scr[retval.offset++]);
			break;

		case Script_Word:
		case Script_SWord:
			sciprintf(" %04x", 0xffff & (scr[retval.offset] | (scr[retval.offset+1] << 8)));
			retval.offset += 2;
			break;

		case Script_SVariable:
		case Script_Variable:
		case Script_Property:
		case Script_Global:
		case Script_Local:
		case Script_Temp:
		case Script_Param:
			if (opsize)
				param_value = scr[retval.offset++];
			else {
				param_value = 0xffff & (scr[retval.offset] | (scr[retval.offset+1] << 8));
				retval.offset += 2;
			}

			if (opcode == op_callk)
				sciprintf(" %s[%x]", (param_value < s->_kernel->_kernelFuncs.size()) ?
							((param_value < s->_kernel->getKernelNamesSize()) ? s->_kernel->getKernelName(param_value).c_str() : "[Unknown(postulated)]")
							: "<invalid>", param_value);
			else
				sciprintf(opsize ? " %02x" : " %04x", param_value);

			break;

		case Script_Offset:
			if (opsize)
				param_value = scr[retval.offset++];
			else {
				param_value = 0xffff & (scr[retval.offset] | (scr[retval.offset+1] << 8));
				retval.offset += 2;
			}
			sciprintf(opsize ? " %02x" : " %04x", param_value);
			break;

		case Script_SRelative:
			if (opsize)
				param_value = scr[retval.offset++];
			else {
				param_value = 0xffff & (scr[retval.offset] | (scr[retval.offset+1] << 8));
				retval.offset += 2;
			}
			sciprintf(opsize ? " %02x  [%04x]" : " %04x  [%04x]", param_value, (0xffff) & (retval.offset + param_value));
			break;

		case Script_End:
			retval = NULL_REG;
			break;

		default:
			sciprintf("Internal assertion failed in 'disassemble', %s, L%d\n", __FILE__, __LINE__);

		}
	}

	if (pos == *debugState.p_pc) { // Extra information if debugging the current opcode
		if ((opcode == op_pTos) || (opcode == op_sTop) || (opcode == op_pToa) || (opcode == op_aTop) ||
		        (opcode == op_dpToa) || (opcode == op_ipToa) || (opcode == op_dpTos) || (opcode == op_ipTos)) {
			int prop_ofs = scr[pos.offset + 1];
			int prop_id = propertyOffsetToId(s, prop_ofs, *debugState.p_objp);

			sciprintf("	(%s)", selector_name(s, prop_id));
		}
	}

	sciprintf("\n");

	if (pos == *debugState.p_pc) { // Extra information if debugging the current opcode
		if (opcode == op_callk) {
			int stackframe = (scr[pos.offset + 2] >> 1) + (*debugState.p_restadjust);
			int argc = ((*debugState.p_sp)[- stackframe - 1]).offset;

			if (!(s->_flags & GF_SCI0_OLD))
				argc += (*debugState.p_restadjust);

			sciprintf(" Kernel params: (");

			for (int j = 0; j < argc; j++) {
				sciprintf("%04x:%04x", PRINT_REG((*debugState.p_sp)[j - stackframe]));
				if (j + 1 < argc)
					sciprintf(", ");
			}
			sciprintf(")\n");
		} else if ((opcode == op_send) || (opcode == op_self)) {
			int restmod = *debugState.p_restadjust;
			int stackframe = (scr[pos.offset + 1] >> 1) + restmod;
			reg_t *sb = *debugState.p_sp;
			uint16 selector;
			reg_t fun_ref;

			while (stackframe > 0) {
				int argc = sb[- stackframe + 1].offset;
				const char *name = NULL;
				reg_t called_obj_addr = *debugState.p_objp;

				if (opcode == op_send)
					called_obj_addr = s->r_acc;
				else if (opcode == op_self)
					called_obj_addr = *debugState.p_objp;

				selector = sb[- stackframe].offset;

				name = obj_get_name(s, called_obj_addr);

				if (!name)
					name = "<invalid>";

				sciprintf("  %s::%s[", name, (selector > s->_kernel->getSelectorNamesSize()) ? "<invalid>" : selector_name(s, selector));

				switch (lookup_selector(s, called_obj_addr, selector, 0, &fun_ref)) {
				case kSelectorMethod:
					sciprintf("FUNCT");
					argc += restmod;
					restmod = 0;
					break;
				case kSelectorVariable:
					sciprintf("VAR");
					break;
				case kSelectorNone:
					sciprintf("INVALID");
					break;
				}

				sciprintf("](");

				while (argc--) {
					sciprintf("%04x:%04x", PRINT_REG(sb[- stackframe + 2]));
					if (argc)
						sciprintf(", ");
					stackframe--;
				}

				sciprintf(")\n");
				stackframe -= 2;
			} // while (stackframe > 0)
		} // Send-like opcodes
	} // (heappos == *p_pc)

	return retval;
}


void script_debug(EngineState *s, reg_t *pc, StackPtr *sp, StackPtr *pp, reg_t *objp, int *restadjust,
	SegmentId *segids, reg_t **variables, reg_t **variables_base, int *variables_nr, int bp) {
	// Do we support a separate console?

	int old_debugstate = g_debugstate_valid;

	debugState.p_var_segs = segids;
	debugState.p_vars = variables;
	debugState.p_var_max = variables_nr;
	debugState.p_var_base = variables_base;
	debugState.p_pc = pc;
	debugState.p_sp = sp;
	debugState.p_pp = pp;
	debugState.p_objp = objp;
	debugState.p_restadjust = restadjust;
	sciprintf("%d: acc=%04x:%04x  ", script_step_counter, PRINT_REG(s->r_acc));
	g_debugstate_valid = 1;
	disassemble(s, *pc, 0, 1);
	if (g_debug_seeking == kDebugSeekGlobal)
		sciprintf("Global %d (0x%x) = %04x:%04x\n", g_debug_seek_special,
		          g_debug_seek_special, PRINT_REG(s->script_000->locals_block->_locals[g_debug_seek_special]));

	g_debugstate_valid = old_debugstate;

	if (g_debug_seeking && !bp) { // Are we looking for something special?
		MemObject *mobj = GET_SEGMENT(*s->seg_manager, pc->segment, MEM_OBJ_SCRIPT);

		if (mobj) {
			Script *scr = (Script *)mobj;
			byte *code_buf = scr->buf;
			int code_buf_size = scr->buf_size;
			int opcode = pc->offset >= code_buf_size ? 0 : code_buf[pc->offset];
			int op = opcode >> 1;
			int paramb1 = pc->offset + 1 >= code_buf_size ? 0 : code_buf[pc->offset + 1];
			int paramf1 = (opcode & 1) ? paramb1 : (pc->offset + 2 >= code_buf_size ? 0 : (int16)READ_LE_UINT16(code_buf + pc->offset + 1));

			switch (g_debug_seeking) {
			case kDebugSeekSpecialCallk:
				if (paramb1 != g_debug_seek_special)
					return;

			case kDebugSeekCallk: {
				if (op != op_callk)
					return;
				break;
			}

			case kDebugSeekLevelRet: {
				if ((op != op_ret) || (g_debug_seek_level < (int)s->_executionStack.size()-1))
					return;
				break;
			}

			case kDebugSeekGlobal:
				if (op < op_sag)
					return;
				if ((op & 0x3) > 1)
					return; // param or temp
				if ((op & 0x3) && s->_executionStack.back().local_segment > 0)
					return; // locals and not running in script.000
				if (paramf1 != g_debug_seek_special)
					return; // CORRECT global?
				break;

			}

			g_debug_seeking = kDebugSeekNothing;
			// OK, found whatever we were looking for
		}
	}

	g_debugstate_valid = (g_debug_step_running == 0);

	if (g_debugstate_valid) {
		debugState.p_pc = pc;
		debugState.p_sp = sp;
		debugState.p_pp = pp;
		debugState.p_objp = objp;
		debugState.p_restadjust = restadjust;
		debugState.p_var_segs = segids;
		debugState.p_vars = variables;
		debugState.p_var_max = variables_nr;
		debugState.p_var_base = variables_base;
		
		sciprintf("Step #%d\n", script_step_counter);
		disassemble(s, *pc, 0, 1);
	}

}

} // End of namespace Sci
