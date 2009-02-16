/***************************************************************************
 pcmout.c Copyright (C) 2002 Solomon Peachy

 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.

***************************************************************************/

#include "sci/include/sfx_pcm.h"
#include "sci/include/resource.h"

#ifndef NO_PCMOUT
extern sfx_pcm_device_t sfx_pcm_driver_scummvm;
#endif

sfx_pcm_device_t *pcmout_drivers[] = {
#ifndef NO_PCMOUT
	&sfx_pcm_driver_scummvm,
#endif
	NULL
};

sfx_pcm_device_t *
sfx_pcm_find_device(char *name) {
	int retval = 0;

	if (!name) { /* Find default driver */
		return pcmout_drivers[0];
	}

	while (pcmout_drivers[retval] &&
	        scumm_stricmp(name, pcmout_drivers[retval]->name))
		retval++;

	return pcmout_drivers[retval];
}

