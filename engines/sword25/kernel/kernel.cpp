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

#include "common/system.h"
#include "sword25/gfx/graphicengine.h"
#include "sword25/fmv/movieplayer.h"
#include "sword25/input/inputengine.h"
#include "sword25/kernel/kernel.h"
#include "sword25/kernel/persistenceservice.h"
#include "sword25/math/geometry.h"
#include "sword25/package/packagemanager.h"
#include "sword25/script/luascript.h"
#include "sword25/sfx/soundengine.h"

namespace Sword25 {

#define BS_LOG_PREFIX "KERNEL"

Kernel *Kernel::_instance = 0;

Kernel::Kernel() :
	_resourceManager(NULL),
	_initSuccess(false),
	_gfx(0),
	_sfx(0),
	_input(0),
	_package(0),
	_script(0),
	_fmv(0)
	{

	// Log that the kernel is beign created
	BS_LOGLN("created.");
	_instance = this;

	// Create the resource manager
	_resourceManager = new ResourceManager(this);

	// Initialise the script engine
	_script = new LuaScriptEngine(this);
	if (!_script || !_script->init()) {
		_initSuccess = false;
		return;
	}

	// Register kernel script bindings
	if (!registerScriptBindings()) {
		BS_LOG_ERRORLN("Script bindings could not be registered.");
		_initSuccess = false;
		return;
	}
	BS_LOGLN("Script bindings registered.");

	_input = new InputEngine(this);
	assert(_input);

	_gfx = new GraphicEngine(this);
	assert(_gfx);

	_sfx = new SoundEngine(this);
	assert(_sfx);

	_package = new PackageManager(this);
	assert(_package);

	_geometry = new Geometry(this);
	assert(_geometry);

#ifdef USE_THEORADEC
	_fmv = new MoviePlayer(this);
	assert(_fmv);
#endif

	_initSuccess = true;
}

Kernel::~Kernel() {
	// Services are de-registered in reverse order of creation

	delete _input;
	_input = 0;

	delete _gfx;
	_gfx = 0;

	delete _sfx;
	_sfx = 0;

	delete _package;
	_package = 0;

	delete _geometry;
	_geometry = 0;

#ifdef USE_THEORADEC
	delete _fmv;
	_fmv = 0;
#endif

	delete _script;
	_script = 0;

	// Resource-Manager freigeben
	delete _resourceManager;

	BS_LOGLN("destroyed.");
}

/**
 * Returns a random number
 * @param Min       The minimum allowed value
 * @param Max       The maximum allowed value
 */
int Kernel::getRandomNumber(int min, int max) {
	BS_ASSERT(min <= max);

	return min + _rnd.getRandomNumber(max - min + 1);
}

/**
 * Returns the elapsed time since startup in milliseconds
 */
uint Kernel::getMilliTicks() {
	return g_system->getMillis();
}

/**
 * Returns how much memory is being used
 */
size_t Kernel::getUsedMemory() {
	return 0;
}

/**
 * Returns a pointer to the active Gfx Service, or NULL if no Gfx service is active.
 */
GraphicEngine *Kernel::getGfx() {
	return _gfx;
}

/**
 * Returns a pointer to the active Sfx Service, or NULL if no Sfx service is active.
 */
SoundEngine *Kernel::getSfx() {
	return _sfx;
}

/**
 * Returns a pointer to the active input service, or NULL if no input service is active.
 */
InputEngine *Kernel::getInput() {
	return _input;
}

/**
 * Returns a pointer to the active package manager, or NULL if no manager is active.
 */
PackageManager *Kernel::getPackage() {
	return _package;
}

/**
 * Returns a pointer to the script engine, or NULL if it is not active.
 */
ScriptEngine *Kernel::getScript() {
	return _script;
}

/**
 * Returns a pointer to the movie player, or NULL if it is not active.
 */
MoviePlayer *Kernel::getFMV() {
	return _fmv;
}

void Kernel::sleep(uint msecs) const {
	g_system->delayMillis(msecs);
}

} // End of namespace Sword25
