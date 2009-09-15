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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#include "teenagent/dialog.h"
#include "teenagent/resources.h"
#include "teenagent/scene.h"

namespace TeenAgent {

void Dialog::show(Scene *scene, uint16 addr, uint16 animation1, uint16 animation2, byte color1, byte color2, byte slot1, byte slot2) {
	--slot1;
	--slot2;
	debug(0, "Dialog::pop(%04x, %u:%u, %u:%u)", addr, slot1, animation1, slot2, animation2);
	Resources * res = Resources::instance();
	int n = 0;
	Common::String message;
	byte color = color1;

	if (animation1 != 0) {
		SceneEvent e(SceneEvent::PlayAnimation);
		e.animation = animation1;
		e.lan = 0xc0 | slot1; //looped, paused
		scene->push(e);
	}

	if (animation2 != 0) {
		SceneEvent e(SceneEvent::PlayAnimation);
		e.animation = animation2;
		e.lan = 0xc0 | slot2; //looped, paused
		scene->push(e);
	}

	while (n < 4) {
		byte c = res->eseg.get_byte(addr++);
		//debug(0, "%02x: %c", c, c > 0x20? c: '.');

		switch (c) {
		case 0:
			++n;
			switch (n) {
			case 1:
				//debug(0, "new line\n");
				message += '\n';
				break;
			case 2:
				//debug(0, "displaymessage\n");

				if (color == color2 && animation2 != 0) {
					//pause animation in other slot
					{
						SceneEvent e(SceneEvent::PauseAnimation);
						e.lan = 0x80 | slot1;
						scene->push(e);
					}
					{
						SceneEvent e(SceneEvent::PlayAnimation);
						e.animation = animation2;
						e.lan = 0x80 | slot2;
						scene->push(e);
					}
				} else if (color == color1 && animation1 != 0) {
					//pause animation in other slot
					{
						SceneEvent e(SceneEvent::PauseAnimation);
						e.lan = 0x80 | slot2;
						scene->push(e);
					}
					{
						SceneEvent e(SceneEvent::PlayAnimation);
						e.animation = animation1;
						e.lan = 0x80 | slot1;
						scene->push(e);
					}
				}

				{
					SceneEvent e(SceneEvent::Message);
					e.message = message;
					e.color = color;
					if (animation1 != 0 && color == color1)
						e.lan = slot1;
					if (animation2 != 0 && color == color2)
						e.lan = slot2;
					scene->push(e);
					message.clear();
				}
				break;

			case 3:
				color = color == color1 ? color2 : color1;
				//debug(0, "changing color to %02x", color);
				break;
			}
			break;

		case 0xff: {
			//fixme : wait for the next cycle of the animation
		}
		break;

		default:
			message += c;
			n = 0;
		}
	}

	SceneEvent e(SceneEvent::ClearAnimations);
	scene->push(e);
}

uint16 Dialog::pop(Scene *scene, uint16 addr, uint16 animation1, uint16 animation2, byte color1, byte color2, byte slot1, byte slot2) {
	debug(0, "Dialog::pop(%04x, %u:%u, %u:%u)", addr, slot1, animation1, slot2, animation2);
	Resources * res = Resources::instance();
	uint16 next;
	do {
		next = res->dseg.get_word(addr);
		addr += 2;
	} while (next == 0);
	uint16 next2 = res->dseg.get_word(addr);
	if (next2 != 0xffff)
		res->dseg.set_word(addr - 2, 0);
	show(scene, next, animation1, animation2, color1, color2, slot1, slot2);
	return next;
}

} // End of namespace TeenAgent
