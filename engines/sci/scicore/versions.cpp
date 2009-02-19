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

#define NEED_SCI_VERSIONS

#include "common/system.h"
#include "common/config-manager.h"

#include "sci/include/versions.h"
#include "sci/include/engine.h"
#include "sci/include/resource.h"
#include "sci/scicore/exe.h"		// for reading version from the executable

void
version_require_earlier_than(state_t *s, sci_version_t version) {
	if (s->version_lock_flag)
		return;

	if (version <= s->min_version) {
		sciprintf("Version autodetect conflict: Less than %d.%03d.%03d was requested, but "
		          "%d.%03d.%03d is the current minimum\n",
		          SCI_VERSION_MAJOR(version), SCI_VERSION_MINOR(version), SCI_VERSION_PATCHLEVEL(version),
		          SCI_VERSION_MAJOR(s->min_version), SCI_VERSION_MINOR(s->min_version),
		          SCI_VERSION_PATCHLEVEL(s->min_version));
		return;
	} else if (version < s->max_version) {
		s->max_version = version - 1;
		if (s->max_version < s->version)
			s->version = s->max_version;
	}
}

void
version_require_later_than(state_t *s, sci_version_t version) {
	if (s->version_lock_flag)
		return;

	if (version > s->max_version) {
		sciprintf("Version autodetect conflict: More than %d.%03d.%03d was requested, but less than"
		          "%d.%03d.%03d is required ATM\n",
		          SCI_VERSION_MAJOR(version), SCI_VERSION_MINOR(version), SCI_VERSION_PATCHLEVEL(version),
		          SCI_VERSION_MAJOR(s->max_version), SCI_VERSION_MINOR(s->max_version),
		          SCI_VERSION_PATCHLEVEL(s->max_version));
		return;
	} else if (version > s->min_version) {
		s->min_version = version;
		if (s->min_version > s->version)
			s->version = s->min_version;
	}
}

int
version_parse(const char *vn, sci_version_t *result) {
	char *endptr[3];
	int major = strtol(vn, &endptr[0], 10);
	int minor = strtol(vn + 2, &endptr[1], 10);
	int patchlevel = strtol(vn + 6, &endptr[2], 10);

	if (endptr[0] != vn + 1 || endptr[1] != vn + 5
	        || *endptr[2] != '\0') {
		sciprintf("Warning: Failed to parse version string '%s'\n", vn);
		return 1;
	}

	*result = SCI_VERSION(major, minor, patchlevel);
	return 0;
}

// Exe scanning functions

/* Maxmimum number of bytes to hash from start of file */
#define VERSION_DETECT_HASH_SIZE 1000000

#define VERSION_DETECT_BUF_SIZE 4096

static int
scan_file(char *filename, sci_version_t *version) {
	char buf[VERSION_DETECT_BUF_SIZE];
	char result_string[10]; /* string-encoded result, copied from buf */
	int characters_left;
	int state = 0;
	/* 'state' encodes how far we have matched the version pattern
	**   "n.nnn.nnn"
	**
	**   n.nnn.nnn
	**  0123456789
	**
	** Since we cannot be certain that the pattern does not begin with an
	** alphanumeric character, some states are ambiguous.
	** The pattern is expected to be terminated with a non-alphanumeric
	** character.
	*/

	exe_file_t *f = exe_open(filename);

	if (!f)
		return 1;

	do {
		int i;
		int accept;

		characters_left = exe_read(f, buf, VERSION_DETECT_BUF_SIZE);

		for (i = 0; i < characters_left; i++) {
			const char ch = buf[i];
			accept = 0; /* By default, we don't like this character */

			if (isalnum((unsigned char) ch)) {
				accept = (state != 1
				          && state != 5
				          && state != 9);
			} else if (ch == '.') {
				accept = (state == 1
				          || state == 5);
			} else if (state == 9) {
				result_string[9] = 0; /* terminate string */

				if (!version_parse(result_string, version)) {
					exe_close(f);
					return 0; /* success! */
				}

				/* Continue searching. */
			}

			if (accept)
				result_string[state++] = ch;
			else
				state = 0;

		}

	} while (characters_left == VERSION_DETECT_BUF_SIZE);

	exe_close(f);
	return 1; /* failure */
}

static guint32
read_uint32(byte *data) {
	return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
}

static guint16
read_uint16(byte *data) {
	return (data[0] << 8) | data[1];
}

static int
is_mac_exe(char *filename) {
	FILE *file;
	byte buf[4];
	guint32 val;
	unsigned int i;

	/* Mac executables have no extension */
	if (strchr(filename, '.'))
		return 0;

	file = fopen(filename, "rb");
	if (!file)
		return 0;

	if (fseek(file, 4, SEEK_SET) == -1) {
		fclose(file);
		return 0;
	}

	/* Read resource map offset */
	if (fread(buf, 1, 4, file) < 4) {
		fclose(file);
		return 0;
	}

	val = read_uint32(buf);

	if (fseek(file, val + 28, SEEK_SET) == -1) {
		fclose(file);
		return 0;
	}

	/* Read number of types in map */
	if (fread(buf, 1, 2, file) < 2) {
		fclose(file);
		return 0;
	}

	val = read_uint16(buf) + 1;

	for (i = 0; i < val; i++) {
		if (fread(buf, 1, 4, file) < 4) {
			fclose(file);
			return 0;
		}

		/* Look for executable code */
		if (!memcmp(buf, "CODE", 4)) {
			fclose(file);
			return 1;
		}

		/* Skip to next list entry */
		if (fseek(file, 4, SEEK_CUR) == -1) {
			fclose(file);
			return 0;
		}
	}

	fclose(file);
	return 0;
}

static int
is_exe(char *filename) {
	FILE *file;
	char buf[4];
	unsigned char header[] = {0x00, 0x00, 0x03, 0xf3};

	/* PC and Atari ST executable extensions */
	if (strstr(filename, ".exe") || strstr(filename, ".EXE")
	        || strstr(filename, ".prg") || strstr(filename, ".PRG"))
		return 1;

	/* Check for Amiga executable */
	if (strchr(filename, '.'))
		return 0;

	file = fopen(filename, "rb");
	if (!file)
		return 0;

	if (fread(buf, 1, 4, file) < 4) {
		fclose(file);
		return 0;
	}

	fclose(file);

	/* Check header bytes */
	return memcmp(buf, header, 4) == 0;
}

int
version_detect_from_executable(char *filename) {
	int mac = 0;
	int result;

	if (mac ? is_mac_exe(filename) : is_exe(filename)) {
		if (scan_file(filename, &result) == 0) {
			return result;
		}
	}

	return 0;
}

#undef VERSION_DETECT_BUF_SIZE
