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

#ifdef WIN32

#include <windows.h>
// winnt.h defines ARRAYSIZE, but we want our own one... - this is needed before including util.h
#undef ARRAYSIZE

#include "backends/platform/sdl/win32/win32.h"
#include "backends/fs/windows/windows-fs-factory.h"

#define DEFAULT_CONFIG_FILE "scummvm.ini"

void OSystem_Win32::init() {
	// Initialze File System Factory
	_fsFactory = new WindowsFilesystemFactory();

	// Invoke parent implementation of this method
	OSystem_SDL::init();
}

Common::String OSystem_Win32::getDefaultConfigFileName() {
	char configFile[MAXPATHLEN];

	OSVERSIONINFO win32OsVersion;
	ZeroMemory(&win32OsVersion, sizeof(OSVERSIONINFO));
	win32OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&win32OsVersion);
	// Check for non-9X version of Windows.
	if (win32OsVersion.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS) {
		// Use the Application Data directory of the user profile.
		if (win32OsVersion.dwMajorVersion >= 5) {
			if (!GetEnvironmentVariable("APPDATA", configFile, sizeof(configFile)))
				error("Unable to access application data directory");
		} else {
			if (!GetEnvironmentVariable("USERPROFILE", configFile, sizeof(configFile)))
				error("Unable to access user profile directory");

			strcat(configFile, "\\Application Data");
			CreateDirectory(configFile, NULL);
		}

		strcat(configFile, "\\ScummVM");
		CreateDirectory(configFile, NULL);
		strcat(configFile, "\\" DEFAULT_CONFIG_FILE);

		FILE *tmp = NULL;
		if ((tmp = fopen(configFile, "r")) == NULL) {
			// Check windows directory
			char oldConfigFile[MAXPATHLEN];
			GetWindowsDirectory(oldConfigFile, MAXPATHLEN);
			strcat(oldConfigFile, "\\" DEFAULT_CONFIG_FILE);
			if ((tmp = fopen(oldConfigFile, "r"))) {
				strcpy(configFile, oldConfigFile);

				fclose(tmp);
			}
		} else {
			fclose(tmp);
		}
	} else {
		// Check windows directory
		GetWindowsDirectory(configFile, MAXPATHLEN);
		strcat(configFile, "\\" DEFAULT_CONFIG_FILE);
	}

	return configFile;
}

#endif
