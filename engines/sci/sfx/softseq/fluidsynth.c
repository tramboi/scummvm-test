/***************************************************************************
 fluidsynth.c Copyright (C) 2008 Walter van Niftrik

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


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

   Walter van Niftrik <w.f.b.w.v.niftrik@stud.tue.nl>

***************************************************************************/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef HAVE_FLUIDSYNTH_H

#include <fluidsynth.h>

#include "../softseq.h"
#include "../sequencer.h"
#include "../device.h"
#include "resource.h"

static sfx_sequencer_t *gmseq;
static fluid_settings_t* settings;
static fluid_synth_t* synth;
static guint8 status;
static char *soundfont = "/etc/midi/8MBGMSFX.SF2";
static int rpn[16];

#define SAMPLE_RATE 44100
#define CHANNELS SFX_PCM_STEREO_LR

/* MIDI writer */

static int
fluidsynth_midi_init(struct _midi_writer *self)
{
	return SFX_OK;
}

static int
fluidsynth_midi_set_option(struct _midi_writer *self, char *name, char *value)
{
	return SFX_ERROR;
}

static int
fluidsynth_midi_write(struct _midi_writer *self, unsigned char *buf, int len)
{
	if (buf[0] == 0xf0)
		sciprintf("FluidSynth: Skipping sysex message.\n");
	else if (len == 2) {
		guint8 command, channel;

		command = buf[0] & 0xf0;
		channel = buf[0] & 0x0f;

		switch(command) {
		case 0xc0:
			fluid_synth_program_change(synth, channel, buf[1]);
			break;
		default:
			printf("FluidSynth: MIDI command [%02x %02x] not supported\n", buf[0], buf[1]);
		}
	}
	else if (len == 3) {
		guint8 command, channel;

		command = buf[0] & 0xf0;
		channel = buf[0] & 0x0f;

		switch(command) {
		case 0x80:
			fluid_synth_noteoff(synth, channel, buf[1]);
			break;
		case 0x90:
			fluid_synth_noteon(synth, channel, buf[1], buf[2]);
			break;
		case 0xb0:
			switch (buf[1]) {
			case 0x06:
				/* Data Entry Slider - course */
				if (rpn[channel] == 0)
					fluid_synth_pitch_wheel_sens(synth, channel, buf[2]);
				else
					sciprintf("FluidSynth: RPN %i not supported\n", rpn[channel]);
			case 0x64:
				/* Registered Parameter Number (RPN) - fine */
				rpn[channel] &= ~0x7f;
				rpn[channel] |= buf[2] & 0x7f;
				break;
			case 0x65:
				/* Registered Parameter Number (RPN) - course */
				rpn[channel] &= ~0x3f80;
				rpn[channel] |= (buf[2] & 0x7f) << 7;
				break;
			default:
				fluid_synth_cc(synth, channel, buf[1], buf[2]);
			}
			break;
		case 0xe0:
			fluid_synth_pitch_bend(synth, channel, (buf[2] << 7) | buf[1]);
			break;
		default:
			sciprintf("FluidSynth: MIDI command [%02x %02x %02x] not supported\n", buf[0], buf[1], buf[2]);
		}
	}
	else
		sciprintf("FluidSynth: Skipping invalid message of %i bytes.\n", len);

	return SFX_OK;
}

static void
fluidsynth_midi_delay(struct _midi_writer *self, int ticks)
{
}

static void
fluidsynth_midi_reset_timer(struct _midi_writer *self)
{
}

static void
fluidsynth_midi_close(struct _midi_writer *self)
{
}

static midi_writer_t midi_writer_fluidsynth = {
	"fluidsynth",
	fluidsynth_midi_init,
	fluidsynth_midi_set_option,
	fluidsynth_midi_write,
	fluidsynth_midi_delay,
	NULL,
	fluidsynth_midi_reset_timer,
	fluidsynth_midi_close
};

/* Software sequencer */

static void
fluidsynth_poll(sfx_softseq_t *self, byte *dest, int count)
{
	fluid_synth_write_s16(synth, count, dest, 0, 2, dest + 2, 0, 2);
}

static int
fluidsynth_init(sfx_softseq_t *self, byte *data_ptr, int data_length,
		byte *data2_ptr, int data2_length)
{
	int sfont_id;
	double min, max;

	if (0) {
		sciprintf("FluidSynth ERROR: Mono sound output not supported.\n");
		return SFX_ERROR;
	}

	gmseq = sfx_find_sequencer("General MIDI");
	if (!gmseq) {
		sciprintf("FluidSynth ERROR: Unable to find General MIDI sequencer.\n");
		return SFX_ERROR;
	}

	settings = new_fluid_settings();

	fluid_settings_getnum_range(settings, "synth.sample-rate", &min, &max);
	if (SAMPLE_RATE < min || SAMPLE_RATE > max) {
		sciprintf("FluidSynth ERROR: Sample rate '%i' not supported. Valid "
			"range is (%i-%i).\n", SAMPLE_RATE, (int) min, (int) max);
		delete_fluid_settings(settings);
		return SFX_ERROR;
	}

	fluid_settings_setnum(settings, "synth.sample-rate", SAMPLE_RATE);
	fluid_settings_setnum(settings, "synth.gain", 0.5f);

	synth = new_fluid_synth(settings);

	if ((sfont_id = fluid_synth_sfload(synth, soundfont, 1)) < 0) {
		delete_fluid_synth(synth);
		delete_fluid_settings(settings);
		return SFX_ERROR;
	}

	gmseq->open(data_length, data_ptr, data2_length, data2_ptr,
		    &midi_writer_fluidsynth);

	return SFX_OK;
}

static void
fluidsynth_exit(sfx_softseq_t *self)
{
	delete_fluid_synth(synth);
	delete_fluid_settings(settings);
}

static void
fluidsynth_allstop(sfx_softseq_t *self)
{
	if (gmseq->allstop)
		gmseq->allstop();
}

static void
fluidsynth_volume(sfx_softseq_t *self, int volume)
{
	if (gmseq->volume)
		gmseq->volume(volume);
}

static int
fluidsynth_set_option(sfx_softseq_t *self, char *name, char *value)
{
	return SFX_ERROR;
}

static void
fluidsynth_event(sfx_softseq_t *self, byte cmd, int argc, byte *argv)
{
	gmseq->event(cmd, argc, argv);
}

sfx_softseq_t sfx_softseq_fluidsynth = {
	"fluidsynth",
	"0.1",
	fluidsynth_set_option,
	fluidsynth_init,
	fluidsynth_exit,
	fluidsynth_volume,
	fluidsynth_event,
	fluidsynth_poll,
	fluidsynth_allstop,
	NULL,
	004,	/* patch.004 */
	001,	/* patch.001 */
	0x01,	/* playflag */
	1,	/* do play channel 9 */
	32, /* Max polypgony */
	{SAMPLE_RATE, CHANNELS, SFX_PCM_FORMAT_S16_NATIVE}
};

#endif /* HAVE_FLUIDSYNTH_H */
