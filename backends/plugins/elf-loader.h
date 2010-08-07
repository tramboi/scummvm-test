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

#ifndef ELF_LOADER_H
#define ELF_LOADER_H

#include "elf32.h"
#include "common/stream.h"
#include "backends/plugins/dynamic-plugin.h"

#if defined(MIPS_TARGET)
#include "shorts-segment-manager.h"
#endif

class DLObject {
protected:
    void *_segment, *_symtab;
    char *_strtab;
    int _symbol_cnt;
    int _symtab_sect;
    void *_dtors_start, *_dtors_end;

    int _segmentSize;

#ifdef MIPS_TARGET
	ShortSegmentManager::Segment *_shortsSegment;			// For assigning shorts ranges
	unsigned int _gpVal;									// Value of Global Pointer
#endif

    //void seterror(const char *fmt, ...);
    void unload();
    virtual bool relocate(Common::SeekableReadStream* DLFile, unsigned long offset, unsigned long size, void *relSegment) = 0;
    bool load(Common::SeekableReadStream* DLFile);

    bool readElfHeader(Common::SeekableReadStream* DLFile, Elf32_Ehdr *ehdr);
    bool readProgramHeaders(Common::SeekableReadStream* DLFile, Elf32_Ehdr *ehdr, Elf32_Phdr *phdr, int num);
    bool loadSegment(Common::SeekableReadStream* DLFile, Elf32_Phdr *phdr);
    Elf32_Shdr *loadSectionHeaders(Common::SeekableReadStream* DLFile, Elf32_Ehdr *ehdr);
    int loadSymbolTable(Common::SeekableReadStream* DLFile, Elf32_Ehdr *ehdr, Elf32_Shdr *shdr);
    bool loadStringTable(Common::SeekableReadStream* DLFile, Elf32_Shdr *shdr);
    void relocateSymbols(Elf32_Addr offset);
    virtual bool relocateRels(Common::SeekableReadStream* DLFile, Elf32_Ehdr *ehdr, Elf32_Shdr *shdr) = 0;

public:
    bool open(const char *path);
    bool close();
    void *symbol(const char *name);
    void discard_symtab();

};

#endif /* ELF_LOADER_H */
