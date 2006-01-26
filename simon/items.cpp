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
 * $Header$
 *
 */

// Item script opcodes for Simon1/Simon2

#include "common/stdafx.h"
#include "simon/simon.h"
#include "simon/intern.h"

#include "common/system.h"

#ifdef _WIN32_WCE
extern bool isSmartphone(void);
#endif

namespace Simon {

int SimonEngine::runScript() {
	byte opcode;
	bool flag, condition;

	do {
		if (_continousMainScript)
			dumpOpcode(_codePtr);

		opcode = getByte();
		if (opcode == 0xFF)
			return 0;

		if (_runScriptReturn1)
			return 1;

		/* Invert condition? */
		flag = false;
		if (opcode == 0) {
			flag = true;
			opcode = getByte();
			if (opcode == 0xFF)
				return 0;
		}

		condition = true;

		switch (opcode) {
		case 1:{										/* ptrA parent is */
				condition = (getItem1Ptr()->parent == getNextItemID());
			}
			break;

		case 2:{										/* ptrA parent is not */
				condition = (getItem1Ptr()->parent != getNextItemID());
			}
			break;

		case 5:{										/* parent is 1 */
				condition = (getNextItemPtr()->parent == getItem1ID());
			}
			break;

		case 6:{										/* parent isnot 1 */
				condition = (getNextItemPtr()->parent != getItem1ID());
			}
			break;

		case 7:{										/* parent is */
				Item *item = getNextItemPtr();
				condition = (item->parent == getNextItemID());
			}
			break;

		case 11:{									/* is zero */
				condition = (getNextVarContents() == 0);
			}
			break;

		case 12:{									/* isnot zero */
				condition = (getNextVarContents() != 0);
			}
			break;

		case 13:{									/* equal */
				uint tmp = getNextVarContents();
				condition = (tmp == getVarOrWord());
			}
			break;

		case 14:{									/* not equal */
				uint tmp = getNextVarContents();
				condition = (tmp != getVarOrWord());
			}
			break;

		case 15:{									/* is greater */
				uint tmp = getNextVarContents();
				condition = (tmp > getVarOrWord());
			}
			break;

		case 16:{									/* is less */
				uint tmp = getNextVarContents();
				condition = (tmp < getVarOrWord());
			}
			break;

		case 17:{									/* is eq f */
				uint tmp = getNextVarContents();
				condition = (tmp == getNextVarContents());
			}
			break;

		case 18:{									/* is not equal f */
				uint tmp = getNextVarContents();
				condition = (tmp != getNextVarContents());
			}
			break;

		case 19:{									/* is greater f */
				uint tmp = getNextVarContents();
				condition = (tmp < getNextVarContents());
			}
			break;

		case 20:{									/* is less f */
				uint tmp = getNextVarContents();
				condition = (tmp > getNextVarContents());
			}
			break;

		case 23:{
				condition = o_unk_23(getVarOrWord());
			}
			break;

		case 25:{									/* has child of type 1 */
				condition = hasChildOfType1(getNextItemPtr());
			}
			break;

		case 26:{									/* has child of type 2 */
				condition = hasChildOfType2(getNextItemPtr());
			}
			break;

		case 27:{									/* item state is */
				Item *item = getNextItemPtr();
				condition = ((uint) item->state == getVarOrWord());
			}
			break;

		case 28:{									/* item has prop */
				Child2 *child = (Child2 *)findChildOfType(getNextItemPtr(), 2);
				byte num = getVarOrByte();
				condition = child != NULL && (child->avail_props & (1 << num)) != 0;
			} break;

		case 31:{									/* set no parent */
				setItemParent(getNextItemPtr(), NULL);
			}
			break;

		case 33:{									/* set item parent */
				Item *item = getNextItemPtr();
				setItemParent(item, getNextItemPtr());
			}
			break;

		case 36:{									/* copy var */
				uint value = getNextVarContents();
				writeNextVarContents(value);
			}
			break;

		case 41:{									/* zero var */
				writeNextVarContents(0);
			}
			break;

		case 42:{									/* set var */
				uint var = getVarOrByte();
				writeVariable(var, getVarOrWord());
			}
			break;

		case 43:{									/* add */
				uint var = getVarOrByte();
				writeVariable(var, readVariable(var) + getVarOrWord());
			}
			break;

		case 44:{									/* sub */
				uint var = getVarOrByte();
				writeVariable(var, readVariable(var) - getVarOrWord());
			}
			break;

		case 45:{									/* add f */
				uint var = getVarOrByte();
				writeVariable(var, readVariable(var) + getNextVarContents());
			}
			break;

		case 46:{									/* sub f */
				uint var = getVarOrByte();
				writeVariable(var, readVariable(var) - getNextVarContents());
			}
			break;

		case 47:{									/* mul */
				uint var = getVarOrByte();
				writeVariable(var, readVariable(var) * getVarOrWord());
			}
			break;

		case 48:{									/* div */
				uint var = getVarOrByte();
				int value = getVarOrWord();
				if (value == 0)
					error("Division by zero in div");
				writeVariable(var, readVariable(var) / value);
			}
			break;

		case 49:{									/* mul f */
				uint var = getVarOrByte();
				writeVariable(var, readVariable(var) * getNextVarContents());
			}
			break;

		case 50:{									/* div f */
				uint var = getVarOrByte();
				int value = getNextVarContents();
				if (value == 0)
					error("Division by zero in div f");
				writeVariable(var, readVariable(var) / value);
			}
			break;

		case 51:{									/* mod */
				uint var = getVarOrByte();
				int value = getVarOrWord();
				if (value == 0)
					error("Division by zero in mod");
				writeVariable(var, readVariable(var) % value);
			}
			break;

		case 52:{									/* mod f */
				uint var = getVarOrByte();
				int value = getNextVarContents();
				if (value == 0)
					error("Division by zero in mod f");
				writeVariable(var, readVariable(var) % value);
			}
			break;

		case 53:{									/* random */
				uint var = getVarOrByte();
				uint value = (uint16)getVarOrWord();
				writeVariable(var, _rnd.getRandomNumber(value - 1));
			}
			break;

		case 55:{									/* set itemA parent */
				setItemParent(getItem1Ptr(), getNextItemPtr());
			}
			break;

		case 56:{									/* set child2 fr bit */
				Child2 *child = (Child2 *)findChildOfType(getNextItemPtr(), 2);
				int value = getVarOrByte();
				if (child != NULL && value >= 0x10)
					child->avail_props |= 1 << value;
			}
			break;

		case 57:{									/* clear child2 fr bit */
				Child2 *child = (Child2 *)findChildOfType(getNextItemPtr(), 2);
				int value = getVarOrByte();
				if (child != NULL && value >= 0x10)
					child->avail_props &= ~(1 << value);
			}
			break;

		case 58:{									/* make siblings */
				Item *item = getNextItemPtr();
				setItemParent(item, derefItem(getNextItemPtr()->parent));
			}
			break;

		case 59:{									/* item inc state */
				Item *item = getNextItemPtr();
				if (item->state <= 30000)
					setItemState(item, item->state + 1);
			}
			break;

		case 60:{									/* item dec state */
				Item *item = getNextItemPtr();
				if (item->state >= 0)
					setItemState(item, item->state - 1);
			}
			break;

		case 61:{									/* item set state */
				Item *item = getNextItemPtr();
				int value = getVarOrWord();
				if (value < 0)
					value = 0;
				if (value > 30000)
					value = 30000;
				setItemState(item, value);
			}
			break;

		case 62:{									/* show int */
				showMessageFormat("%d", getNextVarContents());
			}
			break;

		case 63:{									/* show string nl */
				showMessageFormat("%s\n", getStringPtrByID(getNextStringID()));
			}
			break;

		case 64:{									/* show string */
				showMessageFormat("%s", getStringPtrByID(getNextStringID()));
			}
			break;

		case 65:{									/* add hit area */
				int id = getVarOrWord();
				int x = getVarOrWord();
				int y = getVarOrWord();
				int w = getVarOrWord();
				int h = getVarOrWord();
				int number = getVarOrByte();
				if (number < 20)
					addNewHitArea(id, x, y, w, h, (number << 8) + 129, 0xD0, _dummyItem2);
			}
			break;

		case 66:{									/* set item name */
				uint var = getVarOrByte();
				uint string_id = getNextStringID();
				if (var < 20)
					_stringIdArray2[var] = string_id;
			}
			break;

		case 67:{									/* set item description */
				uint var = getVarOrByte();
				uint string_id = getNextStringID();
				if (getFeatures() & GF_TALKIE) {
					uint speech_id = getNextWord();
					if (var < 20) {
						_stringIdArray3[var] = string_id;
						_speechIdArray4[var] = speech_id;
					}
				} else {
					if (var < 20) {
						_stringIdArray3[var] = string_id;
					}
				}
			}
			break;

		case 68:{									/* exit interpreter */
				shutdown();
			}
			break;

		case 69:{									/* return 1 */
				return 1;
			}

		case 70:{									/* show string from array */
				const char *str = (const char *)getStringPtrByID(_stringIdArray3[getVarOrByte()]);

				if (getGameType() == GType_SIMON2) {
					writeVariable(51, strlen(str) / 53 * 8 + 8);
				}

				showMessageFormat("%s\n", str);
			}
			break;

		case 71:{									/* start subroutine */
				Subroutine *sub = getSubroutineByID(getVarOrWord());
				if (sub != NULL)
					startSubroutine(sub);
			}
			break;

		case 76:{									/* add timeout */
				uint timeout = getVarOrWord();
				addTimeEvent(timeout, getVarOrWord());
			}
			break;

		case 77:{									/* has item minus 1 */
				condition = _subjectItem != NULL;
			}
			break;

		case 78:{									/* has item minus 3 */
				condition = _objectItem != NULL;
			}
			break;

		case 79:{									/* childstruct fr2 is */
				Child2 *child = (Child2 *)findChildOfType(getNextItemPtr(), 2);
				uint string_id = getNextStringID();
				condition = (child != NULL) && child->string_id == string_id;
			}
			break;

		case 80:{									/* item equal */
				condition = getNextItemPtr() == getNextItemPtr();
			}
			break;

		case 82:{									/* dummy opcode */
				getVarOrByte();
			}
			break;

		case 83:{									/* restart subroutine */
				if (getGameType() == GType_SIMON2)
					o_83_helper();
				return -10;
			}

		case 87:{									/* dummy opcode */
				getNextStringID();
			}
			break;

		case 88:{									/* or_lockWord */
				_lockWord |= 0x10;
			}
			break;

		case 89:{									/* and lock word */
				_lockWord &= ~0x10;
			}
			break;

		case 90:{									/* set minusitem to parent */
				Item *item = derefItem(getNextItemPtr()->parent);
				switch (getVarOrByte()) {
				case 0:
					_objectItem = item;
					break;
				case 1:
					_subjectItem = item;
					break;
				default:
					error("set minusitem to parent, invalid subcode");
				}
			}
			break;

		case 91:{									/* set minusitem to sibling */
				Item *item = derefItem(getNextItemPtr()->sibling);
				switch (getVarOrByte()) {
				case 0:
					_objectItem = item;
					break;
				case 1:
					_subjectItem = item;
					break;
				default:
					error("set minusitem to sibling, invalid subcode");
				}
			}
			break;

		case 92:{									/* set minusitem to child */
				Item *item = derefItem(getNextItemPtr()->child);
				switch (getVarOrByte()) {
				case 0:
					_objectItem = item;
					break;
				case 1:
					_subjectItem = item;
					break;
				default:
					error("set minusitem to child, invalid subcode");
				}
			}
			break;

		case 96:{
				uint val = getVarOrWord();
				o_set_video_mode(getVarOrByte(), val);
			}
			break;

		case 97:{									/* load vga */
				ensureVgaResLoadedC(getVarOrWord());
			}
			break;

		case 98:{									/* start vga */
				uint vga_res, vgaSpriteId, windowNum, x, y, palette;
				if (getGameType() == GType_SIMON2) {
					vga_res = getVarOrWord();
					vgaSpriteId = getVarOrWord();
				} else {
					vgaSpriteId = getVarOrWord();
					vga_res = vgaSpriteId / 100;
				}
				windowNum = getVarOrByte();
				x = getVarOrWord();
				y = getVarOrWord();
				palette = getVarOrWord();
				loadSprite(windowNum, vga_res, vgaSpriteId, x, y, palette);
			}
			break;

		case 99:{									/* kill sprite */
				if (getGameType() == GType_SIMON1) {
					o_kill_sprite_simon1(getVarOrWord());
				} else {
					uint a = getVarOrWord();
					uint b = getVarOrWord();
					o_kill_sprite_simon2(a, b);
				}
			}
			break;

		case 100:{									/* vga reset */
				o_vga_reset();
			}
			break;

		case 101:{
				uint a = getVarOrByte();
				uint b = getVarOrWord();
				uint c = getVarOrWord();
				uint d = getVarOrWord();
				uint e = getVarOrWord();
				uint f = getVarOrWord();
				uint g = getVarOrWord();
				o_unk26_helper(a, b, c, d, e, f, g, 0);
			}
			break;

		case 102:{
				fcs_unk_2(getVarOrByte() & 7);
			}
			break;

		case 103:{
				o_unk_103();
			}
			break;

		case 104:{
				fcs_delete(getVarOrByte() & 7);
			}
			break;

		case 107:{									/* add item hitarea */
				uint flags = 0;
				uint id = getVarOrWord();
				uint params = id / 1000;
				uint x, y, w, h, unk3;
				Item *item;

				id = id % 1000;

				if (params & 1)
					flags |= 8;
				if (params & 2)
					flags |= 4;
				if (params & 4)
					flags |= 0x80;
				if (params & 8)
					flags |= 1;
				if (params & 16)
					flags |= 0x10;

				x = getVarOrWord();
				y = getVarOrWord();
				w = getVarOrWord();
				h = getVarOrWord();
				item = getNextItemPtrStrange();
				unk3 = getVarOrWord();
				if (x >= 1000) {
					unk3 += 0x4000;
					x -= 1000;
				}
				addNewHitArea(id, x, y, w, h, flags, unk3, item);
			}
			break;

		case 108:{									/* delete hitarea */
				delete_hitarea(getVarOrWord());
			}
			break;

		case 109:{									/* clear hitarea bit 0x40 */
				clear_hitarea_bit_0x40(getVarOrWord());
			}
			break;

		case 110:{									/* set hitarea bit 0x40 */
				set_hitarea_bit_0x40(getVarOrWord());
			}
			break;

		case 111:{									/* set hitarea xy */
				uint hitarea_id = getVarOrWord();
				uint x = getVarOrWord();
				uint y = getVarOrWord();
				set_hitarea_x_y(hitarea_id, x, y);
			}
			break;

		case 114:{
				Item *item = getNextItemPtr();
				uint fcs_index = getVarOrByte();
				lock();
				fcs_unk_proc_1(fcs_index, item, 0, 0);
				unlock();
			}
			break;

		case 115:{									/* item has flag */
				Item *item = getNextItemPtr();
				condition = (item->classFlags & (1 << getVarOrByte())) != 0;
			}
			break;

		case 116:{									/* item set flag */
				Item *item = getNextItemPtr();
				item->classFlags |= (1 << getVarOrByte());
			}
			break;

		case 117:{									/* item clear flag */
				Item *item = getNextItemPtr();
				item->classFlags &= ~(1 << getVarOrByte());
			}
			break;

		case 119:{									/* wait vga */
				uint var = getVarOrWord();
				_scriptVar2 = (var == 200);

				if (var != 200 || !_skipVgaWait)
					o_wait_for_vga(var);
				_skipVgaWait = false;
			}
			break;

		case 120:{
				o_unk_120(getVarOrWord());
			}
			break;

		case 121:{									/* set vga item */
				uint slot = getVarOrByte();
				_vcItemArray[slot] = getNextItemPtr();
			}
			break;

		case 125:{									/* item is sibling with item 1 */
				Item *item = getNextItemPtr();
				condition = (getItem1Ptr()->parent == item->parent);
			}
			break;

		case 126:{
				Item *item = getNextItemPtr();
				uint fcs_index = getVarOrByte();
				uint a = 1 << getVarOrByte();
				lock();
				fcs_unk_proc_1(fcs_index, item, 1, a);
				unlock();
			}
			break;

		case 127:{									/* deals with music */
				o_play_music_resource();
			}
			break;

		case 128:{									/* dummy instruction */
				getVarOrWord();
			}
			break;

		case 129:{									/* dummy instruction */
				getVarOrWord();
				condition = true;
			}
			break;

		case 130:{									/* set script cond */
				uint var = getVarOrByte();
				getNextWord();
				if (var == 1)
					_scriptCondB = getNextWord();
				else
					_scriptCondC = getNextWord();
			}
			break;

		case 132:{									/* save game */
				_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
				o_save_game();
				_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
			}
			break;

		case 133:{									/* load game */
				_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
				o_load_game();
				_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
			}
			break;

		case 134:{									/* dummy opcode? */
				midi.stop();
				_lastMusicPlayed = -1;
			}
			break;

		case 135:{									/* quit if user presses y */
				_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
				o_quit_if_user_presses_y();
				_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
			}
			break;

		case 136:{									/* set var to item unk3 */
				Item *item = getNextItemPtr();
				writeNextVarContents(item->state);
			}
			break;

		case 137:{
				o_unk_137(getVarOrByte());
			}
			break;

		case 138:{									/* vga pointer op 4 */
				o_unk_138();
			}
			break;

		case 139:{									/* set parent special */
				Item *item = getNextItemPtr();
				_noParentNotify = true;
				setItemParent(item, getNextItemPtr());
				_noParentNotify = false;
			}
			break;

		case 140:{									/* del te and add one */
				killAllTimers();
				addTimeEvent(3, 0xA0);
			}
			break;

		case 141:{									/* set m1 or m3 */
				uint which = getVarOrByte();
				Item *item = getNextItemPtr();
				if (which == 1) {
					_subjectItem = item;
				} else {
					_objectItem = item;
				}
			}
			break;

		case 142:{									/* is hitarea 0x40 clear */
				condition = is_hitarea_0x40_clear(getVarOrWord());
			}
			break;

		case 143:{									/* start item sub */
				Child1 *child = (Child1 *)findChildOfType(getNextItemPtr(), 1);
				if (child != NULL) {
					Subroutine *sub = getSubroutineByID(child->subroutine_id);
					if (sub)
						startSubroutine(sub);
				}
			}
			break;

		case 151:{									/* set array6 to item */
				uint var = getVarOrByte();
				Item *item = getNextItemPtr();
				_itemArray6[var] = item;
			}
			break;

		case 152:{									/* set m1 or m3 to array6 */
				Item *item = _itemArray6[getVarOrByte()];
				uint var = getVarOrByte();
				if (var == 1) {
					_subjectItem = item;
				} else {
					_objectItem = item;
				}
			}
			break;

		case 153:{									/* set bit */
				uint bit = getVarOrByte();
				_bitArray[bit >> 4] |= 1 << (bit & 15);
				break;
			}

		case 154:{									/* clear bit */
				uint bit = getVarOrByte();
				_bitArray[bit >> 4] &= ~(1 << (bit & 15));
				break;
			}

		case 155:{									/* is bit clear? */
				uint bit = getVarOrByte();
				condition = (_bitArray[bit >> 4] & (1 << (bit & 15))) == 0;
			}
			break;

		case 156:{									/* is bit set? */
				uint bit = getVarOrByte();
				if (getGameType() == GType_SIMON1 && _subroutine == 2962 && bit == 63) {
					bit = 50;
				}
				condition = (_bitArray[bit >> 4] & (1 << (bit & 15))) != 0;
			}
			break;

		case 157:{									/* get item int prop */
				Item *item = getNextItemPtr();
				Child2 *child = (Child2 *)findChildOfType(item, 2);
				uint prop = getVarOrByte();

				if (child != NULL && child->avail_props & (1 << prop) && prop < 16) {
					uint offs = getOffsetOfChild2Param(child, 1 << prop);
					writeNextVarContents(child->array[offs]);
				} else {
					writeNextVarContents(0);
				}
			}
			break;

		case 158:{									/* set item prop */
				Item *item = getNextItemPtr();
				Child2 *child = (Child2 *)findChildOfType(item, 2);
				uint prop = getVarOrByte();
				int value = getVarOrWord();

				if (child != NULL && child->avail_props & (1 << prop) && prop < 16) {
					uint offs = getOffsetOfChild2Param(child, 1 << prop);
					child->array[offs] = value;
				}
			}
			break;

		case 160:{
				o_unk_160(getVarOrByte());
			}
			break;

		case 161:{									/* setup text */
				TextLocation *tl = getTextLocation(getVarOrByte());

				tl->x = getVarOrWord();
				tl->y = getVarOrByte();
				tl->width = getVarOrWord();
			}
			break;

		case 162:{									/* print string */
				o_print_str();
			}
			break;

		case 163:{									/* play sound */
				o_play_sound(getVarOrWord());
			}
			break;

		case 164:{
				_showPreposition = true;
				o_setup_cond_c();
				_showPreposition = false;
			}
			break;

		case 165:{									/* item unk1 unk2 is */
				Item *item = getNextItemPtr();
				int16 a = getNextWord(), b = getNextWord();
				condition = (item->adjective == a && item->noun == b);
			} break;

		case 166:{									/* set bit2 */
				uint bit = getVarOrByte();
				_bitArray[(bit >> 4) + 16] |= 1 << (bit & 15);
			}
			break;

		case 167:{									/* clear bit2 */
				uint bit = getVarOrByte();
				_bitArray[(bit >> 4) + 16] &= ~(1 << (bit & 15));
			}
			break;

		case 168:{									/* is bit2 clear */
				uint bit = getVarOrByte();
				condition = (_bitArray[(bit >> 4) + 16] & (1 << (bit & 15))) == 0;
			}
			break;

		case 169:{									/* is bit2 set */
				uint bit = getVarOrByte();
				condition = (_bitArray[(bit >> 4) + 16] & (1 << (bit & 15))) != 0;
			}
			break;

		case 175:{									/* vga pointer op 1 */
				o_unk_175();
			}
			break;

		case 176:{									/* vga pointer op 2 */
				o_unk_176();
			}
			break;

		case 177:{									/* inventory descriptions */
				o_inventory_descriptions();
			}
			break;

		case 178:{									/* path find */
				uint a = getVarOrWord();
				uint b = getVarOrWord();
				uint c = getVarOrByte();
				uint d = getVarOrByte();
				o_pathfind(a, b, c, d);
			}
			break;

		case 179:{									/* conversation responses */
				uint vgaSpriteId = getVarOrByte();				/* and room descriptions */
				uint color = getVarOrByte();
				uint string_id = getVarOrByte();
				uint speech_id = 0;

				const char *string_ptr = (const char *)getStringPtrByID(_stringIdArray3[string_id]);
				TextLocation *tl = getTextLocation(vgaSpriteId);
				if (getFeatures() & GF_TALKIE)
					speech_id = _speechIdArray4[string_id];

				if (_speech && speech_id != 0)
					talk_with_speech(speech_id, vgaSpriteId);
				if (string_ptr != NULL && _subtitles)
					talk_with_text(vgaSpriteId, color, string_ptr, tl->x, tl->y, tl->width);
			}
			break;

		case 180:{									/* force unlock */
				o_force_unlock();
			}
			break;

		case 181:{									/* force lock */
				o_force_lock();
				if (getGameType() == GType_SIMON2) {
					fcs_unk_2(1);
					showMessageFormat("\xC");
				}
			}
			break;

		case 182:{									/* read vgares 328 */
				if (getGameType() == GType_SIMON2)
					goto invalid_opcode;

				o_read_vgares_328();
			}
			break;

		case 183:{									/* read vgares 23 */
				if (getGameType() == GType_SIMON2)
					goto invalid_opcode;

				o_read_vgares_23();
			}
			break;

		case 184:{									/* clear vgapointer entry */
				o_clear_vgapointer_entry(getVarOrWord());
			}
			break;

		case 185:{									/* midi sfx file number */
				if (getGameType() == GType_SIMON2)
					goto invalid_opcode;

				_soundFileId = getVarOrWord();
				if (getPlatform() == Common::kPlatformAmiga && getFeatures() & GF_TALKIE) {
					char buf[10];
					sprintf(buf, "%d%s", _soundFileId, "Effects");
					_sound->readSfxFile(buf);
					sprintf(buf, "%d%s", _soundFileId, "simon");
					_sound->readVoiceFile(buf);
				}

			}
			break;

		case 186:{									/* vga pointer op 3 */
				o_unk_186();
			}
			break;

		case 187:{									/* fade to black */
				if (getGameType() == GType_SIMON2)
					goto invalid_opcode;
				o_fade_to_black();
			}
			break;

		case 188:									/* string2 is */
			if (getGameType() == GType_SIMON1)
				goto invalid_opcode;
			{
				uint i = getVarOrByte();
				uint str = getNextStringID();
				condition = (str < 20 && _stringIdArray2[i] == str);
			}
			break;

		case 189:{									/* clear_op189_flag */
				if (getGameType() == GType_SIMON1)
					goto invalid_opcode;
				_marks = 0;
			}
			break;

		case 190:{
				uint i;
				if (getGameType() == GType_SIMON1)
					goto invalid_opcode;
				i = getVarOrByte();
				if (!(_marks & (1 << i)))
					o_waitForMark(i);
			}
			break;

		default:
		invalid_opcode:;
			error("Invalid opcode '%d'", opcode);
		}

	} while (condition != flag);

	return 0;
}

int SimonEngine::startSubroutine(Subroutine *sub) {
	int result = -1;
	SubroutineLine *sl;
	const byte *old_code_ptr;

	if (_startMainScript)
		dumpSubroutine(sub);

	old_code_ptr = _codePtr;

	if (++_recursionDepth > 40)
		error("Recursion error");

	sl = (SubroutineLine *)((byte *)sub + sub->first);

	while ((byte *)sl != (byte *)sub) {
		if (checkIfToRunSubroutineLine(sl, sub)) {
			result = 0;
			_codePtr = (byte *)sl;
			if (sub->id)
				_codePtr += 2;
			else
				_codePtr += 8;

			if (_continousMainScript)
				fprintf(_dumpFile, "; %d\n", sub->id);
			result = runScript();
			if (result != 0) {
				/* result -10 means restart subroutine */
				if (result == -10) {
					delay(0);							/* maybe leave control to the VGA */
					sl = (SubroutineLine *)((byte *)sub + sub->first);
					continue;
				}
				break;
			}
		}
		sl = (SubroutineLine *)((byte *)sub + sl->next);
	}

	_codePtr = old_code_ptr;

	_recursionDepth--;
	return result;
}

int SimonEngine::startSubroutineEx(Subroutine *sub) {
	return startSubroutine(sub);
}

bool SimonEngine::checkIfToRunSubroutineLine(SubroutineLine *sl, Subroutine *sub) {
	if (sub->id)
		return true;

	if (sl->cond_a != -1 && sl->cond_a != _scriptCondA &&
			(sl->cond_a != -2 || _scriptCondA != -1))
		return false;

	if (sl->cond_b != -1 && sl->cond_b != _scriptCondB &&
			(sl->cond_b != -2 || _scriptCondB != -1))
		return false;

	if (sl->cond_c != -1 && sl->cond_c != _scriptCondC &&
			(sl->cond_c != -2 || _scriptCondC != -1))
		return false;

	return true;
}

void SimonEngine::o_83_helper() {
		if (_exitCutscene) {
			if (vc_get_bit(9)) {
				startSubroutine170();
			}
		} else {
			processSpecialKeys();
		}
}

void SimonEngine::o_waitForMark(uint i) {
	_exitCutscene = false;
	while (!(_marks & (1 << i))) {
		if (_exitCutscene) {
			if (vc_get_bit(9)) {
				startSubroutine170();
				break;
			}
		} else {
			processSpecialKeys();
		}

		delay(10);
	}
}


bool SimonEngine::o_unk_23(uint a) {
	if (a == 0)
		return 0;

	if (a == 100)
		return 1;

	a += _scriptUnk1;
	if (a <= 0) {
		_scriptUnk1 = 0;
		return 0;
	}

	if ((uint)_rnd.getRandomNumber(99) < a) {
		if (_scriptUnk1 <= 0)
			_scriptUnk1 -= 5;
		else
			_scriptUnk1 = 0;
		return 1;
	}

	if (_scriptUnk1 >= 0)
		_scriptUnk1 += 5;
	else
		_scriptUnk1 = 0;

	return 0;
}

void SimonEngine::o_inventory_descriptions() {
	uint vgaSpriteId = getVarOrByte();
	uint color = getVarOrByte();
	const char *string_ptr = NULL;
	TextLocation *tl = NULL;
	char buf[256];

	Child2 *child = (Child2 *)findChildOfType(getNextItemPtr(), 2);
	if (child != NULL && child->avail_props & 1) {
		string_ptr = (const char *)getStringPtrByID(child->array[0]);
		tl = getTextLocation(vgaSpriteId);
	}

	if ((getGameType() == GType_SIMON2) && (getFeatures() & GF_TALKIE)) {
		if (child != NULL && child->avail_props & 0x200) {
			uint speech_id = child->array[getOffsetOfChild2Param(child, 0x200)];

			if (child->avail_props & 0x100) {
				uint speech_id_offs = child->array[getOffsetOfChild2Param(child, 0x100)];

				if (speech_id == 116)
					speech_id = speech_id_offs + 115;
				if (speech_id == 92)
					speech_id = speech_id_offs + 98;
				if (speech_id == 99)
					speech_id = 9;
				if (speech_id == 97) {
					switch (speech_id_offs) {
					case 12:
						speech_id = 109;
						break;
					case 14:
						speech_id = 108;
						break;
					case 18:
						speech_id = 107;
						break;
					case 20:
						speech_id = 106;
						break;
					case 22:
						speech_id = 105;
						break;
					case 28:
						speech_id = 104;
						break;
					case 90:
						speech_id = 103;
						break;
					case 92:
						speech_id = 102;
						break;
					case 100:
						speech_id = 51;
						break;
					default:
						error("o_177: invalid case %d", speech_id_offs);
					}
				}
			}

			if (_speech)
				talk_with_speech(speech_id, vgaSpriteId);
		}

	} else if (getFeatures() & GF_TALKIE) {
		if (child != NULL && child->avail_props & 0x200) {
			uint offs = getOffsetOfChild2Param(child, 0x200);
			talk_with_speech(child->array[offs], vgaSpriteId);
		} else if (child != NULL && child->avail_props & 0x100) {
			uint offs = getOffsetOfChild2Param(child, 0x100);
			talk_with_speech(child->array[offs] + 3550, vgaSpriteId);
		}
	}

	if (child != NULL && (child->avail_props & 1) && _subtitles) {
		if (child->avail_props & 0x100) {
			sprintf(buf, "%d%s", child->array[getOffsetOfChild2Param(child, 0x100)], string_ptr);
			string_ptr = buf;
		}
		if (string_ptr != NULL)
			talk_with_text(vgaSpriteId, color, string_ptr, tl->x, tl->y, tl->width);
	}
}

void SimonEngine::o_quit_if_user_presses_y() {
	// If all else fails, use English as fallback.
	byte keyYes = 'y';
	byte keyNo = 'n';

	switch (_language) {
	case Common::RU_RUS:
		break;
	case Common::PL_POL:
		keyYes = 't';
		break;
	case Common::HB_ISR:
		keyYes = 'f';
		break;
	case Common::ES_ESP:
		keyYes = 's';
		break;
	case Common::IT_ITA:
		keyYes = 's';
		break;
	case Common::FR_FRA:
		keyYes = 'o';
		break;
	case Common::DE_DEU:
		keyYes = 'j';
		break;
	default:
		break;
	}

	for (;;) {
		delay(1);
#ifdef _WIN32_WCE
		if (isSmartphone()) {
			if (_keyPressed) {
				if (_keyPressed == 13)
					shutdown();
				else
					break;
			}
		}
#endif
		if (_keyPressed == keyYes)
			shutdown();
		else if (_keyPressed == keyNo)
			break;
	}
}

void SimonEngine::o_unk_137(uint fcs_index) {
	FillOrCopyStruct *fcs;

	fcs = _fcsPtrArray3[fcs_index & 7];
	if (fcs->fcs_data == NULL)
		return;
	fcs_unk_proc_1(fcs_index, fcs->fcs_data->item_ptr, fcs->fcs_data->unk1, fcs->fcs_data->unk2);
}

void SimonEngine::o_unk_138() {
	_vgaBufStart = _vgaBufFreeStart;
	_vgaFileBufOrg = _vgaBufFreeStart;
}

void SimonEngine::o_unk_186() {
	_vgaBufFreeStart = _vgaFileBufOrg2;
	_vgaBufStart = _vgaFileBufOrg2;
	_vgaFileBufOrg = _vgaFileBufOrg2;
}

void SimonEngine::o_unk_175() {
	_vgaBufStart = _vgaBufFreeStart;
}

void SimonEngine::o_unk_176() {
	_vgaBufFreeStart = _vgaFileBufOrg;
	_vgaBufStart = _vgaFileBufOrg;
}

int SimonEngine::o_unk_132_helper(bool *b, char *buf) {
	HitArea *ha;
	*b = true;

	if (!_saveLoadFlag) {
	strange_jump:;
		_saveLoadFlag = false;
		savegame_dialog(buf);
	}

start_over:;
	_keyPressed = 0;

start_over_2:;
	_lastHitArea = _lastHitArea3 = 0;

	do {
		if (_keyPressed != 0) {
			if (_saveLoadFlag) {
				*b = false;
				return _keyPressed;
			}
			goto start_over;
		}
		delay(100);
	} while (_lastHitArea3 == 0);

	ha = _lastHitArea;

	if (ha == NULL || ha->id < 205)
		goto start_over_2;

	if (ha->id == 205)
		return ha->id;

	if (ha->id == 206) {
		if (_saveLoadRowCurPos == 1)
			goto start_over_2;
		if (_saveLoadRowCurPos < 7)
			_saveLoadRowCurPos = 1;
		else
			_saveLoadRowCurPos -= 6;

		goto strange_jump;
	}

	if (ha->id == 207) {
		if (!_saveDialogFlag)
			goto start_over_2;
		_saveLoadRowCurPos += 6;
		if (_saveLoadRowCurPos >= _numSaveGameRows)
			_saveLoadRowCurPos = _numSaveGameRows;
		goto strange_jump;
	}

	if (ha->id >= 214)
		goto start_over_2;
	return ha->id - 208;
}

void SimonEngine::o_unk_132_helper_3() {
	for (int i = 208; i != 208 + 6; i++)
		set_hitarea_bit_0x40(i);
}

void SimonEngine::o_clear_character(FillOrCopyStruct *fcs, int x, byte b) {
	byte old_text;

	video_putchar(fcs, x, b);
	old_text = fcs->text_color;
	fcs->text_color = fcs->fill_color;

	if (_language == 20) { //Hebrew
		x = 128;
	} else {
		x += 120;
		if (x != 128)
			x = 129;

	}

	video_putchar(fcs, x);

	fcs->text_color = old_text;
	video_putchar(fcs, 8);
}

void SimonEngine::o_play_music_resource() {
	int music = getVarOrWord();
	int track = getVarOrWord();

	// Jamieson630:
	// This appears to be a "load or play music" command.
	// The music resource is specified, and optionally
	// a track as well. Normally we see two calls being
	// made, one to load the resource and another to
	// actually start a track (so the resource is
	// effectively preloaded so there's no latency when
	// starting playback).
	if (getGameType() == GType_SIMON2) {
		int loop = getVarOrByte();

		midi.setLoop (loop != 0);
		if (_lastMusicPlayed != music)
			_nextMusicToPlay = music;
		else
			midi.startTrack (track);
	} else {
		if (music != _lastMusicPlayed) {
			_lastMusicPlayed = music;
			loadMusic (music);
			midi.startTrack (track);
		}
	}
}

void SimonEngine::o_unk_120(uint a) {
	uint16 id = TO_BE_16(a);
	_lockWord |= 0x8000;
	_vcPtr = (byte *)&id;
	vc15_wakeup_id();
	_lockWord &= ~0x8000;
}

void SimonEngine::o_play_sound(uint sound_id) {
	if (getGameId() == GID_SIMON1DOS)
		playSting(sound_id);
	else
		_sound->playEffects(sound_id);
}

void SimonEngine::o_unk_160(uint a) {
	fcs_setTextColor(_fcsPtrArray3[_fcsUnk1], a);
}

void SimonEngine::o_unk_103() {
	lock();
	fcs_unk1(_fcsUnk1);
	showMessageFormat("\x0C");
	unlock();
}

void SimonEngine::o_kill_sprite_simon1(uint a) {
	uint16 b = TO_BE_16(a);
	_lockWord |= 0x4000;
	_vcPtr = (byte *)&b;
	vc60_killSprite();
	_lockWord &= ~0x4000;
}

void SimonEngine::o_kill_sprite_simon2(uint a, uint b) {
	uint16 items[2];

	items[0] = TO_BE_16(a);
	items[1] = TO_BE_16(b);

	_lockWord |= 0x8000;
	_vcPtr = (byte *)&items;
	vc60_killSprite();
	_lockWord &= ~0x8000;
}

/* OK */
void SimonEngine::o_unk26_helper(uint a, uint b, uint c, uint d, uint e, uint f, uint g, uint h) {
	a &= 7;

	if (_fcsPtrArray3[a])
		fcs_delete(a);

	_fcsPtrArray3[a] = fcs_alloc(b, c, d, e, f, g, h);

	if (a == _fcsUnk1) {
		_fcsPtr1 = _fcsPtrArray3[a];
		showmessage_helper_3(_fcsPtr1->textLength, _fcsPtr1->textMaxLength);
	}
}

} // End of namespace Simon
