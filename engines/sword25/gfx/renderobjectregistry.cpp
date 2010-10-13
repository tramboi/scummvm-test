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

#include "sword25/gfx/renderobjectregistry.h"

// TODO: Destroy the singletons when closing the engine;
// even better, turn them into non-singleton members of
// e.g. Sword25Engine.
DECLARE_SINGLETON(Sword25::RenderObjectRegistry)

namespace Sword25 {

#define BS_LOG_PREFIX "RENDEROBJECTREGISTRY"

void RenderObjectRegistry::logErrorLn(const char *message) const {
	BS_LOG_ERRORLN(message);
}

void RenderObjectRegistry::logWarningLn(const char *message) const {
	BS_LOG_WARNINGLN(message);
}

} // End of namespace Sword25
