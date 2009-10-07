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

#include "common/stream.h"
#include "common/system.h"
#include "common/func.h"
#include "common/serializer.h"

#include <time.h>	// FIXME: For struct tm


#include "sci/sci.h"
#include "sci/gfx/operations.h"
#include "sci/gfx/menubar.h"
#include "sci/gfx/gfx_state_internal.h"	// required for GfxPort, GfxContainer
#include "sci/sfx/core.h"
#include "sci/sfx/iterator.h"
#include "sci/engine/state.h"
#include "sci/engine/savegame.h"
#include "sci/gui/gui.h"

namespace Sci {


#define VER(x) Common::Serializer::Version(x)


// OBSOLETE: This const is used for backward compatibility only.
const uint32 INTMAPPER_MAGIC_KEY = 0xDEADBEEF;


// from ksound.cpp:
SongIterator *build_iterator(EngineState *s, int song_nr, SongIteratorType type, songit_id_t id);

#pragma mark -

// TODO: Many of the following sync_*() methods should be turned into member funcs
// of the classes they are syncing.

static void sync_songlib_t(Common::Serializer &s, SongLibrary &obj);

static void sync_reg_t(Common::Serializer &s, reg_t &obj) {
	s.syncAsUint16LE(obj.segment);
	s.syncAsUint16LE(obj.offset);
}

static void sync_song_t(Common::Serializer &s, Song &obj) {
	s.syncAsSint32LE(obj._handle);
	s.syncAsSint32LE(obj._resourceNum);
	s.syncAsSint32LE(obj._priority);
	s.syncAsSint32LE(obj._status);
	s.syncAsSint32LE(obj._restoreBehavior);
	s.syncAsSint32LE(obj._restoreTime);
	s.syncAsSint32LE(obj._loops);
	s.syncAsSint32LE(obj._hold);

	if (s.isLoading()) {
		obj._it = 0;
		obj._delay = 0;
		obj._next = 0;
		obj._nextPlaying = 0;
		obj._nextStopping = 0;
	}
}


// Experimental hack: Use syncWithSerializer to sync. By default, this assume
// the object to be synced is a subclass of Serializable and thus tries to invoke
// the saveLoadWithSerializer() method. But it is possible to specialize this
// template function to handle stuff that is not implementing that interface.
template<typename T>
void syncWithSerializer(Common::Serializer &s, T &obj) {
	obj.saveLoadWithSerializer(s);
}

// By default, sync using syncWithSerializer, which in turn can easily be overloaded.
template <typename T>
struct DefaultSyncer : Common::BinaryFunction<Common::Serializer, T, void> {
	void operator()(Common::Serializer &s, T &obj) const {
		//obj.saveLoadWithSerializer(s);
		syncWithSerializer(s, obj);
	}
};

/**
 * Sync a Common::Array using a Common::Serializer.
 * When saving, this writes the length of the array, then syncs (writes) all entries.
 * When loading, it loads the length of the array, then resizes it accordingly, before
 * syncing all entries.
 *
 * Note: This shouldn't be in common/array.h nor in common/serializer.h, after
 * all, not all code using arrays wants to use the serializer, and vice versa.
 * But we could put this into a separate header file in common/ at some point.
 * Something like common/serializer-extras.h or so.
 *
 * TODO: Add something like this for lists, queues....
 */
template <typename T, class Syncer = DefaultSyncer<T> >
struct ArraySyncer : Common::BinaryFunction<Common::Serializer, T, void> {
	void operator()(Common::Serializer &s, Common::Array<T> &arr) const {
		uint len = arr.size();
		s.syncAsUint32LE(len);
		Syncer sync;

		// Resize the array if loading.
		if (s.isLoading())
			arr.resize(len);

		typename Common::Array<T>::iterator i;
		for (i = arr.begin(); i != arr.end(); ++i) {
			sync(s, *i);
		}
	}
};

// Convenience wrapper
template<typename T>
void syncArray(Common::Serializer &s, Common::Array<T> &arr) {
	ArraySyncer<T> sync;
	sync(s, arr);
}


template <>
void syncWithSerializer(Common::Serializer &s, reg_t &obj) {
	sync_reg_t(s, obj);
}



void MenuItem::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(_type);
	s.syncString(_keytext);
	s.skip(4, VER(9), VER(9)); 	// OBSOLETE: Used to be keytext_size

	s.syncAsSint32LE(_flags);
	s.syncBytes(_said, MENU_SAID_SPEC_SIZE);
	sync_reg_t(s, _saidPos);
	s.syncString(_text);
	sync_reg_t(s, _textPos);
	s.syncAsSint32LE(_modifiers);
	s.syncAsSint32LE(_key);
	s.syncAsSint32LE(_enabled);
	s.syncAsSint32LE(_tag);
}

void Menu::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncString(_title);
	s.syncAsSint32LE(_titleWidth);
	s.syncAsSint32LE(_width);

	syncArray<MenuItem>(s, _items);
}


void Menubar::saveLoadWithSerializer(Common::Serializer &s) {
	syncArray<Menu>(s, _menus);
}

void SegManager::saveLoadWithSerializer(Common::Serializer &s) {
	s.skip(4, VER(9), VER(9));	// OBSOLETE: Used to be reserved_id
	s.syncAsSint32LE(_exportsAreWide);
	s.skip(4, VER(9), VER(9));	// OBSOLETE: Used to be gc_mark_bits

	if (s.isLoading()) {
		// Reset _scriptSegMap, to be restored below
		_scriptSegMap.clear();

		if (s.getVersion() <= 9) {
			// OBSOLETE: Skip over the old id_seg_map when loading (we now
			// regenerate the equivalent data, in _scriptSegMap, from scratch).

			s.skip(4);	// base_value
			while (true) {
				uint32 key = 0;
				s.syncAsSint32LE(key);
				if (key == INTMAPPER_MAGIC_KEY)
					break;
				s.skip(4);	// idx
			}
		}
	}


	uint sync_heap_size = _heap.size();
	s.syncAsUint32LE(sync_heap_size);
	_heap.resize(sync_heap_size);
	for (uint i = 0; i < sync_heap_size; ++i) {
		SegmentObj *&mobj = _heap[i];

		// Sync the segment type
		SegmentType type = (s.isSaving() && mobj) ? mobj->getType() : SEG_TYPE_INVALID;
		s.syncAsUint32LE(type);

		// If we were saving and mobj == 0, or if we are loading and this is an
		// entry marked as empty -> skip to next
		if (type == SEG_TYPE_INVALID) {
			mobj = 0;
			continue;
		}

		if (s.isLoading()) {
			mobj = SegmentObj::createSegmentObj(type);
		}
		assert(mobj);

		s.skip(4, VER(9), VER(9));	// OBSOLETE: Used to be _segManagerId

		// Let the object sync custom data
		mobj->saveLoadWithSerializer(s);

		// If we are loading a script, hook it up in the script->segment map.
		if (s.isLoading() && type == SEG_TYPE_SCRIPT) {
			_scriptSegMap[((Script *)mobj)->_nr] = i;
		}
	}

	s.syncAsSint32LE(Clones_seg_id);
	s.syncAsSint32LE(Lists_seg_id);
	s.syncAsSint32LE(Nodes_seg_id);
}

static void sync_SegManagerPtr(Common::Serializer &s, ResourceManager *&resMan, SegManager *&obj) {
	s.skip(1, VER(9), VER(9));	// obsolete: used to be a flag indicating if we got sci11 or not

	if (s.isLoading()) {
		// FIXME: Do in-place loading at some point, instead of creating a new EngineState instance from scratch.
		delete obj;
		obj = new SegManager(resMan);
	}

	obj->saveLoadWithSerializer(s);
}



template <>
void syncWithSerializer(Common::Serializer &s, Class &obj) {
	s.syncAsSint32LE(obj.script);
	sync_reg_t(s, obj.reg);
}

static void sync_sfx_state_t(Common::Serializer &s, SfxState &obj) {
	sync_songlib_t(s, obj._songlib);
}

static void sync_SavegameMetadata(Common::Serializer &s, SavegameMetadata &obj) {
	// TODO: It would be a good idea to store a magic number & a header size here,
	// so that we can implement backward compatibility if the savegame format changes.

	s.syncString(obj.savegame_name);
	s.syncVersion(CURRENT_SAVEGAME_VERSION);
	obj.savegame_version = s.getVersion();
	s.syncString(obj.game_version);
	s.skip(4, VER(9), VER(9));	// obsolete: used to be game version
	s.syncAsSint32LE(obj.savegame_date);
	s.syncAsSint32LE(obj.savegame_time);
}

void EngineState::saveLoadWithSerializer(Common::Serializer &s) {
	s.skip(4, VER(9), VER(9));	// OBSOLETE: Used to be savegame_version

	Common::String tmp;
	s.syncString(tmp);			// OBSOLETE: Used to be game_version
	s.skip(4, VER(9), VER(9));	// OBSOLETE: Used to be version

	// FIXME: Do in-place loading at some point, instead of creating a new EngineState instance from scratch.
	if (s.isLoading()) {
		//free(menubar);
		_menubar = new Menubar();
	} else
		assert(_menubar);
	_menubar->saveLoadWithSerializer(s);

	s.syncAsSint32LE(status_bar_foreground);
	s.syncAsSint32LE(status_bar_background);

	sync_SegManagerPtr(s, resMan, _segMan);

	syncArray<Class>(s, _segMan->_classtable);

	sync_sfx_state_t(s, _sound);
}

void LocalVariables::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(script_id);
	syncArray<reg_t>(s, _locals);
}

template <>
void syncWithSerializer(Common::Serializer &s, Object &obj) {
	s.syncAsSint32LE(obj._flags);
	sync_reg_t(s, obj._pos);
	s.syncAsSint32LE(obj.variable_names_nr);
	s.syncAsSint32LE(obj.methods_nr);

	syncArray<reg_t>(s, obj._variables);
}

template <>
void syncWithSerializer(Common::Serializer &s, Table<Clone>::Entry &obj) {
	s.syncAsSint32LE(obj.next_free);

	syncWithSerializer<Object>(s, obj);
}

template <>
void syncWithSerializer(Common::Serializer &s, Table<List>::Entry &obj) {
	s.syncAsSint32LE(obj.next_free);

	sync_reg_t(s, obj.first);
	sync_reg_t(s, obj.last);
}

template <>
void syncWithSerializer(Common::Serializer &s, Table<Node>::Entry &obj) {
	s.syncAsSint32LE(obj.next_free);

	sync_reg_t(s, obj.pred);
	sync_reg_t(s, obj.succ);
	sync_reg_t(s, obj.key);
	sync_reg_t(s, obj.value);
}

template <typename T>
void sync_Table(Common::Serializer &s, T &obj) {
	s.syncAsSint32LE(obj.first_free);
	s.syncAsSint32LE(obj.entries_used);

	syncArray<typename T::Entry>(s, obj._table);
}

void CloneTable::saveLoadWithSerializer(Common::Serializer &s) {
	sync_Table<CloneTable>(s, *this);
}

void NodeTable::saveLoadWithSerializer(Common::Serializer &s) {
	sync_Table<NodeTable>(s, *this);
}

void ListTable::saveLoadWithSerializer(Common::Serializer &s) {
	sync_Table<ListTable>(s, *this);
}

void HunkTable::saveLoadWithSerializer(Common::Serializer &s) {
	if (s.isLoading()) {
		initTable();
	}
}

void Script::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(_nr);
	s.syncAsUint32LE(_bufSize);
	s.syncAsUint32LE(_scriptSize);
	s.syncAsUint32LE(_heapSize);

	if (s.getVersion() <= 10) {
		assert((s.isLoading()));
		// OBSOLETE: Skip over the old _objIndices data when loading
		s.skip(4);	// base_value
		while (true) {
			uint32 key = 0;
			s.syncAsSint32LE(key);
			if (key == INTMAPPER_MAGIC_KEY)
				break;
			s.skip(4);	// idx
		}
	}

	s.syncAsSint32LE(_numExports);
	s.syncAsSint32LE(_numSynonyms);
	s.syncAsSint32LE(_lockers);

	// Sync _objects. This is a hashmap, and we use the following on disk format:
	// First we store the number of items in the hashmap, then we store each
	// object (which is an 'Object' instance). For loading, we take advantage
	// of the fact that the key of each Object obj is just obj._pos.offset !
	// By "chance" this format is identical to the format used to sync Common::Array<>,
	// hence we can still old savegames with identical code :).

	uint numObjs = _objects.size();
	s.syncAsUint32LE(numObjs);

	if (s.isLoading()) {
		_objects.clear();
		Object tmp;
		for (uint i = 0; i < numObjs; ++i) {
			syncWithSerializer<Object>(s, tmp);
			_objects[tmp._pos.offset] = tmp;
		}
	} else {
		ObjMap::iterator it;
		const ObjMap::iterator end = _objects.end();
		for (it = _objects.begin(); it != end; ++it) {
			syncWithSerializer<Object>(s, it->_value);
		}
	}

	s.syncAsSint32LE(_localsOffset);
	s.syncAsSint32LE(_localsSegment);

	s.syncAsSint32LE(_markedAsDeleted);
}

static void sync_SystemString(Common::Serializer &s, SystemString &obj) {
	s.syncString(obj._name);
	s.syncAsSint32LE(obj._maxSize);

	// Sync obj._value. We cannot use syncCStr as we must make sure that
	// the allocated buffer has the correct size, i.e., obj._maxSize
	Common::String tmp;
	if (s.isSaving() && obj._value)
		tmp = obj._value;
	s.syncString(tmp);
	if (s.isLoading()) {
		//free(*str);
		obj._value = (char *)calloc(obj._maxSize, sizeof(char));
		strncpy(obj._value, tmp.c_str(), obj._maxSize);
	}
}

void SystemStrings::saveLoadWithSerializer(Common::Serializer &s) {
	for (int i = 0; i < SYS_STRINGS_MAX; ++i)
		sync_SystemString(s, _strings[i]);
}

void DynMem::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(_size);
	s.syncString(_description);
	if (!_buf && _size) {
		_buf = (byte *)calloc(_size, 1);
	}
	if (_size)
		s.syncBytes(_buf, _size);
}

void DataStack::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsUint32LE(_capacity);
	if (s.isLoading()) {
		//free(entries);
		_entries = (reg_t *)calloc(_capacity, sizeof(reg_t));
	}
}

void StringFrag::saveLoadWithSerializer(Common::Serializer &s) {
	// TODO
}

#pragma mark -

static void sync_songlib_t(Common::Serializer &s, SongLibrary &obj) {
	int songcount = 0;
	if (s.isSaving())
		songcount = obj.countSongs();
	s.syncAsUint32LE(songcount);

	if (s.isLoading()) {
		obj._lib = 0;
		while (songcount--) {
			Song *newsong = (Song *)calloc(1, sizeof(Song));
			sync_song_t(s, *newsong);
			obj.addSong(newsong);
		}
	} else {
		Song *seeker = obj._lib;
		while (seeker) {
			seeker->_restoreTime = seeker->_it->getTimepos();
			sync_song_t(s, *seeker);
			seeker = seeker->_next;
		}
	}
}

#pragma mark -


int gamestate_save(EngineState *s, Common::WriteStream *fh, const char* savename, const char *version) {
	tm curTime;
	g_system->getTimeAndDate(curTime);

	SavegameMetadata meta;
	meta.savegame_version = CURRENT_SAVEGAME_VERSION;
	meta.savegame_name = savename;
	meta.game_version = version;
	meta.savegame_date = ((curTime.tm_mday & 0xFF) << 24) | (((curTime.tm_mon + 1) & 0xFF) << 16) | ((curTime.tm_year + 1900) & 0xFFFF);
	meta.savegame_time = ((curTime.tm_hour & 0xFF) << 16) | (((curTime.tm_min) & 0xFF) << 8) | ((curTime.tm_sec) & 0xFF);

	if (s->execution_stack_base) {
		warning("Cannot save from below kernel function");
		return 1;
	}

/*
	if (s->sound_server) {
		if ((s->sound_server->save)(s, dirname)) {
			warning("Saving failed for the sound subsystem");
			//chdir("..");
			return 1;
		}
	}
*/
	// Calculate the time spent with this game
	s->game_time = (g_system->getMillis() - s->game_start_time) / 1000;

	Common::Serializer ser(0, fh);
	sync_SavegameMetadata(ser, meta);
	s->saveLoadWithSerializer(ser);		// FIXME: Error handling?

	return 0;
}

static byte *find_unique_script_block(EngineState *s, byte *buf, int type) {
	bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);

	if (oldScriptHeader)
		buf += 2;

	do {
		int seeker_type = READ_LE_UINT16(buf);

		if (seeker_type == 0) break;
		if (seeker_type == type) return buf;

		int seeker_size = READ_LE_UINT16(buf + 2);
		assert(seeker_size > 0);
		buf += seeker_size;
	} while (1);

	return NULL;
}

// FIXME: This should probably be turned into an EngineState method
static void reconstruct_stack(EngineState *retval) {
	SegmentId stack_seg = retval->_segMan->findSegmentByType(SEG_TYPE_STACK);
	DataStack *stack = (DataStack *)(retval->_segMan->_heap[stack_seg]);

	retval->stack_segment = stack_seg;
	retval->stack_base = stack->_entries;
	retval->stack_top = stack->_entries + stack->_capacity;
}

static void load_script(EngineState *s, Script *scr) {
	Resource *script, *heap = NULL;

	scr->_buf = (byte *)malloc(scr->_bufSize);
	assert(scr->_buf);

	script = s->resMan->findResource(ResourceId(kResourceTypeScript, scr->_nr), 0);
	if (getSciVersion() >= SCI_VERSION_1_1)
		heap = s->resMan->findResource(ResourceId(kResourceTypeHeap, scr->_nr), 0);

	memcpy(scr->_buf, script->data, script->size);
	if (getSciVersion() >= SCI_VERSION_1_1) {
		scr->_heapStart = scr->_buf + scr->_scriptSize;
		memcpy(scr->_heapStart, heap->data, heap->size);
	}
}

// FIXME: The following should likely become a SegManager method
static void reconstruct_scripts(EngineState *s, SegManager *self) {
	uint i;
	SegmentObj *mobj;

	for (i = 0; i < self->_heap.size(); i++) {
		if (self->_heap[i]) {
			mobj = self->_heap[i];
			switch (mobj->getType())  {
			case SEG_TYPE_SCRIPT: {
				Script *scr = (Script *)mobj;

				// FIXME: Unify this code with script_instantiate_*
				load_script(s, scr);
				scr->_localsBlock = (scr->_localsSegment == 0) ? NULL : (LocalVariables *)(s->_segMan->_heap[scr->_localsSegment]);
				if (getSciVersion() >= SCI_VERSION_1_1) {
					scr->_exportTable = 0;
					scr->_synonyms = 0;
					if (READ_LE_UINT16(scr->_buf + 6) > 0) {
						scr->setExportTableOffset(6);
						s->_segMan->scriptRelocateExportsSci11(i);
					}
				} else {
					scr->_exportTable = (uint16 *) find_unique_script_block(s, scr->_buf, SCI_OBJ_EXPORTS);
					scr->_synonyms = find_unique_script_block(s, scr->_buf, SCI_OBJ_SYNONYMS);
					scr->_exportTable += 3;
				}
				scr->_codeBlocks.clear();

				ObjMap::iterator it;
				const ObjMap::iterator end = scr->_objects.end();
				for (it = scr->_objects.begin(); it != end; ++it) {
					byte *data = scr->_buf + it->_value._pos.offset;
					it->_value.base = scr->_buf;
					it->_value.base_obj = data;
				}
				break;
			}
			default:
				break;
			}
		}
	}

	for (i = 0; i < self->_heap.size(); i++) {
		if (self->_heap[i]) {
			mobj = self->_heap[i];
			switch (mobj->getType())  {
			case SEG_TYPE_SCRIPT: {
				Script *scr = (Script *)mobj;

				ObjMap::iterator it;
				const ObjMap::iterator end = scr->_objects.end();
				for (it = scr->_objects.begin(); it != end; ++it) {
					byte *data = scr->_buf + it->_value._pos.offset;

					if (getSciVersion() >= SCI_VERSION_1_1) {
						uint16 *funct_area = (uint16 *) (scr->_buf + READ_LE_UINT16( data + 6 ));
						uint16 *prop_area = (uint16 *) (scr->_buf + READ_LE_UINT16( data + 4 ));

						it->_value.base_method = funct_area;
						it->_value.base_vars = prop_area;
					} else {
						int funct_area = READ_LE_UINT16( data + SCRIPT_FUNCTAREAPTR_OFFSET );
						Object *base_obj;

						base_obj = s->_segMan->getObject(it->_value.getSpeciesSelector());

						if (!base_obj) {
							warning("Object without a base class: Script %d, index %d (reg address %04x:%04x",
								  scr->_nr, i, PRINT_REG(it->_value.getSpeciesSelector()));
							continue;
						}
						it->_value.variable_names_nr = base_obj->_variables.size();
						it->_value.base_obj = base_obj->base_obj;

						it->_value.base_method = (uint16 *)(data + funct_area);
						it->_value.base_vars = (uint16 *)(data + it->_value.variable_names_nr * 2 + SCRIPT_SELECTOR_OFFSET);
					}
				}
				break;
			}
			default:
				break;
			}
		}
	}
}

int _reset_graphics_input(EngineState *s);

static void reconstruct_sounds(EngineState *s) {
	Song *seeker;
	SongIteratorType it_type;

	if (getSciVersion() > SCI_VERSION_01)
		it_type = SCI_SONG_ITERATOR_TYPE_SCI1;
	else
		it_type = SCI_SONG_ITERATOR_TYPE_SCI0;

	seeker = s->_sound._songlib._lib;

	while (seeker) {
		SongIterator *base, *ff;
		int oldstatus;
		SongIterator::Message msg;

		base = ff = build_iterator(s, seeker->_resourceNum, it_type, seeker->_handle);
		if (seeker->_restoreBehavior == RESTORE_BEHAVIOR_CONTINUE)
			ff = new_fast_forward_iterator(base, seeker->_restoreTime);
		ff->init();

		msg = SongIterator::Message(seeker->_handle, SIMSG_SET_LOOPS(seeker->_loops));
		songit_handle_message(&ff, msg);
		msg = SongIterator::Message(seeker->_handle, SIMSG_SET_HOLD(seeker->_hold));
		songit_handle_message(&ff, msg);

		oldstatus = seeker->_status;
		seeker->_status = SOUND_STATUS_STOPPED;
		seeker->_it = ff;
		s->_sound.sfx_song_set_status(seeker->_handle, oldstatus);
		seeker = seeker->_next;
	}
}

void internal_stringfrag_strncpy(EngineState *s, reg_t *dest, reg_t *src, int len);

EngineState *gamestate_restore(EngineState *s, Common::SeekableReadStream *fh) {
	EngineState *retval;
	SongLibrary temp;

/*
	if (s->sound_server) {
		if ((s->sound_server->restore)(s, dirname)) {
			warning("Restoring failed for the sound subsystem");
			return NULL;
		}
	}
*/

	SavegameMetadata meta;

	Common::Serializer ser(fh, 0);
	sync_SavegameMetadata(ser, meta);

	if (fh->eos())
		return false;

	if ((meta.savegame_version < MINIMUM_SAVEGAME_VERSION) ||
	    (meta.savegame_version > CURRENT_SAVEGAME_VERSION)) {
		if (meta.savegame_version < MINIMUM_SAVEGAME_VERSION)
			warning("Old savegame version detected- can't load");
		else
			warning("Savegame version is %d- maximum supported is %0d", meta.savegame_version, CURRENT_SAVEGAME_VERSION);

		return NULL;
	}

	// FIXME: Do in-place loading at some point, instead of creating a new EngineState instance from scratch.
	retval = new EngineState(s->resMan, s->_kernel, s->_voc, s->_flags);

	// Copy some old data
	retval->gfx_state = s->gfx_state;
	retval->sound_mute = s->sound_mute;
	retval->sound_volume = s->sound_volume;

	retval->saveLoadWithSerializer(ser);	// FIXME: Error handling?

	s->_sound.sfx_exit();

	// Set exec stack base to zero
	retval->execution_stack_base = 0;

	// Now copy all current state information
	// Graphics and input state:
	retval->gfx_state = s->gfx_state;
	retval->old_screen = 0;

	temp = retval->_sound._songlib;
	retval->_sound.sfx_init(retval->resMan, s->sfx_init_flags);
	retval->sfx_init_flags = s->sfx_init_flags;
	retval->_sound._songlib.freeSounds();
	retval->_sound._songlib = temp;

	_reset_graphics_input(retval);
	reconstruct_stack(retval);
	reconstruct_scripts(retval, retval->_segMan);
	retval->_segMan->reconstructClones();
	retval->game_obj = s->game_obj;
	retval->script_000 = retval->_segMan->getScript(retval->_segMan->getScriptSegment(0, SCRIPT_GET_DONT_LOAD));
	retval->gc_countdown = GC_INTERVAL - 1;
	retval->sys_strings_segment = retval->_segMan->findSegmentByType(SEG_TYPE_SYS_STRINGS);
	retval->sys_strings = (SystemStrings *)GET_SEGMENT(*retval->_segMan, retval->sys_strings_segment, SEG_TYPE_SYS_STRINGS);

	// Time state:
	retval->last_wait_time = g_system->getMillis();
	retval->game_start_time = g_system->getMillis() - retval->game_time * 1000;

	// static parser information:

	retval->parser_base = make_reg(s->sys_strings_segment, SYS_STRING_PARSER_BASE);

	// Copy breakpoint information from current game instance
	retval->have_bp = s->have_bp;
	retval->bp_list = s->bp_list;

	retval->successor = NULL;
	retval->pic_priority_table = (int *)gfxop_get_pic_metainfo(retval->gfx_state);
	retval->_gameName = s->_gameName;

	retval->_sound._it = NULL;
	retval->_sound._flags = s->_sound._flags;
	retval->_sound._song = NULL;
	retval->_sound._suspended = s->_sound._suspended;
	reconstruct_sounds(retval);

	// Message state:
	retval->_msgState = s->_msgState;

	retval->gui = s->gui;

	return retval;
}

bool get_savegame_metadata(Common::SeekableReadStream *stream, SavegameMetadata *meta) {
	assert(stream);
	assert(meta);

	Common::Serializer ser(stream, 0);
	sync_SavegameMetadata(ser, *meta);

	if (stream->eos())
		return false;

	if ((meta->savegame_version < MINIMUM_SAVEGAME_VERSION) ||
	    (meta->savegame_version > CURRENT_SAVEGAME_VERSION)) {
		if (meta->savegame_version < MINIMUM_SAVEGAME_VERSION)
			warning("Old savegame version detected- can't load");
		else
			warning("Savegame version is %d- maximum supported is %0d", meta->savegame_version, CURRENT_SAVEGAME_VERSION);

		return false;
	}

	return true;
}

} // End of namespace Sci
