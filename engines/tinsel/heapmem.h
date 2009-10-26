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
 * This file contains the handle based Memory Manager defines
 */

#ifndef TINSEL_HEAPMEM_H
#define TINSEL_HEAPMEM_H

#include "tinsel/dw.h"		// new data types

namespace Tinsel {

#define	NUM_MNODES	192	// the number of memory management nodes (was 128, then 192)

struct MEM_NODE {
	MEM_NODE *pNext;	// link to the next node in the list
	MEM_NODE *pPrev;	// link to the previous node in the list
	uint8 *pBaseAddr;	// base address of the memory object
	long size;		// size of the memory object
	uint32 lruTime;		// time when memory object was last accessed
	int flags;		// allocation attributes
};

// allocation flags for the MemoryAlloc function
#define	DWM_DISCARDABLE	0x0004	///< allocates discardable memory
#define	DWM_NOALLOC		0x0008	///< when used with discardable memory - allocates a discarded block

// internal allocation flags
#define	DWM_DISCARDED	0x0100	///< the objects memory block has been discarded
#define	DWM_LOCKED		0x0200	///< the objects memory block is locked
#define	DWM_SENTINEL	0x0400	///< the objects memory block is a sentinel


/*----------------------------------------------------------------------*\
|*			Memory Function Prototypes			*|
\*----------------------------------------------------------------------*/

void MemoryInit(void);		// initialises the memory manager

// allocates a movable block with the specified number of bytes from the heap
MEM_NODE *MemoryAlloc(
	int flags,		// allocation attributes
	long size);		// number of bytes to allocate

// allocates a fixed block with the specified number of bytes
void *MemoryAllocFixed(long size);

void MemoryDiscard(		// discards the specified memory object
	MEM_NODE *pMemNode);	// node of the memory object

void *MemoryLock(		// locks a memory object and returns a pointer to the first byte of the objects memory block
	MEM_NODE *pMemNode);	// node of the memory object

void MemoryReAlloc(	// changes the size or attributes of a specified memory object
	MEM_NODE *pMemNode,	// node of the memory object
	long size,		// new size of block
	int flags);		// how to reallocate the object

void MemoryUnlock(		// unlocks a memory object
	MEM_NODE *pMemNode);	// node of the memory object

bool HeapCompact(		// Allocates the specified number of bytes from the specified heap
	long size,		// number of bytes to free up
	bool bDiscard);		// when set - will discard blocks to fullfill the request

} // End of namespace Tinsel

#endif
