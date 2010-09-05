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

#ifndef PSP_DISPLAY_MAN_H
#define PSP_DISPLAY_MAN_H

#include "backends/platform/psp/thread.h"

/**
 *	Class used only by DisplayManager to start/stop GU rendering
 */
class MasterGuRenderer : public PspThreadable {
public:
	MasterGuRenderer() : _lastRenderTime(0), _renderFinished(true), _callbackId(-1) {}
	void guInit();
	void guPreRender();
	void guPostRender();
	void guShutDown();
	bool isRenderFinished() { return _renderFinished; }
	void setupCallbackThread();	
private:
	virtual void threadFunction();			// for the display callback thread
	static uint32 _displayList[];
	uint32 _lastRenderTime;					// For measuring rendering time
	void guProgramDisplayBufferSizes();
	static int guCallback(int, int, void *__this);	// for the display callback
	bool _renderFinished;					// for sync with render callback
	int _callbackId;						// to keep track of render callback	
};

class Screen;
class Overlay;
class Cursor;
class PSPKeyboard;

/**
 *	Class that manages all display clients
 */
class DisplayManager {
public:
	enum GraphicsModeID {			///> Possible output formats onscreen
		ORIGINAL_RESOLUTION,
		KEEP_ASPECT_RATIO,
		STRETCHED_FULL_SCREEN
	};
	DisplayManager() : _screen(0), _cursor(0), _overlay(0), _keyboard(0), _lastUpdateTime(0), _graphicsMode(0) {}
	~DisplayManager();

	void init();
	bool renderAll();	// return true if rendered or nothing dirty. False otherwise
	bool setGraphicsMode(int mode);
	bool setGraphicsMode(const char *name);
	int getGraphicsMode() const { return _graphicsMode; }
	uint32 getDefaultGraphicsMode() const { return STRETCHED_FULL_SCREEN; }
	const OSystem::GraphicsMode* getSupportedGraphicsModes() const { return _supportedModes; }

	// Setters
	void setScreen(Screen *screen) { _screen = screen; }
	void setCursor(Cursor *cursor) { _cursor = cursor; }
	void setOverlay(Overlay *overlay) { _overlay = overlay; }
	void setKeyboard(PSPKeyboard *keyboard) { _keyboard = keyboard; }
	void setSizeAndPixelFormat(uint width, uint height, const Graphics::PixelFormat *format);

	// Getters
	float getScaleX() const { return _displayParams.scaleX; }
	float getScaleY() const { return _displayParams.scaleY; }
	uint32 getOutputWidth() const { return _displayParams.screenOutput.width; }
	uint32 getOutputHeight() const { return _displayParams.screenOutput.height; }
	uint32 getOutputBitsPerPixel() const { return _displayParams.outputBitsPerPixel; }
	Common::List<Graphics::PixelFormat> getSupportedPixelFormats() const;

private:
	struct GlobalDisplayParams {
		Dimensions screenOutput;
		Dimensions screenSource;
		float scaleX;
		float scaleY;
		uint32 outputBitsPerPixel;		// How many bits end up on-screen
		GlobalDisplayParams() : scaleX(0.0f), scaleY(0.0f), outputBitsPerPixel(0) {}
	};

	// Pointers to DisplayClients
	Screen *_screen;
	Cursor *_cursor;
	Overlay *_overlay;
	PSPKeyboard *_keyboard;

	MasterGuRenderer _masterGuRenderer;
	uint32 _lastUpdateTime;					// For limiting FPS
	int _graphicsMode;
	GlobalDisplayParams _displayParams;
	static const OSystem::GraphicsMode _supportedModes[];

	void calculateScaleParams();	// calculates scaling factor
	bool isTimeToUpdate();			// should we update the screen now
};


#endif /* PSP_DISPLAY_MAN_H */
