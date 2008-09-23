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

#ifndef COMMON_UNZIP_H
#define COMMON_UNZIP_H

#ifdef USE_ZLIB

#include "common/scummsys.h"
#include "common/archive.h"

typedef void *unzFile;

namespace Common {

class ZipArchive : public Archive {
	void *_zipFile;

public:
	ZipArchive(const String &name);
	~ZipArchive();
	
	bool isOpen() const;

	virtual bool hasFile(const String &name);
	virtual int getAllNames(StringList &list);	// FIXME: This one is not (yet?) implemented
	virtual Common::SeekableReadStream *openFile(const Common::String &name);
};

}	// End of namespace Common

#endif // USE_ZLIB

#endif /* _unz_H */
