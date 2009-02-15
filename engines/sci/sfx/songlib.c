/***************************************************************************
 songlib.c Copyright (C) 2002 Christoph Reichenbach

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

   Christoph Reichenbach (CJR) <reichenb@colorado.edu>

***************************************************************************/

#include <stdio.h>
#include <sfx_engine.h>
#include <sci_memory.h>

#define debug_stream stderr

GTimeVal
song_sleep_time(GTimeVal *lastslept, long ticks)
{
	GTimeVal tv;
	long timetosleep;
	long timeslept; /* Time already slept */
	timetosleep = ticks * SOUND_TICK; /* Time to sleep in us */

	sci_get_current_time(&tv);
	timeslept = 1000000 * (tv.tv_sec - lastslept->tv_sec) +
		tv.tv_usec - lastslept->tv_usec;

	timetosleep -= timeslept;

	if (timetosleep < 0)
		timetosleep = 0;

	tv.tv_sec = timetosleep / 1000000;
	tv.tv_usec = timetosleep % 1000000;

	return tv;
}


GTimeVal
song_next_wakeup_time(GTimeVal *lastslept, long ticks)
{
	GTimeVal retval;

	retval.tv_sec = lastslept->tv_sec + (ticks / 60);
	retval.tv_usec = lastslept->tv_usec + ((ticks % 60) * SOUND_TICK);

	if (retval.tv_usec >= 1000000) {
		retval.tv_usec -= 1000000;
		++retval.tv_sec;
	}

	return retval;
}


song_t *
song_new(song_handle_t handle, song_iterator_t *it, int priority)
{
	song_t *retval;
	retval = (song_t*) sci_malloc(sizeof(song_t));

#ifdef SATISFY_PURIFY
	memset(retval, 0, sizeof(song_t));
#endif

	retval->handle = handle;
	retval->priority = priority;
	retval->next = NULL;
	retval->delay = 0;
	retval->it = it;
	retval->status = SOUND_STATUS_STOPPED;
	retval->next_playing = NULL;
	retval->next_stopping = NULL;
	retval->restore_behavior = RESTORE_BEHAVIOR_CONTINUE;
	retval->restore_time = 0;

	return retval;
}


void
song_lib_add(songlib_t songlib, song_t *song)
{
	song_t **seeker = NULL;
	int pri	= song->priority;

	if (NULL == song) {
		 sciprintf("song_lib_add(): NULL passed for song\n");
		 return;
	}

	if (*(songlib.lib) == NULL) {
		*(songlib.lib) = song;
		song->next = NULL;

		return;
	}

	seeker = (songlib.lib);
	while (*seeker && ((*seeker)->priority > pri))
		seeker = &((*seeker)->next);

	song->next = *seeker;
	*seeker = song;
}

static void /* Recursively free a chain of songs */
_songfree_chain(song_t *song)
{
	if (song) {
		_songfree_chain(song->next);
		songit_free(song->it);
		song->it = NULL;
		free(song);
	}
}


void
song_lib_init(songlib_t *songlib)
{
	songlib->lib = &(songlib->_s);
	songlib->_s = NULL;
}

void
song_lib_free(songlib_t songlib)
{
	_songfree_chain(*(songlib.lib));
	*(songlib.lib) = NULL;
}


song_t *
song_lib_find(songlib_t songlib, song_handle_t handle)
{
	song_t *seeker = *(songlib.lib);

	while (seeker) {
		if (seeker->handle == handle)
			break;
		seeker = seeker->next;
	}

	return seeker;
}


song_t *
song_lib_find_next_active(songlib_t songlib, song_t *other)
{
	song_t *seeker = other? other->next : *(songlib.lib);

	while (seeker) {
		if ((seeker->status == SOUND_STATUS_WAITING) ||
		    (seeker->status == SOUND_STATUS_PLAYING))
			break;
		seeker = seeker->next;
	}

	/* Only return songs that have equal priority */
	if (other && seeker && other->priority > seeker->priority)
		return NULL;

	return seeker;
}

song_t *
song_lib_find_active(songlib_t songlib)
{
	return song_lib_find_next_active(songlib, NULL);
}

int
song_lib_remove(songlib_t songlib, song_handle_t handle)
{
	int retval;
	song_t *goner = *(songlib.lib);

	if (!goner)
		return -1;

	if (goner->handle == handle)
		*(songlib.lib) = goner->next;

	else {
		while ((goner->next) && (goner->next->handle != handle))
			goner = goner->next;

		if (goner->next) { /* Found him? */
			song_t *oldnext = goner->next;

			goner->next = goner->next->next;
			goner = oldnext;
		} else return -1; /* No. */
	}

	retval = goner->status;

	songit_free(goner->it);
	free(goner);

	return retval;
}

void
song_lib_resort(songlib_t songlib, song_t *song)
{
	if (*(songlib.lib) == song)
		*(songlib.lib) = song->next;
	else {
		song_t *seeker = *(songlib.lib);

		while (seeker->next && (seeker->next != song))
			seeker = seeker->next;

		if (seeker->next)
			seeker->next = seeker->next->next;
	}

	song_lib_add(songlib, song);
}

int
song_lib_count(songlib_t songlib)
{
	song_t *seeker = *(songlib.lib);
	int retval = 0;

	while (seeker) {
		retval++;
		seeker = seeker->next;
	}

	return retval;
}

void
song_lib_set_restore_behavior(songlib_t songlib, song_handle_t handle, RESTORE_BEHAVIOR action)
{
	song_t *seeker = song_lib_find(songlib, handle);

	seeker->restore_behavior = action;
}

void
song_lib_dump(songlib_t songlib, int line)
{
	song_t *seeker = *(songlib.lib);

	fprintf(debug_stream,"L%d:", line);
	do {
		fprintf(debug_stream,"    %p", (void *)seeker);

		if (seeker) {
			fprintf(debug_stream,"[%04lx,p=%d,s=%d]->", seeker->handle, seeker->priority,seeker->status);
			seeker = seeker->next;
		}
		fprintf(debug_stream,"\n");
	} while (seeker);
	fprintf(debug_stream,"\n");

}

