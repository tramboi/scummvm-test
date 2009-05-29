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

#ifndef SCI_SCICORE_RESOURCE_H
#define SCI_SCICORE_RESOURCE_H

#include "common/str.h"
#include "common/file.h"
#include "common/archive.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"			// for SoundHandle

#include "sci/engine/vm.h"          // for Object
#include "sci/decompressor.h"

namespace Common {
class ReadStream;
}

namespace Sci {

/** The maximum allowed size for a compressed or decompressed resource */
#define SCI_MAX_RESOURCE_SIZE 0x0400000

/** Resource status types */
enum ResourceStatus {
	kResStatusNoMalloc = 0,
	kResStatusAllocated,
	kResStatusEnqueued, /**< In the LRU queue */
	kResStatusLocked /**< Allocated and in use */
};

/** Initialization result types */
enum {
	SCI_ERROR_IO_ERROR = 1,
	SCI_ERROR_INVALID_RESMAP_ENTRY = 2,	/**< Invalid resource.map entry */
	SCI_ERROR_RESMAP_NOT_FOUND = 3,
	SCI_ERROR_NO_RESOURCE_FILES_FOUND = 4,	/**< No resource at all was found */
	SCI_ERROR_UNKNOWN_COMPRESSION = 5,
	SCI_ERROR_DECOMPRESSION_ERROR = 6,	/**< sanity checks failed during decompression */
	SCI_ERROR_RESOURCE_TOO_BIG = 8	/**< Resource size exceeds SCI_MAX_RESOURCE_SIZE */

	/* the first critical error number */
};

#define SCI_VERSION_1 SCI_VERSION_1_EARLY

#define MAX_OPENED_VOLUMES 5 // Max number of simultaneously opened volumes

enum ResSourceType {
	kSourceDirectory = 0,
	kSourcePatch = 1,
	kSourceVolume = 2,
	kSourceExtMap = 3,
	kSourceIntMap = 4,
	kSourceMask = 127
};

#define RESSOURCE_ADDRESSING_BASIC 0
#define RESSOURCE_ADDRESSING_EXTENDED 128
#define RESSOURCE_ADDRESSING_MASK 128

#define RESOURCE_HASH(type, number) (uint32)((type<<16) | number)
#define SCI0_RESMAP_ENTRIES_SIZE 6
#define SCI1_RESMAP_ENTRIES_SIZE 6
#define SCI11_RESMAP_ENTRIES_SIZE 5

extern const char *sci_error_types[];
extern const char *sci_version_types[];
extern const int sci_max_resource_nr[]; /**< Highest possible resource numbers */


enum ResourceType {
	kResourceTypeView = 0,
	kResourceTypePic,
	kResourceTypeScript,
	kResourceTypeText,
	kResourceTypeSound,
	kResourceTypeMemory,
	kResourceTypeVocab,
	kResourceTypeFont,
	kResourceTypeCursor,
	kResourceTypePatch,
	kResourceTypeBitmap,
	kResourceTypePalette,
	kResourceTypeCdAudio,
	kResourceTypeAudio,
	kResourceTypeSync,
	kResourceTypeMessage,
	kResourceTypeMap,
	kResourceTypeHeap,
	kResourceTypeAudio36,
	kResourceTypeSync36,
	kResourceTypeInvalid
};

const char *getResourceTypeName(ResourceType restype);
// Suffixes for SCI1 patch files
const char *getResourceTypeSuffix(ResourceType restype);

#define sci0_last_resource kResourceTypePatch
#define sci1_last_resource kResourceTypeHeap
/* Used for autodetection */


/** resource type for SCI1 resource.map file */
struct resource_index_t {
	uint16 wOffset;
	uint16 wSize;
};

struct ResourceSource {
	ResSourceType source_type;
	bool scanned;
	Common::String location_name;	// FIXME: Replace by FSNode ?
	int volume_number;
	ResourceSource *associated_map;
	ResourceSource *next;
};

/** Class for storing resources in memory */
class Resource {
public:
	Resource();
	~Resource();
	void unalloc();

// NOTE : Currently all member data has the same name and public visibility
// to let the rest of the engine compile without changes
public:
	byte *data;
	uint16 number;
	ResourceType type;
	uint32 id;	//!< contains number and type.
	unsigned int size;
	unsigned int file_offset; /**< Offset in file */
	ResourceStatus status;
	unsigned short lockers; /**< Number of places where this resource was locked */
	ResourceSource *source;
};


class ResourceManager {
public:
	int _sciVersion; //!< SCI resource version to use */
	int _mapVersion; //!< RESOURCE.MAP version
	int _volVersion; //!< RESOURCE.0xx version

	/**
	 * Creates a new SCI resource manager.
	 * @param version		The SCI version to look for; use SCI_VERSION_AUTODETECT
	 *						in the default case.
	 * @param maxMemory		Maximum number of bytes to allow allocated for resources
	 *
	 * @note maxMemory will not be interpreted as a hard limit, only as a restriction
	 *    for resources which are not explicitly locked. However, a warning will be
	 *    issued whenever this limit is exceeded.
	 */
	ResourceManager(int version, int maxMemory);
	~ResourceManager();

	/**
	 * Looks up a resource's data.
	 * @param type: The resource type to look for
	 * @param number: The resource number to search
	 * @param lock: non-zero iff the resource should be locked
	 * @return (Resource *): The resource, or NULL if it doesn't exist
	 * @note Locked resources are guaranteed not to have their contents freed until
	 *       they are unlocked explicitly (by unlockResource).
	 */
	Resource *findResource(ResourceType type, int number, int lock);

	/* Unlocks a previously locked resource
	**             (Resource *) res: The resource to free
	**             (int) number: Number of the resource to check (ditto)
	**             (ResourceType) type: Type of the resource to check (for error checking)
	** Returns   : (void)
	*/
	void unlockResource(Resource *res, int restype, ResourceType resnum);

	/* Tests whether a resource exists
	**             (ResourceType) type: Type of the resource to check
	**             (int) number: Number of the resource to check
	** Returns   : (Resource *) non-NULL if the resource exists, NULL otherwise
	** This function may often be much faster than finding the resource
	** and should be preferred for simple tests.
	** The resource object returned is, indeed, the resource in question, but
	** it should be used with care, as it may be unallocated.
	** Use scir_find_resource() if you want to use the data contained in the resource.
	*/
	Resource *testResource(ResourceType type, int number);

protected:
	int _maxMemory; //!< Config option: Maximum total byte number allocated
	ResourceSource *_sources;
	int _memoryLocked;	//!< Amount of resource bytes in locked memory
	int _memoryLRU;		//!< Amount of resource bytes under LRU control
	Common::List<Resource *> _LRU; //!< Last Resource Used list
	Common::HashMap<uint32, Resource *> _resMap;
	Common::List<Common::File *> _volumeFiles; //!< list of opened volume files

	/**
	 * Add a path to the resource manager's list of sources.
	 * @return a pointer to the added source structure, or NULL if an error occurred.
	 */
	ResourceSource *addPatchDir(const char *path);

	ResourceSource *getVolume(ResourceSource *map, int volume_nr);

	/**
	 * Add a volume to the resource manager's list of sources.
	 * @param map		The map associated with this volume
	 * @param filename	The name of the volume to add
	 * @param extended_addressing	1 if this volume uses extended addressing,
	 *                              0 otherwise.
	 * @return A pointer to the added source structure, or NULL if an error occurred.
	 */
	ResourceSource *addVolume(ResourceSource *map, const char *filename,
	                          int number, int extended_addressing);
	/**
	 * Add an external (i.e., separate file) map resource to the resource manager's list of sources.
	 * @param file_name	 The name of the volume to add
	 * @return		A pointer to the added source structure, or NULL if an error occurred.
	 */
	ResourceSource *addExternalMap(const char *file_name);

	/**
	 * Scans newly registered resource sources for resources, earliest addition first.
	 * @param detected_version: Pointer to the detected version number,
	 *					 used during startup. May be NULL.
	 * @return One of SCI_ERROR_*.
	 */
	int scanNewSources(ResourceSource *source);
	int addAppropriateSources();
	void freeResourceSources(ResourceSource *rss);

	Common::File *getVolumeFile(const char *filename);
	void loadResource(Resource *res);
	bool loadFromPatchFile(Resource *res);
	void freeOldResources(int last_invulnerable);
	int decompress(Resource *res, Common::File *file);
	int readResourceInfo(Resource *res, Common::File *file, uint32&szPacked, ResourceCompression &compression);

	/**--- Resource map decoding functions ---*/
	int detectMapVersion();
	int detectVolVersion();

	/**
	 * Reads the SCI0 resource.map file from a local directory.
	 * @return 0 on success, an SCI_ERROR_* code otherwise
	 */
	int readResourceMapSCI0(ResourceSource *map);

	/**
	 * Reads the SCI1 resource.map file from a local directory.
	 * @return 0 on success, an SCI_ERROR_* code otherwise
	 */
	int readResourceMapSCI1(ResourceSource *map);

	/**--- Patch management functions ---*/

	/**
	 * Reads patch files from a local directory.
	 */
	void readResourcePatches(ResourceSource *source);
	void processPatch(ResourceSource *source, ResourceType restype, int resnumber);

	void printLRU();
	void addToLRU(Resource *res);
	void removeFromLRU(Resource *res);
};

/**
 * Used for lip and animation syncing in CD talkie games
 */
class ResourceSync : public Resource {
public:
	ResourceSync() {}
	~ResourceSync() {}

	void startSync(EngineState *s, reg_t obj);
	void nextSync(EngineState *s, reg_t obj);
	void stopSync();

protected:
	uint16 *_ptr;
	int16 _syncTime, _syncCue;
	//bool _syncStarted;	// not used
};

/**
 * Used for speech playback and digital music playback
 * in CD talkie games
 */
class AudioResource {
public:
	AudioResource(ResourceManager *resMgr, int sciVersion);
	~AudioResource();

	void setAudioRate(uint16 audioRate) { _audioRate = audioRate; }
	void setAudioLang(int16 lang);

	Audio::SoundHandle* getAudioHandle() { return &_audioHandle; }
	int getAudioPosition();

	Audio::AudioStream* getAudioStream(uint32 audioNumber, uint32 volume, int *sampleLen);

	void stop() { g_system->getMixer()->stopHandle(_audioHandle); }
	void pause() { g_system->getMixer()->pauseHandle(_audioHandle, true); }
	void resume() { g_system->getMixer()->pauseHandle(_audioHandle, false); }

private:
	Audio::SoundHandle _audioHandle;
	uint16 _audioRate;
	int16 _lang;
	byte *_audioMapSCI1;
	Resource *_audioMapSCI11;
	ResourceManager *_resMgr;
	int _sciVersion;

	bool findAudEntrySCI1(uint16 audioNumber, byte &volume, uint32 &offset, uint32 &size);
	bool findAudEntrySCI11(uint32 audioNumber, uint32 volume, uint32 &offset);
};

} // End of namespace Sci

#endif // SCI_SCICORE_RESOURCE_H
