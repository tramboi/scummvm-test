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

#include "cruise/cruise_main.h"
#include "common/endian.h"

namespace Cruise {

enum fileTypeEnum {
	type_UNK,
	type_SPL,
	type_SET,
	type_FNT
};

int loadSingleFile;

// TODO: Unify decodeGfxFormat1, decodeGfxFormat4 and decodeGfxFormat5

void decodeGfxFormat1(dataFileEntry *pCurrentFileEntry) {
	uint8 *buffer;
	uint8 *dataPtr = pCurrentFileEntry->subData.ptr;

	int spriteSize = pCurrentFileEntry->height * pCurrentFileEntry->width;
	int x = 0;

	buffer = (uint8 *) malloc(spriteSize);

	while (x < spriteSize) {
		uint8 c;
		uint16 p0;

		p0 = (dataPtr[0] << 8) | dataPtr[1];

		/* decode planes */
		for (c = 0; c < 16; c++) {
			buffer[x + c] = ((p0 >> 15) & 1);

			p0 <<= 1;
		}

		x += 16;

		dataPtr += 2;
	}

	pCurrentFileEntry->subData.ptr = buffer;
}

void decodeGfxFormat4(dataFileEntry *pCurrentFileEntry) {
	uint8 *buffer;
	uint8 *dataPtr = pCurrentFileEntry->subData.ptr;

	int spriteSize = pCurrentFileEntry->height * pCurrentFileEntry->width;
	int x = 0;

	buffer = (uint8 *) malloc(spriteSize);

	while (x < spriteSize) {
		uint8 c;
		uint16 p0;
		uint16 p1;
		uint16 p2;
		uint16 p3;

		p0 = (dataPtr[0] << 8) | dataPtr[1];
		p1 = (dataPtr[2] << 8) | dataPtr[3];
		p2 = (dataPtr[4] << 8) | dataPtr[5];
		p3 = (dataPtr[6] << 8) | dataPtr[7];

		/* decode planes */
		for (c = 0; c < 16; c++) {
			buffer[x + c] = ((p0 >> 15) & 1) | ((p1 >> 14) & 2) | ((p2 >> 13) & 4) | ((p3 >> 12) & 8);

			p0 <<= 1;
			p1 <<= 1;
			p2 <<= 1;
			p3 <<= 1;
		}

		x += 16;

		dataPtr += 8;
	}

	pCurrentFileEntry->subData.ptr = buffer;
}

void decodeGfxFormat5(dataFileEntry *pCurrentFileEntry) {
	uint8 *dataPtr = pCurrentFileEntry->subData.ptr;
	int spriteSize = pCurrentFileEntry->height * pCurrentFileEntry->widthInColumn;
	int range = pCurrentFileEntry->height * pCurrentFileEntry->width;

	uint8 *buffer = (uint8 *)malloc(spriteSize);
	uint8 *destP = buffer;

	for (int line = 0; line < pCurrentFileEntry->height; line++) {
		uint8 p0, p1, p2, p3, p4;

		for (int x = 0; x < pCurrentFileEntry->widthInColumn; x++) {
			int bit = 7 - (x % 8);
			int col = x / 8;

			p0 = (dataPtr[line*pCurrentFileEntry->width + col + range * 0] >> bit) & 1;
			p1 = (dataPtr[line*pCurrentFileEntry->width + col + range * 1] >> bit) & 1;
			p2 = (dataPtr[line*pCurrentFileEntry->width + col + range * 2] >> bit) & 1;
			p3 = (dataPtr[line*pCurrentFileEntry->width + col + range * 3] >> bit) & 1;
			p4 = (dataPtr[line*pCurrentFileEntry->width + col + range * 4] >> bit) & 1;

			*destP++ = p0 | (p1 << 1) | (p2 << 2) | (p3 << 3) | (p4 << 4);
		}
	}

	pCurrentFileEntry->subData.ptr = buffer;
}

int updateResFileEntry(int height, int width, int entryNumber, int resType) {
	int div = 0;

	resetFileEntry(entryNumber);

	filesDatabase[entryNumber].subData.compression = 0;

	int maskSize = height * width;	// for sprites: width * height

	if (resType == 4) {
		div = maskSize / 4;
	} else if (resType == 5) {
		width = (width * 8) / 5;
		maskSize = height * width;
	}

	filesDatabase[entryNumber].subData.ptr = (uint8 *) mallocAndZero(maskSize + div);

	if (!filesDatabase[entryNumber].subData.ptr)
		return (-2);

	filesDatabase[entryNumber].widthInColumn = width;
	filesDatabase[entryNumber].subData.ptrMask = (uint8 *) mallocAndZero(maskSize);
	filesDatabase[entryNumber].width = width / 8;
	filesDatabase[entryNumber].resType = resType;
	filesDatabase[entryNumber].height = height;
	filesDatabase[entryNumber].subData.index = -1;

	return entryNumber;
}

int createResFileEntry(int width, int height, int resType) {
	int i;
	int entryNumber;
	int div = 0;
	int size;

	printf("Executing untested createResFileEntry!\n");
	exit(1);

	for (i = 0; i < NUM_FILE_ENTRIES; i++) {
		if (!filesDatabase[i].subData.ptr)
			break;
	}

	if (i >= NUM_FILE_ENTRIES) {
		return (-19);
	}

	entryNumber = i;

	filesDatabase[entryNumber].subData.compression = 0;

	size = width * height;	// for sprites: width * height

	if (resType == 4) {
		div = size / 4;
	} else if (resType == 5) {
		width = (width * 8) / 5;
	}

	filesDatabase[entryNumber].subData.ptr = (uint8 *) mallocAndZero(size + div);

	if (filesDatabase[entryNumber].subData.ptr) {
		return (-2);
	}

	filesDatabase[entryNumber].widthInColumn = width;
	filesDatabase[entryNumber].subData.ptrMask = filesDatabase[entryNumber].subData.ptr + size;
	filesDatabase[entryNumber].width = width / 8;
	filesDatabase[entryNumber].resType = resType;
	filesDatabase[entryNumber].height = height;
	filesDatabase[entryNumber].subData.index = -1;

	return entryNumber;
}

fileTypeEnum getFileType(const char *name) {
	char extentionBuffer[16];

	fileTypeEnum newFileType = type_UNK;

	getFileExtention(name, extentionBuffer);

	if (!strcmp(extentionBuffer, ".SPL")) {
		newFileType = type_SPL;
	} else if (!strcmp(extentionBuffer, ".SET")) {
		newFileType = type_SET;
	} else if (!strcmp(extentionBuffer, ".FNT")) {
		newFileType = type_FNT;
	}

	ASSERT(newFileType != type_UNK);

	return newFileType;
}

int getNumMaxEntiresInSet(uint8 *ptr) {
	uint16 numEntries = *(uint16 *)(ptr + 4);
	flipShort(&numEntries);

	return numEntries;
}

int loadFile(const char* name, int idx, int destIdx) {
	uint8 *ptr = NULL;
	fileTypeEnum fileType;

	fileType = getFileType(name);

	loadFileSub1(&ptr, name, NULL);

	switch (fileType) {
	case type_SET: {

		int numMaxEntriesInSet = getNumMaxEntiresInSet(ptr);

		if (idx > numMaxEntriesInSet) {
			return 0;	// exit if limit is reached
		}
		return loadSetEntry(name, ptr, idx, destIdx);

		break;
	}
	case type_FNT: {
		return loadFNTSub(ptr, idx);
		break;
	}
	case type_UNK: {
		break;
	}
	case type_SPL: {
		break;
	}
	}
	return -1;
}

int loadFileRange(const char *name, int startIdx, int currentEntryIdx, int numIdx) {
	uint8 *ptr = NULL;
	fileTypeEnum fileType;

	fileType = getFileType(name);

	loadFileSub1(&ptr, name, NULL);

	switch (fileType) {
	case type_SET: {
		int i;
		int numMaxEntriesInSet = getNumMaxEntiresInSet(ptr);

		for (i = 0; i < numIdx; i++) {
			if ((startIdx + i) > numMaxEntriesInSet) {
				return 0;	// exit if limit is reached
			}
			loadSetEntry(name, ptr, startIdx + i, currentEntryIdx + i);
		}

		break;
	}
	case type_FNT: {
		loadFNTSub(ptr, startIdx);
		break;
	}
	case type_UNK: {
		break;
	}
	case type_SPL: {
		break;
	}
	}
	return 0;
}

int loadFullBundle(const char *name, int startIdx) {
	uint8 *ptr = NULL;
	fileTypeEnum fileType;

	fileType = getFileType(name);

	loadFileSub1(&ptr, name, NULL);

	if (ptr == NULL)
		return 0;

	switch (fileType) {
	case type_SET: {
		int i;
		int numMaxEntriesInSet;

		numMaxEntriesInSet = getNumMaxEntiresInSet(ptr);	// get maximum number of sprites/animations in SET file

		for (i = 0; i < numMaxEntriesInSet; i++) {
			loadSetEntry(name, ptr, i, startIdx + i);
		}

		break;
	}
	case type_FNT: {
		loadFNTSub(ptr, startIdx);
		break;
	}
	case type_UNK: {
		break;
	}
	case type_SPL: {
		break;
	}
	}

	return 0;
}

int loadFNTSub(uint8 *ptr, int destIdx) {
	uint8 *ptr2 = ptr;
	uint8 *destPtr;
	int fileIndex;
	uint32 fontSize;

	ptr2 += 4;
	memcpy(&loadFileVar1, ptr2, 4);

	flipLong(&loadFileVar1);

	if (destIdx == -1) {
		fileIndex = createResFileEntry(loadFileVar1, 1, 1);
	} else {
		fileIndex = updateResFileEntry(loadFileVar1, 1, destIdx, 1);
	}

	destPtr = filesDatabase[fileIndex].subData.ptr;

	memcpy(destPtr, ptr2, loadFileVar1);

	memcpy(&fontSize, ptr2, 4);
	flipLong(&fontSize);

	if (destPtr != NULL) {
		int32 i;
		uint8 *currentPtr;

		destPtr = filesDatabase[fileIndex].subData.ptr;

		flipLong((int32 *) destPtr);
		flipLong((int32 *)(destPtr + 4));
		flipGen(destPtr + 8, 6);

		currentPtr = destPtr + 14;

		for (i = 0; i < *(int16 *)(destPtr + 8); i++) {
			flipLong((int32 *) currentPtr);
			currentPtr += 4;

			flipGen(currentPtr, 8);
			currentPtr += 8;
		}
	}

	return 1;
}

int loadSetEntry(const char *name, uint8 *ptr, int currentEntryIdx, int currentDestEntry) {
	uint8 *ptr3;
	int offset;
	int sec = 0;
	uint16 numIdx;

	if (!strcmp((char*)ptr, "SEC")) {
		sec = 1;
	}

	numIdx = READ_BE_UINT16(ptr + 4);

	ptr3 = ptr + 6;

	offset = currentEntryIdx * 16;

	{
		int resourceSize;
		int fileIndex;
		setHeaderEntry localBuffer;
		uint8 *ptr5;

		Common::MemoryReadStream s4(ptr + offset + 6, 16);

		localBuffer.offset = s4.readUint32BE();
		localBuffer.width = s4.readUint16BE();
		localBuffer.height = s4.readUint16BE();
		localBuffer.type = s4.readUint16BE();
		localBuffer.transparency = s4.readUint16BE();
		localBuffer.hotspotY = s4.readUint16BE();
		localBuffer.hotspotX = s4.readUint16BE();

		if (sec == 1) 
			// Type 1: Width - (1*2) , Type 5: Width - (5*2)
			localBuffer.width -= localBuffer.type * 2;

		resourceSize = localBuffer.width * localBuffer.height;

		if (!sec && (localBuffer.type == 5))
			// Type 5: Width - (2*5)
			localBuffer.width -= 10;

		if (currentDestEntry == -1) {
			fileIndex = createResFileEntry(localBuffer.width, localBuffer.height, localBuffer.type);
		} else {
			fileIndex = updateResFileEntry(localBuffer.height, localBuffer.width, currentDestEntry, localBuffer.type);
		}

		if (fileIndex < 0) {
			return -1;	// TODO: buffer is not freed
		}

		if (!sec && (localBuffer.type == 5)) {
			// There are sometimes sprites with a reduced width than what their pixels provide.
			// The original handled this here by copy parts of each line - for ScummVM, we're
			// simply setting the width in bytes and letting the decoder do the rest
			filesDatabase[fileIndex].width += 2;
		}

		ptr5 = ptr3 + localBuffer.offset + numIdx * 16;

		memcpy(filesDatabase[fileIndex].subData.ptr, ptr5, resourceSize);
		ptr5 += resourceSize;

		switch (localBuffer.type) {
		case 0: { // polygon
			filesDatabase[fileIndex].subData.resourceType = OBJ_TYPE_POLY;
			filesDatabase[fileIndex].subData.index = currentEntryIdx;
			break;
		}
		case 1: {
			filesDatabase[fileIndex].width = filesDatabase[fileIndex].widthInColumn * 8;
			filesDatabase[fileIndex].subData.resourceType = OBJ_TYPE_BGMASK;
			decodeGfxFormat1(&filesDatabase[fileIndex]);
			filesDatabase[fileIndex].subData.index = currentEntryIdx;
			filesDatabase[fileIndex].subData.transparency = 0;
			break;
		}
		case 4: {
			filesDatabase[fileIndex].width = filesDatabase[fileIndex].widthInColumn * 2;
			filesDatabase[fileIndex].subData.resourceType = OBJ_TYPE_SPRITE;
			decodeGfxFormat4(&filesDatabase[fileIndex]);
			filesDatabase[fileIndex].subData.index = currentEntryIdx;
			filesDatabase[fileIndex].subData.transparency = localBuffer.transparency % 0x10;
			break;
		}
		case 5: {
			filesDatabase[fileIndex].subData.resourceType = OBJ_TYPE_SPRITE;
			decodeGfxFormat5(&filesDatabase[fileIndex]);
			filesDatabase[fileIndex].width = filesDatabase[fileIndex].widthInColumn;
			filesDatabase[fileIndex].subData.index = currentEntryIdx;
			filesDatabase[fileIndex].subData.transparency = localBuffer.transparency;
			break;
		}
		case 8: {
			filesDatabase[fileIndex].subData.resourceType = OBJ_TYPE_SPRITE;
			filesDatabase[fileIndex].width = filesDatabase[fileIndex].widthInColumn;
			filesDatabase[fileIndex].subData.index = currentEntryIdx;
			filesDatabase[fileIndex].subData.transparency = localBuffer.transparency;
			break;
		}
		default: {
			printf("Unsuported gfx loading type: %d\n", localBuffer.type);
			break;
		}
		}

		strcpy(filesDatabase[fileIndex].subData.name, name);

		// create the mask
		switch (localBuffer.type) {
		case 1:
		case 4:
		case 5:
		case 8: {
			int maskX;
			int maskY;

			memset(filesDatabase[fileIndex].subData.ptrMask, 0, filesDatabase[fileIndex].width / 8 * filesDatabase[fileIndex].height);

			for (maskY = 0; maskY < filesDatabase[fileIndex].height; maskY++) {
				for (maskX = 0; maskX < filesDatabase[fileIndex].width; maskX++) {
					if (*(filesDatabase[fileIndex].subData.ptr + filesDatabase[fileIndex].width * maskY + maskX) != filesDatabase[fileIndex].subData.transparency) {
						*(filesDatabase[fileIndex].subData.ptrMask + filesDatabase[fileIndex].width / 8 * maskY + maskX / 8) |= 0x80 >> (maskX & 7);
					}
				}
			}

			break;
		}
		default: {
		}
		}
	}

	// TODO: free

	return 1;
}

} // End of namespace Cruise
