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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/branches/gsoc2010-plugins/backends/plugins/arm-loader.h $
 * $Id: arm-loader.h 52053 2010-08-13 02:58:52Z toneman1138 $
 *
 */

#if defined(DYNAMIC_MODULES) && defined(PPC_TARGET)

#include "backends/plugins/elf-loader.h"

class PPCDLObject : public DLObject {
protected:
	virtual bool relocate(Common::SeekableReadStream* DLFile, unsigned long offset, unsigned long size, void *relSegment);
	virtual bool relocateRels(Common::SeekableReadStream* DLFile, Elf32_Ehdr *ehdr, Elf32_Shdr *shdr);

public:
    PPCDLObject() : DLObject() {}
};

#endif /* defined(DYNAMIC_MODULES) && defined(PPC_TARGET) */

