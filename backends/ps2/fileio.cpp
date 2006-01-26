/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
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
 * $Header$
 *
 */

#include "backends/ps2/fileio.h"

#include <tamtypes.h>
#include <kernel.h>
#include <fileio.h>
#include <assert.h>
#include <string.h>
#include "backends/ps2/asyncfio.h"
#include "base/engine.h"
#include "common/file.h"
#include "eecodyvdfs.h"

#define CACHE_SIZE (2048 * 32)
#define MAX_READ_STEP (2048 * 16)
#define MAX_CACHED_FILES 6
#define CACHE_READ_THRESHOLD (16 * 2048)
#define CACHE_FILL_MIN (2048 * 24)
#define READ_ALIGN 64	// align all reads to the size of an EE cache line
#define READ_ALIGN_MASK (READ_ALIGN - 1)

extern void sioprintf(const char *zFormat, ...);

AsyncFio fio;

Ps2File::Ps2File(int64 cacheId) {
	_cacheId = cacheId;
}

Ps2File::~Ps2File(void) {
}

class Ps2ReadFile : public Ps2File {
public:
	Ps2ReadFile(int64 cacheId, bool stream);
	virtual ~Ps2ReadFile(void);
	virtual bool open(const char *name);
	virtual uint32 read(void *dest, uint32 len);
	virtual uint32 write(const void *src, uint32 len);
	virtual uint32 tell(void);
	virtual uint32 size(void);
	virtual int seek(int32 offset, int origin);
	virtual bool eof(void);
private:
	void cacheReadAhead(void);
	void cacheReadSync(void);
	int _fd, _sema;
	uint8 *_cacheBuf;
	bool _cacheOpRunning;
	uint32 _filePos, _physFilePos, _cachePos;
	uint32 _fileSize, _bytesInCache, _cacheOfs;

	uint32 _readBytesBlock;
	bool _stream;
};

Ps2ReadFile::Ps2ReadFile(int64 cacheId, bool stream) : Ps2File(cacheId) {
	_fd = -1;
	_cacheBuf = (uint8*)memalign(64, CACHE_SIZE);

	_cacheOpRunning = 0;
	_filePos = _physFilePos = _cachePos = 0;
	_fileSize = _bytesInCache = _cacheOfs = 0;
	_cacheOpRunning = false;
	_readBytesBlock = 0;
	_stream = stream;

	ee_sema_t newSema;
	newSema.init_count = 1;
	newSema.max_count = 1;
	_sema = CreateSema(&newSema);
	assert(_sema >= 0);
}

Ps2ReadFile::~Ps2ReadFile(void) {
	if (_cacheOpRunning)
		cacheReadSync();
	free(_cacheBuf);
	if (_fd >= 0)
		fio.close(_fd);
	DeleteSema(_sema);
}

bool Ps2ReadFile::open(const char *name) {
	assert(_fd < 0);
	_fd = fio.open(name, O_RDONLY);
	if (_fd >= 0) {
		_fileSize = fio.seek(_fd, 0, SEEK_END);
		fio.seek(_fd, 0, SEEK_SET);
		return true;
	} else
		return false;
}

uint32 Ps2ReadFile::tell(void) {
	WaitSema(_sema);
	uint32 res = _filePos;
	SignalSema(_sema);
	return res;
}

uint32 Ps2ReadFile::size(void) {
	WaitSema(_sema);
	uint32 res = _fileSize;
	SignalSema(_sema);
	return res;
}

bool Ps2ReadFile::eof(void) {
	WaitSema(_sema);
	bool res = (_filePos == _fileSize);
	SignalSema(_sema);
	return res;
}

int Ps2ReadFile::seek(int32 offset, int origin) {
	WaitSema(_sema);
	int seekDest;
	int res = -1;
	switch (origin) {
		case SEEK_SET:
			seekDest = offset;
			break;
		case SEEK_CUR:
			seekDest = _filePos + offset;
			break;
		case SEEK_END:
			seekDest = _fileSize + offset;
			break;
		default:
			seekDest = -1;
			break;
	}
	if ((seekDest >= 0) && (seekDest <= (int)_fileSize)) {
		_filePos = seekDest;
		res = 0;
	}
	SignalSema(_sema);
	return res;
}

void Ps2ReadFile::cacheReadAhead(void) {
	if (_cacheOpRunning) {
		// there's already some cache read running
		if (fio.poll(_fd)) // did it finish?
			cacheReadSync(); // yes.
	}
	if ((!_cacheOpRunning) && ((_readBytesBlock >= CACHE_READ_THRESHOLD) || _stream) && fio.fioAvail()) {
		// the engine seems to do sequential reads and there are no other I/Os going on. read ahead.
		uint32 cachePosEnd = _cachePos + _bytesInCache;

		if (_cachePos > _filePos)
			return; // there was a seek in the meantime, don't cache.
		if (cachePosEnd - _filePos >= CACHE_FILL_MIN)
			return; // cache is full enough.
		if (cachePosEnd == _fileSize)
			return; // can't read beyond EOF.

		assert(cachePosEnd < _fileSize);

		if (_cachePos + _bytesInCache <= _filePos) {
			_cacheOfs = _bytesInCache = 0;
			_cachePos = cachePosEnd = _filePos & ~READ_ALIGN_MASK;
			assert(_filePos == _physFilePos);
		} else {
            uint32 cacheDiff = _filePos - _cachePos;
			assert(_bytesInCache >= cacheDiff);
			cacheDiff &= ~READ_ALIGN_MASK;
			_bytesInCache -= cacheDiff;
			_cachePos += cacheDiff;
			_cacheOfs = (_cacheOfs + cacheDiff) % CACHE_SIZE;
		}

		if (_physFilePos != cachePosEnd) {
			sioprintf("unexpected _physFilePos %d cache %d %d", _physFilePos, _cacheOfs, _bytesInCache);
			assert(!(cachePosEnd & READ_ALIGN_MASK));
			_physFilePos = fio.seek(_fd, cachePosEnd, SEEK_SET);
			if (_physFilePos != cachePosEnd) {
				sioprintf("cache seek error: seek to %d instead of %d, fs = %d", _physFilePos, cachePosEnd, _fileSize);
				return;
			}
		}

		uint32 cacheDest = (_cacheOfs + _bytesInCache) % CACHE_SIZE;
		uint32 cacheRead = CACHE_SIZE - _bytesInCache;
		if (cacheDest + cacheRead > CACHE_SIZE)
			cacheRead = CACHE_SIZE - cacheDest;
		if (cacheRead > MAX_READ_STEP)
			cacheRead = MAX_READ_STEP;

		assert((!(cacheRead & READ_ALIGN_MASK)) && cacheRead);

		_cacheOpRunning = true;
		fio.read(_fd, _cacheBuf + cacheDest, cacheRead);
	}
}

void Ps2ReadFile::cacheReadSync(void) {
	if (_cacheOpRunning) {
		int res = fio.sync(_fd);
		assert(res >= 0);
		_bytesInCache += res;
		_physFilePos += res;
		_cacheOpRunning = false;
	}
}

uint32 Ps2ReadFile::read(void *dest, uint32 len) {
	WaitSema(_sema);
	uint8 *destBuf = (uint8*)dest;
	if ((_filePos < _cachePos) || (_filePos + len > _cachePos + _bytesInCache))
		cacheReadSync(); // we have to read from CD, sync cache.

	while (len && (_filePos != _fileSize)) {
		if ((_filePos >= _cachePos) && (_filePos < _cachePos + _bytesInCache)) { // read from cache
			uint32 staPos = (_cacheOfs + (_filePos - _cachePos)) % CACHE_SIZE;
			uint32 cpyLen = _bytesInCache - (_filePos - _cachePos);
			if (cpyLen > len)
				cpyLen = len;
			if (staPos + cpyLen > CACHE_SIZE)
				cpyLen = CACHE_SIZE - staPos;

			assert(cpyLen);
			memcpy(destBuf, _cacheBuf + staPos, cpyLen);
			_filePos += cpyLen;
			destBuf += cpyLen;
			_readBytesBlock += len;
			len -= cpyLen;
		} else { // cache miss
			assert(!_cacheOpRunning);
			if (_physFilePos != _filePos) {
				if ((_filePos < _physFilePos) || (_filePos > _physFilePos + (CACHE_SIZE / 2)))
					_readBytesBlock = 0; // reset cache hit count

				_physFilePos = _filePos & ~READ_ALIGN_MASK;
				if (fio.seek(_fd, _physFilePos, SEEK_SET) != (int)_physFilePos)
					break; // read beyond EOF
			}

			int doRead = len + (_filePos - _physFilePos);
			doRead = (doRead + READ_ALIGN_MASK) & ~READ_ALIGN_MASK;

			if (doRead > MAX_READ_STEP)
				doRead = MAX_READ_STEP;
			if (doRead < 2048)
				doRead = 2048;

			fio.read(_fd, _cacheBuf, doRead);
			_cachePos = _physFilePos;
			_cacheOfs = 0;
			_bytesInCache = fio.sync(_fd);
			_physFilePos += _bytesInCache;
			if (!_bytesInCache)
				break; // EOF
		}
	}
	cacheReadAhead();
	SignalSema(_sema);
	return destBuf - (uint8*)dest;
}

uint32 Ps2ReadFile::write(const void *src, uint32 len) {
	sioprintf("write request on Ps2ReadFile!");
	SleepThread();
	return 0;
}

class Ps2WriteFile : public Ps2File {
public:
	Ps2WriteFile(int64 cacheId);
	virtual ~Ps2WriteFile(void);
	virtual bool open(const char *name);
	virtual uint32 read(void *dest, uint32 len);
	virtual uint32 write(const void *src, uint32 len);
	virtual uint32 tell(void);
	virtual uint32 size(void);
	virtual int seek(int32 offset, int origin);
	virtual bool eof(void);
private:
	int _fd;
	uint8 *_cacheBuf;
	uint32 _filePos, _bytesInCache;
};

Ps2WriteFile::Ps2WriteFile(int64 cacheId) : Ps2File(cacheId) {
	_fd = -1;
	_cacheBuf = (uint8*)malloc(CACHE_SIZE);
	_filePos = _bytesInCache = 0;
}

Ps2WriteFile::~Ps2WriteFile(void) {
	if ((_fd >= 0) && (_bytesInCache)) {
		fio.write(_fd, _cacheBuf, _bytesInCache);
		int wrRes = fio.sync(_fd);
		if (wrRes != (int)_bytesInCache) // too late to return an error
			printf("Cache flush on fclose(): Unable to write %d cached bytes to mc, only %d bytes written\n", _bytesInCache, wrRes);
	}
	if (_fd >= 0)
		fio.close(_fd);
	free(_cacheBuf);
}

bool Ps2WriteFile::open(const char *name) {
	_fd = fio.open(name, O_WRONLY | O_CREAT | O_TRUNC);
    return (_fd >= 0);
}

uint32 Ps2WriteFile::read(void *dest, uint32 len) {
	printf("ERROR: Read request on Ps2WriteFile\n");
	SleepThread();
	return 0;
}

uint32 Ps2WriteFile::write(const void *src, uint32 len) {
	uint32 size = len;
	uint8 *srcBuf = (uint8*)src;
	while (size) {
		uint32 doCpy = (len > CACHE_SIZE - _bytesInCache) ? (CACHE_SIZE - _bytesInCache) : len;
		if (doCpy) {
			memcpy(_cacheBuf + _bytesInCache, srcBuf, doCpy);
			_bytesInCache += doCpy;
			srcBuf += doCpy;
			size -= doCpy;
		}

		if (_bytesInCache == CACHE_SIZE) {
			fio.write(_fd, _cacheBuf, _bytesInCache);
			if (fio.sync(_fd) != (int)_bytesInCache) {
				printf("Unable to flush %d cached bytes to memory card!\n", _bytesInCache);
				return 0;
			}
			_filePos += _bytesInCache;
			_bytesInCache = 0;
		}
	}
	return len;
}

uint32 Ps2WriteFile::tell(void) {
	return _bytesInCache + _filePos;
}

uint32 Ps2WriteFile::size(void) {
	return tell();
}

int Ps2WriteFile::seek(int32 offset, int origin) {
	printf("Seek(%d/%d) request on Ps2WriteFile\n", offset, origin);
	SleepThread();
	return 0;
}

bool Ps2WriteFile::eof(void) {
	return true;
}

struct TocNode {
	char name[64];
	TocNode *next, *sub;
	bool isDir;
	uint8 nameLen;
};

class TocManager {
public:
	TocManager(void);
	~TocManager(void);
	void readEntries(const char *root);
    int64 fileExists(const char *name);
	bool haveEntries(void);
private:
	void readDir(const char *path, TocNode **node, int level);
	TocNode *_rootNode;
	char _root[256];
	uint8 _rootLen;
};

static TocManager tocManager;

struct FioHandleCache {
	Ps2File *file;
	FioHandleCache *next, *prev;
};

static FioHandleCache *cacheListStart = NULL;
static FioHandleCache *cacheListEnd = NULL;
static int cacheListLen = 0;
static int openFileCount = 0;
static int cacheListSema = -1;

static bool checkedPath = false;

Ps2File *findInCache(int64 id);

FILE *ps2_fopen(const char *fname, const char *mode) {
	if (cacheListSema == -1) {
		ee_sema_t newSema;
		newSema.init_count = 1;
		newSema.max_count = 1;
		cacheListSema = CreateSema(&newSema);
		assert(cacheListSema >= 0);
	}

	if (!checkedPath && g_engine) {
		// are we playing from cd/dvd?
		if (strncmp(g_engine->getGameDataPath(), "cdfs:", 5) != 0)
			driveStop(); // no, we aren't. stop the drive. it's noisy.
		// now cache the dir tree
		tocManager.readEntries(g_engine->getGameDataPath());
		checkedPath = true;
	}

	if (((mode[0] != 'r') && (mode[0] != 'w')) || ((mode[1] != '\0') && (mode[1] != 'b'))) {
		printf("unsupported mode \"%s\" for file \"%s\"\n", mode, fname);
		return NULL;
	}

	bool rdOnly = (mode[0] == 'r');

	int64 cacheId = -1;
	if (rdOnly && tocManager.haveEntries())
		cacheId = tocManager.fileExists(fname);

	if (cacheId != 0) {
		Ps2File *file = findInCache(cacheId);
		if (file)
			return (FILE*)file;

		if (rdOnly) {
			bool isAudioFile = strstr(fname, ".bun") || strstr(fname, ".BUN") || strstr(fname, ".Bun");
			file = new Ps2ReadFile(cacheId, isAudioFile);
		} else
			file = new Ps2WriteFile(cacheId);

		if (file->open(fname)) {
			openFileCount++;
			return (FILE*)file;
		} else
			delete file;
	}
	return NULL;
}

void checkCacheListLen(void) {
	while ((cacheListLen > MAX_CACHED_FILES) || ((openFileCount > 13) && cacheListLen)) {
		assert(cacheListEnd && cacheListStart);
		delete cacheListEnd->file;
		if (cacheListEnd->prev) {
			cacheListEnd->prev->next = NULL;
			FioHandleCache *temp = cacheListEnd;
			cacheListEnd = cacheListEnd->prev;
			delete temp;
		} else {
			assert(cacheListEnd == cacheListStart);
			assert(cacheListLen == 1);
			delete cacheListEnd;
			cacheListEnd = cacheListStart = NULL;
		}
		cacheListLen--;
		openFileCount--;
	}
}

int ps2_fclose(FILE *stream) {
	Ps2File *file = (Ps2File*)stream;
	if (file->_cacheId > 0) { // this is a file on the CD, could be smart to cache it
		FioHandleCache *newHandle = new FioHandleCache;
		newHandle->file = file;
		file->seek(0, SEEK_SET);

		WaitSema(cacheListSema);
		if (!cacheListEnd) {
			assert(!cacheListStart);
			newHandle->prev = newHandle->next = NULL;
			cacheListEnd = cacheListStart = newHandle;
		} else {
			assert(cacheListStart);
			newHandle->prev = NULL;
			newHandle->next = cacheListStart;
			cacheListStart->prev = newHandle;
			cacheListStart = newHandle;
		}
		cacheListLen++;
		checkCacheListLen();
		SignalSema(cacheListSema);
	} else {
		openFileCount--;
		delete file;
	}
    return 0;
}

Ps2File *findInCache(int64 id) {
	if (id <= 0)
		return NULL;
	WaitSema(cacheListSema);
	FioHandleCache *node = cacheListStart;
	while (node) {
		if (node->file->_cacheId == id) {
			if (node == cacheListStart)
				cacheListStart = node->next;
			if (node == cacheListEnd)
				cacheListEnd = node->prev;
			if (node->prev)
				node->prev->next = node->next;
			if (node->next)
				node->next->prev = node->prev;
			Ps2File *ret = node->file;
			delete node;
			cacheListLen--;
			SignalSema(cacheListSema);
			return ret;
		} else
			node = node->next;
	}
	SignalSema(cacheListSema);
	return NULL;
}

int ps2_fseek(FILE *stream, long offset, int origin) {
	return ((Ps2File*)stream)->seek(offset, origin);
}

uint32 ps2_ftell(FILE *stream) {
	return ((Ps2File*)stream)->tell();
}

uint32 ps2_fsize(FILE *stream) {
	return ((Ps2File*)stream)->size();
}

int ps2_feof(FILE *stream) {
	return ((Ps2File*)stream)->eof();
}

size_t ps2_fread(void *buf, size_t r, size_t n, FILE *stream) {
	assert(r != 0);
	return ((Ps2File*)stream)->read(buf, r * n) / r;
}

int ps2_fgetc(FILE *stream) {
	uint8 temp;
	if (((Ps2File*)stream)->read(&temp, 1))
		return temp;
	else
		return EOF;
}

char *ps2_fgets(char *buf, int n, FILE *stream) {
	char *retVal = buf;
	while (n--) {
		if (n == 0)
			*buf = '\0';
		else {
			char c = ps2_fgetc(stream);
			if (c == EOF)
				return NULL;
			if ((c == '\r') || (c == '\n')) {
				*buf++ = '\0';
                return retVal;
			}
			*buf++ = c;
		}
	}
	return retVal;
}

int ps2_fprintf(FILE *pOut, const char *zFormat, ...) {
	va_list ap;
	char resStr[2048];

	va_start(ap,zFormat);
	int res = vsnprintf(resStr, 2048, zFormat, ap);
	va_end(ap);
	if ((pOut == stderr) || (pOut == stdout)) {
		printf("%s", resStr);
		sioprintf("%s", resStr);
	} else
		res = ps2_fwrite(resStr, 1, res, pOut);
	return res;
}

size_t ps2_fwrite(const void *buf, size_t r, size_t n, FILE *stream) {
	assert(r != 0);
	return ((Ps2File*)stream)->write(buf, r * n) / r;
}

int ps2_fputc(int c, FILE *stream) {
	if (((Ps2File*)stream)->write(&c, 1) == 1)
		return c;
	else
		return -1;
}

int ps2_fputs(const char *s, FILE *stream) {
	int len = strlen(s);
	if (ps2_fwrite(s, 1, len, stream) == (size_t)len)
		return len;
	else
		return EOF;
}

int ps2_fflush(FILE *stream) {
	printf("fflush not implemented\n");
	return 0;
}

TocManager::TocManager(void) {
	_rootNode = NULL;
}

TocManager::~TocManager(void) {
}

bool TocManager::haveEntries(void) {
	return _rootNode != NULL;
}

void TocManager::readEntries(const char *root) {
    _rootLen = strlen(root);
	strcpy(_root, root);
	if (_root[_rootLen - 1] == '/') {
		_rootLen--;
		_root[_rootLen] = '\0';
	}
	char readPath[256];
	sprintf(readPath, "%s/", _root);
	readDir(readPath, &_rootNode, 0);
}

void TocManager::readDir(const char *path, TocNode **node, int level) {
	if (level <= 2) { // we don't scan deeper than that
		iox_dirent_t dirent;
		int fd = fio.dopen(path);
		if (fd >= 0) {
			while (fio.dread(fd, &dirent) > 0)
				if (dirent.name[0] != '.') { // skip '.' and '..'
					*node = new TocNode;
					(*node)->sub = (*node)->next = NULL;

					(*node)->nameLen = strlen(dirent.name);
					memcpy((*node)->name, dirent.name, (*node)->nameLen + 1);

					if (dirent.stat.mode & FIO_S_IFDIR) { // directory
						(*node)->isDir = true;
						char nextPath[256];
						sprintf(nextPath, "%s%s/", path, dirent.name);
						readDir(nextPath, &((*node)->sub), level + 1);
					} else
						(*node)->isDir = false;
					node = &((*node)->next);
				}
			fio.dclose(fd);
		} else
			printf("Can't open path: %s\n", path);
	}
}

int64 TocManager::fileExists(const char *name) {
	if (((name[_rootLen] != '/') && (name[_rootLen] != '\0')) || (strnicmp(name, _root, _rootLen) != 0)) {
		for (int i = 0; i < 8; i++)
			if (name[i] == ':')	// we don't know the content of other drives,
				return -1;		// assume file exists
			else if ((name[i] == '/') || (name[i] == '\\'))
				return 0;		// does not exists (this is probably ScummVM trying the 'current directory'.)
		return 0;
	}

	uint8 nameLen = strlen(name);

	name += _rootLen + 1;
	nameLen -= _rootLen + 1;

	TocNode *node = _rootNode;
	int64 retId = 1;
	while (node) {
		if (((name[node->nameLen] == '/') || (name[node->nameLen] == '\0')) && (strnicmp(name, node->name, node->nameLen) == 0)) {
			name += node->nameLen;
			nameLen -= node->nameLen;
			if (node->isDir) {
				if (nameLen) {
					name++;	// skip '/'
					nameLen--;
					node = node->sub;
					retId <<= 10;
				} else
					return 0; // can't open a directory with fopen()
			} else {
				if (nameLen == 0)
					return retId; // ok, found
				else
					return 0; // here's a file, but there's something left in the path
			}
		} else {
			node = node->next;
			retId++;
		}
	}
	return 0; // does not exist
}

