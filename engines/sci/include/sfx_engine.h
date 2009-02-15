/***************************************************************************
 sfx_engine.h Copyright (C) 2003,04 Christoph Reichenbach


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

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/
/* Sound engine */
#ifndef _SFX_ENGINE_H_
#define _SFX_ENGINE_H_

#include <sfx_core.h>
#include <sfx_songlib.h>
#include <sfx_iterator.h>
#include <sciresource.h>

#define SOUND_TICK 1000000 / 60
/* Approximately 16666 microseconds */


#define SFX_STATE_FLAG_MULTIPLAY (1 << 0) /* More than one song playable
					  ** simultaneously ? */
#define SFX_STATE_FLAG_NOSOUND	 (1 << 1) /* Completely disable sound playing */


#define SFX_DEBUG_SONGS		(1 << 0) /* Debug song changes */
#define SFX_DEBUG_CUES		(1 << 1) /* Debug cues, loops, and
					 ** song completions */

typedef struct {
	song_iterator_t *it; /* The song iterator at the heart of things */
	unsigned int flags; /* SFX_STATE_FLAG_* */
	songlib_t songlib; /* Song library */
	song_t *song; /* Active song, or start of active song chain */
	int suspended; /* Whether we are suspended */
	unsigned int debug; /* Debug flags */

} sfx_state_t;

/***********/
/* General */
/***********/

void
sfx_init(sfx_state_t *self, resource_mgr_t *resmgr, int flags);
/* Initializes the sound engine
** Parameters: (resource_mgr_t *) resmgr: Resource manager for initialization
**             (int) flags: SFX_STATE_FLAG_*
*/

void
sfx_exit(sfx_state_t *self);
/* Deinitializes the sound subsystem
*/

void
sfx_suspend(sfx_state_t *self, int suspend);
/* Suspends/unsuspends the sound sybsystem
** Parameters: (int) suspend: Whether to suspend (non-null) or to unsuspend
*/

int
sfx_poll(sfx_state_t *self, song_handle_t *handle, int *cue);
/* Polls the sound server for cues etc.
** Returns   : (int) 0 if the cue queue is empty, SI_LOOP, SI_CUE, or SI_FINISHED otherwise
**             (song_handle_t) *handle: The affected handle
**             (int) *cue: The sound cue number (if SI_CUE), or the loop number (if SI_LOOP)
*/

int
sfx_poll_specific(sfx_state_t *self, song_handle_t handle, int *cue);
/* Polls the sound server for cues etc.
** Parameters: (song_handle_t) handle: The handle to poll
** Returns   : (int) 0 if the cue queue is empty, SI_LOOP, SI_CUE, or SI_FINISHED otherwise
**             (int) *cue: The sound cue number (if SI_CUE), or the loop number (if SI_LOOP)
*/

int
sfx_get_volume(sfx_state_t *self);
/* Determines the current global volume settings
** Returns   : (int) The global volume, between 0 (silent) and 127 (max. volume)
*/

void
sfx_set_volume(sfx_state_t *self, int volume);
/* Determines the current global volume settings
** Parameters: (int) volume: The new global volume, between 0 and 127 (see above)
*/

void
sfx_all_stop(sfx_state_t *self);
/* Stops all songs currently playing, purges song library
*/


/*****************/
/*  Song basics  */
/*****************/

int
sfx_add_song(sfx_state_t *self, song_iterator_t *it, int priority, song_handle_t handle, int resnum);
/* Adds a song to the internal sound library
** Parameters: (song_iterator_t *) it: The iterator describing the song
**             (int) priority: Initial song priority (higher <-> more important)
**             (song_handle_t) handle: The handle to associate with the song
** Returns   : (int) 0 on success, nonzero on error
*/


void
sfx_remove_song(sfx_state_t *self, song_handle_t handle);
/* Deletes a song and its associated song iterator from the song queue
** Parameters: (song_handle_t) handle: The song to remove
*/


/**********************/
/* Song modifications */
/**********************/


void
sfx_song_set_status(sfx_state_t *self, song_handle_t handle, int status);
/* Sets the song status, i.e. whether it is playing, suspended, or stopped.
** Parameters: (song_handle_t) handle: Handle of the song to modify
**             (int) status: The song status the song should assume
** WAITING and PLAYING are set implicitly and essentially describe the same state
** as far as this function is concerned.
*/

void
sfx_song_renice(sfx_state_t *self, song_handle_t handle, int priority);
/* Sets the new song priority
** Parameters: (song_handle_t) handle: The handle to modify
**             (int) priority: The priority to set
*/

void
sfx_song_set_loops(sfx_state_t *self, song_handle_t handle, int loops);
/* Sets the number of loops for the specified song
** Parameters: (song_handle_t) handle: The song handle to reference
**             (int) loops: Number of loops to set
*/

void
sfx_song_set_hold(sfx_state_t *self, song_handle_t handle, int hold);
/* Sets the number of loops for the specified song
** Parameters: (song_handle_t) handle: The song handle to reference
**             (int) hold: Number of loops to setn
*/

void
sfx_song_set_fade(sfx_state_t *self, song_handle_t handle, fade_params_t *fade_setup);
/* Instructs a song to be faded out
** Parameters: (song_handle_t) handle: The song handle to reference
**             (fade_params_t *) fade_setup: The precise fade-out configuration to use
*/


#endif /* !defined(_SFX_ENGINE_H_) */
