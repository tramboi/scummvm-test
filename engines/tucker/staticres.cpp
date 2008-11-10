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

#include "tucker/tucker.h"

namespace Tucker {

const int TuckerEngine::_locationWidthTable[85] = {
	1, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 4, 2, 1,
	1, 2, 1, 2, 4, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 4, 1,
	1, 1, 1, 1, 2, 1, 2, 2, 2, 4, 4, 2, 2, 1, 1, 1, 4, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0
};

const uint8 TuckerEngine::_sprA02LookupTable[88] = {
	0, 6, 2, 8, 1, 0, 6, 0, 2, 2, 2, 1, 2, 0, 1, 1, 6, 0,
	1, 2, 1, 2, 3, 0, 6, 12, 7, 7, 1, 8, 1, 0, 3, 0, 4,
	5, 0, 0, 3, 3, 2, 7, 7, 0, 4, 1, 5, 2, 4, 1, 1, 2, 4,
	3, 1, 0, 2, 3, 4, 1, 1, 5, 3, 3, 1, 5, 3, 0, 1, 0, 0,
	2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const uint8 TuckerEngine::_sprC02LookupTable[100] = {
	0, 0, 6, 20, 3, 3, 15, 5, 9, 6, 7, 8, 8, 6, 3, 6, 13,
	3, 4, 10, 0, 7, 2, 34, 14, 0, 2, 3, 8, 3, 3, 3, 19,
	13, 1, 0, 2, 3, 0, 0, 0, 5, 5, 12, 0, 1, 0, 1, 3, 6,
	7, 6, 0, 7, 5, 1, 2, 6, 3, 4, 9, 18, 0, 12, 0, 2, 10,
	0, 0, 19, 0, 2, 2, 1, 22, 0, 0, 0, 0, 3, 0, 3, 2, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0
};

const uint8 TuckerEngine::_sprC02LookupTable2[100] = {
	0, 0, 1, 3, 1, 2, 3, 2, 2, 1, 1, 5, 2, 1, 1, 3, 3, 1,
	1, 4, 0, 1, 1, 4, 4, 0, 2, 1, 3, 3, 3, 2, 4, 4, 1, 0,
	1, 3, 0, 0, 0, 1, 1, 7, 0, 1, 0, 1, 1, 1, 7, 3, 0, 2,
	1, 1, 1, 2, 1, 1, 2, 3, 0, 5, 0, 1, 5, 0, 1, 4, 0, 1,
	1, 1, 1, 0, 0, 0, 0, 1, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 0
};

const int TuckerEngine::_staticData3Table[1600] = {
	0x014, 0x014, 0x015, 0x016, 0x017, 0x017, 0x016, 0x015, 0x3E7, 0x0A9, 0x0A9, 0x0AA, 0x0AA, 0x0AB, 0x0AB, 0x0AC,
	0x0AC, 0x3E7, 0x05E, 0x05F, 0x060, 0x061, 0x3E7, 0x0AD, 0x0AE, 0x0AF, 0x0B0, 0x3E7, 0x052, 0x053, 0x054, 0x055,
	0x3E7, 0x056, 0x057, 0x058, 0x059, 0x3E7, 0x05A, 0x05B, 0x05C, 0x05D, 0x3E7, 0x062, 0x063, 0x064, 0x065, 0x3E7,
	0x066, 0x067, 0x068, 0x069, 0x3E7, 0x00C, 0x00D, 0x00E, 0x00F, 0x3E7, 0x01A, 0x01B, 0x01C, 0x01D, 0x3E7, 0x01E,
	0x01F, 0x020, 0x021, 0x3E7, 0x024, 0x025, 0x026, 0x027, 0x3E7, 0x0B1, 0x0B2, 0x0B3, 0x0B4, 0x3E7, 0x0CB, 0x0CC,
	0x0CD, 0x0CE, 0x3E7, 0x0CF, 0x0D0, 0x0D1, 0x0D2, 0x3E7, 0x0D3, 0x0D4, 0x0D5, 0x0D6, 0x3E7, 0x0D7, 0x0D8, 0x0D9,
	0x0DA, 0x3E7, 0x0B5, 0x0B6, 0x0B7, 0x0B8, 0x3E7, 0x04A, 0x04B, 0x04C, 0x04D, 0x3E7, 0x04E, 0x04F, 0x050, 0x051,
	0x3E7, 0x02A, 0x02B, 0x02C, 0x02D, 0x3E7, 0x02E, 0x02F, 0x030, 0x031, 0x3E6, 0x0E6, 0x001, 0x3E7, 0x0B9, 0x0BA,
	0x0BB, 0x0BC, 0x3E7, 0x06A, 0x06B, 0x06C, 0x06D, 0x3E7, 0x032, 0x033, 0x034, 0x035, 0x3E7, 0x036, 0x037, 0x038,
	0x039, 0x3E7, 0x03A, 0x03B, 0x03C, 0x03D, 0x3E7, 0x03E, 0x03F, 0x040, 0x041, 0x3E7, 0x042, 0x043, 0x044, 0x045,
	0x3E7, 0x046, 0x047, 0x048, 0x049, 0x3E7, 0x06E, 0x06F, 0x070, 0x071, 0x3E6, 0x045, 0x000, 0x3E7, 0x072, 0x073,
	0x074, 0x075, 0x3E7, 0x076, 0x077, 0x078, 0x079, 0x3E7, 0x07A, 0x07B, 0x07C, 0x07D, 0x3E7, 0x07F, 0x080, 0x081,
	0x082, 0x3E7, 0x085, 0x086, 0x087, 0x088, 0x3E7, 0x089, 0x08A, 0x08B, 0x08C, 0x3E7, 0x08F, 0x090, 0x091, 0x092,
	0x3E7, 0x0BD, 0x0BE, 0x0BF, 0x0C0, 0x3E7, 0x0C1, 0x0C2, 0x0C3, 0x0C4, 0x3E7, 0x0C5, 0x0C6, 0x0C7, 0x0C8, 0x3E7,
	0x093, 0x094, 0x095, 0x096, 0x3E7, 0x099, 0x099, 0x099, 0x09A, 0x09A, 0x09A, 0x09B, 0x09B, 0x09B, 0x09C, 0x09C,
	0x09C, 0x3E7, 0x09D, 0x09D, 0x09E, 0x09E, 0x09F, 0x09F, 0x0A0, 0x0A0, 0x3E7, 0x0A1, 0x0A1, 0x0A2, 0x0A2, 0x0A3,
	0x0A3, 0x0A4, 0x0A4, 0x3E7, 0x0A5, 0x0A5, 0x0A6, 0x0A6, 0x0A7, 0x0A7, 0x0A8, 0x0A8, 0x3E7, 0x0DE, 0x0DF, 0x0E0,
	0x0E1, 0x3E7, 0x010, 0x011, 0x012, 0x013, 0x3E7, 0x0E3, 0x0E4, 0x0E5, 0x0E6, 0x3E6, 0x03F, 0x000, 0x3E7, 0x000,
	0x001, 0x002, 0x003, 0x3E7, 0x004, 0x005, 0x006, 0x007, 0x3E7, 0x008, 0x009, 0x00A, 0x00B, 0x3E7, 0x00C, 0x00D,
	0x00E, 0x00F, 0x3E7, 0x010, 0x011, 0x012, 0x013, 0x3E7, 0x014, 0x015, 0x016, 0x017, 0x3E7, 0x018, 0x019, 0x01A,
	0x01B, 0x3E7, 0x01D, 0x01E, 0x01F, 0x020, 0x3E7, 0x021, 0x022, 0x023, 0x024, 0x3E7, 0x025, 0x026, 0x027, 0x028,
	0x3E7, 0x029, 0x02A, 0x02B, 0x02C, 0x3E7, 0x02D, 0x02E, 0x02F, 0x030, 0x3E7, 0x031, 0x032, 0x033, 0x034, 0x3E7,
	0x035, 0x036, 0x037, 0x038, 0x3E7, 0x039, 0x03A, 0x03B, 0x03C, 0x3E7, 0x03D, 0x03E, 0x03F, 0x040, 0x3E7, 0x041,
	0x042, 0x043, 0x044, 0x3E7, 0x049, 0x04A, 0x04B, 0x04C, 0x3E7, 0x04D, 0x04E, 0x04F, 0x050, 0x3E7, 0x051, 0x052,
	0x053, 0x054, 0x3E7, 0x055, 0x056, 0x057, 0x058, 0x3E7, 0x059, 0x05A, 0x05B, 0x05C, 0x3E7, 0x05D, 0x05E, 0x05F,
	0x060, 0x3E7, 0x061, 0x062, 0x063, 0x064, 0x3E7, 0x068, 0x069, 0x06A, 0x06B, 0x3E7, 0x06C, 0x06D, 0x06E, 0x06F,
	0x3E7, 0x070, 0x071, 0x072, 0x073, 0x3E7, 0x074, 0x075, 0x076, 0x077, 0x3E7, 0x07A, 0x07B, 0x07C, 0x07D, 0x3E7,
	0x07E, 0x07F, 0x080, 0x081, 0x3E7, 0x082, 0x083, 0x084, 0x085, 0x3E7, 0x086, 0x087, 0x088, 0x089, 0x3E7, 0x08A,
	0x08B, 0x08C, 0x08D, 0x3E7, 0x08E, 0x08E, 0x08F, 0x08F, 0x090, 0x090, 0x091, 0x091, 0x3E7, 0x000, 0x001, 0x002,
	0x003, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x3E7,
	0x004, 0x005, 0x006, 0x007, 0x3E7, 0x008, 0x009, 0x00A, 0x00B, 0x3E7, 0x00C, 0x00D, 0x00E, 0x00F, 0x3E7, 0x010,
	0x011, 0x012, 0x013, 0x3E7, 0x014, 0x015, 0x016, 0x017, 0x3E7, 0x018, 0x019, 0x01A, 0x01B, 0x01C, 0x01D, 0x01E,
	0x01F, 0x020, 0x021, 0x022, 0x023, 0x024, 0x025, 0x026, 0x027, 0x028, 0x029, 0x02A, 0x02B, 0x02C, 0x02D, 0x02E,
	0x02F, 0x030, 0x031, 0x032, 0x3E7, 0x033, 0x034, 0x035, 0x3E7, 0x036, 0x037, 0x038, 0x3E7, 0x039, 0x03A, 0x03B,
	0x3E6, 0x091, 0x003, 0x3E7, 0x03B, 0x03A, 0x039, 0x3E6, 0x091, 0x000, 0x3E7, 0x03C, 0x03D, 0x03E, 0x03F, 0x040,
	0x041, 0x042, 0x043, 0x044, 0x3E6, 0x06E, 0x000, 0x3E7, 0x045, 0x046, 0x047, 0x048, 0x049, 0x04A, 0x04B, 0x04C,
	0x04D, 0x04E, 0x3E6, 0x068, 0x002, 0x3E7, 0x04F, 0x050, 0x051, 0x052, 0x3E7, 0x052, 0x051, 0x050, 0x04F, 0x3E7,
	0x053, 0x054, 0x055, 0x056, 0x3E7, 0x057, 0x058, 0x059, 0x05A, 0x05B, 0x05C, 0x05D, 0x05E, 0x05F, 0x060, 0x061,
	0x062, 0x063, 0x064, 0x065, 0x3E7, 0x066, 0x067, 0x068, 0x069, 0x06A, 0x06B, 0x06C, 0x06D, 0x06E, 0x06F, 0x070,
	0x071, 0x072, 0x073, 0x074, 0x075, 0x076, 0x3E7, 0x077, 0x078, 0x079, 0x07A, 0x07B, 0x07C, 0x07D, 0x3E6, 0x069,
	0x001, 0x3E7, 0x07D, 0x07C, 0x07B, 0x07A, 0x079, 0x078, 0x077, 0x3E6, 0x069, 0x000, 0x3E7, 0x07E, 0x07F, 0x080,
	0x081, 0x082, 0x083, 0x084, 0x3E6, 0x06A, 0x001, 0x3E7, 0x084, 0x083, 0x082, 0x081, 0x080, 0x07F, 0x07E, 0x3E6,
	0x06A, 0x000, 0x3E7, 0x085, 0x086, 0x087, 0x088, 0x089, 0x08A, 0x08B, 0x08C, 0x08D, 0x08E, 0x08F, 0x090, 0x091,
	0x092, 0x093, 0x094, 0x095, 0x096, 0x097, 0x098, 0x099, 0x09A, 0x09B, 0x09C, 0x09D, 0x09E, 0x09F, 0x0A0, 0x0A1,
	0x0A2, 0x0A3, 0x0A4, 0x0A5, 0x0A6, 0x0A7, 0x0A8, 0x0A9, 0x3E7, 0x0AA, 0x0AB, 0x0AC, 0x0AD, 0x0AE, 0x0AF, 0x0B0,
	0x0B1, 0x0B2, 0x0B3, 0x3E6, 0x06B, 0x000, 0x3E7, 0x0B3, 0x3E7, 0x0B4, 0x0B4, 0x0B5, 0x0B5, 0x0B6, 0x0B6, 0x0B7,
	0x0B7, 0x3E7, 0x0B8, 0x0B9, 0x0B9, 0x0BB, 0x0BC, 0x0BD, 0x0BE, 0x0BF, 0x3E7, 0x0C0, 0x0C1, 0x0C2, 0x0C3, 0x0C4,
	0x0C5, 0x0C6, 0x0C7, 0x0C8, 0x0C9, 0x0CA, 0x0CB, 0x0CC, 0x0CD, 0x0CE, 0x0CF, 0x0D0, 0x0D1, 0x0D2, 0x0D3, 0x0D4,
	0x0D5, 0x0D6, 0x0D7, 0x0D8, 0x3E7, 0x0D9, 0x0DA, 0x0DB, 0x0DC, 0x0DD, 0x0DE, 0x0DF, 0x0E0, 0x0E1, 0x0E2, 0x0E3,
	0x0E4, 0x0E5, 0x0E6, 0x0E7, 0x0E8, 0x0E9, 0x3E7, 0x1C5, 0x0EA, 0x0EB, 0x0EC, 0x0ED, 0x0EE, 0x0EF, 0x0F0, 0x0F1,
	0x0F2, 0x3E7, 0x0F3, 0x0F4, 0x0F5, 0x0F6, 0x0F7, 0x0F8, 0x0F9, 0x0FA, 0x3E6, 0x074, 0x001, 0x3E7, 0x0FB, 0x0FC,
	0x0FD, 0x0FE, 0x3E7, 0x0FF, 0x100, 0x101, 0x102, 0x3E7, 0x103, 0x104, 0x105, 0x106, 0x107, 0x108, 0x109, 0x10A,
	0x10B, 0x10C, 0x10D, 0x10E, 0x10F, 0x110, 0x111, 0x112, 0x113, 0x114, 0x115, 0x116, 0x3E7, 0x116, 0x115, 0x114,
	0x113, 0x112, 0x111, 0x110, 0x10F, 0x10E, 0x3E7, 0x117, 0x118, 0x119, 0x11A, 0x3E7, 0x11B, 0x11C, 0x11D, 0x11E,
	0x3E7, 0x11F, 0x120, 0x121, 0x122, 0x3E7, 0x123, 0x124, 0x125, 0x3E6, 0x091, 0x003, 0x3E7, 0x125, 0x124, 0x123,
	0x3E7, 0x126, 0x127, 0x128, 0x3E7, 0x128, 0x127, 0x126, 0x3E7, 0x129, 0x12A, 0x12B, 0x12C, 0x12D, 0x12E, 0x12F,
	0x130, 0x131, 0x132, 0x133, 0x134, 0x135, 0x136, 0x137, 0x138, 0x139, 0x13A, 0x13B, 0x13C, 0x13D, 0x13E, 0x13F,
	0x140, 0x141, 0x142, 0x143, 0x144, 0x145, 0x146, 0x147, 0x148, 0x149, 0x14A, 0x14B, 0x14C, 0x14D, 0x14E, 0x14F,
	0x150, 0x151, 0x152, 0x3E7, 0x153, 0x154, 0x155, 0x156, 0x157, 0x159, 0x15A, 0x15B, 0x15C, 0x3E7, 0x15E, 0x15F,
	0x160, 0x161, 0x162, 0x163, 0x164, 0x165, 0x166, 0x167, 0x168, 0x169, 0x16A, 0x16B, 0x16C, 0x16D, 0x16E, 0x16F,
	0x170, 0x171, 0x172, 0x3E7, 0x173, 0x173, 0x173, 0x173, 0x173, 0x173, 0x173, 0x174, 0x174, 0x174, 0x174, 0x174,
	0x174, 0x3E7, 0x175, 0x175, 0x175, 0x175, 0x175, 0x175, 0x176, 0x3E7, 0x177, 0x178, 0x179, 0x17A, 0x17B, 0x17C,
	0x17D, 0x17E, 0x17F, 0x180, 0x181, 0x182, 0x183, 0x184, 0x185, 0x186, 0x187, 0x3E7, 0x188, 0x189, 0x18A, 0x18B,
	0x18C, 0x3E7, 0x198, 0x199, 0x19A, 0x19B, 0x19C, 0x19D, 0x19E, 0x19F, 0x1A0, 0x1A1, 0x1A2, 0x1A3, 0x1A4, 0x1A5,
	0x1A6, 0x1A7, 0x1A8, 0x1A9, 0x1AA, 0x1AB, 0x1AC, 0x1AD, 0x1AE, 0x1AF, 0x3E7, 0x1B0, 0x1B1, 0x1B2, 0x3E7, 0x1B3,
	0x1B4, 0x1B5, 0x1B6, 0x1B7, 0x1B8, 0x1B9, 0x1BA, 0x3E7, 0x1BB, 0x1BC, 0x1BD, 0x1BE, 0x1BF, 0x1C0, 0x1C1, 0x1C2,
	0x1C3, 0x1C4, 0x3E6, 0x06F, 0x000, 0x3E7, 0x098, 0x099, 0x09A, 0x3E7, 0x09A, 0x099, 0x098, 0x3E7, 0x09D, 0x09E,
	0x09F, 0x3E7, 0x09F, 0x09E, 0x09D, 0x3E7, 0x0A1, 0x0A2, 0x0A3, 0x3E7, 0x0A6, 0x0A7, 0x0A8, 0x0A9, 0x0AA, 0x0AB,
	0x0AC, 0x0AD, 0x3E6, 0x08E, 0x002, 0x3E7, 0x0AE, 0x0AF, 0x0B0, 0x0B1, 0x0B2, 0x0B3, 0x0B4, 0x0B5, 0x0B6, 0x0B7,
	0x0B8, 0x0B9, 0x0BA, 0x0BB, 0x0BC, 0x0BD, 0x0BE, 0x3E7, 0x0F2, 0x0F2, 0x0F3, 0x0F3, 0x0F4, 0x0F4, 0x3E6, 0x030,
	0x002, 0x3E7, 0x1D3, 0x1D4, 0x1D5, 0x1D6, 0x1D7, 0x1D8, 0x3E7, 0x0F6, 0x0F7, 0x0F8, 0x0F8, 0x3E7, 0x0FB, 0x0FC,
	0x0FD, 0x0FE, 0x0FF, 0x100, 0x101, 0x102, 0x103, 0x104, 0x105, 0x3E6, 0x07F, 0x002, 0x3E7, 0x106, 0x107, 0x108,
	0x109, 0x10A, 0x10B, 0x10C, 0x10D, 0x10E, 0x10F, 0x110, 0x111, 0x3E7, 0x1E8, 0x1E7, 0x1E6, 0x1E5, 0x1E4, 0x1E3,
	0x1E2, 0x3E6, 0x095, 0x002, 0x3E7, 0x1A7, 0x1A8, 0x3E7, 0x1A9, 0x1AA, 0x1AB, 0x1AC, 0x1AD, 0x1A9, 0x1AA, 0x1AB,
	0x1AC, 0x3E6, 0x09D, 0x003, 0x3E7, 0x1A0, 0x1A1, 0x3E7, 0x0EA, 0x0EB, 0x0EC, 0x0ED, 0x0EE, 0x0EF, 0x0F0, 0x0F1,
	0x0F2, 0x0F3, 0x0F4, 0x0F5, 0x3E7, 0x0F6, 0x0F7, 0x0F8, 0x0F9, 0x0FA, 0x0FB, 0x0FC, 0x0FD, 0x0FE, 0x0FF, 0x100,
	0x101, 0x3E7, 0x102, 0x103, 0x104, 0x105, 0x106, 0x107, 0x108, 0x109, 0x10A, 0x10B, 0x10C, 0x10D, 0x3E7, 0x10E,
	0x10F, 0x110, 0x111, 0x3E7, 0x112, 0x113, 0x114, 0x115, 0x3E7, 0x116, 0x117, 0x3E7, 0x118, 0x119, 0x11A, 0x11B,
	0x3E7, 0x11C, 0x11D, 0x3E7, 0x11E, 0x11F, 0x120, 0x121, 0x122, 0x123, 0x124, 0x125, 0x126, 0x127, 0x128, 0x129,
	0x3E7, 0x12A, 0x12B, 0x12C, 0x12D, 0x12E, 0x12F, 0x130, 0x131, 0x132, 0x133, 0x134, 0x135, 0x3E7, 0x136, 0x137,
	0x138, 0x139, 0x13A, 0x13B, 0x13C, 0x13D, 0x13E, 0x13F, 0x140, 0x141, 0x3E7, 0x106, 0x107, 0x108, 0x109, 0x10A,
	0x10B, 0x10C, 0x10D, 0x10E, 0x10F, 0x110, 0x111, 0x3E7, 0x1A2, 0x1A3, 0x1A4, 0x1A2, 0x1A3, 0x1A4, 0x1A2, 0x1A3,
	0x1A4, 0x1A2, 0x1A3, 0x1A4, 0x3E6, 0x09D, 0x005, 0x3E7, 0x3E7, 0x3E7, 0x3E7, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000
};

const int TuckerEngine::_instructionIdTable[] = {
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'l', 'm',
	'n', 'o', 'p', 'r', 's', 't', 'v', 'w', 'x', '+'
};

int TuckerEngine::_locationHeightTable[80] = {
	0x00, 0x1C, 0x3C, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x3C, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

int TuckerEngine::_objectKeysPosXTable[80] = {
	0x000, 0x0A0, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x12B, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x140, 0x000, 0x000, 0x000, 0x000, 0x09E, 0x060, 0x0C0, 0x040, 0x0A0, 0x12C, 0x068, 0x098,
	0x08E, 0x09A, 0x0A0, 0x098, 0x092, 0x096, 0x09A, 0x09A, 0x08C, 0x000, 0x000, 0x000, 0x000, 0x000, 0x0A0, 0x000,
	0x086, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x0A0, 0x185, 0x000, 0x0A0, 0x140, 0x140, 0x000,
	0x000, 0x124, 0x140, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
};

int TuckerEngine::_objectKeysPosYTable[80] = {
	0x000, 0x06B, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x080, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x075, 0x000, 0x000, 0x000, 0x000, 0x086, 0x02B, 0x079, 0x07C, 0x07C, 0x07B, 0x073, 0x07B,
	0x06C, 0x08A, 0x086, 0x086, 0x086, 0x086, 0x083, 0x083, 0x07B, 0x000, 0x000, 0x000, 0x000, 0x000, 0x078, 0x000,
	0x082, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x089, 0x08A, 0x000, 0x088, 0x082, 0x076, 0x000,
	0x000, 0x07F, 0x083, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
};

int TuckerEngine::_objectKeysLocationTable[80] = {
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x001, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x001, 0x000, 0x000, 0x000, 0x000, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001,
	0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x000, 0x000, 0x000, 0x000, 0x000, 0x001, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x001, 0x000, 0x000, 0x001, 0x001, 0x001, 0x000,
	0x000, 0x001, 0x001, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
};

int TuckerEngine::_mapSequenceFlagsLocationTable[70] = {
	0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
	1, 1, 1, 1, 0, 0,
};

const uint8 TuckerEngine::_charWidthCharset1[224] = {
	0x06, 0x06, 0x04, 0x06, 0x07, 0x08, 0x08, 0x02, 0x04, 0x04, 0x06, 0x06, 0x06, 0x06, 0x06, 0x04,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x06, 0x06, 0x06, 0x06, 0x06, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x07, 0x07, 0x08, 0x04, 0x04, 0x04, 0x04, 0x07,
	0x06, 0x07, 0x07, 0x07, 0x07, 0x07, 0x05, 0x07, 0x07, 0x03, 0x05, 0x07, 0x04, 0x08, 0x07, 0x07,
	0x07, 0x08, 0x07, 0x07, 0x04, 0x07, 0x07, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x06, 0x07, 0x04, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08,
	0x08, 0x04, 0x08, 0x08, 0x07, 0x07, 0x06, 0x05, 0x07, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x00, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07
};

const uint8 TuckerEngine::_charWidthCharset2[58] = {
	0x13, 0x0F, 0x10, 0x10, 0x10, 0x0E, 0x11, 0x10, 0x0D, 0x0A, 0x11, 0x0D, 0x14, 0x13, 0x13, 0x11,
	0x13, 0x12, 0x10, 0x11, 0x13, 0x14, 0x14, 0x10, 0x13, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x13, 0x0F, 0x10, 0x10, 0x10, 0x0E, 0x11, 0x10, 0x0D, 0x0A, 0x11, 0x0D, 0x14, 0x13, 0x13, 0x11,
	0x13, 0x12, 0x10, 0x11, 0x13, 0x14, 0x14, 0x10, 0x13, 0x10,
};

} // namespace Tucker
