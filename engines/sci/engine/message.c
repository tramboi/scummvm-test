/***************************************************************************
 message.c Copyright (C) 2008 Lars Skovlund


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

    Lars Skovlund (LS) [lskovlun@image.dk]

***************************************************************************/

#include "message.h"

static
int get_talker_trivial(index_record_cursor_t *cursor)
{
	return -1;
}

/* Version 2.101 and later code ahead */

static
void index_record_parse_2101(index_record_cursor_t *cursor, message_tuple_t *t)
{
	int noun = *(cursor->index_record + 0);
	int verb = *(cursor->index_record + 1);

	t->noun = noun;
	t->verb = verb;
	t->cond = t->seq = 0;
}

static
void index_record_get_text_2101(index_record_cursor_t *cursor, char *buffer, int buffer_size)
{
	int offset = getUInt16(cursor->index_record + 2);
	char *stringptr = (char *)cursor->resource_beginning + offset;
	
	strncpy(buffer, stringptr, buffer_size);
}

static
int header_get_index_record_count_2101(byte *header)
{
	return getUInt16(header + 4);
}

/* Version 3.411 and later code ahead */

static
void index_record_parse_3411(index_record_cursor_t *cursor, message_tuple_t *t)
{
	int noun = *(cursor->index_record + 0);
	int verb = *(cursor->index_record + 1);
	int cond = *(cursor->index_record + 2);
	int seq = *(cursor->index_record + 3);

	t->noun = noun;
	t->verb = verb;
	t->cond = cond;
	t->seq = seq;
}

static
int index_record_get_talker_3411(index_record_cursor_t *cursor)
{
	return *(cursor->index_record + 4);
}

static
void index_record_get_text_3411(index_record_cursor_t *cursor, char *buffer, int buffer_size)
{
	int offset = getUInt16(cursor->index_record + 5);
	char *stringptr = (char *)cursor->resource_beginning + offset;
	
	strncpy(buffer, stringptr, buffer_size);
}

static
int header_get_index_record_count_3411(byte *header)
{
	return getUInt16(header + 8);
}

/* Generic code from here on */

static
int four_tuple_match(message_tuple_t *t1, message_tuple_t *t2)
{
	return 
		t1->noun == t2->noun &&
		t1->verb == t2->verb &&
		t1->cond == t2->cond &&
		t1->seq == t2->seq;
}

static
void index_record_cursor_initialize(message_state_t *state, index_record_cursor_t *cursor)
{
	cursor->resource_beginning = state->current_res->data;
	cursor->index_record = state->index_records;
	cursor->index = 1;
}

static
int index_record_next(message_state_t *state, index_record_cursor_t *cursor)
{
	if (cursor->index == state->record_count)
		return 0;
	cursor->index_record += state->handler->index_record_size;
	cursor->index ++;
	return 1;
}

static
int index_record_find(message_state_t *state, message_tuple_t *t, index_record_cursor_t *cursor)
{
	message_tuple_t looking_at;
	int found = 0;

	index_record_cursor_initialize(state, cursor);

	do
	{
		state->handler->parse(cursor, &looking_at);
		if (four_tuple_match(t, &looking_at))
			found = 1;
	} while (!found && index_record_next(state, cursor));

	// FIXME: Recursion not handled yet

	return found;
}

int message_get_specific(message_state_t *state, message_tuple_t *t)
{
	return index_record_find(state, t, &state->engine_cursor);
}

int message_get_next(message_state_t *state)
{
	return index_record_next(state, &state->engine_cursor);
}

int message_get_talker(message_state_t *state)
{
	return state->handler->get_talker(&state->engine_cursor);
}

int message_get_text(message_state_t *state, char *buffer, int length)
{
	state->handler->get_text(&state->engine_cursor, buffer, length);
	return strlen(buffer);
}

int message_get_length(message_state_t *state)
{
	char buffer[500];

	state->handler->get_text(&state->engine_cursor, buffer, sizeof(buffer));
	return strlen(buffer);
}

int message_state_load_res(message_state_t *state, int module)
{
	if (state->module == module) 
		return 1;

	state->module = module;
	state->current_res = scir_find_resource(state->resmgr, sci_message, module, 0);

	if (state->current_res == NULL ||
	    state->current_res->data == NULL)
	{
		sciprintf("Message subsystem: Failed to load %d.MSG\n", module);
		return 0;
	}

	state->record_count = state->handler->index_record_count(state->current_res->data);
	state->index_records = state->current_res->data + state->handler->header_size;

	index_record_cursor_initialize(state, &state->engine_cursor);
	return 1;
}

static message_handler_t fixed_handler = {3411, 
					  index_record_parse_3411, 
					  index_record_get_talker_3411,
					  index_record_get_text_3411,
					  header_get_index_record_count_3411,

					  10,
					  11};

void message_state_initialize(resource_mgr_t *resmgr, message_state_t *state)
{
	resource_t *tester = scir_find_resource(resmgr, sci_message, 0, 0);
	int version;

//	if (tester == NULL) return;

//	version = getUInt16(tester->data);

	state->initialized = 1;
	state->module = -1;
	state->resmgr = resmgr;
	state->current_res = NULL;
	state->record_count = 0;
	state->handler = &fixed_handler;
}
