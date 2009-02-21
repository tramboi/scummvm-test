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

#ifndef _SCI_VM_TYPES_H_
#define _SCI_VM_TYPES_H_

#include "sci/include/scitypes.h"

#define SCI_REG_SIZE 16;
#define SCI_SEG_SIZE 16;

typedef int seg_id_t; /* Segment ID type */

struct _state; /* engine.h */

struct reg_t {
	uint16 segment;
	uint16 offset;
};

#define PREG "%04x:%04x"
#define PRINT_REG(r) (0xffff) & (unsigned) (r).segment, (unsigned) (r).offset

typedef reg_t *stack_ptr_t; /* Stack pointer type */
typedef int selector_t; /* Selector ID */
#define NULL_SELECTOR -1

#define PSTK "ST:%04x"
#define PRINT_STK(v) (unsigned) (v - s->stack_base)

static inline reg_t
make_reg(int segment, int offset) {
	reg_t r;
	r.offset = offset;
	r.segment = segment;
	return r;
}

#define IS_NULL_REG(r) (!((r).offset || (r).segment))
#define REG_EQ(a, b) (((a).offset == (b).offset) && ((a).segment == (b).segment))
#define NULL_REG_INITIALIZER {0, 0}
extern reg_t NULL_REG;


#endif /* !_SCI_VM_TYPES_H_ */
