/***************************************************************************
  Copyright (C) 2008 Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantability,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <creichen@gmail.com>

***************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include "../sequencer.h"
#include "../device.h"
#include "instrument-map.h"
#include <resource.h>

static midi_writer_t *writer = NULL;

static int
midi_gm_open(int patch_len, byte *data, int patch2_len, byte *data2, void *device)
{
	sfx_instrument_map_t *instrument_map = sfx_instrument_map_load_sci(data, patch_len);

	if (!instrument_map) {
		fprintf(stderr, "[GM]  No GM instrument map found, trying MT-32 instrument map..\n");
		instrument_map = sfx_instrument_map_mt32_to_gm(data2, patch2_len);
	}

	writer = sfx_mapped_writer((midi_writer_t *) device, instrument_map);

	if (!writer)
		return SFX_ERROR;

	if (writer->reset_timer)
		writer->reset_timer(writer);

	return SFX_OK;
}

static int
midi_gm_close(void)
{
	return SFX_OK;
}

static int
midi_gm_event(byte command, int argc, byte *argv)
{
	byte data[4];

	assert (argc < 4);
	data[0] = command;
	memcpy(data + 1, argv, argc);

	writer->write(writer, data, argc + 1);

	return SFX_OK;
}

static int
midi_gm_delay(int ticks)
{
	writer->delay(writer, ticks);

	return SFX_OK;
}

static int
midi_gm_reset_timer(GTimeVal ts)
{
	writer->reset_timer(writer);

	return SFX_OK;
}

#define MIDI_MASTER_VOLUME_LEN 8

static int
midi_gm_volume(guint8 volume)
{
	byte data[MIDI_MASTER_VOLUME_LEN] = {
		0xf0,
		0x7f,
		0x7f,
		0x04,
		0x01,
		volume,
		volume,
		0xf7};

	writer->write(writer, data, MIDI_MASTER_VOLUME_LEN);
	if (writer->flush)
		writer->flush(writer);

	return SFX_OK;
}

static int
midi_gm_allstop(void)
{
	byte data[3] = { 0xb0,
			 0x78, /* all sound off */
			 0 };
	int i;

	/* All sound off on all channels */
	for (i = 0; i < 16; i++) {
		data[0] = 0xb0 | i;
		writer->write(writer, data, 3);
	}
	if (writer->flush)
		writer->flush(writer);

	return SFX_OK;
}

static int
midi_gm_reverb(int reverb)
{
	byte data[3] = { 0xb0,
			 91, /* set reverb */
			 reverb };
	int i;

	/* Set reverb on all channels */
	for (i = 0; i < 16; i++)
		if (i != 9) {
			data[0] = 0xb0 | i;
			writer->write(writer, data, 3);
		}
	if (writer->flush)
		writer->flush(writer);

	return SFX_OK;
}

static int
midi_gm_set_option(char *x, char *y)
{
	return SFX_ERROR;
}

sfx_sequencer_t sfx_sequencer_gm = {
	"General MIDI",
	"0.1",
	SFX_DEVICE_MIDI,
	&midi_gm_set_option,
	&midi_gm_open,
	&midi_gm_close,
	&midi_gm_event,
	&midi_gm_delay,
	&midi_gm_reset_timer,
	&midi_gm_allstop,
	&midi_gm_volume,
	&midi_gm_reverb,
	004,	/* patch.004 */
	001,	/* patch.001 */
	0x01,	/* playflag */
	1,	/* do play rhythm */
	64,	/* max polyphony */
	0	/* no write-ahead needed inherently */
};
