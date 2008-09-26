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
 */

#ifndef GUI_THEME_H
#define GUI_THEME_H

#include "common/system.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/config-file.h"

#include "graphics/surface.h"
#include "graphics/fontman.h"

#define THEME_VERSION 24

namespace GUI {

class Eval;

//! Hint to the theme engine that the widget is used in a non-standard way.
enum ThemeHint {
	//! Indicates that this is the first time the widget is drawn.
	THEME_HINT_FIRST_DRAW = 1 << 0,

	/**
	 * Indicates that the widget will be redrawn often, e.g. list widgets.
	 * It may therefore be a good idea to save the background so that it
	 * can be redrawn quickly.
	 */
	THEME_HINT_SAVE_BACKGROUND = 1 << 1,

	//! Indicates that this is the launcher dialog (maybe delete this in the future)
	THEME_HINT_MAIN_DIALOG = 1 << 2,

	//! Indicates special colorfade
	THEME_HINT_SPECIAL_COLOR = 1 << 3,

	//! Indicates no colorfade
	THEME_HINT_PLAIN_COLOR = 1 << 4,

	//! Indictaes that a shadows should be drawn around the background
	THEME_HINT_USE_SHADOW = 1 << 5,

	/**
	 * Indicates that no background should be restored when drawing the widget
	 * (note that this can be silently ignored if for example the theme does
	 * alpha blending and would blend over an already drawn widget)
	 * TODO: currently only ThemeModern::drawButton supports this
	 */
	THEME_HINT_NO_BACKGROUND_RESTORE = 1 << 6
};

/**
 * Our theme renderer class.
 *
 * It is used to draw the different widgets and
 * getting the layout of the widgets for different
 * resolutions.
 */
class Theme {
public:
	Theme();

	virtual ~Theme();

	//! Defined the align of the text
	enum TextAlign {
		kTextAlignLeft,		//! Text should be aligned to the left
		kTextAlignCenter,	//! Text should be centered
		kTextAlignRight		//! Text should be aligned to the right
	};

	//! Widget background type
	enum WidgetBackground {
		kWidgetBackgroundNo,			//! No background at all
		kWidgetBackgroundPlain,			//! Simple background, this may not include borders
		kWidgetBackgroundBorder,		//! Same as kWidgetBackgroundPlain just with a border
		kWidgetBackgroundBorderSmall,	//! Same as kWidgetBackgroundPlain just with a small border
		kWidgetBackgroundEditText,		//! Background used for edit text fields
		kWidgetBackgroundSlider			//! Background used for sliders
	};

	//! State of the widget to be drawn
	enum State {
		kStateDisabled,		//! Indicates that the widget is disabled, that does NOT include that it is invisible
		kStateEnabled,		//! Indicates that the widget is enabled
		kStateHighlight		//! Indicates that the widget is highlighted by the user
	};

	typedef State WidgetStateInfo;

	enum ScrollbarState {
		kScrollbarStateNo,
		kScrollbarStateUp,
		kScrollbarStateDown,
		kScrollbarStateSlider,
		kScrollbarStateSinglePage
	};

	//! Font style selector
	enum FontStyle {
		kFontStyleBold = 0,			//! A bold font. This is also the default font.
		kFontStyleNormal = 1,		//! A normal font.
		kFontStyleItalic = 2,		//! Italic styled font.
		kFontStyleFixedNormal = 3,	//! Fixed size font.
		kFontStyleFixedBold = 4,	//! Fixed size bold font.
		kFontStyleFixedItalic = 5,	//! Fixed size italic font.
		kFontStyleMax
	};

	//! Function used to process areas other than the current dialog
	enum ShadingStyle {
		kShadingNone,		//! No special post processing
		kShadingDim,		//! Dimming unused areas
		kShadingLuminance	//! Converting colors to luminance for unused areas
	};

	/**
	 * This initializes all the data needed by the theme renderer.
	 * It should just be called *once*, when first using the renderer.
	 *
	 * Other functions of the renderer should just be used after
	 * calling this function, else the result is undefined.
	 *
	 * If used again it should just be used after deinit,
	 * if there is need to use the renderer again.
	 *
	 * @see deinit
	 */
	virtual bool init() = 0;

	/**
	 * Unloads all data used by the theme renderer.
	 */
	virtual void deinit() = 0;

	/**
	 * Updates the renderer to changes to resolution,
	 * bit depth and other video related configuration.
	 */
	virtual void refresh() = 0;

	/**
	 * Checks if the theme supplies its own cursor.
	 *
	 * @return true if using an own cursor
	 */
	virtual bool ownCursor() const { return false; }

	/**
	 * Enables the theme renderer for use.
	 *
	 * This for examples displays the overlay, clears the
	 * renderer's temporary screen buffers and does other
	 * things to make the renderer for use.
	 *
	 * This will NOT back up the data on the overlay.
	 * So if you've got data in the overlay save it before
	 * calling this.
	 *
	 * Unlike init, this makes the renderer ready to draw
	 * something to the screen. And of course it relies on the data
	 * loaded by init.
	 *
	 * @see disable
	 * @see init
	 */
	virtual void enable() = 0;

	/**
	 * Disables the theme renderer.
	 *
	 * This for example hides the overlay and undoes
	 * other things done by enable.
	 *
	 * Unlike uninit, this just makes the renderer unable
	 * to do any screen drawing, but still keeps all data
	 * loaded into memory.
	 *
	 * @see enable
	 * @see uninit
	 */
	virtual void disable() = 0;

	/**
	 * Tells the theme renderer that a new dialog is opened.
	 *
	 * This can be used for internal caching and marking
	 * area of all but the not top dialog in a special way.
	 *
	 * TODO: This needs serious reworking, since at least for
	 * normal usage, a dialog opened with openDialog should always
	 * be the top dialog. Currently our themes have no good enough
	 * implementation to handle a single open dialog though, so we
	 * have to stay this way until we implement proper dialog
	 * 'caching'/handling.
	 *
	 * @param topDialog	if true it indicates that this is the top dialog
	 *
	 * @see closeAllDialogs
	 */
	virtual void openDialog(bool topDialog) = 0;

	/**
	 * This indicates that all dialogs have been closed.
	 *
	 * @see openDialog
	 */
	virtual void closeAllDialogs() = 0;

	/**
	 * Clear the complete GUI screen.
	 */
	virtual void clearAll() = 0;

	/**
	 * Update the GUI screen aka overlay.
	 *
	 * This does NOT call OSystem::updateScreen,
	 * it just copies all (changed) data to the overlay.
	 */
	virtual void updateScreen() = 0;

	/**
	 * Set the active screen area, in which the renderer is able to
	 * draw.
	 *
	 * This does not affect the coordinates for the draw* functions,
	 * it just marks the screen rect given in param r as writeable.
	 *
	 * This is for example used in the credits dialog, which, if not
	 * just a part of the screen would be marked as writeable, would
	 * draw parts of the scrolling text outside the dialog box and
	 * thus would look strange.
	 *
	 * The active area defaults to the whole screen, so there is just
	 * need to use this function if you want to limit it.
	 *
	 * @param r	rect of the screen, which should be writeable
	 *
	 * @see resetDrawArea
	 */
	virtual void setDrawArea(const Common::Rect &r) { _drawArea = r; }

	/**
	 * Resets the draw area to the whole screen.
	 *
	 * @see setDrawArea
	 */
	virtual void resetDrawArea() = 0;

	virtual const Common::ConfigFile &getConfigFile() const { return _configFile; }

	virtual const Graphics::Font *getFont(FontStyle font = kFontStyleBold) const = 0;
	virtual int getFontHeight(FontStyle font = kFontStyleBold) const = 0;
	virtual int getStringWidth(const Common::String &str, FontStyle font = kFontStyleBold) const = 0;
	virtual int getCharWidth(byte c, FontStyle font = kFontStyleBold) const = 0;

	virtual void drawDialogBackground(const Common::Rect &r, uint16 hints, WidgetStateInfo state = kStateEnabled) = 0;
	virtual void drawText(const Common::Rect &r, const Common::String &str, WidgetStateInfo state = kStateEnabled, TextAlign align = kTextAlignCenter, bool inverted = false, int deltax = 0, bool useEllipsis = true, FontStyle font = kFontStyleBold) = 0;
	// this should ONLY be used by the debugger until we get a nicer solution
	virtual void drawChar(const Common::Rect &r, byte ch, const Graphics::Font *font, WidgetStateInfo state = kStateEnabled) = 0;

	virtual void drawWidgetBackground(const Common::Rect &r, uint16 hints, WidgetBackground background = kWidgetBackgroundPlain, WidgetStateInfo state = kStateEnabled) = 0;
	virtual void drawButton(const Common::Rect &r, const Common::String &str, WidgetStateInfo state = kStateEnabled, uint16 hints = 0) = 0;
	virtual void drawSurface(const Common::Rect &r, const Graphics::Surface &surface, WidgetStateInfo state = kStateEnabled, int alpha = 256, bool themeTrans = false) = 0;
	virtual void drawSlider(const Common::Rect &r, int width, WidgetStateInfo state = kStateEnabled) = 0;
	virtual void drawCheckbox(const Common::Rect &r, const Common::String &str, bool checked, WidgetStateInfo state = kStateEnabled) = 0;
	virtual void drawTab(const Common::Rect &r, int tabHeight, int tabWidth, const Common::Array<Common::String> &tabs, int active, uint16 hints, int titleVPad, WidgetStateInfo state = kStateEnabled) = 0;
	virtual void drawScrollbar(const Common::Rect &r, int sliderY, int sliderHeight, ScrollbarState, WidgetStateInfo state = kStateEnabled) = 0;
	virtual void drawPopUpWidget(const Common::Rect &r, const Common::String &sel, int deltax, WidgetStateInfo state = kStateEnabled, TextAlign align = kTextAlignLeft) = 0;
	virtual void drawCaret(const Common::Rect &r, bool erase, WidgetStateInfo state = kStateEnabled) = 0;
	virtual void drawLineSeparator(const Common::Rect &r, WidgetStateInfo state = kStateEnabled) = 0;

	virtual void restoreBackground(Common::Rect r, bool special = false) = 0;
	virtual bool addDirtyRect(Common::Rect r, bool save = false, bool special = false) = 0;

	virtual int getTabSpacing() const = 0;
	virtual int getTabPadding() const = 0;

	Graphics::TextAlignment convertAligment(TextAlign align) const {
		switch (align) {
		case kTextAlignLeft:
			return Graphics::kTextAlignLeft;
			break;

		case kTextAlignRight:
			return Graphics::kTextAlignRight;
			break;

		default:
			break;
		};
		return Graphics::kTextAlignCenter;
	}

	TextAlign convertAligment(Graphics::TextAlignment align) const {
		switch (align) {
		case Graphics::kTextAlignLeft:
			return kTextAlignLeft;
			break;

		case Graphics::kTextAlignRight:
			return kTextAlignRight;
			break;

		default:
			break;
		}
		return kTextAlignCenter;
	}

	void processResSection(Common::ConfigFile &config, const Common::String &name, bool skipDefs = false, const Common::String &prefix = "");
	void processSingleLine(const Common::String &section, const Common::String &prefix, const Common::String &name, const Common::String &str);
	void setSpecialAlias(const Common::String &alias, const Common::String &name);

	bool isThemeLoadingRequired();
	bool sectionIsSkipped(Common::ConfigFile &config, const char *name, int w, int h);
	void loadTheme(Common::ConfigFile &config, bool reset = true);
	void loadTheme(Common::ConfigFile &config, bool reset, bool doBackendSpecificPostProcessing);
	Eval *_evaluator;

	static bool themeConfigUseable(const Common::String &file, const Common::String &style="", Common::String *cStyle=0, Common::ConfigFile *cfg=0);

	const Common::String &getStylefileName() const { return _stylefile; }
	const Common::String &getThemeName() const { return _stylename; }

	/**
	 * Checks if the theme renderer supports drawing of images.
	 *
	 * @return true on support, else false
	 */
	virtual bool supportsImages() const { return false; }

	//! Special image ids for images used in the GUI
	enum kThemeImages {
		kImageLogo = 0,		//! ScummVM Logo used in the launcher
		kImageLogoSmall		//! ScummVM logo used in the GMM
	};

	/**
	 * Returns the given image.
	 *
	 * @param n	id of the image, see kThemeImages
	 * @return 0 if no such image exists for the theme, else pointer to the image
	 *
	 * @see kThemeImages
	 */
	virtual const Graphics::Surface *getImageSurface(const kThemeImages n) const { return 0; }
protected:
	bool loadConfigFile(const Common::String &file);
	void getColorFromConfig(const Common::String &name, OverlayColor &col);
	void getColorFromConfig(const Common::String &value, uint8 &r, uint8 &g, uint8 &b);

	const Graphics::Font *loadFont(const char *filename);
	Common::String genCacheFilename(const char *filename);

	Common::String _stylefile, _stylename;

	Common::Rect _drawArea;
	Common::ConfigFile _configFile;
	Common::ConfigFile _defaultConfig;

public:
	bool needThemeReload() { return ((_loadedThemeX != g_system->getOverlayWidth()) ||
									 (_loadedThemeY != g_system->getOverlayHeight())); }

private:
	static const char *_defaultConfigINI;
	int _loadedThemeX, _loadedThemeY;
};
} // end of namespace GUI

#endif // GUI_THEME_H
