/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
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
 */

#ifndef SIMON_DEBUG_H
#define SIMON_DEBUG_H

namespace Simon {

static const char *const simon1dos_opcode_name_table[256] = {
	/* 0 */
	"|INV_COND",
	"IJ|PTRA_PARENT_IS",
	"IJ|PTRA_PARENT_ISNOT",
	NULL,
	/* 4 */
	NULL,
	"IJ|PARENT_IS_1",
	"IJ|PARENT_ISNOT_1",
	"IIJ|PARENT_IS",
	/* 8 */
	NULL,
	NULL,
	NULL,
	"VJ|IS_ZERO",
	/* 12 */
	"VJ|ISNOT_ZERO",
	"VWJ|IS_EQ",
	"VWJ|IS_NEQ",
	"VWJ|IS_LE",
	/* 16 */
	"VWJ|IS_GE",
	"VVJ|IS_EQF",
	"VVJ|IS_NEQF",
	"VVJ|IS_LEF",
	/* 20 */
	"VVJ|IS_GEF",
	NULL,
	NULL,
	"WJ|UNK_23",
	/* 24 */
	NULL,
	"IJ|HAS_CHILD_1",
	"IJ|HAS_CHILD_2",
	"IWJ|ITEM_UNK3_IS",
	/* 28 */
	"IBJ|CHILD_HAS_FLAG",
	NULL,
	NULL,
	"I|SET_NO_PARENT",
	/* 32 */
	NULL,
	"II|SET_PARENT",
	NULL,
	NULL,
	/* 36 */
	"VV|MOVE",
	NULL,
	NULL,
	NULL,
	/* 40 */
	NULL,
	"V|ZERO",
	"VW|SET",
	"VW|ADD",
	/* 44 */
	"VW|SUB",
	"VV|ADDF",
	"VV|SUBF",
	"VW|MUL",
	/* 48 */
	"VW|DIV",
	"VV|MULF",
	"VV|DIVF",
	"VW|MOD",
	/* 52 */
	"VV|MODF",
	"VW|RANDOM",
	NULL,
	"I|SET_A_PARENT",
	/* 56 */
	"IB|SET_CHILD2_BIT",
	"IB|CLEAR_CHILD2_BIT",
	"II|MAKE_SIBLING",
	"I|INC_UNK3",
	/* 60 */
	"I|DEC_UNK3",
	"IW|SET_UNK3",
	"V|SHOW_INT",
	"T|SHOW_STRING_NL",
	/* 64 */
	"T|SHOW_STRING",
	"WWWWWB|ADD_HITAREA",
	"BT|SET_ITEM_NAME",
	"BT|SET_ITEM_DESC",
	/* 68 */
	"x|HALT",
	"x|RET1",
	"V|SHOW_STRING_AR3",
	"W|START_SUB",
	/* 72 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 76 */
	"WW|ADD_TIMEOUT",
	"J|IS_M1_EMPTY",
	"J|IS_M3_EMPTY",
	"ITJ|CHILD_FR2_IS",
	/* 80 */
	"IIJ|IS_ITEM_EQ",
	NULL,
	"B|UNK_82",
	"|RETM10",
	/* 84 */
	NULL,
	NULL,
	NULL,
	"W|UNK_87",
	/* 88 */
	"|OR_SCRIPT_WORD_10",
	"|AND_SCRIPT_WORD_10",
	"IB|SET_M_TO_PARENT",
	"IB|SET_M_TO_SIBLING",
	/* 92 */
	"IB|SET_M_TO_CHILD",
	NULL,
	NULL,
	NULL,
	/* 96 */
	"WB|UNK_96",
	"W|LOAD_VGA",
	"WBWWW|START_VGA",
	"W|KILL_SPRITE",
	/* 100 */
	"|VGA_RESET",
	"BWWWWWW|UNK_101",
	"B|UNK_102",
	"|UNK_103",
	/* 104 */
	"B|UNK_104",
	NULL,
	NULL,
	"WWWWWIW|ADD_ITEM_HITAREA",
	/* 108 */
	"W|DEL_HITAREA",
	"W|CLEAR_HITAREA_0x40",
	"W|SET_HITAREA_0x40",
	"WWW|SET_HITAREA_XY",
	/* 112 */
	NULL,
	NULL,
	"IB|UNK_114",
	"IBJ|HAS_FLAG",
	/* 116 */
	"IB|SET_FLAG",
	"IB|CLEAR_FLAG",
	NULL,
	"W|WAIT_VGA",
	/* 120 */
	"W|UNK_120",
	"BI|SET_VGA_ITEM",
	NULL,
	NULL,
	/* 124 */
	NULL,
	"IJ|IS_SIBLING_WITH_A",
	"IBB|UNK_126",
	"WW|PLAY_MUSIC_RESOURCE",
	/* 128 */
	"W|GET_DUMMY_WORD",
	"W|GET_WORD_COND_TRUE",
	"Bww|UNK_130",
	NULL,													/* opcode 131 doesn't exist */
	/* 132 */
	"|SAVE_GAME",
	"|LOAD_GAME",
	"|DUMMYPROC_134",
	"|QUIT_IF_USER_PRESSES_Y",
	/* 136 */
	"IV|GET_ITEM_UNK3",
	"B|UNK_137",
	"|VGA_POINTER_OP_4",
	"II|SET_PARENT_SPECIAL",
	/* 140 */
	"|DEL_TE_AND_ADD_ONE",
	"BI|SET_M1_OR_M3",
	"WJ|IS_HITAREA_0x40_CLEAR",
	"I|START_ITEM_SUB",
	/* 144 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 148 */
	NULL,
	NULL,
	NULL,
	"BI|SET_ARRAY6_TO",
	/* 152 */
	"BB|SET_M1_M3_TO_ARRAY6",
	"B|SET_BIT",
	"B|CLEAR_BIT",
	"BJ|IS_BIT_CLEAR",
	/* 156 */
	"BJ|IS_BIT_SET",
	"IBB|GET_ITEM_PROP",
	"IBW|SET_ITEM_PROP",
	NULL,
	/* 160 */
	"B|UNK_160",
	"BWBW|SETUP_TEXT",
	"BBT|PRINT_STR",
	"W|SOUND_1",
	/* 164 */
	"|UNK_164",
	"IWWJ|ITEM_UNK1_UNK2_IS",
	"B|SET_BIT2",
	"B|CLEAR_BIT2",
	/* 168 */
	"BJ|IS_BIT2_CLEAR",
	"BJ|IS_BIT2_SET",
	NULL,
	NULL,
	/* 172 */
	NULL,
	NULL,
	NULL,
	"|VGA_POINTER_OP_1",
	/* 176 */
	"|VGA_POINTER_OP_2",
	"BBI|INVENTORY_DESCRIPTION",
	"WWBB|PATHFIND",
	"BBB|ROOM_DESCRIPTION",
	/* 180 */
	"|FORCE_UNLOCK",
	"|FORCE_LOCK",
	"|READ_VGARES_328",
	"|READ_VGARES_23",
	/* 184 */
	"W|CLEAR_VGAPOINTER_ENTRY",
	"W|MIDI_SFX_FILE_NUM",
	"|VGA_POINTER_OP_3",
	"|FADE_TO_BLACK",
};

static const char *const simon1talkie_opcode_name_table[256] = {
	/* 0 */
	"|INV_COND",
	"IJ|PTRA_PARENT_IS",
	"IJ|PTRA_PARENT_ISNOT",
	NULL,
	/* 4 */
	NULL,
	"IJ|PARENT_IS_1",
	"IJ|PARENT_ISNOT_1",
	"IIJ|PARENT_IS",
	/* 8 */
	NULL,
	NULL,
	NULL,
	"VJ|IS_ZERO",
	/* 12 */
	"VJ|ISNOT_ZERO",
	"VWJ|IS_EQ",
	"VWJ|IS_NEQ",
	"VWJ|IS_LE",
	/* 16 */
	"VWJ|IS_GE",
	"VVJ|IS_EQF",
	"VVJ|IS_NEQF",
	"VVJ|IS_LEF",
	/* 20 */
	"VVJ|IS_GEF",
	NULL,
	NULL,
	"WJ|UNK_23",
	/* 24 */
	NULL,
	"IJ|HAS_CHILD_1",
	"IJ|HAS_CHILD_2",
	"IWJ|ITEM_UNK3_IS",
	/* 28 */
	"IBJ|CHILD_HAS_FLAG",
	NULL,
	NULL,
	"I|SET_NO_PARENT",
	/* 32 */
	NULL,
	"II|SET_PARENT",
	NULL,
	NULL,
	/* 36 */
	"VV|MOVE",
	NULL,
	NULL,
	NULL,
	/* 40 */
	NULL,
	"V|ZERO",
	"VW|SET",
	"VW|ADD",
	/* 44 */
	"VW|SUB",
	"VV|ADDF",
	"VV|SUBF",
	"VW|MUL",
	/* 48 */
	"VW|DIV",
	"VV|MULF",
	"VV|DIVF",
	"VW|MOD",
	/* 52 */
	"VV|MODF",
	"VW|RANDOM",
	NULL,
	"I|SET_A_PARENT",
	/* 56 */
	"IB|SET_CHILD2_BIT",
	"IB|CLEAR_CHILD2_BIT",
	"II|MAKE_SIBLING",
	"I|INC_UNK3",
	/* 60 */
	"I|DEC_UNK3",
	"IW|SET_UNK3",
	"V|SHOW_INT",
	"T|SHOW_STRING_NL",
	/* 64 */
	"T|SHOW_STRING",
	"WWWWWB|ADD_HITAREA",
	"BT|SET_ITEM_NAME",
	"BTw|SET_ITEM_DESC",
	/* 68 */
	"x|HALT",
	"x|RET1",
	"V|SHOW_STRING_AR3",
	"W|START_SUB",
	/* 72 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 76 */
	"WW|ADD_TIMEOUT",
	"J|IS_M1_EMPTY",
	"J|IS_M3_EMPTY",
	"ITJ|CHILD_FR2_IS",
	/* 80 */
	"IIJ|IS_ITEM_EQ",
	NULL,
	"B|UNK_82",
	"|RETM10",
	/* 84 */
	NULL,
	NULL,
	NULL,
	"W|UNK_87",
	/* 88 */
	"|OR_SCRIPT_WORD_10",
	"|AND_SCRIPT_WORD_10",
	"IB|SET_M_TO_PARENT",
	"IB|SET_M_TO_SIBLING",
	/* 92 */
	"IB|SET_M_TO_CHILD",
	NULL,
	NULL,
	NULL,
	/* 96 */
	"WB|UNK_96",
	"W|LOAD_VGA",
	"WBWWW|START_VGA",
	"W|KILL_SPRITE",
	/* 100 */
	"|VGA_RESET",
	"BWWWWWW|UNK_101",
	"B|UNK_102",
	"|UNK_103",
	/* 104 */
	"B|UNK_104",
	NULL,
	NULL,
	"WWWWWIW|ADD_ITEM_HITAREA",
	/* 108 */
	"W|DEL_HITAREA",
	"W|CLEAR_HITAREA_0x40",
	"W|SET_HITAREA_0x40",
	"WWW|SET_HITAREA_XY",
	/* 112 */
	NULL,
	NULL,
	"IB|UNK_114",
	"IBJ|HAS_FLAG",
	/* 116 */
	"IB|SET_FLAG",
	"IB|CLEAR_FLAG",
	NULL,
	"W|WAIT_VGA",
	/* 120 */
	"W|UNK_120",
	"BI|SET_VGA_ITEM",
	NULL,
	NULL,
	/* 124 */
	NULL,
	"IJ|IS_SIBLING_WITH_A",
	"IBB|UNK_126",
	"WW|PLAY_MUSIC_RESOURCE",
	/* 128 */
	"W|GET_DUMMY_WORD",
	"W|GET_WORD_COND_TRUE",
	"Bww|UNK_130",
	NULL,													/* opcode 131 doesn't exist */
	/* 132 */
	"|SAVE_GAME",
	"|LOAD_GAME",
	"|DUMMYPROC_134",
	"|QUIT_IF_USER_PRESSES_Y",
	/* 136 */
	"IV|GET_ITEM_UNK3",
	"B|UNK137",
	"|VGA_POINTER_OP_4",
	"II|SET_PARENT_SPECIAL",
	/* 140 */
	"|DEL_TE_AND_ADD_ONE",
	"BI|SET_M1_OR_M3",
	"WJ|IS_HITAREA_0x40_CLEAR",
	"I|START_ITEM_SUB",
	/* 144 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 148 */
	NULL,
	NULL,
	NULL,
	"BI|SET_ARRAY6_TO",
	/* 152 */
	"BB|SET_M1_M3_TO_ARRAY6",
	"B|SET_BIT",
	"B|CLEAR_BIT",
	"BJ|IS_BIT_CLEAR",
	/* 156 */
	"BJ|IS_BIT_SET",
	"IBB|GET_ITEM_PROP",
	"IBW|SET_ITEM_PROP",
	NULL,
	/* 160 */
	"B|UNK_160",
	"BWBW|SETUP_TEXT",
	"BBTW|PRINT_STR",
	"W|SOUND_1",
	/* 164 */
	"|UNK_164",
	"IWWJ|ITEM_UNK1_UNK2_IS",
	"B|SET_BIT2",
	"B|CLEAR_BIT2",
	/* 168 */
	"BJ|IS_BIT2_CLEAR",
	"BJ|IS_BIT2_SET",
	NULL,
	NULL,
	/* 172 */
	NULL,
	NULL,
	NULL,
	"|VGA_POINTER_OP_1",
	/* 176 */
	"|VGA_POINTER_OP_2",
	"BBI|INVENTORY_DESCRIPTION",
	"WWBB|PATHFIND",
	"BBB|ROOM_DESCRIPTION",
	/* 180 */
	"|FORCE_UNLOCK",
	"|FORCE_LOCK",
	"|READ_VGARES_328",
	"|READ_VGARES_23",
	/* 184 */
	"W|CLEAR_VGAPOINTER_ENTRY",
	"W|DUMMY_185",
	"|VGA_POINTER_OP_3",
	"|FADE_TO_BLACK",
};

static const char *const simon2dos_opcode_name_table[256] = {
	/* 0 */
	"|INV_COND",
	"IJ|PTRA_PARENT_IS",
	"IJ|PTRA_PARENT_ISNOT",
	NULL,
	/* 4 */
	NULL,
	"IJ|PARENT_IS_1",
	"IJ|PARENT_ISNOT_1",
	"IIJ|PARENT_IS",
	/* 8 */
	NULL,
	NULL,
	NULL,
	"VJ|IS_ZERO",
	/* 12 */
	"VJ|ISNOT_ZERO",
	"VWJ|IS_EQ",
	"VWJ|IS_NEQ",
	"VWJ|IS_LE",
	/* 16 */
	"VWJ|IS_GE",
	"VVJ|IS_EQF",
	"VVJ|IS_NEQF",
	"VVJ|IS_LEF",
	/* 20 */
	"VVJ|IS_GEF",
	NULL,
	NULL,
	"WJ|UNK_23",
	/* 24 */
	NULL,
	"IJ|HAS_CHILD_1",
	"IJ|HAS_CHILD_2",
	"IWJ|ITEM_UNK3_IS",
	/* 28 */
	"IBJ|CHILD_HAS_FLAG",
	NULL,
	NULL,
	"I|SET_NO_PARENT",
	/* 32 */
	NULL,
	"II|SET_PARENT",
	NULL,
	NULL,
	/* 36 */
	"VV|MOVE",
	NULL,
	NULL,
	NULL,
	/* 40 */
	NULL,
	"V|ZERO",
	"VW|SET",
	"VW|ADD",
	/* 44 */
	"VW|SUB",
	"VV|ADDF",
	"VV|SUBF",
	"VW|MUL",
	/* 48 */
	"VW|DIV",
	"VV|MULF",
	"VV|DIVF",
	"VW|MOD",
	/* 52 */
	"VV|MODF",
	"VW|RANDOM",
	NULL,
	"I|SET_A_PARENT",
	/* 56 */
	"IB|SET_CHILD2_BIT",
	"IB|CLEAR_CHILD2_BIT",
	"II|MAKE_SIBLING",
	"I|INC_UNK3",
	/* 60 */
	"I|DEC_UNK3",
	"IW|SET_UNK3",
	"V|SHOW_INT",
	"T|SHOW_STRING_NL",
	/* 64 */
	"T|SHOW_STRING",
	"WWWWWB|ADD_HITAREA",
	"BT|SET_ITEM_NAME",
	"BT|SET_ITEM_DESC",
	/* 68 */
	"x|HALT",
	"x|RET1",
	"V|SHOW_STRING_AR3",
	"W|START_SUB",
	/* 72 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 76 */
	"WW|ADD_TIMEOUT",
	"J|IS_M1_EMPTY",
	"J|IS_M3_EMPTY",
	"ITJ|CHILD_FR2_IS",
	/* 80 */
	"IIJ|IS_ITEM_EQ",
	NULL,
	"B|UNK_82",
	"|RETM10",
	/* 84 */
	NULL,
	NULL,
	NULL,
	"W|UNK_87",
	/* 88 */
	"|OR_SCRIPT_WORD_10",
	"|AND_SCRIPT_WORD_10",
	"IB|SET_M_TO_PARENT",
	"IB|SET_M_TO_SIBLING",
	/* 92 */
	"IB|SET_M_TO_CHILD",
	NULL,
	NULL,
	NULL,
	/* 96 */
	"WB|UNK_96",
	"W|LOAD_VGA",
	"WWBWWW|START_VGA",
	"WW|KILL_SPRITE",
	/* 100 */
	"|VGA_RESET",
	"BWWWWWW|UNK_101",
	"B|UNK_102",
	"|UNK_103",
	/* 104 */
	"B|UNK_104",
	NULL,
	NULL,
	"WWWWWIW|ADD_ITEM_HITAREA",
	/* 108 */
	"W|DEL_HITAREA",
	"W|CLEAR_HITAREA_0x40",
	"W|SET_HITAREA_0x40",
	"WWW|SET_HITAREA_XY",
	/* 112 */
	NULL,
	NULL,
	"IB|UNK_114",
	"IBJ|HAS_FLAG",
	/* 116 */
	"IB|SET_FLAG",
	"IB|CLEAR_FLAG",
	NULL,
	"W|WAIT_VGA",
	/* 120 */
	"W|UNK_120",
	"BI|SET_VGA_ITEM",
	NULL,
	NULL,
	/* 124 */
	NULL,
	"IJ|IS_SIBLING_WITH_A",
	"IBB|UNK_126",
	"WW|PLAY_MUSIC_RESOURCE",
	/* 128 */
	"W|GET_DUMMY_WORD",
	"W|GET_WORD_COND_TRUE",
	"Bww|UNK_130",
	NULL,													/* opcode 131 doesn't exist */
	/* 132 */
	"|SAVE_GAME",
	"|LOAD_GAME",
	"|DUMMYPROC_134",
	"|QUIT_IF_USER_PRESSES_Y",
	/* 136 */
	"IV|GET_ITEM_UNK3",
	"B|UNK_137",
	"|VGA_POINTER_OP_4",
	"II|SET_PARENT_SPECIAL",
	/* 140 */
	"|DEL_TE_AND_ADD_ONE",
	"BI|SET_M1_OR_M3",
	"WJ|IS_HITAREA_0x40_CLEAR",
	"I|START_ITEM_SUB",
	/* 144 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 148 */
	NULL,
	NULL,
	NULL,
	"BI|SET_ARRAY6_TO",
	/* 152 */
	"BB|SET_M1_M3_TO_ARRAY6",
	"B|SET_BIT",
	"B|CLEAR_BIT",
	"BJ|IS_BIT_CLEAR",
	/* 156 */
	"BJ|IS_BIT_SET",
	"IBB|GET_ITEM_PROP",
	"IBW|SET_ITEM_PROP",
	NULL,
	/* 160 */
	"B|UNK_160",
	"BWBW|SETUP_TEXT",
	"BBT|PRINT_STR",
	"W|SOUND_1",
	/* 164 */
	"|UNK_164",
	"IWWJ|ITEM_UNK1_UNK2_IS",
	"B|SET_BIT2",
	"B|CLEAR_BIT2",
	/* 168 */
	"BJ|IS_BIT2_CLEAR",
	"BJ|IS_BIT2_SET",
	NULL,
	NULL,
	/* 172 */
	NULL,
	NULL,
	NULL,
	"|VGA_POINTER_OP_1",
	/* 176 */
	"|VGA_POINTER_OP_2",
	"BBI|INVENTORY_DESCRIPTION",
	"WWBB|PATHFIND",
	"BBB|ROOM_DESCRIPTION",
	/* 180 */
	"|FORCE_UNLOCK",
	"|FORCE_LOCK",
	"|READ_VGARES_328",
	"|READ_VGARES_23",
	/* 184 */
	"W|CLEAR_VGAPOINTER_ENTRY",
	"W|DUMMY_185",
	"|VGA_POINTER_OP_3",
	"|FADE_TO_BLACK",
	/* 188 */
	"BSJ|STRING2_IS",
	"|UNK_189",
	"B|UNK_190",
};

static const char *const simon2talkie_opcode_name_table[256] = {
	/* 0 */
	"|INV_COND",
	"IJ|PTRA_PARENT_IS",
	"IJ|PTRA_PARENT_ISNOT",
	NULL,
	/* 4 */
	NULL,
	"IJ|PARENT_IS_1",
	"IJ|PARENT_ISNOT_1",
	"IIJ|PARENT_IS",
	/* 8 */
	NULL,
	NULL,
	NULL,
	"VJ|IS_ZERO",
	/* 12 */
	"VJ|ISNOT_ZERO",
	"VWJ|IS_EQ",
	"VWJ|IS_NEQ",
	"VWJ|IS_LE",
	/* 16 */
	"VWJ|IS_GE",
	"VVJ|IS_EQF",
	"VVJ|IS_NEQF",
	"VVJ|IS_LEF",
	/* 20 */
	"VVJ|IS_GEF",
	NULL,
	NULL,
	"WJ|UNK_23",
	/* 24 */
	NULL,
	"IJ|HAS_CHILD_1",
	"IJ|HAS_CHILD_2",
	"IWJ|ITEM_UNK3_IS",
	/* 28 */
	"IBJ|CHILD_HAS_FLAG",
	NULL,
	NULL,
	"I|SET_NO_PARENT",
	/* 32 */
	NULL,
	"II|SET_PARENT",
	NULL,
	NULL,
	/* 36 */
	"VV|MOVE",
	NULL,
	NULL,
	NULL,
	/* 40 */
	NULL,
	"V|ZERO",
	"VW|SET",
	"VW|ADD",
	/* 44 */
	"VW|SUB",
	"VV|ADDF",
	"VV|SUBF",
	"VW|MUL",
	/* 48 */
	"VW|DIV",
	"VV|MULF",
	"VV|DIVF",
	"VW|MOD",
	/* 52 */
	"VV|MODF",
	"VW|RANDOM",
	NULL,
	"I|SET_A_PARENT",
	/* 56 */
	"IB|SET_CHILD2_BIT",
	"IB|CLEAR_CHILD2_BIT",
	"II|MAKE_SIBLING",
	"I|INC_UNK3",
	/* 60 */
	"I|DEC_UNK3",
	"IW|SET_UNK3",
	"V|SHOW_INT",
	"T|SHOW_STRING_NL",
	/* 64 */
	"T|SHOW_STRING",
	"WWWWWB|ADD_HITAREA",
	"BT|SET_ITEM_NAME",
	"BTw|SET_ITEM_DESC",
	/* 68 */
	"x|HALT",
	"x|RET1",
	"V|SHOW_STRING_AR3",
	"W|START_SUB",
	/* 72 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 76 */
	"WW|ADD_TIMEOUT",
	"J|IS_M1_EMPTY",
	"J|IS_M3_EMPTY",
	"ITJ|CHILD_FR2_IS",
	/* 80 */
	"IIJ|IS_ITEM_EQ",
	NULL,
	"B|UNK_82",
	"|RETM10",
	/* 84 */
	NULL,
	NULL,
	NULL,
	"W|UNK_87",
	/* 88 */
	"|OR_SCRIPT_WORD_10",
	"|AND_SCRIPT_WORD_10",
	"IB|SET_M_TO_PARENT",
	"IB|SET_M_TO_SIBLING",
	/* 92 */
	"IB|SET_M_TO_CHILD",
	NULL,
	NULL,
	NULL,
	/* 96 */
	"WB|UNK_96",
	"W|LOAD_VGA",
	"WWBWWW|START_VGA",
	"WW|KILL_SPRITE",
	/* 100 */
	"|VGA_RESET",
	"BWWWWWW|UNK_101",
	"B|UNK_102",
	"|UNK_103",
	/* 104 */
	"B|UNK_104",
	NULL,
	NULL,
	"WWWWWIW|ADD_ITEM_HITAREA",
	/* 108 */
	"W|DEL_HITAREA",
	"W|CLEAR_HITAREA_0x40",
	"W|SET_HITAREA_0x40",
	"WWW|SET_HITAREA_XY",
	/* 112 */
	NULL,
	NULL,
	"IB|UNK_114",
	"IBJ|HAS_FLAG",
	/* 116 */
	"IB|SET_FLAG",
	"IB|CLEAR_FLAG",
	NULL,
	"W|WAIT_VGA",
	/* 120 */
	"W|UNK_120",
	"BI|SET_VGA_ITEM",
	NULL,
	NULL,
	/* 124 */
	NULL,
	"IJ|IS_SIBLING_WITH_A",
	"IBB|UNK_126",
	"WW|PLAY_MUSIC_RESOURCE",
	/* 128 */
	"W|GET_DUMMY_WORD",
	"W|GET_WORD_COND_TRUE",
	"Bww|UNK_130",
	NULL,													/* opcode 131 doesn't exist */
	/* 132 */
	"|SAVE_GAME",
	"|LOAD_GAME",
	"|DUMMYPROC_134",
	"|QUIT_IF_USER_PRESSES_Y",
	/* 136 */
	"IV|GET_ITEM_UNK3",
	"B|UNK_137",
	"|VGA_POINTER_OP_4",
	"II|SET_PARENT_SPECIAL",
	/* 140 */
	"|DEL_TE_AND_ADD_ONE",
	"BI|SET_M1_OR_M3",
	"WJ|IS_HITAREA_0x40_CLEAR",
	"I|START_ITEM_SUB",
	/* 144 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 148 */
	NULL,
	NULL,
	NULL,
	"BI|SET_ARRAY6_TO",
	/* 152 */
	"BB|SET_M1_M3_TO_ARRAY6",
	"B|SET_BIT",
	"B|CLEAR_BIT",
	"BJ|IS_BIT_CLEAR",
	/* 156 */
	"BJ|IS_BIT_SET",
	"IBB|GET_ITEM_PROP",
	"IBW|SET_ITEM_PROP",
	NULL,
	/* 160 */
	"B|UNK_160",
	"BWBW|SETUP_TEXT",
	"BBTW|PRINT_STR",
	"W|SOUND_1",
	/* 164 */
	"|UNK_164",
	"IWWJ|ITEM_UNK1_UNK2_IS",
	"B|SET_BIT2",
	"B|CLEAR_BIT2",
	/* 168 */
	"BJ|IS_BIT2_CLEAR",
	"BJ|IS_BIT2_SET",
	NULL,
	NULL,
	/* 172 */
	NULL,
	NULL,
	NULL,
	"|VGA_POINTER_OP_1",
	/* 176 */
	"|VGA_POINTER_OP_2",
	"BBI|INVENTORY_DESCRIPTION",
	"WWBB|PATHFIND",
	"BBB|ROOM_DESCRIPTION",
	/* 180 */
	"|FORCE_UNLOCK",
	"|FORCE_LOCK",
	"|READ_VGARES_328",
	"|READ_VGARES_23",
	/* 184 */
	"W|CLEAR_VGAPOINTER_ENTRY",
	"W|DUMMY_185",
	"|VGA_POINTER_OP_3",
	"|FADE_TO_BLACK",
	/* 188 */
	"BSJ|STRING2_IS",
	"|UNK_189",
	"B|UNK_190",
};

const char *const simon1_video_opcode_name_table[] = {
	/* 0 */
	"x|RET",
	"ddd|FADEOUT",
	"d|CALL",
	"ddddd|NEW_SPRITE",
	/* 4 */
	"ddd|FADEIN",
	"vd|SKIP_IF_NEQ",
	"d|SKIP_IFN_SIB_WITH_A",
	"d|SKIP_IF_SIB_WITH_A",
	/* 8 */
	"dd|SKIP_IF_PARENT_IS",
	"dd|SKIP_IF_UNK3_IS",
	"ddddd|DRAW",
	"|CLEAR_PATHFIND_ARRAY",
	/* 12 */
	"d|DELAY",
	"d|SET_SPRITE_OFFSET_X",
	"d|SET_SPRITE_OFFSET_Y",
	"d|IDENT_WAKEUP",
	/* 16 */
	"d|IDENT_SLEEP",
	"dq|SET_PATHFIND_ITEM",
	"i|JUMP_REL",
	"|CHAIN_TO",
	/* 20 */
	"dd|SET_CODE_WORD",
	"i|JUMP_IF_CODE_WORD",
	"dd|SET_SPRITE_PALETTE",
	"d|SET_SPRITE_PRIORITY",
	/* 24 */
	"diid|SET_SPRITE_XY",
	"x|HALT_SPRITE",
	"ddddd|SET_WINDOW",
	"|RESET",
	/* 28 */
	"dddd|DUMMY_28",
	"|STOP_ALL_SOUNDS",
	"d|SET_BASE_DELAY",
	"d|SET_PALETTE_MODE",
	/* 32 */
	"vv|COPY_VAR",
	"|FORCE_UNLOCK",
	"|FORCE_LOCK",
	"dd|VC35",
	/* 36 */
	"dd|SAVELOAD_THING",
	"v|SET_SPRITE_OFFSET_Y",
	"v|SKIP_IF_VAR_ZERO",
	"vd|SET_VAR",
	/* 40 */
	"vd|ADD_VAR",
	"vd|SUB_VAR",
	"vd|DELAY_IF_NOT_EQ",
	"d|SKIP_IF_BIT_CLEAR",
	/* 44 */
	"d|SKIP_IF_BIT_SET",
	"v|SET_SPRITE_X",
	"v|SET_SPRITE_Y",
	"vv|ADD_VAR_F",
	/* 48 */
	"|VC_48",
	"d|SET_BIT",
	"d|CLEAR_BIT",
	"d|CLEAR_HITAREA_BIT_0x40",
	/* 52 */
	"d|PLAY_SOUND",
	"dd|DUMMY_53",
	"ddd|DUMMY_54",
	"ddd|OFFSET_HIT_AREA",
	/* 56 */
	"|DUMMY_56",
	"|DUMMY_57",
	"|UNK_58",
	"|SKIP_IF_SPEECH_ENDED",
	/* 60 */
	"d|KILL_SPRITE",
	"ddd|INIT_SPRITE",
	"|FASTFADEOUT",
	"|FASTFADEIN",
};

const char *const simon2_video_opcode_name_table[] = {
	/* 0 */
	"x|RET",
	"ddd|FADEOUT",
	"d|CALL",
	"ddddd|NEW_SPRITE",
	/* 4 */
	"ddd|FADEIN",
	"vd|SKIP_IF_NEQ",
	"d|SKIP_IFN_SIB_WITH_A",
	"d|SKIP_IF_SIB_WITH_A",
	/* 8 */
	"dd|SKIP_IF_PARENT_IS",
	"dd|SKIP_IF_UNK3_IS",
	"ddddb|DRAW",
	"|CLEAR_PATHFIND_ARRAY",
	/* 12 */
	"b|DELAY",
	"d|SET_SPRITE_OFFSET_X",
	"d|SET_SPRITE_OFFSET_Y",
	"d|IDENT_WAKEUP",
	/* 16 */
	"d|IDENT_SLEEP",
	"dq|SET_PATHFIND_ITEM",
	"i|JUMP_REL",
	"|CHAIN_TO",
	/* 20 */
	"dd|SET_CODE_WORD",
	"i|JUMP_IF_CODE_WORD",
	"dd|SET_SPRITE_PALETTE",
	"d|SET_SPRITE_PRIORITY",
	/* 24 */
	"diid|SET_SPRITE_XY",
	"x|HALT_SPRITE",
	"ddddd|SET_WINDOW",
	"|RESET",
	/* 28 */
	"dddd|DUMMY_28",
	"|STOP_ALL_SOUNDS",
	"d|SET_BASE_DELAY",
	"d|SET_PALETTE_MODE",
	/* 32 */
	"vv|COPY_VAR",
	"|FORCE_UNLOCK",
	"|FORCE_LOCK",
	"dd|VC35",
	/* 36 */
	"dd|SAVELOAD_THING",
	"v|SET_SPRITE_OFFSET_Y",
	"v|SKIP_IF_VAR_ZERO",
	"vd|SET_VAR",
	/* 40 */
	"vd|ADD_VAR",
	"vd|SUB_VAR",
	"vd|DELAY_IF_NOT_EQ",
	"d|SKIP_IF_BIT_CLEAR",
	/* 44 */
	"d|SKIP_IF_BIT_SET",
	"v|SET_SPRITE_X",
	"v|SET_SPRITE_Y",
	"vv|ADD_VAR_F",
	/* 48 */
	"|VC_48",
	"d|SET_BIT",
	"d|CLEAR_BIT",
	"d|CLEAR_HITAREA_BIT_0x40",
	/* 52 */
	"d|PLAY_SOUND",
	"dd|DUMMY_53",
	"ddd|DUMMY_54",
	"ddd|OFFSET_HIT_AREA",
	/* 56 */
	"i|SLEEP_EX",
	"|DUMMY_57",
	"|UNK_58",
	"ddd|KILL_MULTI_SPRITE",
	/* 60 */
	"dd|KILL_SPRITE",
	"ddd|INIT_SPRITE",
	"|FASTFADEOUT",
	"|FASTFADEIN",
	/* 64 */
	"|SKIP_IF_SPEECH_ENDED",
	"|PALETTE_THING_3",
	"|SKIP_IF_NZ",
	"|SKIP_IF_GE",
	/* 68 */
	"|SKIP_IF_LE",
	"dd|PLAY_TRACK",
	"dd|QUEUE_MUSIC",
	"|CHECK_MUSIC_QUEUE",
	/* 72 */
	"dd|PLAY_TRACK_2",
	"bb|SET_MARK",
	"bb|CLEAR_MARK",
};

} // End of namespace Simon

#endif

