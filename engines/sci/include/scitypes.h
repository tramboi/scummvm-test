/***************************************************************************
 scitypes.h Copyright (C) 2001 Christoph Reichenbach

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

    Christoph Reichenbach (CR) [creichen@rbg.informatik.tu-darmstadt.de]

***************************************************************************/

#ifndef SCI_TYPES
#define SCI_TYPES

#include "common/scummsys.h"

// TODO: rework sci_dir_t to use common/fs.h and remove these includes
#include <sys/types.h>
#ifndef _MSC_VER
#include <dirent.h>
#else
#include <io.h>
#endif

typedef int8 gint8;
typedef uint8 guint8;

typedef int16 gint16;
typedef uint16 guint16;

typedef int32 gint32;
typedef uint32 guint32;

#undef byte

typedef gint8 sbyte;
typedef guint8 byte;
typedef guint16 word;

typedef struct {
	long tv_sec;
	long tv_usec;
} GTimeVal;

typedef struct {
#ifdef WIN32
	long search;
	struct _finddata_t fileinfo;
#else
	DIR *dir;
	char *mask_copy;
#endif
} sci_dir_t; /* used by sci_find_first and friends */

#endif /* !SCI_TYPES */
