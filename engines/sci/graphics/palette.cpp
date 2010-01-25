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

#include "common/timer.h"
#include "common/util.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/palette.h"

namespace Sci {

SciPalette::SciPalette(ResourceManager *resMan, Screen *screen, bool autoSetPalette)
	: _resMan(resMan), _screen(screen) {
	int16 color;

	_sysPalette.timestamp = 0;
	for (color = 0; color < 256; color++) {
		_sysPalette.colors[color].used = 0;
		_sysPalette.colors[color].r = 0;
		_sysPalette.colors[color].g = 0;
		_sysPalette.colors[color].b = 0;
		_sysPalette.intensity[color] = 100;
		_sysPalette.mapping[color] = color;
	}
	// Black and white are hardcoded
	_sysPalette.colors[0].used = 1;
	_sysPalette.colors[255].used = 1;
	_sysPalette.colors[255].r = 255;
	_sysPalette.colors[255].g = 255;
	_sysPalette.colors[255].b = 255;

	if (autoSetPalette) {
		if (_resMan->getViewType() == kViewEga)
			setEGA();
		else if (_resMan->isAmiga32color())
			setAmiga();
		else
			setFromResource(999, 2);
	}
}

SciPalette::~SciPalette() {
}

#define SCI_PAL_FORMAT_CONSTANT 1
#define SCI_PAL_FORMAT_VARIABLE 0

void SciPalette::createFromData(byte *data, Palette *paletteOut) {
	int palFormat = 0;
	int palOffset = 0;
	int palColorStart = 0;
	int palColorCount = 0;
	int colorNo = 0;

	memset(paletteOut, 0, sizeof(Palette));
	// Setup default mapping
	for (colorNo = 0; colorNo < 256; colorNo++) {
		paletteOut->mapping[colorNo] = colorNo;
	}
	if ((data[0] == 0 && data[1] == 1) || (data[0] == 0 && data[1] == 0 && READ_LE_UINT16(data + 29) == 0)) {
		// SCI0/SCI1 palette
		palFormat = SCI_PAL_FORMAT_VARIABLE; // CONSTANT;
		palOffset = 260;
		palColorStart = 0; palColorCount = 256;
		//memcpy(&paletteOut->mapping, data, 256);
	} else {
		// SCI1.1 palette
		palFormat = data[32];
		palOffset = 37;
		palColorStart = READ_LE_UINT16(data + 25); palColorCount = READ_LE_UINT16(data + 29);
	}
	switch (palFormat) {
		case SCI_PAL_FORMAT_CONSTANT:
			for (colorNo = palColorStart; colorNo < palColorStart + palColorCount; colorNo++) {
				paletteOut->colors[colorNo].used = 1;
				paletteOut->colors[colorNo].r = data[palOffset++];
				paletteOut->colors[colorNo].g = data[palOffset++];
				paletteOut->colors[colorNo].b = data[palOffset++];
			}
			break;
		case SCI_PAL_FORMAT_VARIABLE:
			for (colorNo = palColorStart; colorNo < palColorStart + palColorCount; colorNo++) {
				paletteOut->colors[colorNo].used = data[palOffset++];
				paletteOut->colors[colorNo].r = data[palOffset++];
				paletteOut->colors[colorNo].g = data[palOffset++];
				paletteOut->colors[colorNo].b = data[palOffset++];
			}
			break;
	}
}

static const byte EGApalette[16][3] = {
	{ 0x000, 0x000, 0x000 },
	{ 0x000, 0x000, 0x0AA },
	{ 0x000, 0x0AA, 0x000 },
	{ 0x000, 0x0AA, 0x0AA },
	{ 0x0AA, 0x000, 0x000 },
	{ 0x0AA, 0x000, 0x0AA },
	{ 0x0AA, 0x055, 0x000 },
	{ 0x0AA, 0x0AA, 0x0AA },
	{ 0x055, 0x055, 0x055 },
	{ 0x055, 0x055, 0x0FF },
	{ 0x055, 0x0FF, 0x055 },
	{ 0x055, 0x0FF, 0x0FF },
	{ 0x0FF, 0x055, 0x055 },
	{ 0x0FF, 0x055, 0x0FF },
	{ 0x0FF, 0x0FF, 0x055 },
	{ 0x0FF, 0x0FF, 0x0FF }
};

// Will try to set amiga palette by using "spal" file. If not found, we return false
bool SciPalette::setAmiga() {
	Common::File file;
	int curColor, byte1, byte2;

	if (file.open("spal")) {
		for (curColor = 0; curColor < 32; curColor++) {
			byte1 = file.readByte();
			byte2 = file.readByte();
			if ((byte1 == EOF) || (byte2 == EOF))
				error("Amiga palette file ends prematurely");
			_sysPalette.colors[curColor].used = 1;
			_sysPalette.colors[curColor].r = (byte1 & 0x0F) * 0x11;
			_sysPalette.colors[curColor].g = ((byte2 & 0xF0) >> 4) * 0x11;
			_sysPalette.colors[curColor].b = (byte2 & 0x0F) * 0x11;
		}
		file.close();
		// Directly set the palette, because setOnScreen() wont do a thing for amiga
		_screen->setPalette(&_sysPalette);
		return true;
	}
	return false;
}

// Called from picture class, some amiga sci1 games set half of the palette
void SciPalette::modifyAmigaPalette(byte *data) {
	int16 curColor, curPos = 0;
	byte byte1, byte2;
	for (curColor = 0; curColor < 16; curColor++) {
		byte1 = data[curPos++];
		byte2 = data[curPos++];
		_sysPalette.colors[curColor].r = (byte1 & 0x0F) * 0x11;
		_sysPalette.colors[curColor].g = ((byte2 & 0xF0) >> 4) * 0x11;
		_sysPalette.colors[curColor].b = (byte2 & 0x0F) * 0x11;
	}
	_screen->setPalette(&_sysPalette);
}

void SciPalette::setEGA() {
	int curColor;
	byte color1, color2;
	for (curColor = 0; curColor <= 15; curColor++) {
		_sysPalette.colors[curColor].used = 1;
		_sysPalette.colors[curColor].r = EGApalette[curColor][0];
		_sysPalette.colors[curColor].g = EGApalette[curColor][1];
		_sysPalette.colors[curColor].b = EGApalette[curColor][2];
	}
	// Now setting colors 16-254 to the correct mix colors that occur when not doing a dithering run on
	//  finished pictures
	for (curColor = 0x10; curColor <= 0xFE; curColor++) {
		_sysPalette.colors[curColor].used = curColor;
		color1 = curColor & 0x0F; color2 = curColor >> 4;
		_sysPalette.colors[curColor].r = (_sysPalette.colors[color1].r >> 1) + (_sysPalette.colors[color2].r >> 1);
		_sysPalette.colors[curColor].g = (_sysPalette.colors[color1].g >> 1) + (_sysPalette.colors[color2].g >> 1);
		_sysPalette.colors[curColor].b = (_sysPalette.colors[color1].b >> 1) + (_sysPalette.colors[color2].b >> 1);
	}
	setOnScreen();
}

bool SciPalette::setFromResource(GuiResourceId resourceId, uint16 flag) {
	Resource *palResource = _resMan->findResource(ResourceId(kResourceTypePalette, resourceId), 0);
	Palette palette;

	if (palResource) {
		createFromData(palResource->data, &palette);
		set(&palette, 2);
		return true;
	}
	return false;
}

void SciPalette::set(Palette *sciPal, uint16 flag) {
	uint32 systime = _sysPalette.timestamp;
	if (flag == 2 || sciPal->timestamp != systime) {
		merge(sciPal, &_sysPalette, flag);
		sciPal->timestamp = _sysPalette.timestamp;
		if (_screen->_picNotValid == 0 && systime != _sysPalette.timestamp)
			setOnScreen();
	}
}

void SciPalette::merge(Palette *pFrom, Palette *pTo, uint16 flag) {
	uint16 res;
	int i,j;
	// colors 0 (black) and 255 (white) are not affected by merging
	for (i = 1 ; i < 255; i++) {
		if (!pFrom->colors[i].used)// color is not used - so skip it
			continue;
		// forced palette merging or dest color is not used yet or bit 1 of new color is set
		if (flag == 2 || (!pTo->colors[i].used) || (pFrom->colors[i].used & 2)) {
			pTo->colors[i].used = pFrom->colors[i].used;
			pTo->colors[i].r = pFrom->colors[i].r;
			pTo->colors[i].g = pFrom->colors[i].g;
			pTo->colors[i].b = pFrom->colors[i].b;
			pFrom->mapping[i] = i;
			continue;
		}
		// is the same color already at the same position? -> match it directly w/o lookup
		//  this fixes games like lsl1demo/sq5 where the same rgb color exists multiple times and where we would
		//  otherwise match the wrong one (which would result into the pixels affected (or not) by palette changes)
		if ((pTo->colors[i].r == pFrom->colors[i].r) && (pTo->colors[i].g == pFrom->colors[i].g) && (pTo->colors[i].b == pFrom->colors[i].b)) {
			pFrom->mapping[i] = i;
			continue;
		}
		// check if exact color could be matched
		res = matchColor(pTo, pFrom->colors[i].r, pFrom->colors[i].g, pFrom->colors[i].b);
		if (res & 0x8000) { // exact match was found
			pFrom->mapping[i] = res & 0xFF;
			continue;
		}
		// no exact match - see if there is an unused color
		for (j = 1; j < 256; j++)
			if (!pTo->colors[j].used) {
				pTo->colors[j].used = pFrom->colors[i].used;
				pTo->colors[j].r = pFrom->colors[i].r;
				pTo->colors[j].g = pFrom->colors[i].g;
				pTo->colors[j].b = pFrom->colors[i].b;
				pFrom->mapping[i] = j;
				break;
			}
		// if still no luck - set an approximate color
		if (j == 256) {
			pFrom->mapping[i] = res & 0xFF;
			pTo->colors[res & 0xFF].used |= 0x10;
		}
	}
	pTo->timestamp = g_system->getMillis() * 60 / 1000;
}

uint16 SciPalette::matchColor(Palette *pPal, byte r, byte g, byte b) {
	byte found = 0xFF;
	int diff = 0x2FFFF, cdiff;
	int16 dr,dg,db;

	for (int i = 1; i < 255; i++) {
		if ((!pPal->colors[i].used))
			continue;
		dr = pPal->colors[i].r - r;
		dg = pPal->colors[i].g - g;
		db = pPal->colors[i].b - b;
//		minimum squares match
		cdiff = (dr*dr) + (dg*dg) + (db*db);
//		minimum sum match (Sierra's)
//		cdiff = ABS(dr) + ABS(dg) + ABS(db);
		if (cdiff < diff) {
			if (cdiff == 0)
				return i | 0x8000; // setting this flag to indicate exact match
			found = i;
			diff = cdiff;
		}
	}
	return found;
}

void SciPalette::getSys(Palette *pal) {
	if (pal != &_sysPalette)
		memcpy(pal, &_sysPalette,sizeof(Palette));
}

void SciPalette::setOnScreen() {
//	if (pal != &_sysPalette)
//		memcpy(&_sysPalette,pal,sizeof(Palette));
	// We dont change palette at all times for amiga
	if (_resMan->isAmiga32color())
		return;
	_screen->setPalette(&_sysPalette);
}

void SciPalette::setFlag(uint16 fromColor, uint16 toColor, uint16 flag) {
	uint16 colorNr;
	for (colorNr = fromColor; colorNr < toColor; colorNr++) {
		_sysPalette.colors[colorNr].used |= flag;
	}
}

void SciPalette::unsetFlag(uint16 fromColor, uint16 toColor, uint16 flag) {
	uint16 colorNr;
	for (colorNr = fromColor; colorNr < toColor; colorNr++) {
		_sysPalette.colors[colorNr].used &= ~flag;
	}
}

void SciPalette::setIntensity(uint16 fromColor, uint16 toColor, uint16 intensity, bool setPalette) {
	memset(&_sysPalette.intensity[0] + fromColor, intensity, toColor - fromColor);
	if (setPalette)
		setOnScreen();
}

// Returns true, if palette got changed
bool SciPalette::animate(byte fromColor, byte toColor, int speed) {
	Color col;
	//byte colorNr;
	int16 colorCount;
	uint32 now = g_system->getMillis() * 60 / 1000;

	// search for sheduled animations with the same 'from' value
	// schedule animation...
	int scheduleCount = _schedules.size();
	int scheduleNr;
	for (scheduleNr = 0; scheduleNr < scheduleCount; scheduleNr++) {
		if (_schedules[scheduleNr].from == fromColor)
			break;
	}
	if (scheduleNr == scheduleCount) {
		// adding a new schedule
		PalSchedule newSchedule;
		newSchedule.from = fromColor;
		newSchedule.schedule = now + ABS(speed);
		_schedules.push_back(newSchedule);
		scheduleCount++;
	}

	for (scheduleNr = 0; scheduleNr < scheduleCount; scheduleNr++) {
		if (_schedules[scheduleNr].from == fromColor) {
			if (_schedules[scheduleNr].schedule <= now) {
				if (speed > 0) {
					// TODO: Not really sure about this, sierra sci seems to do exactly this here
					col = _sysPalette.colors[fromColor];
					if (fromColor < toColor) {
						colorCount = toColor - fromColor - 1;
						memmove(&_sysPalette.colors[fromColor], &_sysPalette.colors[fromColor + 1], colorCount * sizeof(Color));
					}
					_sysPalette.colors[toColor - 1] = col;
				} else {
					col = _sysPalette.colors[toColor - 1];
					if (fromColor < toColor) {
						colorCount = toColor - fromColor - 1;
						memmove(&_sysPalette.colors[fromColor + 1], &_sysPalette.colors[fromColor], colorCount * sizeof(Color));
					}
					_sysPalette.colors[fromColor] = col;
				}
				// removing schedule
				_schedules[scheduleNr].schedule = now + ABS(speed);
				// TODO: Not sure when sierra actually removes a schedule
				//_schedules.remove_at(scheduleNr);
				return true;
			}
			return false;
		}
	}
	return false;
}

// palVary
//  init - only does, if palVaryOn == false
//         target, start, new palette allocation
//         palVaryOn = true
//         palDirection = 1
//         palStop = 64
//         palTime = from caller
//         copy resource palette to target
//         init target palette (used = 1 on all colors, color 0 = RGB 0, 0, 0 color 255 = RGB 0xFF, 0xFF, 0xFF
//         copy sysPalette to startPalette
//         init new palette like target palette
//         palPercent = 1
//         do various things
//         return 1
//  deinit - unloads target palette, kills timer hook, disables palVaryOn
//  pause - counts up or down, if counter != 0 -> signal wont get counted up by timer
//           will only count down to 0
//
// Restarting game
//         palVary = false
//         palPercent = 0
//         call palVary deinit
//
// Saving/restoring
//         need to save start and target-palette, when palVaryOn = true

} // End of namespace Sci
