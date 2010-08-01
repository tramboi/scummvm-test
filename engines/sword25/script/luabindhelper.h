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

#ifndef SWORD25_LUABINDHELPER_H
#define SWORD25_LUABINDHELPER_H

#include "sword25/kernel/common.h"

namespace {

extern "C"
{
	#include "sword25/util/lua/lua.h"
	#include "sword25/util/lua/lauxlib.h"
}

}

namespace Sword25 {

#define lua_pushbooleancpp(L, b) (lua_pushboolean(L, b ? 1 : 0))
#define lua_tobooleancpp(L, i) (lua_toboolean(L, i) == 0 ? false : true)

struct lua_constant_reg {
	const char *	Name;
	lua_Number		Value;
};

class BS_LuaBindhelper {
public:
	/**
	 * Registers a set of functions into a Lua library.
	 * @param L				A pointer to the Lua VM 
	 * @param LibName		The name of the library.
	 * If this is an empty string, the functions will be added to the global namespace.
	 * @param Functions		An array of function pointers along with their names.
	 * The array must be terminated with the enry (0, 0)
	 * @return				Returns true if successful, otherwise false.
	 */
	static bool AddFunctionsToLib(::lua_State *L, const Common::String &LibName, const luaL_reg *Functions);

	/**
	 * Adds a set of constants to the Lua library
	 * @param L				A pointer to the Lua VM
	 * @param LibName		The name of the library.
	 * If this is an empty string, the functions will be added to the global namespace.
	 * @param Constants		An array of the constant values along with their names.
	 * The array must be terminated with the enry (0, 0)
	 * @return				Returns true if successful, otherwise false.
	 */
	static bool AddConstantsToLib(::lua_State * L, const Common::String & LibName, const lua_constant_reg * Constants);

	/**
	 * Adds a set of methods to a Lua class
	 * @param L				A pointer to the Lua VM 
	 * @param ClassName		The name of the class
	 * When the class name specified does not exist, it is created.
	 * @param Methods		An array of function pointers along with their method names.
	 * The array must be terminated with the enry (0, 0)
	 * @return				Returns true if successful, otherwise false.
	 */
	static bool AddMethodsToClass(::lua_State *L, const Common::String &ClassName, const luaL_reg *Methods);

	/**
	 * Sets the garbage collector callback method when items of a particular class are deleted
	 * @param L				A pointer to the Lua VM 
	 * @param ClassName		The name of the class
	 * When the class name specified does not exist, it is created.
	 * @param GCHandler		A function pointer
	 * @return				Returns true if successful, otherwise false.
	 */
	static bool SetClassGCHandler(::lua_State *L, const Common::String &ClassName, lua_CFunction GCHandler);

	/**
	 * Returns a string containing a stack dump of the Lua stack
	 * @param L				A pointer to the Lua VM 
	 */
	static Common::String StackDump(::lua_State *L);

	/**
	 * Returns a string that describes the contents of a table
 	 * @param L				A pointer to the Lua VM 
	 * @remark				The table must be on the Lua stack to be read out.
	 */
	static Common::String TableDump(::lua_State *L);

	static bool GetMetatable(::lua_State *L, const Common::String &TableName);

private:
	static bool _CreateTable(::lua_State *L, const Common::String &TableName);
};

} // End of namespace Sword25

#endif
