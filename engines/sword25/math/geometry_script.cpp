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

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "common/array.h"
#include "sword25/gfx/graphicengine.h"
#include "sword25/kernel/common.h"
#include "sword25/kernel/kernel.h"
#include "sword25/script/script.h"
#include "sword25/script/luabindhelper.h"

#include "sword25/math/geometry.h"
#include "sword25/math/region.h"
#include "sword25/math/regionregistry.h"
#include "sword25/math/walkregion.h"
#include "sword25/math/vertex.h"

namespace Sword25 {

// These strings are defined as #defines to enable compile-time string composition
#define REGION_CLASS_NAME "Geo.Region"
#define WALKREGION_CLASS_NAME "Geo.WalkRegion"

// How luaL_checkudata, only without that no error is generated.
static void *my_checkudata(lua_State *L, int ud, const char *tname) {
	int top = lua_gettop(L);

	void *p = lua_touserdata(L, ud);
	if (p != NULL) { /* value is a userdata? */
		if (lua_getmetatable(L, ud)) { /* does it have a metatable? */
			// lua_getfield(L, LUA_REGISTRYINDEX, tname);  /* get correct metatable */
			LuaBindhelper::getMetatable(L, tname);
			/* does it have the correct mt? */
			if (lua_rawequal(L, -1, -2)) {
				lua_settop(L, top);
				return p;
			}
		}
	}

	lua_settop(L, top);
	return NULL;
}

static void newUintUserData(lua_State *L, uint value) {
	void *userData = lua_newuserdata(L, sizeof(value));
	memcpy(userData, &value, sizeof(value));
}

static bool isValidPolygonDefinition(lua_State *L) {
#ifdef DEBUG
	int __startStackDepth = lua_gettop(L);
#endif

	// Ensure that we actually consider a table
	if (!lua_istable(L, -1)) {
		luaL_error(L, "Invalid polygon definition. Unexpected type, \"table\" needed.");
		return false;
	}

	int tableSize = luaL_getn(L, -1);

	// Make sure that there are at least three Vertecies
	if (tableSize < 6) {
		luaL_error(L, "Invalid polygon definition. At least three vertecies needed.");
		return false;
	}

	// Make sure that the number of table elements is divisible by two.
	// Since any two elements is a vertex, an odd number of elements is not allowed
	if ((tableSize % 2) != 0) {
		luaL_error(L, "Invalid polygon definition. Even number of table elements needed.");
		return false;
	}

	// Ensure that all elements in the table are of type Number
	for (int i = 1; i <= tableSize; i++) {
		lua_rawgeti(L, -1, i);
		if (!lua_isnumber(L, -1)) {
			luaL_error(L, "Invalid polygon definition. All table elements have to be numbers.");
			return false;
		}
		lua_pop(L, 1);
	}

#ifdef DEBUG
	BS_ASSERT(__startStackDepth == lua_gettop(L));
#endif

	return true;
}

static void tablePolygonToPolygon(lua_State *L, Polygon &polygon) {
#ifdef DEBUG
	int __startStackDepth = lua_gettop(L);
#endif

	// Ensure that a valid polygon definition is on the stack.
	// It is not necessary to catch the return value, since all errors are reported on luaL_error
	// End script.
	isValidPolygonDefinition(L);

	int vertexCount = luaL_getn(L, -1) / 2;

	// Memory is reserved for Vertecies
	Common::Array<Vertex> vertices;
	vertices.reserve(vertexCount);

	// Create Vertecies
	for (int i = 0; i < vertexCount; i++) {
		// X Value
		lua_rawgeti(L, -1, (i * 2) + 1);
		int X = static_cast<int>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		// Y Value
		lua_rawgeti(L, -1, (i * 2) + 2);
		int Y = static_cast<int>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		// Vertex
		vertices.push_back(Vertex(X, Y));
	}
	BS_ASSERT((int)vertices.size() == vertexCount);

#ifdef DEBUG
	BS_ASSERT(__startStackDepth == lua_gettop(L));
#endif

	// Create polygon
	polygon.init(vertexCount, &vertices[0]);
}

static uint tableRegionToRegion(lua_State *L, const char *className) {
#ifdef DEBUG
	int __startStackDepth = lua_gettop(L);
#endif

	// You can define a region in Lua in two ways:
	// 1. A table that defines a polygon (polgon = table with numbers, which define
	// two consecutive numbers per vertex)
	// 2. A table containing more polygon definitions
	// Then the first polygon is the contour of the region, and the following are holes
	// defined in the first polygon.

	// It may be passed only one parameter, and this must be a table
	if (lua_gettop(L) != 1 || !lua_istable(L, -1)) {
		luaL_error(L, "First and only parameter has to be of type \"table\".");
		return 0;
	}

	uint regionHandle = 0;
	if (!strcmp(className, REGION_CLASS_NAME)) {
		regionHandle = Region::create(Region::RT_REGION);
	} else if (!strcmp(className, WALKREGION_CLASS_NAME)) {
		regionHandle = WalkRegion::create(Region::RT_WALKREGION);
	} else {
		BS_ASSERT(false);
	}

	BS_ASSERT(regionHandle);

	// If the first element of the parameter is a number, then case 1 is accepted
	// If the first element of the parameter is a table, then case 2 is accepted
	// If the first element of the parameter has a different type, there is an error
	lua_rawgeti(L, -1, 1);
	int firstElementType = lua_type(L, -1);
	lua_pop(L, 1);

	switch (firstElementType) {
	case LUA_TNUMBER: {
		Polygon polygon;
		tablePolygonToPolygon(L, polygon);
		RegionRegistry::instance().resolveHandle(regionHandle)->init(polygon);
	}
	break;

	case LUA_TTABLE: {
		lua_rawgeti(L, -1, 1);
		Polygon polygon;
		tablePolygonToPolygon(L, polygon);
		lua_pop(L, 1);

		int polygonCount = luaL_getn(L, -1);
		if (polygonCount == 1)
			RegionRegistry::instance().resolveHandle(regionHandle)->init(polygon);
		else {
			Common::Array<Polygon> holes;
			holes.reserve(polygonCount - 1);

			for (int i = 2; i <= polygonCount; i++) {
				lua_rawgeti(L, -1, i);
				holes.resize(holes.size() + 1);
				tablePolygonToPolygon(L, holes.back());
				lua_pop(L, 1);
			}
			BS_ASSERT((int)holes.size() == polygonCount - 1);

			RegionRegistry::instance().resolveHandle(regionHandle)->init(polygon, &holes);
		}
	}
	break;

	default:
		luaL_error(L, "Illegal region definition.");
		return 0;
	}

#ifdef DEBUG
	BS_ASSERT(__startStackDepth == lua_gettop(L));
#endif

	return regionHandle;
}

static void newUserdataRegion(lua_State *L, const char *className) {
	// Region due to the Lua code to create
	// Any errors that occur will be intercepted to the luaL_error
	uint regionHandle = tableRegionToRegion(L, className);
	BS_ASSERT(regionHandle);

	newUintUserData(L, regionHandle);
	// luaL_getmetatable(L, className);
	LuaBindhelper::getMetatable(L, className);
	BS_ASSERT(!lua_isnil(L, -1));
	lua_setmetatable(L, -2);
}

static int newRegion(lua_State *L) {
	newUserdataRegion(L, REGION_CLASS_NAME);
	return 1;
}

static int newWalkRegion(lua_State *L) {
	newUserdataRegion(L, WALKREGION_CLASS_NAME);
	return 1;
}

static const char *GEO_LIBRARY_NAME = "Geo";

static const luaL_reg GEO_FUNCTIONS[] = {
	{"NewRegion", newRegion},
	{"NewWalkRegion", newWalkRegion},
	{0, 0}
};

static Region *checkRegion(lua_State *L) {
	// The first parameter must be of type 'userdata', and the Metatable class Geo.Region or Geo.WalkRegion
	uint *regionHandlePtr;
	if ((regionHandlePtr = reinterpret_cast<uint *>(my_checkudata(L, 1, REGION_CLASS_NAME))) != 0 ||
	        (regionHandlePtr = reinterpret_cast<uint *>(my_checkudata(L, 1, WALKREGION_CLASS_NAME))) != 0) {
		return RegionRegistry::instance().resolveHandle(*regionHandlePtr);
	} else {
		luaL_argcheck(L, 0, 1, "'" REGION_CLASS_NAME "' expected");
	}

	// Compilation fix. Execution never reaches this point
	return 0;
}

static int r_isValid(lua_State *L) {
	Region *pR = checkRegion(L);
	BS_ASSERT(pR);

	lua_pushbooleancpp(L, pR->isValid());
	return 1;
}

static int r_getX(lua_State *L) {
	Region *pR = checkRegion(L);
	BS_ASSERT(pR);

	lua_pushnumber(L, pR->getPosX());
	return 1;
}

static int r_getY(lua_State *L) {
	Region *pR = checkRegion(L);
	BS_ASSERT(pR);

	lua_pushnumber(L, pR->getPosY());
	return 1;
}

static int r_getPos(lua_State *L) {
	Region *pR = checkRegion(L);
	BS_ASSERT(pR);

	Vertex::vertexToLuaVertex(L, pR->getPosition());
	return 1;
}

static int r_isPointInRegion(lua_State *L) {
	Region *pR = checkRegion(L);
	BS_ASSERT(pR);

	Vertex vertex;
	Vertex::luaVertexToVertex(L, 2, vertex);
	lua_pushbooleancpp(L, pR->isPointInRegion(vertex));
	return 1;
}

static int r_setPos(lua_State *L) {
	Region *pR = checkRegion(L);
	BS_ASSERT(pR);

	Vertex vertex;
	Vertex::luaVertexToVertex(L, 2, vertex);
	pR->setPos(vertex.x, vertex.y);

	return 0;
}

static int r_setX(lua_State *L) {
	Region *pR = checkRegion(L);
	BS_ASSERT(pR);

	pR->setPosX(static_cast<int>(luaL_checknumber(L, 2)));

	return 0;
}

static int r_setY(lua_State *L) {
	Region *pR = checkRegion(L);
	BS_ASSERT(pR);

	pR->setPosY(static_cast<int>(luaL_checknumber(L, 2)));

	return 0;
}

static void drawPolygon(const Polygon &polygon, uint color, const Vertex &offset) {
	GraphicEngine *pGE = Kernel::GetInstance()->GetGfx();
	BS_ASSERT(pGE);

	for (int i = 0; i < polygon.vertexCount - 1; i++)
		pGE->drawDebugLine(polygon.vertices[i] + offset, polygon.vertices[i + 1] + offset, color);

	pGE->drawDebugLine(polygon.vertices[polygon.vertexCount - 1] + offset, polygon.vertices[0] + offset, color);
}

static void drawRegion(const Region &region, uint color, const Vertex &offset) {
	drawPolygon(region.getContour(), color, offset);
	for (int i = 0; i < region.getHoleCount(); i++)
		drawPolygon(region.getHole(i), color, offset);
}

static int r_draw(lua_State *L) {
	Region *pR = checkRegion(L);
	BS_ASSERT(pR);

	switch (lua_gettop(L)) {
	case 3: {
		Vertex offset;
		Vertex::luaVertexToVertex(L, 3, offset);
		drawRegion(*pR, GraphicEngine::luaColorToARGBColor(L, 2), offset);
	}
	break;

	case 2:
		drawRegion(*pR, GraphicEngine::luaColorToARGBColor(L, 2), Vertex(0, 0));
		break;

	default:
		drawRegion(*pR, BS_RGB(255, 255, 255), Vertex(0, 0));
	}

	return 0;
}

static int r_getCentroid(lua_State *L) {
	Region *RPtr = checkRegion(L);
	BS_ASSERT(RPtr);

	Vertex::vertexToLuaVertex(L, RPtr->getCentroid());

	return 1;
}

static int r_delete(lua_State *L) {
	Region *pR = checkRegion(L);
	BS_ASSERT(pR);
	delete pR;
	return 0;
}

static const luaL_reg REGION_METHODS[] = {
	{"SetPos", r_setPos},
	{"SetX", r_setX},
	{"SetY", r_setY},
	{"GetPos", r_getPos},
	{"IsPointInRegion", r_isPointInRegion},
	{"GetX", r_getX},
	{"GetY", r_getY},
	{"IsValid", r_isValid},
	{"Draw", r_draw},
	{"GetCentroid", r_getCentroid},
	{0, 0}
};

static WalkRegion *checkWalkRegion(lua_State *L) {
	// The first parameter must be of type 'userdate', and the Metatable class Geo.WalkRegion
	uint regionHandle;
	if ((regionHandle = *reinterpret_cast<uint *>(my_checkudata(L, 1, WALKREGION_CLASS_NAME))) != 0) {
		return reinterpret_cast<WalkRegion *>(RegionRegistry::instance().resolveHandle(regionHandle));
	} else {
		luaL_argcheck(L, 0, 1, "'" WALKREGION_CLASS_NAME "' expected");
	}

	// Compilation fix. Execution never reaches this point
	return 0;
}

static int wr_getPath(lua_State *L) {
	WalkRegion *pWR = checkWalkRegion(L);
	BS_ASSERT(pWR);

	Vertex start;
	Vertex::luaVertexToVertex(L, 2, start);
	Vertex end;
	Vertex::luaVertexToVertex(L, 3, end);
	BS_Path path;
	if (pWR->queryPath(start, end, path)) {
		lua_newtable(L);
		BS_Path::const_iterator it = path.begin();
		for (; it != path.end(); it++) {
			lua_pushnumber(L, (it - path.begin()) + 1);
			Vertex::vertexToLuaVertex(L, *it);
			lua_settable(L, -3);
		}
	} else
		lua_pushnil(L);

	return 1;
}

static const luaL_reg WALKREGION_METHODS[] = {
	{"GetPath", wr_getPath},
	{0, 0}
};

bool Geometry::registerScriptBindings() {
	Kernel *pKernel = Kernel::GetInstance();
	BS_ASSERT(pKernel);
	ScriptEngine *pScript = pKernel->GetScript();
	BS_ASSERT(pScript);
	lua_State *L = static_cast< lua_State *>(pScript->getScriptObject());
	BS_ASSERT(L);

	if (!LuaBindhelper::addMethodsToClass(L, REGION_CLASS_NAME, REGION_METHODS)) return false;
	if (!LuaBindhelper::addMethodsToClass(L, WALKREGION_CLASS_NAME, REGION_METHODS)) return false;
	if (!LuaBindhelper::addMethodsToClass(L, WALKREGION_CLASS_NAME, WALKREGION_METHODS)) return false;

	if (!LuaBindhelper::setClassGCHandler(L, REGION_CLASS_NAME, r_delete)) return false;
	if (!LuaBindhelper::setClassGCHandler(L, WALKREGION_CLASS_NAME, r_delete)) return false;

	if (!LuaBindhelper::addFunctionsToLib(L, GEO_LIBRARY_NAME, GEO_FUNCTIONS)) return false;

	return true;
}

} // End of namespace Sword25
