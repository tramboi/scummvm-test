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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/backends/platform/sdl/events.cpp $
 * $Id: events.cpp 43636 2009-08-22 12:35:49Z sev $
 *
 */

#include "backends/platform/samsungtv/samsungtv.h"
#include "common/util.h"
#include "common/events.h"

#if defined(SAMSUNGTV)

void OSystem_SDL_SamsungTV::generateMouseMoveEvent(int x, int y) {
	SDL_Event event;
	memset(&event, 0, sizeof(event));
	event.type = SDL_MOUSEMOTION;
	event.motion.x = x;
	event.motion.y = y;
	SDL_PushEvent(&event);
}

void OSystem_SDL_SamsungTV::handleKbdMouse() {
	uint32 curTime = getMillis();
	if (curTime >= _km.last_time + _km.delay_time) {
		_km.last_time = curTime;
		if (_km.x_down_count == 1) {
			_km.x_down_time = curTime;
			_km.x_down_count = 2;
		}
		if (_km.y_down_count == 1) {
			_km.y_down_time = curTime;
			_km.y_down_count = 2;
		}

		if (_km.x_vel || _km.y_vel) {
			if (_km.x_down_count) {
				if (curTime > _km.x_down_time + _km.delay_time * 12) {
					if (_km.x_vel > 0)
						_km.x_vel++;
					else
						_km.x_vel--;
				} else if (curTime > _km.x_down_time + _km.delay_time * 8) {
					if (_km.x_vel > 0)
						_km.x_vel = 5;
					else
						_km.x_vel = -5;
				}
			}
			if (_km.y_down_count) {
				if (curTime > _km.y_down_time + _km.delay_time * 12) {
					if (_km.y_vel > 0)
						_km.y_vel++;
					else
						_km.y_vel--;
				} else if (curTime > _km.y_down_time + _km.delay_time * 8) {
					if (_km.y_vel > 0)
						_km.y_vel = 5;
					else
						_km.y_vel = -5;
				}
			}

			_km.x += _km.x_vel;
			_km.y += _km.y_vel;

			if (_km.x < 0) {
				_km.x = 0;
				_km.x_vel = -1;
				_km.x_down_count = 1;
			} else if (_km.x > _km.x_max) {
				_km.x = _km.x_max;
				_km.x_vel = 1;
				_km.x_down_count = 1;
			}

			if (_km.y < 0) {
				_km.y = 0;
				_km.y_vel = -1;
				_km.y_down_count = 1;
			} else if (_km.y > _km.y_max) {
				_km.y = _km.y_max;
				_km.y_vel = 1;
				_km.y_down_count = 1;
			}

			generateMouseMoveEvent(_km.x, _km.y);
		}
	}
}

bool OSystem_SDL_SamsungTV::pollEvent(Common::Event &event) {
	SDL_Event ev;

	handleKbdMouse();

	// If the screen mode changed, send an Common::EVENT_SCREEN_CHANGED
	if (_modeChanged) {
		_modeChanged = false;
		event.type = Common::EVENT_SCREEN_CHANGED;
		return true;
	}

	while (SDL_PollEvent(&ev)) {
		preprocessEvents(&ev);
		if (dispatchSDLEvent(ev, event))
			return true;
	}
	return false;
}

bool OSystem_SDL_SamsungTV::remapKey(SDL_Event &ev, Common::Event &event) {
	switch (ev.type) {
		case SDL_KEYDOWN:{
			if (ev.key.keysym.sym == SDLK_UP) {
				_km.y_vel = -1;
				_km.y_down_count = 1;
				event.type = Common::EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);
				setMousePos(event.mouse.x, event.mouse.y);
				return true;
			} else if (ev.key.keysym.sym == SDLK_DOWN) {
				_km.y_vel = 1;
				_km.y_down_count = 1;
				event.type = Common::EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);
				setMousePos(event.mouse.x, event.mouse.y);
				return true;
			} else if (ev.key.keysym.sym == SDLK_LEFT) {
				_km.x_vel = -1;
				_km.x_down_count = 1;
				event.type = Common::EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);
				setMousePos(event.mouse.x, event.mouse.y);
				return true;
			} else if (ev.key.keysym.sym == SDLK_RIGHT) {
				_km.x_vel = 1;
				_km.x_down_count = 1;
				event.type = Common::EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);
				setMousePos(event.mouse.x, event.mouse.y);
				return true;
			} else if (ev.key.keysym.sym == SDLK_z) {
				event.type = Common::EVENT_LBUTTONDOWN;
				fillMouseEvent(event, _km.x, _km.y);
				return true;
			} else if (ev.key.keysym.sym == SDLK_F1) {
				event.type = Common::EVENT_RBUTTONDOWN;
				fillMouseEvent(event, _km.x, _km.y);
				return true;
			} else if (ev.key.keysym.sym == SDLK_POWER) {
				event.type = Common::EVENT_QUIT;
				return true;
			} else if (ev.key.keysym.sym == SDLK_F2) {
				event.type = Common::EVENT_KEYDOWN;
				event.kbd.keycode = Common::KEYCODE_F5;
				event.kbd.ascii = Common::ASCII_F5;
				return true;
			} else if (ev.key.keysym.sym == SDLK_0) {
				event.type = Common::EVENT_KEYDOWN;
				event.kbd.keycode = Common::KEYCODE_F7;
				event.kbd.ascii = Common::ASCII_F7;
				return true;
			}
			break;
		}
		case SDL_KEYUP: {
			if (ev.key.keysym.sym == SDLK_UP || ev.key.keysym.sym == SDLK_DOWN || ev.key.keysym.sym == SDLK_LEFT || ev.key.keysym.sym == SDLK_RIGHT) {
				_km.x_vel = 0;
				_km.x_down_count = 0;
				_km.y_vel = 0;
				_km.y_down_count = 0;
				event.type = Common::EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);
				return true;
			} else if (ev.key.keysym.sym == SDLK_z) {
				event.type = Common::EVENT_LBUTTONUP;
				fillMouseEvent(event, _km.x, _km.y);
				return true;
			} else if (ev.key.keysym.sym == SDLK_F1) {
				event.type = Common::EVENT_RBUTTONUP;
				fillMouseEvent(event, _km.x, _km.y);
				return true;
			} else if (ev.key.keysym.sym == SDLK_POWER) {
				event.type = Common::EVENT_QUIT;
				return true;
			} else if (ev.key.keysym.sym == SDLK_F2) {
				event.type = Common::EVENT_KEYUP;
				event.kbd.keycode = Common::KEYCODE_F5;
				event.kbd.ascii = Common::ASCII_F5;
				return true;
			} else if (ev.key.keysym.sym == SDLK_0) {
				event.type = Common::EVENT_KEYUP;
				event.kbd.keycode = Common::KEYCODE_F7;
				event.kbd.ascii = Common::ASCII_F7;
				return true;
			}
			break;
		}
	}

	return false;
}

#endif
