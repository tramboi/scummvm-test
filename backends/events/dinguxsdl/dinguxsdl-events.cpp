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

#if defined(DINGUX)

#include "backends/events/dinguxsdl/dinguxsdl-events.h"
#include "graphics/scaler/aspect.h"	// for aspect2Real 

#define PAD_UP    SDLK_UP
#define PAD_DOWN  SDLK_DOWN
#define PAD_LEFT  SDLK_LEFT
#define PAD_RIGHT SDLK_RIGHT
#define BUT_A     SDLK_LCTRL
#define BUT_B     SDLK_LALT
#define BUT_X     SDLK_SPACE
#define BUT_Y     SDLK_LSHIFT
#define BUT_SELECT   SDLK_ESCAPE
#define BUT_START    SDLK_RETURN
#define TRIG_L    SDLK_TAB
#define TRIG_R    SDLK_BACKSPACE

static int mapKey(SDLKey key, SDLMod mod, Uint16 unicode) {
	if (key >= SDLK_F1 && key <= SDLK_F9) {
		return key - SDLK_F1 + Common::ASCII_F1;
	} else if (key >= SDLK_KP0 && key <= SDLK_KP9) {
		return key - SDLK_KP0 + '0';
	} else if (key >= SDLK_UP && key <= SDLK_PAGEDOWN) {
		return key;
	} else if (unicode) {
		return unicode;
	} else if (key >= 'a' && key <= 'z' && (mod & KMOD_SHIFT)) {
		return key & ~0x20;
	} else if (key >= SDLK_NUMLOCK && key <= SDLK_EURO) {
		return 0;
	}
	return key;
}

DINGUXSdlEventSource::DINGUXSdlEventSource() : SdlEventSource() {
	;
}

bool DINGUXSdlEventSource::remapKey(SDL_Event &ev, Common::Event &event) {
	if (ev.key.keysym.sym == PAD_UP) {
		if (ev.type == SDL_KEYDOWN) {
			_km.y_vel = -1;
			_km.y_down_count = 1;
		} else {
			_km.y_vel = 0;
			_km.y_down_count = 0;
		}

		event.type = Common::EVENT_MOUSEMOVE;
		fillMouseEvent(event, _km.x, _km.y);

		return true;
	} else if (ev.key.keysym.sym == PAD_DOWN) {
		if (ev.type == SDL_KEYDOWN) {
			_km.y_vel = 1;
			_km.y_down_count = 1;
		} else {
			_km.y_vel = 0;
			_km.y_down_count = 0;
		}

		event.type = Common::EVENT_MOUSEMOVE;
		fillMouseEvent(event, _km.x, _km.y);

		return true;
	} else if (ev.key.keysym.sym == PAD_LEFT) {
		if (ev.type == SDL_KEYDOWN) {
			_km.x_vel = -1;
			_km.x_down_count = 1;
		} else {
			_km.x_vel = 0;
			_km.x_down_count = 0;
		}

		event.type = Common::EVENT_MOUSEMOVE;
		fillMouseEvent(event, _km.x, _km.y);

		return true;
	} else if (ev.key.keysym.sym == PAD_RIGHT) {
		if (ev.type == SDL_KEYDOWN) {
			_km.x_vel = 1;
			_km.x_down_count = 1;
		} else {
			_km.x_vel = 0;
			_km.x_down_count = 0;
		}

		event.type = Common::EVENT_MOUSEMOVE;
		fillMouseEvent(event, _km.x, _km.y);

		return true;
	} else if (ev.key.keysym.sym == BUT_Y) { // left mouse button
		if (ev.type == SDL_KEYDOWN) {
			event.type = Common::EVENT_LBUTTONDOWN;
		} else {
			event.type = Common::EVENT_LBUTTONUP;
		}

		fillMouseEvent(event, _km.x, _km.y);

		return true;
	} else if (ev.key.keysym.sym == BUT_B) { // right mouse button
		if (ev.type == SDL_KEYDOWN) {
			event.type = Common::EVENT_RBUTTONDOWN;
		} else {
			event.type = Common::EVENT_RBUTTONUP;
		}

		fillMouseEvent(event, _km.x, _km.y);

		return true;
	} else if (ev.key.keysym.sym == BUT_X) { // '.' skip dialogue
		ev.key.keysym.sym = SDLK_PERIOD;
		ev.key.keysym.mod = (SDLMod)0;
		ev.key.keysym.unicode = '.';
	} else if (ev.key.keysym.sym == TRIG_L) { // global menu
		ev.key.keysym.sym = SDLK_F5;
		event.kbd.keycode = Common::KEYCODE_F5;
		event.kbd.ascii = Common::ASCII_F5;
		event.kbd.flags = Common::KBD_CTRL;

		if (ev.type == SDL_KEYDOWN) {
			event.type = Common::EVENT_KEYDOWN;
		} else {
			event.type = Common::EVENT_KEYUP;
		}

		return true;
	} else if (ev.key.keysym.sym == BUT_A) { // key '0'
		ev.key.keysym.sym = SDLK_0;

		event.kbd.keycode = Common::KEYCODE_0;
		event.kbd.ascii = '0';
		event.kbd.flags = 0;

		if (ev.type == SDL_KEYDOWN) {
			event.type = Common::EVENT_KEYDOWN;
		} else {
			event.type = Common::EVENT_KEYUP;
		}

		return true;
	} else if (ev.key.keysym.sym == BUT_SELECT) { // virtual keyboard
		ev.key.keysym.sym = SDLK_F7;

	} else if (ev.key.keysym.sym == BUT_START) { // F5, menu in some games
		ev.key.keysym.sym = SDLK_F5;

	}  else if (ev.key.keysym.sym == TRIG_R) { // ESC
		ev.key.keysym.sym = SDLK_ESCAPE;
	} else {
		event.kbd.keycode = (Common::KeyCode)ev.key.keysym.sym;
		event.kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);
	}

	return false;
}

void DINGUXSdlEventSource::fillMouseEvent(Common::Event &event, int x, int y) {
	if (_grpMan->getVideoMode()->mode == GFX_HALF && !(_grpMan->isOverlayVisible())) {
		event.mouse.x = x * 2;
		event.mouse.y = y * 2;
	} else {
		event.mouse.x = x;
		event.mouse.y = y;
	}

	// Update the "keyboard mouse" coords
	_km.x = x;
	_km.y = y;

	// Adjust for the screen scaling
	if (!(_grpMan->isOverlayVisible())) {
		event.mouse.x /= (_grpMan->getVideoMode())->scaleFactor;
		event.mouse.y /= (_grpMan->getVideoMode())->scaleFactor;
#if 0
		if (_grpMan->getVideoMode()->aspectRatioCorrection)
			event.mouse.y = aspect2Real(event.mouse.y);
#endif
	}
}

void DINGUXSdlEventSource::warpMouse(int x, int y) {
	int mouse_cur_x = _grpMan->getMouseCurState()->x;
	int mouse_cur_y = _grpMan->getMouseCurState()->y;

	if ((mouse_cur_x != x) || (mouse_cur_y != y)) {
		if (_grpMan->getVideoMode()->mode == GFX_HALF && !(_grpMan->isOverlayVisible())) {
			x = x / 2;
			y = y / 2;
		}
	}
	SDL_WarpMouse(x, y);
}

void DINGUXSdlEventSource::setCurrentGraphMan(DINGUXSdlGraphicsManager *_graphicManager) {
	_grpMan = _graphicManager;
}

#endif /* DINGUX */
