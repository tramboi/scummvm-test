// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsd�rfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

#ifndef BS_LUACALLBACK_H
#define BS_LUACALLBACK_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "kernel/common.h"

// -----------------------------------------------------------------------------
// Forward Deklarationen
// -----------------------------------------------------------------------------

struct lua_State;

// -----------------------------------------------------------------------------
// Klassendeklaration
// -----------------------------------------------------------------------------

class BS_LuaCallback
{
public:
	BS_LuaCallback(lua_State * L);
	virtual ~BS_LuaCallback();

	// Funktion muss auf dem Lua-Stack liegen.
	void RegisterCallbackFunction(lua_State * L, unsigned int ObjectHandle);

	// Funktion muss auf dem Lua-Stack liegen.
	void UnregisterCallbackFunction(lua_State * L, unsigned int ObjectHandle);

	void RemoveAllObjectCallbacks(lua_State * L, unsigned int ObjectHandle);

	void InvokeCallbackFunctions(lua_State * L, unsigned int ObjectHandle);

protected:
	virtual int PreFunctionInvokation(lua_State * L) { return 0; }

private:
	void EnsureObjectCallbackTableExists(lua_State * L,unsigned int ObjectHandle);
	void PushCallbackTable(lua_State * L);
	void PushObjectCallbackTable(lua_State * L, unsigned int ObjectHandle);
};

#endif
