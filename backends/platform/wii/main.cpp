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
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#include <fat.h>

#include "osystem.h"

#ifdef DEBUG_WII
#include <debug.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

bool reset_btn_pressed = false;
bool power_btn_pressed = false;

void reset_cb(void) {
	reset_btn_pressed = true;
}

void power_cb(void) {
	power_btn_pressed = true;
}

int main(int argc, char *argv[]) {
	s32 res;

	VIDEO_Init();
	PAD_Init();
	AUDIO_Init(NULL);

#ifdef DEBUG_WII
	CON_EnableGecko(1, false);
	//DEBUG_Init(GDBSTUB_DEVICE_USB, 1);
#endif

	printf("startup as ");
	if (argc > 0)
		printf("'%s'\n", argv[0]);
	else
		printf("<unknown>\n");

	SYS_SetResetCallback(reset_cb);
#ifndef GAMECUBE
	SYS_SetPowerCallback(power_cb);
#endif

	if (!fatInitDefault()) {
		printf("fatInitDefault failed\n");
	} else {
#ifdef LIBFAT_READAHEAD_CACHE
		fatSetReadAheadDefault(16, 32);
#else
		printf("read ahead cache not available\n");
#endif
		// set the default path if libfat couldnt set it
		// this allows loading over tcp/usbgecko
		char cwd[MAXPATHLEN];

		if (getcwd(cwd, MAXPATHLEN)) {
			size_t len = strlen(cwd);

			if (len > 2 && (cwd[len - 1] == ':' || cwd[len - 2] == ':')) {
				printf("chdir to default\n");
				chdir("/apps/scummvm");
			}
		}
	}

	g_system = new OSystem_Wii();
	assert(g_system);

	res = scummvm_main(argc, argv);
	g_system->quit();

	printf("shutdown\n");

#ifdef LIBFAT_READAHEAD_CACHE
	fatUnmountDefault();
#endif

	if (power_btn_pressed) {
		printf("shutting down\n");
		SYS_ResetSystem(SYS_POWEROFF, 0, 0);
	}

	printf("reloading\n");

	return res;
}

#ifdef __cplusplus
}
#endif

