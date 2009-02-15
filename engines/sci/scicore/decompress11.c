/***************************************************************************
 decompress11.c Copyright (C) 1999 The FreeSCI project


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

    Christoph Reichenbach (CJR) [creichen@rbg.informatik.tu-darmstadt.de]

***************************************************************************/
/* Reads data from a resource file and stores the result in memory */

#include <sci_memory.h>
#include <sciresource.h>

#define DDEBUG if (0) printf

void decryptinit3(void);
int decrypt3(guint8* dest, guint8* src, int length, int complength);
int decrypt4(guint8* dest, guint8* src, int length, int complength);

int decompress11(resource_t *result, int resh, int sci_version)
{
	guint16 compressedLength;
	guint16 compressionMethod, result_size;
	guint8 *buffer;
	guint8 tempid;

	DDEBUG("d1");

	if (read(resh, &tempid, 1) != 1)
		return SCI_ERROR_IO_ERROR;

	result->id = tempid;

	result->type = result->id &0x7f;
	if (read(resh, &(result->number), 2) != 2)
		return SCI_ERROR_IO_ERROR;

#ifdef WORDS_BIGENDIAN
	result->number = GUINT16_SWAP_LE_BE_CONSTANT(result->number);
#endif /* WORDS_BIGENDIAN */
	if ((result->type > sci_invalid_resource))
		return SCI_ERROR_DECOMPRESSION_INSANE;

	if ((read(resh, &compressedLength, 2) != 2) ||
	    (read(resh, &result_size, 2) != 2) ||
	    (read(resh, &compressionMethod, 2) != 2))
		return SCI_ERROR_IO_ERROR;

#ifdef WORDS_BIGENDIAN
	compressedLength = GUINT16_SWAP_LE_BE_CONSTANT(compressedLength);
	result_size = GUINT16_SWAP_LE_BE_CONSTANT(result_size);
	compressionMethod = GUINT16_SWAP_LE_BE_CONSTANT(compressionMethod);
#endif
	result->size = result_size;

  /*  if ((result->size < 0) || (compressedLength < 0))
      return SCI_ERROR_DECOMPRESSION_INSANE; */
  /* This return will never happen in SCI0 or SCI1 (does it have any use?) */

	if ((result->size > SCI_MAX_RESOURCE_SIZE) ||
	    (compressedLength > SCI_MAX_RESOURCE_SIZE))
		return SCI_ERROR_RESOURCE_TOO_BIG;

	if (compressedLength > 0)
		compressedLength -= 0;
	else { /* Object has size zero (e.g. view.000 in sq3) (does this really exist?) */
		result->data = 0;
		result->status = SCI_STATUS_NOMALLOC;
		return SCI_ERROR_EMPTY_OBJECT;
	}

	buffer = (guint8*)sci_malloc(compressedLength);
	result->data = (unsigned char*)sci_malloc(result->size);

	if (read(resh, buffer, compressedLength) != compressedLength) {
		free(result->data);
		free(buffer);
		return SCI_ERROR_IO_ERROR;
	};

	if (!(compressedLength & 1)) { /* Align */
		int foo;
		read(resh, &foo, 1);
	}

#ifdef _SCI_DECOMPRESS_DEBUG
	fprintf(stderr, "Resource %i.%s encrypted with method SCI1.1/%hi at %.2f%%"
		" ratio\n",
		result->number, sci_resource_type_suffixes[result->type],
		compressionMethod,
		(result->size == 0)? -1.0 :
		(100.0 * compressedLength / result->size));
	fprintf(stderr, "  compressedLength = 0x%hx, actualLength=0x%hx\n",
		compressedLength, result->size);
#endif

	DDEBUG("/%d[%d]", compressionMethod, result->size);

	switch(compressionMethod) {

	case 0: /* no compression */
		if (result->size != compressedLength) {
			free(result->data);
			result->data = NULL;
			result->status = SCI_STATUS_NOMALLOC;
			free(buffer);
			return SCI_ERROR_DECOMPRESSION_OVERFLOW;
		}
		memcpy(result->data, buffer, compressedLength);
		result->status = SCI_STATUS_ALLOCATED;
		break;

	case 18:
	case 19:
	case 20:
		if (decrypt4(result->data, buffer, result->size, compressedLength)) {
			free(result->data);
			result->data = 0; /* So that we know that it didn't work */
			result->status = SCI_STATUS_NOMALLOC;
			free(buffer);
			return SCI_ERROR_DECOMPRESSION_OVERFLOW;
		}
		result->status = SCI_STATUS_ALLOCATED;
		break;

	case 3:
	case 4: /* NYI */
		fprintf(stderr,"Resource %d.%s: Warning: compression type #%d not yet implemented\n",
			result->number, sci_resource_type_suffixes[result->type], compressionMethod);
		free(result->data);
		result->data = NULL;
		result->status = SCI_STATUS_NOMALLOC;
		break;

	default:
		fprintf(stderr,"Resource %d.%s: Compression method SCI1/%hi not "
			"supported!\n", result->number, sci_resource_type_suffixes[result->type],
			compressionMethod);
		free(result->data);
		result->data = NULL; /* So that we know that it didn't work */
		result->status = SCI_STATUS_NOMALLOC;
		free(buffer);
		return SCI_ERROR_UNKNOWN_COMPRESSION;
	}

  free(buffer);
  return 0;
}

