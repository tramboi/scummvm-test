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


#ifndef TEENAGENT_OBJECTS_H
#define TEENAGENT_OBJECTS_H

#include "common/rect.h"
#include "common/stream.h"
#include "graphics/surface.h"

namespace TeenAgent {

struct Rect {
	uint16 left, top, right, bottom;

	inline Rect() : left(0), top(0), right(0), bottom(0), _base(NULL) {}

	inline Rect(uint16 l, uint16 t, uint16 r, uint16 b) : left(l), top(t), right(r), bottom(b), _base(NULL) {}

	inline bool in(const Common::Point &point) const {
		return point.x >= left && point.x <= right && point.y >= top && point.y <= bottom;
	}

	inline Common::Point center() const {
		return Common::Point((right + left) / 2, (bottom + top) / 2);
	}

	inline bool valid() const {
		return left < 320 && right < 320 && top < 200 && bottom < 200;
	}

	void render(Graphics::Surface *surface, uint8 color) const;

	void dump() const {
		debug(0, "rect[%u, %u, %u, %u]", left, top, right, bottom);
	}

	inline void clear() {
		left = top = right = bottom = 0;
	}
	
	void load(byte *src); //8 bytes
	void save();

protected:
	byte * _base;
};

struct Object {
	enum {kActorUp = 1, kActorRight = 2, kActorDown = 3, kActorLeft = 4 };

	byte id;
	Rect rect;
	Rect actor_rect;
	byte actor_orientation;
	byte enabled; //19 bytes
	Common::String name, description;

	Object(): _base(NULL) {}
	void dump();
	void setName(const Common::String &name);
	void load(byte *addr);
	
	static Common::String parse_description(const char *name);

protected:
	byte * _base;
};

struct InventoryObject {
	byte id;
	byte animated;
	Common::String name, description;
	
	InventoryObject(): id(0), animated(0), _base(0) {}
	void load(byte *addr);

protected:
	byte * _base;
};

struct UseHotspot {
	byte inventory_id;
	byte object_id;
	byte unk02;
	uint16 x, y;
	uint16 callback;
	void load(byte *src);
};

struct Walkbox {
	byte unk00;
	byte orientation;
	Rect rect;
	byte unk0a;
	byte unk0b;
	byte unk0c;
	byte unk0d;

	Walkbox() : _base(NULL) {}
	void dump();
	void load(byte *src);
	void save();

protected:
	byte * _base;
};

} // End of namespace TeenAgent

#endif
