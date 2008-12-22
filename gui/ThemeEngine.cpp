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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/util.h"
#include "common/system.h"
#include "common/events.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/unzip.h"

#include "graphics/surface.h"
#include "graphics/colormasks.h"
#include "graphics/imageman.h"
#include "graphics/cursorman.h"
#include "graphics/VectorRenderer.h"

#include "gui/launcher.h"
#include "gui/ThemeEngine.h"
#include "gui/ThemeEval.h"
#include "gui/ThemeParser.h"
#include "gui/ThemeData.h"

#define GUI_ENABLE_BUILTIN_THEME

namespace GUI {

/**********************************************************
 *	ThemeEngine class
 *********************************************************/
ThemeEngine::ThemeEngine(Common::String fileName, GraphicsMode mode) :
	_system(0), _vectorRenderer(0), _screen(0), _backBuffer(0),
	_buffering(false), _bytesPerPixel(0),  _graphicsMode(kGfxDisabled),
	_font(0), _initOk(false), _themeOk(false), _enabled(false), _cursor(0),
	_loadedThemeX(0), _loadedThemeY(0) {
		
	_system = g_system;
	_parser = new ThemeParser(this);
	_themeEval = new GUI::ThemeEval();

	_useCursor = false;

	for (int i = 0; i < kDrawDataMAX; ++i) {
		_widgets[i] = 0;
	}

	for (int i = 0; i < kTextDataMAX; ++i) {
		_texts[i] = 0;
	}

	_graphicsMode = mode;
	_themeFileName = fileName;
	_initOk = false;
}

ThemeEngine::~ThemeEngine() {
	freeRenderer();
	freeScreen();
	freeBackbuffer();
	unloadTheme();
	delete _parser;
	delete _themeEval;
	delete[] _cursor;

	for (ImagesMap::iterator i = _bitmaps.begin(); i != _bitmaps.end(); ++i)
		ImageMan.unregisterSurface(i->_key);
}





/**********************************************************
 *	Rendering mode management
 *********************************************************/
const ThemeEngine::Renderer ThemeEngine::_rendererModes[] = {
	{ "Disabled GFX", "none", kGfxDisabled },
	{ "Standard Renderer (16bpp)", "normal_16bpp", kGfxStandard16bit },
#ifndef DISABLE_FANCY_THEMES
	{ "Antialiased Renderer (16bpp)", "aa_16bpp", kGfxAntialias16bit }
#endif
};

const uint ThemeEngine::_rendererModesSize = ARRAYSIZE(ThemeEngine::_rendererModes);

const ThemeEngine::GraphicsMode ThemeEngine::_defaultRendererMode =
#ifndef DISABLE_FANCY_THEMES
	ThemeEngine::kGfxAntialias16bit;
#else
	ThemeEngine::kGfxStandard16bit;
#endif

ThemeEngine::GraphicsMode ThemeEngine::findMode(const Common::String &cfg) {
	for (uint i = 0; i < _rendererModesSize; ++i) {
		if (cfg.equalsIgnoreCase(_rendererModes[i].cfg))
			return _rendererModes[i].mode;
	}

	return kGfxDisabled;
}

const char *ThemeEngine::findModeConfigName(GraphicsMode mode) {
	for (uint i = 0; i < _rendererModesSize; ++i) {
		if (mode == _rendererModes[i].mode)
			return _rendererModes[i].cfg;
	}

	return findModeConfigName(kGfxDisabled);
}





/**********************************************************
 *	Theme setup/initialization
 *********************************************************/
bool ThemeEngine::init() {
	// reset everything and reload the graphics
	deinit();
	setGraphicsMode(_graphicsMode);

	if (_screen->pixels && _backBuffer->pixels) {
		_initOk = true;
		clearAll();
		resetDrawArea();
	}

	if (_screen->w >= 400 && _screen->h >= 300) {
		_font = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
	} else {
		_font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	}

	if (isThemeLoadingRequired() || !_themeOk) {
		loadTheme(_themeFileName);
	}

	return true;
}

void ThemeEngine::deinit() {
	if (_initOk) {
		_system->hideOverlay();
		freeRenderer();
		freeScreen();
		freeBackbuffer();
		_initOk = false;
	}
}

void ThemeEngine::freeRenderer() {
	delete _vectorRenderer;
	_vectorRenderer = 0;
}

void ThemeEngine::freeBackbuffer() {
	if (_backBuffer != 0) {
		_backBuffer->free();
		delete _backBuffer;
		_backBuffer = 0;
	}
}

void ThemeEngine::freeScreen() {
	if (_screen != 0) {
		_screen->free();
		delete _screen;
		_screen = 0;
	}
}


void ThemeEngine::unloadTheme() {
	if (!_themeOk)
		return;

	for (int i = 0; i < kDrawDataMAX; ++i) {
		delete _widgets[i];
		_widgets[i] = 0;
	}

	for (int i = 0; i < kTextDataMAX; ++i) {
		delete _texts[i];
		_texts[i] = 0;
	}

	for (ImagesMap::iterator i = _bitmaps.begin(); i != _bitmaps.end(); ++i)
		ImageMan.unregisterSurface(i->_key);

	ImageMan.removeArchive(_themeFileName);

	_themeEval->reset();
	_themeOk = false;
}

void ThemeEngine::clearAll() {
	if (!_initOk)
		return;

	_system->clearOverlay();
	_system->grabOverlay((OverlayColor *)_screen->pixels, _screen->w);
}

void ThemeEngine::refresh() {
	init();
	if (_enabled) {
		_system->showOverlay();

		if (_useCursor) {
			CursorMan.replaceCursorPalette(_cursorPal, 0, MAX_CURS_COLORS);
			CursorMan.replaceCursor(_cursor, _cursorWidth, _cursorHeight, _cursorHotspotX, _cursorHotspotY, 255, _cursorTargetScale);
		}
	}
}

void ThemeEngine::enable() {
	init();
	resetDrawArea();

	if (_useCursor)
		setUpCursor();

	_system->showOverlay();
	clearAll();
	_enabled = true;
}

void ThemeEngine::disable() {
	_system->hideOverlay();

	if (_useCursor) {
		CursorMan.popCursorPalette();
		CursorMan.popCursor();
	}

	_enabled = false;
}

template<typename PixelType>
void ThemeEngine::screenInit(bool backBuffer) {
	uint32 width = _system->getOverlayWidth();
	uint32 height = _system->getOverlayHeight();

	if (backBuffer) {
		freeBackbuffer();
		_backBuffer = new Graphics::Surface;
		_backBuffer->create(width, height, sizeof(PixelType));
	}

	freeScreen();
	_screen = new Graphics::Surface;
	_screen->create(width, height, sizeof(PixelType));
	_system->clearOverlay();
}

void ThemeEngine::setGraphicsMode(GraphicsMode mode) {
	switch (mode) {
	case kGfxStandard16bit:
#ifndef DISABLE_FANCY_THEMES
	case kGfxAntialias16bit:
#endif
		_bytesPerPixel = sizeof(uint16);
		screenInit<uint16>(kEnableBackCaching);
		break;

	default:
		error("Invalid graphics mode");
	}

	freeRenderer();
	_vectorRenderer = Graphics::createRenderer(mode);
	_vectorRenderer->setSurface(_screen);
}

bool ThemeEngine::isWidgetCached(DrawData type, const Common::Rect &r) {
	return _widgets[type] && _widgets[type]->_cached &&
		_widgets[type]->_surfaceCache->w == r.width() &&
		_widgets[type]->_surfaceCache->h == r.height();
}

void ThemeEngine::drawCached(DrawData type, const Common::Rect &r) {
	assert(_widgets[type]->_surfaceCache->bytesPerPixel == _screen->bytesPerPixel);
	_vectorRenderer->blitSurface(_widgets[type]->_surfaceCache, r);
}

void ThemeEngine::calcBackgroundOffset(DrawData type) {
	uint maxShadow = 0;
	for (Common::List<Graphics::DrawStep>::const_iterator step = _widgets[type]->_steps.begin();
		step != _widgets[type]->_steps.end(); ++step) {
		if ((step->autoWidth || step->autoHeight) && step->shadow > maxShadow)
			maxShadow = step->shadow;

		if (step->drawingCall == &Graphics::VectorRenderer::drawCallback_BEVELSQ && step->bevel > maxShadow)
			maxShadow = step->bevel;
	}

	_widgets[type]->_backgroundOffset = maxShadow;
}

void ThemeEngine::restoreBackground(Common::Rect r) {
	r.clip(_screen->w, _screen->h); // AHAHAHAHAHAHAHAHAHAHAHAHAHAHAHAHAHAHAHAHA... Oh god. :(
	_vectorRenderer->blitSurface(_backBuffer, r);
}

bool ThemeEngine::isThemeLoadingRequired() {
	int x = g_system->getOverlayWidth(), y = g_system->getOverlayHeight();

	if (_loadedThemeX == x && _loadedThemeY == y)
		return false;

	_loadedThemeX = x;
	_loadedThemeY = y;

	return true;
}




/**********************************************************
 *	Theme elements management
 *********************************************************/
void ThemeEngine::addDrawStep(const Common::String &drawDataId, const Graphics::DrawStep &step) {
	DrawData id = getDrawDataId(drawDataId);

	assert(_widgets[id] != 0);
	_widgets[id]->_steps.push_back(step);
}

bool ThemeEngine::addTextData(const Common::String &drawDataId, const Common::String &textDataId, Graphics::TextAlign alignH, TextAlignVertical alignV) {
	DrawData id = getDrawDataId(drawDataId);
	TextData textId = getTextDataId(textDataId);

	if (id == -1 || textId == -1 || !_widgets[id])
		return false;

	_widgets[id]->_textDataId = textId;
	_widgets[id]->_textAlignH = alignH;
	_widgets[id]->_textAlignV = alignV;

	return true;
}

bool ThemeEngine::addFont(const Common::String &fontId, const Common::String &file, int r, int g, int b) {
	TextData textId = getTextDataId(fontId);

	if (textId == -1)
		return false;

	if (_texts[textId] != 0)
		delete _texts[textId];

	_texts[textId] = new TextDrawData;

	if (file == "default") {
		_texts[textId]->_fontPtr = _font;
	} else {
		_texts[textId]->_fontPtr = FontMan.getFontByName(file);

		if (!_texts[textId]->_fontPtr) {
			_texts[textId]->_fontPtr = loadFont(file);

			if (!_texts[textId]->_fontPtr)
				error("Couldn't load %s font '%s'", fontId.c_str(), file.c_str());

			FontMan.assignFontToName(file, _texts[textId]->_fontPtr);
		}
	}

	_texts[textId]->_color.r = r;
	_texts[textId]->_color.g = g;
	_texts[textId]->_color.b = b;
	return true;

}

bool ThemeEngine::addBitmap(const Common::String &filename) {
	if (_bitmaps.contains(filename))
		ImageMan.unregisterSurface(filename);

	ImageMan.registerSurface(filename, 0);
	_bitmaps[filename] = ImageMan.getSurface(filename);

	return _bitmaps[filename] != 0;
}

bool ThemeEngine::addDrawData(const Common::String &data, bool cached) {
	DrawData data_id = getDrawDataId(data);

	if (data_id == -1)
		return false;

	if (_widgets[data_id] != 0)
		delete _widgets[data_id];

	_widgets[data_id] = new WidgetDrawData;
	_widgets[data_id]->_cached = cached;
	_widgets[data_id]->_buffer = kDrawDataDefaults[data_id].buffer;
	_widgets[data_id]->_surfaceCache = 0;
	_widgets[data_id]->_textDataId = -1;

	return true;
}


/**********************************************************
 *	Theme XML loading
 *********************************************************/
bool ThemeEngine::loadTheme(const Common::String &fileName) {
	unloadTheme();

	bool tryAgain = false;
	if (fileName != "builtin") {
		ImageMan.addArchive(fileName);
		if (!loadThemeXML(fileName)) {
			warning("Could not parse custom theme '%s'. Falling back to default theme", fileName.c_str());
			tryAgain = true;	// Fall back to default builtin theme
		}
	}

	if (fileName == "builtin" || tryAgain) {
		if (!loadDefaultXML()) // if we can't load the embedded theme, this is a complete failure
			error("Could not load default embedded theme");
	}

	for (int i = 0; i < kDrawDataMAX; ++i) {
		if (_widgets[i] == 0) {
			warning("Missing data asset: '%s'", kDrawDataDefaults[i].name);
		} else {
			calcBackgroundOffset((DrawData)i);

			// TODO: draw the cached widget to the cache surface
			if (_widgets[i]->_cached) {}
		}
	}

	_themeOk = true;
	return true;
}

bool ThemeEngine::loadDefaultXML() {

	// The default XML theme is included on runtime from a pregenerated
	// file inside the themes directory.
	// Use the Python script "makedeftheme.py" to convert a normal XML theme
	// into the "default.inc" file, which is ready to be included in the code.
	bool result;

#ifdef GUI_ENABLE_BUILTIN_THEME
	const char *defaultXML =
#include "themes/default.inc"
	;

	if (!_parser->loadBuffer((const byte*)defaultXML, strlen(defaultXML), false))
		return false;

	_themeName = "ScummVM Classic Theme (Builtin Version)";
	_themeFileName = "builtin";

	result = _parser->parse();
	_parser->close();

	return result;
#else
	warning("The built-in theme is not enabled in the current build. Please load an external theme");
	return false;
#endif
}

bool ThemeEngine::loadThemeXML(const Common::String &themeName) {
	assert(_parser);
	_themeName.clear();

	Common::FSNode node(themeName);
	if (!node.exists() || !node.isReadable())
		return false;

	Common::Archive *archive = 0;

	if (node.getName().hasSuffix(".zip") && !node.isDirectory()) {
#ifdef USE_ZLIB
		Common::ZipArchive *zipArchive = new Common::ZipArchive(node);
		archive = zipArchive;

		if (!zipArchive || !zipArchive->isOpen()) {
			delete zipArchive;
			warning("Failed to open Zip archive '%s'.", themeName.c_str());
			return false;
		}

#endif
	} else if (node.isDirectory()) {

// 		FIXME: This warning makes no sense whatsoever. Who added this?
//		warning("Don't know how to open theme '%s'", themeName.c_str());
		archive = new Common::FSDirectory(node);
	}

	if (!archive)
		return false;

	Common::File themercFile;
	themercFile.open("THEMERC", *archive);
	if (!themercFile.isOpen()) {
		delete archive;
		warning("Theme '%s' contains no 'THEMERC' file.", themeName.c_str());
		return false;
	}

	Common::String stxHeader = themercFile.readLine();
	if (!themeConfigParseHeader(stxHeader, _themeName) || _themeName.empty()) {
		delete archive;
		warning("Corrupted 'THEMERC' file in theme '%s'", _themeFileName.c_str());
		return false;
	}

	Common::ArchiveMemberList members;
	if (0 == archive->listMatchingMembers(members, "*.stx")) {
		delete archive;
		warning("Found no STX files for theme '%s'.", themeName.c_str());
		return false;
	}

	// Loop over all STX files
	for (Common::ArchiveMemberList::iterator i = members.begin(); i != members.end(); ++i) {
		assert((*i)->getName().hasSuffix(".stx"));

		if (_parser->loadStream((*i)->open()) == false) {
			delete archive;
			warning("Failed to load STX file '%s'", (*i)->getDisplayName().c_str());
			_parser->close();
			return false;
		}

		if (_parser->parse() == false) {
			delete archive;
			warning("Failed to parse STX file '%s'", (*i)->getDisplayName().c_str());
			_parser->close();
			return false;
		}

		_parser->close();
	}

	delete archive;
	assert(!_themeName.empty());
	return true;
}



/**********************************************************
 *	Drawing Queue management
 *********************************************************/
void ThemeEngine::queueDD(DrawData type, const Common::Rect &r, uint32 dynamic) {
	if (_widgets[type] == 0)
		return;

	Common::Rect area = r;
	area.clip(_screen->w, _screen->h);

	ThemeItemDrawData *q = new ThemeItemDrawData(this, _widgets[type], area, dynamic);

	if (_buffering) {
		if (_widgets[type]->_buffer) {
			_bufferQueue.push_back(q);
		} else {
			if (kDrawDataDefaults[type].parent != kDDNone && kDrawDataDefaults[type].parent != type)
				queueDD(kDrawDataDefaults[type].parent, r);

			_screenQueue.push_back(q);
		}
	} else {
		q->drawSelf(!_widgets[type]->_buffer, _widgets[type]->_buffer);
		delete q;
	}
}

void ThemeEngine::queueDDText(TextData type, const Common::Rect &r, const Common::String &text, bool restoreBg,
	bool ellipsis, Graphics::TextAlign alignH, TextAlignVertical alignV, int deltax) {

	if (_texts[type] == 0)
		return;

	Common::Rect area = r;
	area.clip(_screen->w, _screen->h);

	ThemeItemTextData *q = new ThemeItemTextData(this, _texts[type], area, text, alignH, alignV, ellipsis, restoreBg, deltax);

	if (_buffering) {
		_screenQueue.push_back(q);
	} else {
		q->drawSelf(true, false);
		delete q;
	}
}

void ThemeEngine::queueBitmap(const Graphics::Surface *bitmap, const Common::Rect &r, bool alpha) {

	Common::Rect area = r;
	area.clip(_screen->w, _screen->h);

	ThemeItemBitmap *q = new ThemeItemBitmap(this, area, bitmap, alpha);

	if (_buffering) {
		_bufferQueue.push_back(q);
	} else {
		q->drawSelf(true, false);
		delete q;
	}
}



/**********************************************************
 *	Widget drawing functions
 *********************************************************/
void ThemeEngine::drawButton(const Common::Rect &r, const Common::String &str, WidgetStateInfo state, uint16 hints) {
	if (!ready())
		return;

	DrawData dd = kDDButtonIdle;

	if (state == kStateEnabled)
		dd = kDDButtonIdle;
	else if (state == kStateHighlight)
		dd = kDDButtonHover;
	else if (state == kStateDisabled)
		dd = kDDButtonDisabled;

	queueDD(dd, r);
	queueDDText(getTextData(dd), r, str, false, false, _widgets[dd]->_textAlignH, _widgets[dd]->_textAlignV);
}

void ThemeEngine::drawLineSeparator(const Common::Rect &r, WidgetStateInfo state) {
	if (!ready())
		return;

	queueDD(kDDSeparator, r);
}

void ThemeEngine::drawCheckbox(const Common::Rect &r, const Common::String &str, bool checked, WidgetStateInfo state) {
	if (!ready())
		return;

	Common::Rect r2 = r;
	DrawData dd = kDDCheckboxDefault;

	if (checked)
		dd = kDDCheckboxSelected;

	if (state == kStateDisabled)
		dd = kDDCheckboxDisabled;

	TextData td = (state == kStateHighlight) ? kTextDataHover : getTextData(dd);
	const int checkBoxSize = MIN((int)r.height(), getFontHeight());

	r2.bottom = r2.top + checkBoxSize;
	r2.right = r2.left + checkBoxSize;

	queueDD(dd, r2);

	r2.left = r2.right + checkBoxSize;
	r2.right = r.right;

	queueDDText(td, r2, str, false, false, _widgets[kDDCheckboxDefault]->_textAlignH, _widgets[dd]->_textAlignV);
}

void ThemeEngine::drawSlider(const Common::Rect &r, int width, WidgetStateInfo state) {
	if (!ready())
		return;

	DrawData dd = kDDSliderFull;

	if (state == kStateHighlight)
		dd = kDDSliderHover;
	else if (state == kStateDisabled)
		dd = kDDSliderDisabled;

	Common::Rect r2 = r;
	r2.setWidth(MIN((int16)width, r.width()));
//	r2.top++; r2.bottom--; r2.left++; r2.right--;

	drawWidgetBackground(r, 0, kWidgetBackgroundSlider, kStateEnabled);

	if (width > r.width() * 5 / 100)
		queueDD(dd, r2);
}

void ThemeEngine::drawScrollbar(const Common::Rect &r, int sliderY, int sliderHeight, ScrollbarState scrollState, WidgetStateInfo state) {
	if (!ready())
		return;

	queueDD(kDDScrollbarBase, r);

	Common::Rect r2 = r;
	const int buttonExtra = (r.width() * 120) / 100;

	r2.bottom = r2.top + buttonExtra;
	queueDD(scrollState == kScrollbarStateUp ? kDDScrollbarButtonHover : kDDScrollbarButtonIdle, r2, Graphics::VectorRenderer::kTriangleUp);

	r2.translate(0, r.height() - r2.height());
	queueDD(scrollState == kScrollbarStateDown ? kDDScrollbarButtonHover : kDDScrollbarButtonIdle, r2, Graphics::VectorRenderer::kTriangleDown);

	r2 = r;
	r2.left += 1;
	r2.right -= 1;
	r2.top += sliderY;
	r2.bottom = r2.top + sliderHeight - 1;

	r2.top += r.width() / 5;
	r2.bottom -= r.width() / 5;
	queueDD(scrollState == kScrollbarStateSlider ? kDDScrollbarHandleHover : kDDScrollbarHandleIdle, r2);
}

void ThemeEngine::drawDialogBackground(const Common::Rect &r, DialogBackground bgtype, WidgetStateInfo state) {
	if (!ready())
		return;

	switch (bgtype) {
		case kDialogBackgroundMain:
			queueDD(kDDMainDialogBackground, r);
			break;

		case kDialogBackgroundSpecial:
			queueDD(kDDSpecialColorBackground, r);
			break;

		case kDialogBackgroundPlain:
			queueDD(kDDPlainColorBackground, r);
			break;

		case kDialogBackgroundDefault:
			queueDD(kDDDefaultBackground, r);
			break;
	}
}

void ThemeEngine::drawCaret(const Common::Rect &r, bool erase, WidgetStateInfo state) {
	if (!ready())
		return;

	if (erase) {
		restoreBackground(r);
		addDirtyRect(r);
	} else
		queueDD(kDDCaret, r);
}

void ThemeEngine::drawPopUpWidget(const Common::Rect &r, const Common::String &sel, int deltax, WidgetStateInfo state, Graphics::TextAlign align) {
	if (!ready())
		return;

	DrawData dd = kDDPopUpIdle;

	if (state == kStateEnabled)
		dd = kDDPopUpIdle;
	else if (state == kStateHighlight)
		dd = kDDPopUpHover;
	else if (state == kStateDisabled)
		dd = kDDPopUpDisabled;

	queueDD(dd, r);

	if (!sel.empty()) {
		Common::Rect text(r.left, r.top, r.right - 16, r.bottom);
		queueDDText(getTextData(dd), text, sel, false, false, _widgets[dd]->_textAlignH, _widgets[dd]->_textAlignV, deltax);
	}
}

void ThemeEngine::drawSurface(const Common::Rect &r, const Graphics::Surface &surface, WidgetStateInfo state, int alpha, bool themeTrans) {
	if (!ready())
		return;

	queueBitmap(&surface, r, themeTrans);
}

void ThemeEngine::drawWidgetBackground(const Common::Rect &r, uint16 hints, WidgetBackground background, WidgetStateInfo state) {
	if (!ready())
		return;

	switch (background) {
	case kWidgetBackgroundBorderSmall:
		queueDD(kDDWidgetBackgroundSmall, r);
		break;

	case kWidgetBackgroundEditText:
		queueDD(kDDWidgetBackgroundEditText, r);
		break;

	case kWidgetBackgroundSlider:
		queueDD(kDDWidgetBackgroundSlider, r);
		break;

	default:
		queueDD(kDDWidgetBackgroundDefault, r);
		break;
	}
}

void ThemeEngine::drawTab(const Common::Rect &r, int tabHeight, int tabWidth, const Common::Array<Common::String> &tabs, int active, uint16 hints, int titleVPad, WidgetStateInfo state) {
	if (!ready())
		return;

	const int tabOffset = 2;
	tabWidth -= tabOffset;

	queueDD(kDDTabBackground, Common::Rect(r.left, r.top, r.right, r.top + tabHeight));

	for (int i = 0; i < (int)tabs.size(); ++i) {
		if (i == active)
			continue;

		Common::Rect tabRect(r.left + i * (tabWidth + tabOffset), r.top, r.left + i * (tabWidth + tabOffset) + tabWidth, r.top + tabHeight);
		queueDD(kDDTabInactive, tabRect);
		queueDDText(getTextData(kDDTabInactive), tabRect, tabs[i], false, false, _widgets[kDDTabInactive]->_textAlignH, _widgets[kDDTabInactive]->_textAlignV);
	}

	if (active >= 0) {
		Common::Rect tabRect(r.left + active * (tabWidth + tabOffset), r.top, r.left + active * (tabWidth + tabOffset) + tabWidth, r.top + tabHeight);
		const uint16 tabLeft = active * (tabWidth + tabOffset);
		const uint16 tabRight =  MAX(r.right - tabRect.right, 0);
		queueDD(kDDTabActive, tabRect, (tabLeft << 16) | (tabRight & 0xFFFF));
		queueDDText(getTextData(kDDTabActive), tabRect, tabs[active], false, false, _widgets[kDDTabActive]->_textAlignH, _widgets[kDDTabActive]->_textAlignV);
	}
}

void ThemeEngine::drawText(const Common::Rect &r, const Common::String &str, WidgetStateInfo state, Graphics::TextAlign align, bool inverted, int deltax, bool useEllipsis, FontStyle font) {
	if (!ready())
		return;

	if (inverted) {
		queueDD(kDDTextSelectionBackground, r);
		queueDDText(kTextDataInverted, r, str, false, useEllipsis, align, kTextAlignVCenter, deltax);
		return;
	}

	switch (font) {
		case kFontStyleNormal:
			queueDDText(kTextDataNormalFont, r, str, true, useEllipsis, align, kTextAlignVCenter, deltax);
			return;

		default:
			break;
	}

	switch (state) {
		case kStateDisabled:
			queueDDText(kTextDataDisabled, r, str, true, useEllipsis, align, kTextAlignVCenter, deltax);
			return;

		case kStateHighlight:
			queueDDText(kTextDataHover, r, str, true, useEllipsis, align, kTextAlignVCenter, deltax);
			return;

		case kStateEnabled:
			queueDDText(kTextDataDefault, r, str, true, useEllipsis, align, kTextAlignVCenter, deltax);
			return;
	}
}

void ThemeEngine::drawChar(const Common::Rect &r, byte ch, const Graphics::Font *font, WidgetStateInfo state) {
	if (!ready())
		return;

	Common::Rect charArea = r;
	charArea.clip(_screen->w, _screen->h);

	Graphics::PixelFormat format = _system->getOverlayFormat();
	uint32 color = Graphics::RGBToColor(_texts[kTextDataDefault]->_color.r, _texts[kTextDataDefault]->_color.g, _texts[kTextDataDefault]->_color.b, format);

	restoreBackground(charArea);
	font->drawChar(_screen, ch, charArea.left, charArea.top, color);
	addDirtyRect(charArea);
}

void ThemeEngine::debugWidgetPosition(const char *name, const Common::Rect &r) {
	_font->drawString(_screen, name, r.left, r.top, r.width(), 0xFFFF, Graphics::kTextAlignRight, 0, true);
	_screen->hLine(r.left, r.top, r.right, 0xFFFF);
	_screen->hLine(r.left, r.bottom, r.right, 0xFFFF);
	_screen->vLine(r.left, r.top, r.bottom, 0xFFFF);
	_screen->vLine(r.right, r.top, r.bottom, 0xFFFF);
}



/**********************************************************
 *	Screen/overlay management
 *********************************************************/
void ThemeEngine::updateScreen() {
	if (!_bufferQueue.empty()) {
		_vectorRenderer->setSurface(_backBuffer);

		for (Common::List<ThemeItem*>::iterator q = _bufferQueue.begin(); q != _bufferQueue.end(); ++q) {
			(*q)->drawSelf(true, false);
			delete *q;
		}

		_vectorRenderer->setSurface(_screen);
		memcpy(_screen->getBasePtr(0,0), _backBuffer->getBasePtr(0,0), _screen->pitch * _screen->h);
		_bufferQueue.clear();
	}

	if (!_screenQueue.empty()) {
		_vectorRenderer->disableShadows();
		for (Common::List<ThemeItem*>::iterator q = _screenQueue.begin(); q != _screenQueue.end(); ++q) {
			(*q)->drawSelf(true, false);
			delete *q;
		}

		_vectorRenderer->enableShadows();
		_screenQueue.clear();
	}

	renderDirtyScreen();
}

void ThemeEngine::renderDirtyScreen() {
	if (_dirtyScreen.empty())
		return;

	Common::List<Common::Rect>::const_iterator i, j;
	for (i = _dirtyScreen.begin(); i != _dirtyScreen.end(); ++i) {
		for (j = i; j != _dirtyScreen.end(); ++j)
			if (j != i && i->contains(*j))
				j = _dirtyScreen.reverse_erase(j);

		_vectorRenderer->copyFrame(_system, *i);
	}

	_dirtyScreen.clear();
}

void ThemeEngine::openDialog(bool doBuffer, ShadingStyle style) {
	if (doBuffer)
		_buffering = true;

	if (style != kShadingNone) {
		_vectorRenderer->applyScreenShading(style);
		addDirtyRect(Common::Rect(0, 0, _screen->w, _screen->h));
	}

	memcpy(_backBuffer->getBasePtr(0,0), _screen->getBasePtr(0,0), _screen->pitch * _screen->h);
	_vectorRenderer->setSurface(_screen);
}

void ThemeEngine::setUpCursor() {
	CursorMan.pushCursorPalette(_cursorPal, 0, MAX_CURS_COLORS);
	CursorMan.pushCursor(_cursor, _cursorWidth, _cursorHeight, _cursorHotspotX, _cursorHotspotY, 255, _cursorTargetScale);
	CursorMan.showMouse(true);
}

bool ThemeEngine::createCursor(const Common::String &filename, int hotspotX, int hotspotY, int scale) {
	if (!_system->hasFeature(OSystem::kFeatureCursorHasPalette))
		return true;

	// Try to locate the specified file among all loaded bitmaps
	const Graphics::Surface *cursor = _bitmaps[filename];
	if (!cursor)
		return false;

	// Set up the cursor parameters
	_cursorHotspotX = hotspotX;
	_cursorHotspotY = hotspotY;
	_cursorTargetScale = scale;

	_cursorWidth = cursor->w;
	_cursorHeight = cursor->h;

	// Allocate a new buffer for the cursor
	delete[] _cursor;
	_cursor = new byte[_cursorWidth * _cursorHeight];
	assert(_cursor);
	memset(_cursor, 0xFF, sizeof(byte) * _cursorWidth * _cursorHeight);

	// Now, scan the bitmap. We have to convert it from 16 bit color mode
	// to 8 bit mode, and have to create a suitable palette on the fly.
	uint colorsFound = 0;
	Common::HashMap<int, int>	colorToIndex;
	const OverlayColor *src = (const OverlayColor*)cursor->pixels;
	Graphics::PixelFormat format = _system->getOverlayFormat();
	for (uint y = 0; y < _cursorHeight; ++y) {
		for (uint x = 0; x < _cursorWidth; ++x) {
			byte r, g, b;
			Graphics::colorToRGB(src[x], r, g, b, format);
			const int col = (r << 16) | (g << 8) | b;

			// Skip transparency (the transparent color actually is 0xFF00FF,
			// but the RGB conversion chops of the lower bits).
			if ((r > 0xF1) && (g < 0x03) && (b > 0xF1))
				continue;

			// If there is no entry yet for this color in the palette: Add one
			if (!colorToIndex.contains(col)) {
				const int index = colorsFound++;
				colorToIndex[col] = index;

				_cursorPal[index * 4 + 0] = r;
				_cursorPal[index * 4 + 1] = g;
				_cursorPal[index * 4 + 2] = b;
				_cursorPal[index * 4 + 3] = 0xFF;

				if (colorsFound > MAX_CURS_COLORS) {
					warning("Cursor contains too many colors (%d, but only %d are allowed)", colorsFound, MAX_CURS_COLORS);
					return false;
				}
			}

			// Copy pixel from the 16 bit source surface to the 8bit target surface
			const int index = colorToIndex[col];
			_cursor[y * _cursorWidth + x] = index;
		}
		src += _cursorWidth;
	}

	_useCursor = true;

	return true;
}


/**********************************************************
 *	Legacy GUI::Theme support functions
 *********************************************************/

const Graphics::Font *ThemeEngine::getFont(FontStyle font) const {
	return _texts[fontStyleToData(font)]->_fontPtr;
}

int ThemeEngine::getFontHeight(FontStyle font) const {
	return ready() ? _texts[fontStyleToData(font)]->_fontPtr->getFontHeight() : 0;
}

int ThemeEngine::getStringWidth(const Common::String &str, FontStyle font) const {
	return ready() ? _texts[fontStyleToData(font)]->_fontPtr->getStringWidth(str) : 0;
}

int ThemeEngine::getCharWidth(byte c, FontStyle font) const {
	return ready() ? _texts[fontStyleToData(font)]->_fontPtr->getCharWidth(c) : 0;
}

ThemeEngine::TextData ThemeEngine::getTextData(DrawData ddId) {
	return _widgets[ddId] ? (TextData)_widgets[ddId]->_textDataId : kTextDataNone;
}



/**********************************************************
 *	External data loading
 *********************************************************/

const Graphics::Font *ThemeEngine::loadFontFromArchive(const Common::String &filename) {
	Common::Archive *arch = 0;
	const Graphics::NewFont *font = 0;

	Common::FSNode node(getThemeFileName());

	if (node.getName().hasSuffix(".zip")) {
#ifdef USE_ZLIB
		Common::ZipArchive *zip = new Common::ZipArchive(node);
		if (!zip || !zip->isOpen())
			return 0;

		arch = zip;
#else
		return 0;
#endif
	} else {
		Common::FSDirectory *dir = new Common::FSDirectory(node);
		if (!dir || !dir->getFSNode().isDirectory())	
			return 0;

		arch = dir;
	}

	Common::SeekableReadStream *stream(arch->openFile(filename));
	if (stream) {
		font = Graphics::NewFont::loadFromCache(*stream);
		delete stream;
	}

	delete arch;
	return font;
}

const Graphics::Font *ThemeEngine::loadFont(const Common::String &filename) {
	const Graphics::Font *font = 0;
	Common::String cacheFilename = genCacheFilename(filename.c_str());
	Common::File fontFile;

	if (!cacheFilename.empty()) {
		if (fontFile.open(cacheFilename))
			font = Graphics::NewFont::loadFromCache(fontFile);

		if (font)
			return font;

		if ((font = loadFontFromArchive(cacheFilename)))
			return font;
	}

	// normal open
	if (fontFile.open(filename)) {
		font = Graphics::NewFont::loadFont(fontFile);
	}

	if (!font) {
		font = loadFontFromArchive(filename);
	}

	if (font) {
		if (!cacheFilename.empty()) {
			if (!Graphics::NewFont::cacheFontData(*(const Graphics::NewFont*)font, cacheFilename)) {
				warning("Couldn't create cache file for font '%s'", filename.c_str());
			}
		}
	}

	return font;
}

Common::String ThemeEngine::genCacheFilename(const char *filename) {
	Common::String cacheName(filename);
	for (int i = cacheName.size() - 1; i >= 0; --i) {
		if (cacheName[i] == '.') {
			while ((uint)i < cacheName.size() - 1) {
				cacheName.deleteLastChar();
			}

			cacheName += "fcc";
			return cacheName;
		}
	}

	return "";
}


/**********************************************************
 *	Static Theme XML functions
 *********************************************************/

bool ThemeEngine::themeConfigParseHeader(Common::String header, Common::String &themeName) {	
	header.trim();

	if (header.empty())
		return false;

	if (header[0] != '[' || header.lastChar() != ']')
		return false;

	header.deleteChar(0);
	header.deleteLastChar();

	Common::StringTokenizer tok(header, ":");

	if (tok.nextToken() != SCUMMVM_THEME_VERSION_STR)
		return false;

	themeName = tok.nextToken();
	Common::String author = tok.nextToken();

	return tok.empty();
}

bool ThemeEngine::themeConfigUseable(const Common::FSNode &node, Common::String &themeName) {
	Common::File stream;
	bool foundHeader = false;

	if (node.getName().hasSuffix(".zip") && !node.isDirectory()) {
#ifdef USE_ZLIB
		Common::ZipArchive zipArchive(node);
		if (zipArchive.hasFile("THEMERC")) {
			stream.open("THEMERC", zipArchive);
		}
#endif
	} else if (node.isDirectory()) {			
		Common::FSNode headerfile = node.getChild("THEMERC");
		if (!headerfile.exists() || !headerfile.isReadable() || headerfile.isDirectory())
			return false;
		stream.open(headerfile);
	}

	if (stream.isOpen()) {
		Common::String stxHeader = stream.readLine();
		foundHeader = themeConfigParseHeader(stxHeader, themeName);
	}

	return foundHeader;
}


} // end of namespace GUI.
