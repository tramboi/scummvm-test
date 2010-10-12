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

#ifndef SWORD25_LUASCRIPT_H
#define SWORD25_LUASCRIPT_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/script/script.h"

// -----------------------------------------------------------------------------
// Forward declarations
// -----------------------------------------------------------------------------

class BS_Kernel;
struct lua_State;

// -----------------------------------------------------------------------------
// Class declaration
// -----------------------------------------------------------------------------

class BS_LuaScriptEngine : public BS_ScriptEngine
{
public:
	// -----------------------------------------------------------------------------
	// Konstruktion / Destruktion
	// -----------------------------------------------------------------------------

	BS_LuaScriptEngine(BS_Kernel * KernelPtr);
	virtual ~BS_LuaScriptEngine();

	/**
		@brief Initialisiert die Scriptengine.
		@return Gibt true bei Erfolg zur�ck, ansonsten false.
	*/
	virtual bool Init();

	/**
		@brief L�dt eine Skriptdatei und f�hrt diese aus.
		@param FileName der Dateiname der Skriptdatei
		@return Gibt true bei Erfolg zur�ck, ansonsten false.
	*/
	virtual bool ExecuteFile(const std::string & FileName);

	/**
		@brief F�hrt einen String mit Skriptcode aus.
		@param Code ein String der Skriptcode enth�lt.
		@return Gibt true bei Erfolg zur�ck, ansonsten false.
	*/
	virtual bool ExecuteString(const std::string & Code);

	/**
		@brief Gibt einen Pointer auf das Hauptobjekt der Skriptsprache zur�ck.
		@remark Durch die Benutzung dieser Methode wird die Kapselung der Sprache aufgehoben.
	*/
	virtual void * GetScriptObject() { return m_State; }

	/**
		@brief Macht die Kommandozeilen-Parameter f�r die Skriptumgebung zug�nglich.
		@param CommandLineParameters ein string vector der alle Kommandozeilenparameter enth�lt.
		@remark Auf welche Weise die Kommandozeilen-Parameter durch Skripte zugegriffen werden k�nnen h�ngt von der jeweiligen Implementierung ab.
	*/
	virtual void SetCommandLine(const std::vector<std::string> & CommandLineParameters);

	/**
		@remark Der Lua-Stack wird durch diese Methode geleert.
	*/
	virtual bool Persist(BS_OutputPersistenceBlock & Writer);
	/**
		@remark Der Lua-Stack wird durch diese Methode geleert.
	*/
	virtual bool Unpersist(BS_InputPersistenceBlock & Reader);

private:
	lua_State * m_State;
	int m_PcallErrorhandlerRegistryIndex;

	bool RegisterStandardLibs();
	bool RegisterStandardLibExtensions();
	bool ExecuteBuffer(const char * Data, unsigned int Size, const std::string & Name) const;
};

#endif
