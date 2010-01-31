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
#include "common/stack.h"
#include "graphics/primitives.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/ports.h"
#include "sci/graphics/paint16.h"
#include "sci/graphics/font.h"
#include "sci/graphics/text.h"

namespace Sci {

Text::Text(ResourceManager *resMan, GfxCache *cache, GfxPorts *ports, GfxPaint16 *paint16, GfxScreen *screen)
	: _resMan(resMan), _cache(cache), _ports(ports), _paint16(paint16), _screen(screen) {
	init();
}

Text::~Text() {
}

void Text::init() {
	_font = NULL;
	_codeFonts = NULL;
	_codeFontsCount = 0;
	_codeColors = NULL;
	_codeColorsCount = 0;
}

GuiResourceId Text::GetFontId() {
	return _ports->_curPort->fontId;
}

Font *Text::GetFont() {
	if ((_font == NULL) || (_font->getResourceId() != _ports->_curPort->fontId))
		_font = _cache->getFont(_ports->_curPort->fontId);

	return _font;
}

void Text::SetFont(GuiResourceId fontId) {
	if ((_font == NULL) || (_font->getResourceId() != fontId))
		_font = _cache->getFont(fontId);

	_ports->_curPort->fontId = _font->getResourceId();
	_ports->_curPort->fontHeight = _font->getHeight();
}

void Text::CodeSetFonts(int argc, reg_t *argv) {
	int i;

	delete _codeFonts;
	_codeFontsCount = argc;
	_codeFonts = new GuiResourceId[argc];
	for (i = 0; i < argc; i++) {
		_codeFonts[i] = (GuiResourceId)argv[i].toUint16();
	}
}

void Text::CodeSetColors(int argc, reg_t *argv) {
	int i;

	delete _codeColors;
	_codeColorsCount = argc;
	_codeColors = new uint16[argc];
	for (i = 0; i < argc; i++) {
		_codeColors[i] = argv[i].toUint16();
	}
}

void Text::ClearChar(int16 chr) {
	if (_ports->_curPort->penMode != 1)
		return;
	Common::Rect rect;
	rect.top = _ports->_curPort->curTop;
	rect.bottom = rect.top + _ports->_curPort->fontHeight;
	rect.left = _ports->_curPort->curLeft;
	rect.right = rect.left + GetFont()->getCharWidth(chr);
	_paint16->eraseRect(rect);
}

void Text::DrawChar(int16 chr) {
	chr = chr & 0xFF;
	ClearChar(chr);
	StdChar(chr);
	_ports->_curPort->curLeft += GetFont()->getCharWidth(chr);
}

void Text::StdChar(int16 chr) {
#if 0
	CResFont*res = getResFont();
	if (res)
		res->Draw(chr, _curPort->top + _curPort->curTop, _curPort->left
				+ _curPort->curLeft, _vSeg, 320, _curPort->penClr,
				_curPort->textFace);
#endif
}

// This internal function gets called as soon as a '|' is found in a text
//  It will process the encountered code and set new font/set color
//  We only support one-digit codes currently, don't know if multi-digit codes are possible
//  Returns textcode character count
int16 Text::CodeProcessing(const char *&text, GuiResourceId orgFontId, int16 orgPenColor) {
	const char *textCode = text;
	int16 textCodeSize = 0;
	char curCode;
	unsigned char curCodeParm;

	// Find the end of the textcode
	while ((++textCodeSize) && (*text != 0) && (*text++ != 0x7C)) { }

	// possible TextCodes:
	//  c -> sets textColor to current port pen color
	//  cX -> sets textColor to _textColors[X-1]
	curCode = textCode[0];
	curCodeParm = textCode[1];
	if (isdigit(curCodeParm)) {
		curCodeParm -= '0';
	} else {
		curCodeParm = 0;
	}
	switch (curCode) {
	case 'c': // set text color
		if (curCodeParm == 0) {
			_ports->_curPort->penClr = orgPenColor;
		} else {
			if (curCodeParm < _codeColorsCount) {
				_ports->_curPort->penClr = _codeColors[curCodeParm];
			}
		}
		break;
	case 'f':
		if (curCodeParm == 0) {
			SetFont(orgFontId);
		} else {
			if (curCodeParm < _codeFontsCount) {
				SetFont(_codeFonts[curCodeParm]);
			}
		}
		break;
	}
	return textCodeSize;
}

// return max # of chars to fit maxwidth with full words
int16 Text::GetLongest(const char *text, int16 maxWidth, GuiResourceId orgFontId) {
	char curChar;
	int16 maxChars = 0, curCharCount = 0;
	uint16 width = 0;
	GuiResourceId oldFontId = GetFontId();
	int16 oldPenColor = _ports->_curPort->penClr;

	GetFont();
	if (!_font)
		return 0;

	while (width <= maxWidth) {
		curChar = *text++;
		switch (curChar) {
		case 0x7C:
			if (getSciVersion() >= SCI_VERSION_1_1) {
				curCharCount++;
				curCharCount += CodeProcessing(text, orgFontId, oldPenColor);
				continue;
			}
			break;

		case 0xD:
			curCharCount++;
			continue;

		case 0xA:
			curCharCount++;
		case 0:
			SetFont(oldFontId);
			_ports->penColor(oldPenColor);
			return curCharCount;

		case ' ':
			maxChars = curCharCount + 1;
			break;
		}
		width += _font->getCharWidth(curChar);
		curCharCount++;
	}
	SetFont(oldFontId);
	_ports->penColor(oldPenColor);
	return maxChars;
}

void Text::Width(const char *text, int16 from, int16 len, GuiResourceId orgFontId, int16 &textWidth, int16 &textHeight) {
	unsigned char curChar;
	GuiResourceId oldFontId = GetFontId();
	int16 oldPenColor = _ports->_curPort->penClr;

	textWidth = 0; textHeight = 0;

	GetFont();
	if (_font) {
		text += from;
		while (len--) {
			curChar = *text++;
			switch (curChar) {
			case 0x0A:
			case 0x0D:
				textHeight = MAX<int16> (textHeight, _ports->_curPort->fontHeight);
				break;
			case 0x7C:
				if (getSciVersion() >= SCI_VERSION_1_1) {
					len -= CodeProcessing(text, orgFontId, 0);
					break;
				}
			default:
				textHeight = MAX<int16> (textHeight, _ports->_curPort->fontHeight);
				textWidth += _font->getCharWidth(curChar);
			}
		}
	}
	SetFont(oldFontId);
	_ports->penColor(oldPenColor);
	return;
}

void Text::StringWidth(const char *str, GuiResourceId orgFontId, int16 &textWidth, int16 &textHeight) {
	Width(str, 0, (int16)strlen(str), orgFontId, textWidth, textHeight);
}

void Text::ShowString(const char *str, GuiResourceId orgFontId, int16 orgPenColor) {
	Show(str, 0, (int16)strlen(str), orgFontId, orgPenColor);
}
void Text::DrawString(const char *str, GuiResourceId orgFontId, int16 orgPenColor) {
	Draw(str, 0, (int16)strlen(str), orgFontId, orgPenColor);
}

int16 Text::Size(Common::Rect &rect, const char *str, GuiResourceId fontId, int16 maxWidth) {
	GuiResourceId oldFontId = GetFontId();
	int16 oldPenColor = _ports->_curPort->penClr;
	int16 charCount;
	int16 maxTextWidth = 0, textWidth;
	int16 totalHeight = 0, textHeight;

	if (fontId != -1)
		SetFont(fontId);
	rect.top = rect.left = 0;

	if (maxWidth < 0) { // force output as single line
		StringWidth(str, oldFontId, textWidth, textHeight);
		rect.bottom = textHeight;
		rect.right = textWidth;
	} else {
		// rect.right=found widest line with RTextWidth and GetLongest
		// rect.bottom=num. lines * GetPointSize
		rect.right = (maxWidth ? maxWidth : 192);
		const char*p = str;
		while (*p) {
			//if (*p == 0xD || *p == 0xA) {
			//	p++;
			//	continue;
			//}
			charCount = GetLongest(p, rect.right, oldFontId);
			if (charCount == 0)
				break;
			Width(p, 0, charCount, oldFontId, textWidth, textHeight);
			maxTextWidth = MAX(textWidth, maxTextWidth);
			totalHeight += textHeight;
			p += charCount;
		}
		rect.bottom = totalHeight;
		rect.right = maxWidth ? maxWidth : MIN(rect.right, maxTextWidth);
	}
	SetFont(oldFontId);
	_ports->penColor(oldPenColor);
	return rect.right;
}

// returns maximum font height used
void Text::Draw(const char *text, int16 from, int16 len, GuiResourceId orgFontId, int16 orgPenColor) {
	int16 curChar, charWidth;
	Common::Rect rect;

	GetFont();
	if (!_font)
		return;

	rect.top = _ports->_curPort->curTop;
	rect.bottom = rect.top + _ports->_curPort->fontHeight;
	text += from;
	while (len--) {
		curChar = (*text++);
		switch (curChar) {
		case 0x0A:
		case 0x0D:
		case 0:
			break;
		case 0x7C:
			if (getSciVersion() >= SCI_VERSION_1_1) {
				len -= CodeProcessing(text, orgFontId, orgPenColor);
				break;
			}
		default:
			charWidth = _font->getCharWidth(curChar);
			// clear char
			if (_ports->_curPort->penMode == 1) {
				rect.left = _ports->_curPort->curLeft;
				rect.right = rect.left + charWidth;
				_paint16->eraseRect(rect);
			}
			// CharStd
			_font->draw(curChar, _ports->_curPort->top + _ports->_curPort->curTop, _ports->_curPort->left + _ports->_curPort->curLeft, _ports->_curPort->penClr, _ports->_curPort->greyedOutput);
			_ports->_curPort->curLeft += charWidth;
		}
	}
}

// returns maximum font height used
void Text::Show(const char *text, int16 from, int16 len, GuiResourceId orgFontId, int16 orgPenColor) {
	Common::Rect rect;

	rect.top = _ports->_curPort->curTop;
	rect.bottom = rect.top + _ports->getPointSize();
	rect.left = _ports->_curPort->curLeft;
	Draw(text, from, len, orgFontId, orgPenColor);
	rect.right = _ports->_curPort->curLeft;
	_paint16->bitsShow(rect);
}

// Draws a text in rect.
void Text::Box(const char *text, int16 bshow, const Common::Rect &rect, TextAlignment alignment, GuiResourceId fontId) {
	int16 textWidth, textHeight, charCount;
	int16 offset = 0;
	int16 hline = 0;
	GuiResourceId orgFontId = GetFontId();
	int16 orgPenColor = _ports->_curPort->penClr;

	if (fontId != -1)
		SetFont(fontId);

	while (*text) {
//		if (*text == 0xD || *text == 0xA) {
//			text++;
//			continue;
//		}
		charCount = GetLongest(text, rect.width(), orgFontId);
		if (charCount == 0)
			break;
		Width(text, 0, charCount, orgFontId, textWidth, textHeight);
		switch (alignment) {
		case SCI_TEXT_ALIGNMENT_RIGHT:
			offset = rect.width() - textWidth;
			break;
		case SCI_TEXT_ALIGNMENT_CENTER:
			offset = (rect.width() - textWidth) / 2;
			break;
		case SCI_TEXT_ALIGNMENT_LEFT:
			offset = 0;
			break;

		default: // left-aligned
			warning("Invalid alignment %d used in TextBox()", alignment);
		}
		_ports->moveTo(rect.left + offset, rect.top + hline);

		if (bshow) {
			Show(text, 0, charCount, orgFontId, orgPenColor);
		} else {
			Draw(text, 0, charCount, orgFontId, orgPenColor);
		}

		hline += textHeight;
		text += charCount;
	}
	SetFont(orgFontId);
	_ports->penColor(orgPenColor);
}

void Text::Draw_String(const char *text) {
	GuiResourceId orgFontId = GetFontId();
	int16 orgPenColor = _ports->_curPort->penClr;

	Draw(text, 0, strlen(text), orgFontId, orgPenColor);
	SetFont(orgFontId);
	_ports->penColor(orgPenColor);
}

} // End of namespace Sci
