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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/branches/gsoc2010-opengl/backends/graphics/opengl/glerrorcheck.cpp $
 * $Id: glerrorcheck.cpp 51300 2010-07-26 06:58:33Z vgvgf $
 *
 */

#if defined(DEBUG) && defined(USE_OPENGL)

#include "backends/graphics/opengl/glerrorcheck.h"
#include "common/debug.h"

#ifdef WIN32
#if defined(ARRAYSIZE) && !defined(_WINDOWS_)
#undef ARRAYSIZE
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef ARRAYSIZE
#endif

#ifdef MACOSX
#include <OpenGL/gl.h>
#elif defined(USE_GLES)
#include <GLES/gl.h>
#else
#include <GL/gl.h>
#endif

static const char *getGlErrStr(GLenum error) {
	switch (error) {
	case GL_NO_ERROR:			return "GL_NO_ERROR";
	case GL_INVALID_ENUM:		return "GL_INVALID_ENUM";
	case GL_INVALID_OPERATION:	return "GL_INVALID_OPERATION";
	case GL_STACK_OVERFLOW:		return "GL_STACK_OVERFLOW";
	case GL_STACK_UNDERFLOW:	return "GL_STACK_UNDERFLOW";
	case GL_OUT_OF_MEMORY:		return "GL_OUT_OF_MEMORY";
	}

	// FIXME: Convert to use Common::String::format()
	static char buf[40];
	snprintf(buf, sizeof(buf), "(Unknown GL error code 0x%x)", error);
	return buf;
}

void checkGlError(const char *file, int line) {
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
		warning("%s:%d: GL error: %s", file, line, getGlErrStr(error));
}

#endif
