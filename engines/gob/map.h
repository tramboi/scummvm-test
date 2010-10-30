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

#ifndef GOB_MAP_H
#define GOB_MAP_H

#include "gob/mult.h"

namespace Gob {

enum RelativeDirection {
	kRelDirNone  = 0 ,

	kRelDirLeft  = (1 << 0),
	kRelDirUp    = (1 << 1),
	kRelDirRight = (1 << 2),
	kRelDirDown  = (1 << 3),

	kRelDirLeftUp    = kRelDirLeft  | kRelDirUp,
	kRelDirLeftDown  = kRelDirLeft  | kRelDirDown,
	kRelDirRightUp   = kRelDirRight | kRelDirUp,
	kRelDirRightDown = kRelDirRight | kRelDirDown
};

// The same numeric values are also used for the arrow keys.
enum Direction {
	kDirNone = 0x0000,
	kDirNW   = 0x4700,
	kDirN    = 0x4800,
	kDirNE   = 0x4900,
	kDirW    = 0x4B00,
	kDirE    = 0x4D00,
	kDirSW   = 0x4F00,
	kDirS    = 0x5000,
	kDirSE   = 0x5100
};

class Map {
public:
#include "common/pack-start.h"	// START STRUCT PACKING

	struct Point {
		int16 x;
		int16 y;
		int16 notWalkable;
	} PACKED_STRUCT;

#define szMap_ItemPos 3

	struct ItemPos {
		int8 x;
		int8 y;
		int8 orient;
	} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

	byte _widthByte;
	int16 _mapWidth;
	int16 _mapHeight;
	int16 _screenWidth;
	int16 _screenHeight;
	int16 _tilesWidth;
	int16 _tilesHeight;
	int16 _passWidth;
	bool _bigTiles;
	bool _mapUnknownBool;

	int8 *_passMap; // [y * _mapWidth + x], getPass(x, y);
	int16 **_itemsMap;	// [y][x]
	int16 _wayPointsCount;
	Point *_wayPoints;
	int16 _nearestWayPoint;
	int16 _nearestDest;

	int16 _curGoblinX;
	int16 _curGoblinY;
	int16 _destX;
	int16 _destY;

	ItemPos _itemPoses[40];
	char _sourceFile[15];

	Map(GobEngine *vm);
	virtual ~Map();

	int8 getPass(int x, int y, int width = -1) const;
	void setPass(int x, int y, int8 pass, int width = -1);

	void findNearestWalkable(int16 &gobDestX, int16 &gobDestY,
		int16 mouseX, int16 mouseY);

	void placeItem(int16 x, int16 y, int16 id);

	Direction getDirection(int16 x0, int16 y0, int16 x1, int16 y1);

	int16 checkDirectPath(Mult::Mult_Object *obj, int16 x0,
			int16 y0, int16 x1, int16 y1);
	int16 checkLongPath(int16 x0, int16 y0,
			int16 x1, int16 y1, int16 i0, int16 i1);

	void loadMapsInitGobs();

	virtual int16 getItem(int x, int y) = 0;
	virtual void setItem(int x, int y, int16 item) = 0;

	virtual void loadMapObjects(const char *avjFile) = 0;
	virtual void findNearestToGob(Mult::Mult_Object *obj) = 0;
	virtual void findNearestToDest(Mult::Mult_Object *obj) = 0;
	virtual void optimizePoints(Mult::Mult_Object *obj, int16 x, int16 y) = 0;

protected:
	bool _loadFromAvo;

	GobEngine *_vm;

	int16 findNearestWayPoint(int16 x, int16 y);

private:
	// Move the x and y values according to the direction
	void moveDirection(Direction dir, int16 &x, int16 &y);
};

class Map_v1 : public Map {
public:
	virtual void loadMapObjects(const char *avjFile);
	virtual void findNearestToGob(Mult::Mult_Object *obj);
	virtual void findNearestToDest(Mult::Mult_Object *obj);
	virtual void optimizePoints(Mult::Mult_Object *obj, int16 x, int16 y);

	virtual int16 getItem(int x, int y) {
		assert(_itemsMap);

		x = CLIP<int>(x, 0, _mapWidth - 1);
		y = CLIP<int>(y, 0, _mapHeight - 1);

		return _itemsMap[y][x];
	}
	virtual void setItem(int x, int y, int16 item) {
		assert(_itemsMap);

		x = CLIP<int>(x, 0, _mapWidth - 1);
		y = CLIP<int>(y, 0, _mapHeight - 1);

		_itemsMap[y][x] = item;
	}

	Map_v1(GobEngine *vm);
	virtual ~Map_v1();

protected:
	void init();
	void loadSounds(Common::SeekableReadStream &data);
	void loadGoblins(Common::SeekableReadStream &data, uint32 gobsPos);
	void loadObjects(Common::SeekableReadStream &data, uint32 objsPos);
	void loadItemToObject(Common::SeekableReadStream &data);
};

class Map_v2 : public Map_v1 {
public:
	virtual void loadMapObjects(const char *avjFile);
	virtual void findNearestToGob(Mult::Mult_Object *obj);
	virtual void findNearestToDest(Mult::Mult_Object *obj);
	virtual void optimizePoints(Mult::Mult_Object *obj, int16 x, int16 y);

	Map_v2(GobEngine *vm);
	virtual ~Map_v2();

protected:
	void loadGoblinStates(Common::SeekableReadStream &data, int index);
};

} // End of namespace Gob

#endif // GOB_MAP_H
