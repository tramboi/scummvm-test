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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#if defined(UNIX)

#include "backends/fs/posix/posix-fs.h"
#include "backends/fs/stdiostream.h"
#include "common/algorithm.h"

#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>

#ifdef __OS2__
#define INCL_DOS
#include <os2.h>
#endif


void POSIXFilesystemNode::setFlags() {
	struct stat st;

	_isValid = (0 == stat(_path.c_str(), &st));
	_isDirectory = _isValid ? S_ISDIR(st.st_mode) : false;
}

POSIXFilesystemNode::POSIXFilesystemNode() {
	// The root dir.
	_displayName = _path = "/";
	_isValid = true;
	_isDirectory = true;
}

POSIXFilesystemNode::POSIXFilesystemNode(const Common::String &p, bool verify) {
	assert(p.size() > 0);

	// Expand "~/" to the value of the HOME env variable
	if (p.hasPrefix("~/")) {
		const char *home = getenv("HOME");
		if (home != NULL && strlen(home) < MAXPATHLEN) {
			_path = home;
			// Skip over the tilda.  We know that p contains at least
			// two chars, so this is safe:
			_path += p.c_str() + 1;
		}
	} else {
		_path = p;
	}
	
	// Normalize the path (that is, remove unneeded slashes etc.)
	_path = Common::normalizePath(_path, '/');
	_displayName = Common::lastPathComponent(_path, '/');

	// TODO: should we turn relative paths into absolut ones?
	// Pro: Ensures the "getParent" works correctly even for relative dirs.
	// Contra: The user may wish to use (and keep!) relative paths in his
	//   config file, and converting relative to absolute paths may hurt him...
	//
	// An alternative approach would be to change getParent() to work correctly
	// if "_path" is the empty string.
#if 0
	if (!_path.hasPrefix("/")) {
		char buf[MAXPATHLEN+1];
		getcwd(buf, MAXPATHLEN);
		strcat(buf, "/");
		_path = buf + _path;
	}
#endif
	// TODO: Should we enforce that the path is absolute at this point?
	//assert(_path.hasPrefix("/"));

	if (verify) {
		setFlags();
	}
}

AbstractFilesystemNode *POSIXFilesystemNode::getChild(const Common::String &n) const {
	assert(_isDirectory);
	
	// Make sure the string contains no slashes
	assert(Common::find(n.begin(), n.end(), '/') == n.end());

	// We assume here that _path is already normalized (hence don't bother to call
	//  Common::normalizePath on the final path
	Common::String newPath(_path);
	newPath += '/';
	newPath += n;

	return new POSIXFilesystemNode(newPath, true);
}

bool POSIXFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
	assert(_isDirectory);

#ifdef __OS2__
	if (_path == "/") {
		// Special case for the root dir: List all DOS drives
		ULONG ulDrvNum;
		ULONG ulDrvMap;
	
		DosQueryCurrentDisk(&ulDrvNum, &ulDrvMap);
	
		for (int i = 0; i < 26; i++) {
			if (ulDrvMap & 1) {
				char drive_root[4];
	
				drive_root[0] = i + 'A';
				drive_root[1] = ':';
				drive_root[2] = '/';
				drive_root[3] = 0;
	
				POSIXFilesystemNode entry;
	
				entry._isDirectory = true;
				entry._isValid = true;
				entry._path = drive_root;
				entry._displayName = "[" + Common::String(drive_root, 2) + "]";
				myList.push_back(new POSIXFilesystemNode(entry));
			}
	
			ulDrvMap >>= 1;
		}
		
		return true;
	}
#endif

	DIR *dirp = opendir(_path.c_str());
	struct dirent *dp;

	if (dirp == NULL)
		return false;

	// loop over dir entries using readdir
	while ((dp = readdir(dirp)) != NULL) {
		// Skip 'invisible' files if necessary
		if (dp->d_name[0] == '.' && !hidden) {
			continue;
		}
		// Skip '.' and '..' to avoid cycles
		if ((dp->d_name[0] == '.' && dp->d_name[1] == 0) || (dp->d_name[0] == '.' && dp->d_name[1] == '.')) {
			continue;
		}

		Common::String newPath(_path);
		newPath += '/';
		newPath += dp->d_name;

		POSIXFilesystemNode entry(newPath, false);

#if defined(SYSTEM_NOT_SUPPORTING_D_TYPE)
		/* TODO: d_type is not part of POSIX, so it might not be supported
		 * on some of our targets. For those systems where it isn't supported,
		 * add this #elif case, which tries to use stat() instead.
		 *
		 * The d_type method is used to avoid costly recurrent stat() calls in big
		 * directories.
		 */
		entry.setFlags();
#else
		if (dp->d_type == DT_UNKNOWN) {
			// Fall back to stat()
			entry.setFlags();
		} else {
			entry._isValid = (dp->d_type == DT_DIR) || (dp->d_type == DT_REG) || (dp->d_type == DT_LNK);
			if (dp->d_type == DT_LNK) {
				struct stat st;
				if (stat(entry._path.c_str(), &st) == 0)
					entry._isDirectory = S_ISDIR(st.st_mode);
				else
					entry._isDirectory = false;
			} else {
				entry._isDirectory = (dp->d_type == DT_DIR);
			}
		}
#endif

		// Skip files that are invalid for some reason (e.g. because we couldn't
		// properly stat them).
		if (!entry._isValid)
			continue;

		// Honor the chosen mode
		if ((mode == Common::FilesystemNode::kListFilesOnly && entry._isDirectory) ||
			(mode == Common::FilesystemNode::kListDirectoriesOnly && !entry._isDirectory))
			continue;

		myList.push_back(new POSIXFilesystemNode(entry));
	}
	closedir(dirp);

	return true;
}

AbstractFilesystemNode *POSIXFilesystemNode::getParent() const {
	if (_path == "/")
		return 0;	// The filesystem root has no parent

	const char *start = _path.c_str();
	const char *end = start + _path.size();
	
	// Strip of the last component. We make use of the fact that at this
	// point, _path is guaranteed to be normalized
	while (end > start && *end != '/')
		end--;

	if (end == start)
		return new POSIXFilesystemNode();
	else
		return new POSIXFilesystemNode(Common::String(start, end), true);
}

Common::SeekableReadStream *POSIXFilesystemNode::openForReading() {
	return StdioStream::makeFromPath(getPath().c_str(), false);
}

Common::WriteStream *POSIXFilesystemNode::openForWriting() {
	return StdioStream::makeFromPath(getPath().c_str(), true);
}

#endif //#if defined(UNIX)
