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

#include "common/util.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/graphics/coordadjuster.h"
#include "sci/graphics/ports.h"

namespace Sci {

GfxCoordAdjuster::GfxCoordAdjuster() {
}

GfxCoordAdjuster16::GfxCoordAdjuster16(GfxPorts *ports)
	: _ports(ports) {
}

GfxCoordAdjuster16::~GfxCoordAdjuster16() {
}

void GfxCoordAdjuster16::kernelGlobalToLocal(int16 &x, int16 &y, reg_t planeObject) {
	Port *curPort = _ports->getPort();
	x -= curPort->left;
	y -= curPort->top;
}

void GfxCoordAdjuster16::kernelLocalToGlobal(int16 &x, int16 &y, reg_t planeObject) {
	Port *curPort = _ports->getPort();
	x += curPort->left;
	y += curPort->top;
}

#ifdef ENABLE_SCI32
GfxCoordAdjuster32::GfxCoordAdjuster32(SegManager *segMan)
	: _segMan(segMan) {
}

GfxCoordAdjuster32::~GfxCoordAdjuster32() {
}

void GfxCoordAdjuster32::kernelGlobalToLocal(int16 &x, int16 &y, reg_t planeObject) {
	//int16 resY = GET_SEL32V(_s->_segMan, planeObj, resY);
	//int16 resX = GET_SEL32V(_s->_segMan, planeObj, resX);
	//*x = ( *x * _screen->getWidth()) / resX;
	//*y = ( *y * _screen->getHeight()) / resY;
	x -= GET_SEL32V(_segMan, planeObject, left);
	y -= GET_SEL32V(_segMan, planeObject, top);
}
void GfxCoordAdjuster32::kernelLocalToGlobal(int16 &x, int16 &y, reg_t planeObject) {
	//int16 resY = GET_SEL32V(_s->_segMan, planeObj, resY);
	//int16 resX = GET_SEL32V(_s->_segMan, planeObj, resX);
	x += GET_SEL32V(_segMan, planeObject, left);
	y += GET_SEL32V(_segMan, planeObject, top);
	//*x = ( *x * resX) / _screen->getWidth();
	//*y = ( *y * resY) / _screen->getHeight();
}
#endif

} // End of namespace Sci
