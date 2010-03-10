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

#include <pspgu.h>
#include <pspdisplay.h>

#include <time.h>
#include <zlib.h>

#include "common/config-manager.h"
#include "common/events.h"
#include "common/scummsys.h"

#include "osys_psp.h"
#include "trace.h"
#include "powerman.h"

#include "backends/saves/psp/psp-saves.h"
#include "backends/timer/default/default-timer.h"
#include "graphics/surface.h"
#include "sound/mixer_intern.h"

#include "backends/platform/psp/pspkeyboard.h"


#define	SAMPLES_PER_SEC	44100

#define PIXEL_SIZE (4)
#define BUF_WIDTH (512)
#define	PSP_SCREEN_WIDTH	480
#define	PSP_SCREEN_HEIGHT	272
#define PSP_FRAME_SIZE (BUF_WIDTH * PSP_SCREEN_HEIGHT * PIXEL_SIZE)
#define MOUSE_SIZE	128
#define	KBD_DATA_SIZE	130560

#define	MAX_FPS	30

unsigned int __attribute__((aligned(16))) displayList[2048];
unsigned short __attribute__((aligned(16))) clut256[256];
unsigned short __attribute__((aligned(16))) mouseClut[256];
unsigned short __attribute__((aligned(16))) cursorPalette[256];
unsigned int __attribute__((aligned(16))) mouseBuf256[MOUSE_SIZE*MOUSE_SIZE];


unsigned long RGBToColour(unsigned long r, unsigned long g, unsigned long b) {
	return (((b >> 3) << 10) | ((g >> 3) << 5) | ((r >> 3) << 0)) | 0x8000;
}

static int timer_handler(int t) {
	DefaultTimerManager *tm = (DefaultTimerManager *)g_system->getTimerManager();
	tm->handler();
	return t;
}

const OSystem::GraphicsMode OSystem_PSP::s_supportedGraphicsModes[] = {
	{ "320x200 (centered)", "320x200 16-bit centered", CENTERED_320X200 },
	{ "435x272 (best-fit, centered)", "435x272 16-bit centered", CENTERED_435X272 },
	{ "480x272 (full screen)", "480x272 16-bit stretched", STRETCHED_480X272 },
	{ "362x272 (4:3, centered)", "362x272 16-bit centered", CENTERED_362X272 },
	{0, 0, 0}
};


OSystem_PSP::OSystem_PSP() : _screenWidth(0), _screenHeight(0), _overlayWidth(0), _overlayHeight(0),
		_offscreen(0), _overlayBuffer(0), _overlayVisible(false), _shakePos(0), _lastScreenUpdate(0),
		_mouseBuf(0), _prevButtons(0), _lastPadCheck(0), _mixer(0) {
	memset(_palette, 0, sizeof(_palette));

	_cursorPaletteDisabled = true;

	//init SDL
	uint32	sdlFlags = SDL_INIT_AUDIO | SDL_INIT_TIMER;
	SDL_Init(sdlFlags);

	_clut = clut256;
	_mouseBuf = (byte *)mouseBuf256;
	_graphicMode = STRETCHED_480X272;

	_mouseX = PSP_SCREEN_WIDTH >> 1;	// Mouse in the middle of the screen
	_mouseY = PSP_SCREEN_HEIGHT >> 1;
	_dpadX = _dpadY = 0;


	// Init GU
	sceGuInit();
	sceGuStart(0, displayList);
	sceGuDrawBuffer(GU_PSM_8888, (void *)0, BUF_WIDTH);
	sceGuDispBuffer(PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT, (void*)PSP_FRAME_SIZE, BUF_WIDTH);
	sceGuDepthBuffer((void*)(PSP_FRAME_SIZE * 2), BUF_WIDTH);
	sceGuOffset(2048 - (PSP_SCREEN_WIDTH/2), 2048 - (PSP_SCREEN_HEIGHT/2));
	sceGuViewport(2048, 2048, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);
	sceGuDepthRange(0xC350, 0x2710);
	sceGuScissor(0, 0, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuFrontFace(GU_CW);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(1);

}

OSystem_PSP::~OSystem_PSP() {

	free(_offscreen);
	free(_overlayBuffer);
	free(_mouseBuf);
	delete _keyboard;

	_offscreen = 0;
	_overlayBuffer = 0;
	_mouseBuf = 0;
	 sceGuTerm();
}


void OSystem_PSP::initBackend() {
	_savefile = new PSPSaveFileManager;

	_timer = new DefaultTimerManager();

	_keyboard = new PSPKeyboard();
	_keyboard->load();

	setTimerCallback(&timer_handler, 10);

	setupMixer();

	OSystem::initBackend();
}


bool OSystem_PSP::hasFeature(Feature f) {
	return (f == kFeatureOverlaySupportsAlpha || f == kFeatureCursorHasPalette);
}

void OSystem_PSP::setFeatureState(Feature f, bool enable) {
}

bool OSystem_PSP::getFeatureState(Feature f) {
	return false;
}

const OSystem::GraphicsMode* OSystem_PSP::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}


int OSystem_PSP::getDefaultGraphicsMode() const {
	return STRETCHED_480X272;
}

bool OSystem_PSP::setGraphicsMode(int mode) {
	_graphicMode = mode;
	return true;
}

bool OSystem_PSP::setGraphicsMode(const char *name) {
	int i = 0;

	while (s_supportedGraphicsModes[i].name) {
		if (!strcmpi(s_supportedGraphicsModes[i].name, name)) {
			_graphicMode = s_supportedGraphicsModes[i].id;
			return true;
		}
		i++;
	}

	return false;
}

int OSystem_PSP::getGraphicsMode() const {
	return _graphicMode;
}

void OSystem_PSP::initSize(uint width, uint height, const Graphics::PixelFormat *format) {
	PSPDebugTrace("initSize\n");

	_screenWidth = width;
	_screenHeight = height;

	const int scrBufSize = _screenWidth * _screenHeight * (format ? format->bytesPerPixel : 4);

	_overlayWidth = PSP_SCREEN_WIDTH;	//width;
	_overlayHeight = PSP_SCREEN_HEIGHT;	//height;

	free(_overlayBuffer);
	_overlayBuffer = (OverlayColor *)memalign(16, _overlayWidth * _overlayHeight * sizeof(OverlayColor));

	free(_offscreen);
	_offscreen = (byte *)memalign(16, scrBufSize);
	bzero(_offscreen, scrBufSize);

	clearOverlay();
	memset(_palette, 0xFFFF, 256 * sizeof(unsigned short));

	_mouseVisible = false;
	sceKernelDcacheWritebackAll();
}

int16 OSystem_PSP::getWidth() {
	return _screenWidth;
}

int16 OSystem_PSP::getHeight() {
	return _screenHeight;
}

void OSystem_PSP::setPalette(const byte *colors, uint start, uint num) {
	const byte *b = colors;

	for (uint i = 0; i < num; ++i) {
		_palette[start + i] = RGBToColour(b[0], b[1], b[2]);
		b += 4;
	}

	//copy to CLUT
	memcpy(_clut, _palette, 256 * sizeof(unsigned short));

	//force update of mouse CLUT as well, as it may have been set up before this palette was set
	memcpy(mouseClut, _palette, 256 * sizeof(unsigned short));
	mouseClut[_mouseKeyColour] = 0;

	sceKernelDcacheWritebackAll();
}

void OSystem_PSP::setCursorPalette(const byte *colors, uint start, uint num) {
	const byte *b = colors;

	for (uint i = 0; i < num; ++i) {
		cursorPalette[start + i] = RGBToColour(b[0], b[1], b[2]);
		b += 4;
	}

	cursorPalette[0] = 0;

	_cursorPaletteDisabled = false;

	sceKernelDcacheWritebackAll();
}

void OSystem_PSP::disableCursorPalette(bool disable) {
	_cursorPaletteDisabled = disable;
}

void OSystem_PSP::copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h) {
	//Clip the coordinates
	if (x < 0) {
		w += x;
		buf -= x;
		x = 0;
	}

	if (y < 0) {
		h += y;
		buf -= y * pitch;
		y = 0;
	}

	if (w > _screenWidth - x) {
		w = _screenWidth - x;
	}

	if (h > _screenHeight - y) {
		h = _screenHeight - y;
	}

	if (w <= 0 || h <= 0)
		return;


	byte *dst = _offscreen + y * _screenWidth + x;

	if (_screenWidth == pitch && pitch == w) {
		memcpy(dst, buf, h * w);
	} else {
		do {
			memcpy(dst, buf, w);
			buf += pitch;
			dst += _screenWidth;
		} while (--h);
	}
	sceKernelDcacheWritebackAll();

}

Graphics::Surface *OSystem_PSP::lockScreen() {
	_framebuffer.pixels = _offscreen;
	_framebuffer.w = _screenWidth;
	_framebuffer.h = _screenHeight;
	_framebuffer.pitch = _screenWidth;
	_framebuffer.bytesPerPixel = 1;

	return &_framebuffer;
}

void OSystem_PSP::unlockScreen() {
	// The screen is always completely update anyway, so we don't have to force a full update here.
	sceKernelDcacheWritebackAll();
}

void OSystem_PSP::updateScreen() {
	u32 now = getMillis();
	if (now - _lastScreenUpdate < 1000 / MAX_FPS)
		return;

	_lastScreenUpdate = now;

	sceGuStart(0, displayList);

	sceGuClearColor(0xFF000000);
	sceGuClear(GU_COLOR_BUFFER_BIT);

	sceGuClutMode(GU_PSM_5551, 0, 0xFF, 0);
	sceGuClutLoad(32, clut256); // upload 32*8 entries (256)
	sceGuTexMode(GU_PSM_T8, 0, 0, 0); // 8-bit image
	if (_screenWidth == 320)
		sceGuTexImage(0, 512, 256, _screenWidth, _offscreen);
	else
		sceGuTexImage(0, 512, 512, _screenWidth, _offscreen);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGB);
	sceGuTexFilter(GU_LINEAR, GU_LINEAR);
	sceGuTexOffset(0,0);
	sceGuAmbientColor(0xFFFFFFFF);
	sceGuColor(0xFFFFFFFF);

	Vertex *vertices = (Vertex *)sceGuGetMemory(2 * sizeof(Vertex));
	vertices[0].u = 0.5f;
	vertices[0].v = 0.5f;
	vertices[1].u = _screenWidth - 0.5f;
	vertices[1].v = _screenHeight - 0.5f;

	switch (_graphicMode) {
		case CENTERED_320X200:
			vertices[0].x = (PSP_SCREEN_WIDTH - 320) / 2;
			vertices[0].y = (PSP_SCREEN_HEIGHT - 200) / 2;
			vertices[0].z = 0;
			vertices[1].x = PSP_SCREEN_WIDTH - (PSP_SCREEN_WIDTH - 320) / 2;
			vertices[1].y = PSP_SCREEN_HEIGHT - (PSP_SCREEN_HEIGHT - 200) / 2;
			vertices[1].z = 0;
		break;
		case CENTERED_435X272:
			vertices[0].x = (PSP_SCREEN_WIDTH - 435) / 2;
			vertices[0].y = 0; vertices[0].z = 0;
			vertices[1].x = PSP_SCREEN_WIDTH - (PSP_SCREEN_WIDTH - 435) / 2;
			vertices[1].y = PSP_SCREEN_HEIGHT;
			vertices[1].z = 0;
		break;
		case STRETCHED_480X272:
			vertices[0].x = 0;
			vertices[0].y = 0;
			vertices[0].z = 0;
			vertices[1].x = PSP_SCREEN_WIDTH;
			vertices[1].y = PSP_SCREEN_HEIGHT;
			vertices[1].z = 0;
		break;
		case CENTERED_362X272:
			vertices[0].x = (PSP_SCREEN_WIDTH - 362) / 2;
			vertices[0].y = 0;
			vertices[0].z = 0;
			vertices[1].x = PSP_SCREEN_WIDTH - (PSP_SCREEN_WIDTH - 362) / 2;
			vertices[1].y = PSP_SCREEN_HEIGHT;
			vertices[1].z = 0;
		break;
	}

	if (_shakePos) {
		vertices[0].y += _shakePos;
		vertices[1].y += _shakePos;
	}

	sceGuDrawArray(GU_SPRITES, GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 2, 0, vertices);
	if (_screenWidth == 640) {
		// 2nd draw
		Vertex *vertices2 = (Vertex *)sceGuGetMemory(2 * sizeof(Vertex));
		sceGuTexImage(0, 512, 512, _screenWidth, _offscreen+512);
		vertices2[0].u = 512 + 0.5f;
		vertices2[0].v = vertices[0].v;
		vertices2[1].u = vertices[1].u;
		vertices2[1].v = _screenHeight - 0.5f;
		vertices2[0].x = vertices[0].x + (vertices[1].x - vertices[0].x) * 511 / 640;
		vertices2[0].y = 0;
		vertices2[0].z = 0;
		vertices2[1].x = vertices[1].x;
		vertices2[1].y = vertices[1].y;
		vertices2[1].z = 0;
		sceGuDrawArray(GU_SPRITES, GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 2, 0, vertices2);
	}


	// draw overlay
	if (_overlayVisible) {
		Vertex *vertOverlay = (Vertex *)sceGuGetMemory(2 * sizeof(Vertex));
		vertOverlay[0].x = 0;
		vertOverlay[0].y = 0;
		vertOverlay[0].z = 0;
		vertOverlay[1].x = PSP_SCREEN_WIDTH;
		vertOverlay[1].y = PSP_SCREEN_HEIGHT;
		vertOverlay[1].z = 0;
		vertOverlay[0].u = 0.5f;
		vertOverlay[0].v = 0.5f;
		vertOverlay[1].u = _overlayWidth - 0.5f;
		vertOverlay[1].v = _overlayHeight - 0.5f;
		sceGuTexMode(GU_PSM_4444, 0, 0, 0); // 16-bit image
		sceGuDisable(GU_ALPHA_TEST);
		sceGuEnable(GU_BLEND);

		//sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
		sceGuBlendFunc(GU_ADD, GU_FIX, GU_ONE_MINUS_SRC_ALPHA, 0xFFFFFFFF, 0);

		if (_overlayWidth > 320)
			sceGuTexImage(0, 512, 512, _overlayWidth, _overlayBuffer);
		else
			sceGuTexImage(0, 512, 256, _overlayWidth, _overlayBuffer);

		sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,vertOverlay);
		// need to render twice for textures > 512
		if ( _overlayWidth > 512) {
			Vertex *vertOverlay2 = (Vertex *)sceGuGetMemory(2 * sizeof(Vertex));
			sceGuTexImage(0, 512, 512, _overlayWidth, _overlayBuffer + 512);
			vertOverlay2[0].u = 512 + 0.5f;
			vertOverlay2[0].v = vertOverlay[0].v;
			vertOverlay2[1].u = vertOverlay[1].u;
			vertOverlay2[1].v = _overlayHeight - 0.5f;
			vertOverlay2[0].x = PSP_SCREEN_WIDTH * 512 / 640;
			vertOverlay2[0].y = 0;
			vertOverlay2[0].z = 0;
			vertOverlay2[1].x = PSP_SCREEN_WIDTH;
			vertOverlay2[1].y = PSP_SCREEN_HEIGHT;
			vertOverlay2[1].z = 0;
			sceGuDrawArray(GU_SPRITES, GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 2, 0, vertOverlay2);
		}
		sceGuDisable(GU_BLEND);
	}

	// draw mouse
	if (_mouseVisible) {
		sceGuTexMode(GU_PSM_T8, 0, 0, 0); // 8-bit image
		sceGuClutMode(GU_PSM_5551, 0, 0xFF, 0);
		sceGuClutLoad(32, _cursorPaletteDisabled ? mouseClut : cursorPalette); // upload 32*8 entries (256)
		sceGuAlphaFunc(GU_GREATER, 0, 0xFF);
		sceGuEnable(GU_ALPHA_TEST);
		sceGuTexImage(0, MOUSE_SIZE, MOUSE_SIZE, MOUSE_SIZE, _mouseBuf);
		sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);

		Vertex *vertMouse = (Vertex *)sceGuGetMemory(2 * sizeof(Vertex));
		vertMouse[0].u = 0.5f;
		vertMouse[0].v = 0.5f;
		vertMouse[1].u = _mouseWidth - 0.5f;
		vertMouse[1].v = _mouseHeight - 0.5f;

		//adjust cursor position
		int mX = _mouseX - _mouseHotspotX;
		int mY = _mouseY - _mouseHotspotY;

		if (_overlayVisible) {
			float scalex, scaley;

			scalex = (float)PSP_SCREEN_WIDTH /_overlayWidth;
			scaley = (float)PSP_SCREEN_HEIGHT /_overlayHeight;

			vertMouse[0].x = mX * scalex;
			vertMouse[0].y = mY * scaley;
			vertMouse[0].z = 0;
			vertMouse[1].x = vertMouse[0].x + _mouseWidth * scalex;
			vertMouse[1].y = vertMouse[0].y + _mouseHeight * scaley;
			vertMouse[1].z = 0;
		} else
			switch (_graphicMode) {
			case CENTERED_320X200:
				vertMouse[0].x = (PSP_SCREEN_WIDTH - 320) / 2 + mX;
				vertMouse[0].y = (PSP_SCREEN_HEIGHT - 200) / 2 + mY;
				vertMouse[0].z = 0;
				vertMouse[1].x = vertMouse[0].x + _mouseWidth;
				vertMouse[1].y = vertMouse[0].y + _mouseHeight;
				vertMouse[1].z = 0;
			break;
			case CENTERED_435X272:
			{
				float scalex, scaley;

				scalex = 435.0f / _screenWidth;
				scaley = 272.0f / _screenHeight;

				vertMouse[0].x = (PSP_SCREEN_WIDTH - 435) / 2 + mX * scalex;
				vertMouse[0].y = mY * scaley;
				vertMouse[0].z = 0;
				vertMouse[1].x = vertMouse[0].x + _mouseWidth * scalex;
				vertMouse[1].y = vertMouse[0].y + _mouseHeight * scaley;
				vertMouse[1].z = 0;
			}
			break;
			case CENTERED_362X272:
			{
				float scalex, scaley;

				scalex = 362.0f / _screenWidth;
				scaley = 272.0f / _screenHeight;

				vertMouse[0].x = (PSP_SCREEN_WIDTH - 362) / 2 + mX * scalex;
				vertMouse[0].y = mY * scaley;
				vertMouse[0].z = 0;
				vertMouse[1].x = vertMouse[0].x + _mouseWidth * scalex;
				vertMouse[1].y = vertMouse[0].y + _mouseHeight * scaley;
				vertMouse[1].z = 0;
			}
			break;
			case STRETCHED_480X272:
			{
				float scalex, scaley;

				scalex = (float)PSP_SCREEN_WIDTH / _screenWidth;
				scaley = (float)PSP_SCREEN_HEIGHT / _screenHeight;

				vertMouse[0].x = mX * scalex;
				vertMouse[0].y = mY * scaley;
				vertMouse[0].z = 0;
				vertMouse[1].x = vertMouse[0].x + _mouseWidth * scalex;
				vertMouse[1].y = vertMouse[0].y + _mouseHeight * scaley;
				vertMouse[1].z = 0;
			}
			break;
		}
		sceGuDrawArray(GU_SPRITES, GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 2, 0, vertMouse);
	}

	if (_keyboard->isVisible()) {
		_keyboard->render();
	}

	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuSwapBuffers();
}

void OSystem_PSP::setShakePos(int shakeOffset) {
	_shakePos = shakeOffset;
}

void OSystem_PSP::showOverlay() {
	_overlayVisible = true;
}

void OSystem_PSP::hideOverlay() {
	PSPDebugTrace("OSystem_PSP::hideOverlay called\n");
	_overlayVisible = false;
}

void OSystem_PSP::clearOverlay() {
	PSPDebugTrace("clearOverlay\n");

	bzero(_overlayBuffer, _overlayWidth * _overlayHeight * sizeof(OverlayColor));
	sceKernelDcacheWritebackAll();
}

void OSystem_PSP::grabOverlay(OverlayColor *buf, int pitch) {
	int h = _overlayHeight;
	OverlayColor *src = _overlayBuffer;

	do {
		memcpy(buf, src, _overlayWidth * sizeof(OverlayColor));
		src += _overlayWidth;
		buf += pitch;
	} while (--h);
}

void OSystem_PSP::copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h) {
	PSPDebugTrace("copyRectToOverlay\n");

	//Clip the coordinates
	if (x < 0) {
		w += x;
		buf -= x;
		x = 0;
	}

	if (y < 0) {
		h += y;
		buf -= y * pitch;
		y = 0;
	}

	if (w > _overlayWidth - x) {
		w = _overlayWidth - x;
	}

	if (h > _overlayHeight - y) {
		h = _overlayHeight - y;
	}

	if (w <= 0 || h <= 0)
		return;


	OverlayColor *dst = _overlayBuffer + (y * _overlayWidth + x);

	if (_overlayWidth == pitch && pitch == w) {
		memcpy(dst, buf, h * w * sizeof(OverlayColor));
	} else {
		do {
			memcpy(dst, buf, w * sizeof(OverlayColor));
			buf += pitch;
			dst += _overlayWidth;
		} while (--h);
	}
	sceKernelDcacheWritebackAll();
}

int16 OSystem_PSP::getOverlayWidth() {
	return _overlayWidth;
}

int16 OSystem_PSP::getOverlayHeight() {
	return _overlayHeight;
}


void OSystem_PSP::grabPalette(byte *colors, uint start, uint num) {
	uint i;
	uint16 color;

	for (i = start; i < start + num; i++) {
		color = _palette[i];
		*colors++ = ((color & 0x1F) << 3);
		*colors++ = (((color >> 5) & 0x1F) << 3);
		*colors++ = (((color >> 10) & 0x1F) << 3);
		*colors++ = (color & 0x8000 ? 255 : 0);
	}
}

bool OSystem_PSP::showMouse(bool visible) {
	bool last = _mouseVisible;
	_mouseVisible = visible;
	return last;
}

void OSystem_PSP::warpMouse(int x, int y) {
	//assert(x > 0 && x < _screenWidth);
	//assert(y > 0 && y < _screenHeight);
	_mouseX = x;
	_mouseY = y;
}

void OSystem_PSP::setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, int cursorTargetScale, const Graphics::PixelFormat *format) {
	//TODO: handle cursorTargetScale
	_mouseWidth = w;
	_mouseHeight = h;

	_mouseHotspotX = hotspotX;
	_mouseHotspotY = hotspotY;

	_mouseKeyColour = keycolor & 0xFF;

	memcpy(mouseClut, _palette, 256 * sizeof(unsigned short));
	mouseClut[_mouseKeyColour] = 0;

	for (unsigned int i = 0; i < h; i++)
		memcpy(_mouseBuf + i * MOUSE_SIZE, buf + i * w, w);

	sceKernelDcacheWritebackAll();
}

#define PAD_CHECK_TIME	40
#define PAD_DIR_MASK	(PSP_CTRL_UP | PSP_CTRL_DOWN | PSP_CTRL_LEFT | PSP_CTRL_RIGHT)

bool OSystem_PSP::processInput(Common::Event &event) {
	s8 analogStepAmountX = 0;
	s8 analogStepAmountY = 0;

	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(1);
	sceCtrlReadBufferPositive(&pad, 1);

	bool usedInput, haveEvent;

	haveEvent = _keyboard->processInput(event, pad, usedInput);

	if (usedInput) 	// Check if the keyboard used up the input
		return haveEvent;

	uint32 buttonsChanged = pad.Buttons ^ _prevButtons;

	int newDpadX = 0, newDpadY = 0;
	event.kbd.ascii = 0;
	event.kbd.flags = 0;

	if (pad.Buttons & PSP_CTRL_UP) {
		newDpadY += 1;
		if (pad.Buttons & PSP_CTRL_RTRIGGER)
			newDpadX += 1;
	}
	if (pad.Buttons & PSP_CTRL_RIGHT) {
		newDpadX += 1;
		if (pad.Buttons & PSP_CTRL_RTRIGGER)
			newDpadY -= 1;
	}
	if (pad.Buttons & PSP_CTRL_DOWN) {
		newDpadY -= 1;
		if (pad.Buttons & PSP_CTRL_RTRIGGER)
			newDpadX -= 1;
	}
	if (pad.Buttons & PSP_CTRL_LEFT) {
		newDpadX -= 1;
		if (pad.Buttons & PSP_CTRL_RTRIGGER)
			newDpadY += 1;
	}
	//fprintf(stderr, "x=%d, y=%d, oldx=%d, oldy=%d\n", newDpadX, newDpadY, _dpadX, _dpadY);
	if (newDpadX != _dpadX || newDpadY != _dpadY) {
		if (_dpadX == 0 && _dpadY == 0)	{// We pressed dpad
			event.type = Common::EVENT_KEYDOWN;
			event.kbd.keycode = getDpadEvent(newDpadX, newDpadY);
			event.kbd.ascii = event.kbd.keycode - Common::KEYCODE_KP0 + '0';
			_dpadX = newDpadX;
			_dpadY = newDpadY;
		}
		else if (newDpadX == 0 && newDpadY == 0) {// We unpressed dpad
			event.type = Common::EVENT_KEYUP;
			event.kbd.keycode = getDpadEvent(_dpadX, _dpadY);
			event.kbd.ascii = event.kbd.keycode - Common::KEYCODE_KP0 + '0';
			_dpadX = newDpadX;
			_dpadY = newDpadY;
		} else { // we moved from one pressed dpad to another one
			event.type = Common::EVENT_KEYUP;	// first release the last dpad direction
			event.kbd.keycode = getDpadEvent(_dpadX, _dpadY);
			event.kbd.ascii = event.kbd.keycode - Common::KEYCODE_KP0 + '0';
			_dpadX = 0; // so that we'll pick up a new dpad movement
			_dpadY = 0;
		}

		_prevButtons = pad.Buttons;
		return true;

	} else if (buttonsChanged & (PSP_CTRL_CROSS | PSP_CTRL_CIRCLE | PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_START |
						PSP_CTRL_SELECT | PSP_CTRL_SQUARE | PSP_CTRL_TRIANGLE)) {
		if (buttonsChanged & PSP_CTRL_CROSS) {
			event.type = (pad.Buttons & PSP_CTRL_CROSS) ? Common::EVENT_LBUTTONDOWN : Common::EVENT_LBUTTONUP;
		} else if (buttonsChanged & PSP_CTRL_CIRCLE) {
			event.type = (pad.Buttons & PSP_CTRL_CIRCLE) ? Common::EVENT_RBUTTONDOWN : Common::EVENT_RBUTTONUP;
		} else {
			//any of the other buttons.
			event.type = buttonsChanged & pad.Buttons ? Common::EVENT_KEYDOWN : Common::EVENT_KEYUP;

			if (buttonsChanged & PSP_CTRL_LTRIGGER) {
				event.kbd.keycode = Common::KEYCODE_ESCAPE;
				event.kbd.ascii = 27;
			} else if (buttonsChanged & PSP_CTRL_START) {
				event.kbd.keycode = Common::KEYCODE_F5;
				event.kbd.ascii = Common::ASCII_F5;
				if (pad.Buttons & PSP_CTRL_RTRIGGER) {
					event.kbd.flags = Common::KBD_CTRL;	// Main menu to allow RTL
				}
/*			} else if (buttonsChanged & PSP_CTRL_SELECT) {
				event.kbd.keycode = Common::KEYCODE_0;
				event.kbd.ascii = '0';
*/			} else if (buttonsChanged & PSP_CTRL_SQUARE) {
				event.kbd.keycode = Common::KEYCODE_PERIOD;
				event.kbd.ascii = '.';
			} else if (buttonsChanged & PSP_CTRL_TRIANGLE) {
				event.kbd.keycode = Common::KEYCODE_RETURN;
				event.kbd.ascii = 13;
			} else if (pad.Buttons & PSP_CTRL_RTRIGGER) {
				event.kbd.flags |= Common::KBD_SHIFT;
			}

		}

		event.mouse.x = _mouseX;
		event.mouse.y = _mouseY;
		_prevButtons = pad.Buttons;
		return true;
	}

	uint32 time = getMillis();
	if (time - _lastPadCheck > PAD_CHECK_TIME) {
		_lastPadCheck = time;
		int16 newX = _mouseX;
		int16 newY = _mouseY;

		if (pad.Lx < 100) {
			analogStepAmountX = pad.Lx - 100;
		} else if (pad.Lx > 155) {
			analogStepAmountX = pad.Lx - 155;
		}

		if (pad.Ly < 100) {
			analogStepAmountY = pad.Ly - 100;
		} else if (pad.Ly > 155) {
			analogStepAmountY = pad.Ly - 155;
		}

		if (analogStepAmountX != 0 || analogStepAmountY != 0) {

			_prevButtons = pad.Buttons;

			// If no movement then this has no effect
			if (pad.Buttons & PSP_CTRL_RTRIGGER) {
				// Fine control mode for analog
					if (analogStepAmountX != 0) {
						if (analogStepAmountX > 0)
							newX += 1;
						else
							newX -= 1;
					}

					if (analogStepAmountY != 0) {
						if (analogStepAmountY > 0)
							newY += 1;
						else
							newY -= 1;
					}
			} else {
				newX += analogStepAmountX >> ((_screenWidth == 640) ? 2 : 3);
				newY += analogStepAmountY >> ((_screenWidth == 640) ? 2 : 3);
			}

			if (newX < 0)
				newX = 0;
			if (newY < 0)
				newY = 0;
			if (_overlayVisible) {
				if (newX >= _overlayWidth)
					newX = _overlayWidth - 1;
				if (newY >= _overlayHeight)
					newY = _overlayHeight - 1;
			} else {
				if (newX >= _screenWidth)
					newX = _screenWidth - 1;
				if (newY >= _screenHeight)
					newY = _screenHeight - 1;
			}

			if ((_mouseX != newX) || (_mouseY != newY)) {
				event.type = Common::EVENT_MOUSEMOVE;
				event.mouse.x = _mouseX = newX;
				event.mouse.y = _mouseY = newY;
				return true;
			}
		}
	}

	return false;
}

inline Common::KeyCode OSystem_PSP::getDpadEvent(int x, int y) {
	Common::KeyCode key;

	if (x == -1) {
		if (y == -1)
			key = Common::KEYCODE_KP1;
		else if (y == 0)
			key = Common::KEYCODE_KP4;
		else /* y == 1 */
			key = Common::KEYCODE_KP7;
	} else if (x == 0) {
		if (y == -1)
			key = Common::KEYCODE_KP2;
		else /* y == 1 */
			key = Common::KEYCODE_KP8;
	} else {/* x == 1 */
		if (y == -1)
			key = Common::KEYCODE_KP3;
		else if (y == 0)
			key = Common::KEYCODE_KP6;
		else /* y == 1 */
			key = Common::KEYCODE_KP9;
	}

	return key;
}

bool OSystem_PSP::pollEvent(Common::Event &event) {

	// If we're polling for events, we should check for pausing the engine
	// Pausing the engine is a necessary fix for games that use the timer for music synchronization
	// 	recovering many hours later causes the game to crash. We're polling without mutexes since it's not critical to
	//  get it right now.

	PowerMan.pollPauseEngine();

	return processInput(event);

}


uint32 OSystem_PSP::getMillis() {
	return SDL_GetTicks();
}

void OSystem_PSP::delayMillis(uint msecs) {
	SDL_Delay(msecs);
}

void OSystem_PSP::setTimerCallback(TimerProc callback, int interval) {
	SDL_SetTimer(interval, (SDL_TimerCallback)callback);
}

OSystem::MutexRef OSystem_PSP::createMutex(void) {
	return (MutexRef)SDL_CreateMutex();
}

void OSystem_PSP::lockMutex(MutexRef mutex) {
	SDL_mutexP((SDL_mutex *)mutex);
}

void OSystem_PSP::unlockMutex(MutexRef mutex) {
	SDL_mutexV((SDL_mutex *)mutex);
}

void OSystem_PSP::deleteMutex(MutexRef mutex) {
	SDL_DestroyMutex((SDL_mutex *)mutex);
}

void OSystem_PSP::mixCallback(void *sys, byte *samples, int len) {
	OSystem_PSP *this_ = (OSystem_PSP *)sys;
	assert(this_);

	if (this_->_mixer)
		this_->_mixer->mixCallback(samples, len);
}

void OSystem_PSP::setupMixer(void) {
	SDL_AudioSpec desired;
	SDL_AudioSpec obtained;
	uint32 samplesPerSec;

	memset(&desired, 0, sizeof(desired));

	if (ConfMan.hasKey("output_rate"))
		samplesPerSec = ConfMan.getInt("output_rate");
	else
		samplesPerSec = SAMPLES_PER_SEC;

	// Originally, we always used 2048 samples. This loop will produce the
	// same result at 22050 Hz, and should hopefully produce something
	// sensible for other frequencies. Note that it must be a power of two.

	uint32 samples = 0x8000;

	for (;;) {
		if (samples / (samplesPerSec / 1000) < 100)
			break;
		samples >>= 1;
	}

	desired.freq = samplesPerSec;
	desired.format = AUDIO_S16SYS;
	desired.channels = 2;
	desired.samples = samples;
	desired.callback = mixCallback;
	desired.userdata = this;

	assert(!_mixer);
	_mixer = new Audio::MixerImpl(this);
	assert(_mixer);

	if (SDL_OpenAudio(&desired, &obtained) != 0) {
		warning("Could not open audio: %s", SDL_GetError());
		samplesPerSec = 0;
		_mixer->setReady(false);
	} else {
		// Note: This should be the obtained output rate, but it seems that at
		// least on some platforms SDL will lie and claim it did get the rate
		// even if it didn't. Probably only happens for "weird" rates, though.
		samplesPerSec = obtained.freq;

		// Tell the mixer that we are ready and start the sound processing
		_mixer->setOutputRate(samplesPerSec);
		_mixer->setReady(true);

		SDL_PauseAudio(0);
	}
}

void OSystem_PSP::quit() {
	SDL_CloseAudio();
	SDL_Quit();
	sceGuTerm();
	sceKernelExitGame();
}

void OSystem_PSP::getTimeAndDate(TimeDate &td) const {
	time_t curTime = time(0);
	struct tm t = *localtime(&curTime);
	td.tm_sec = t.tm_sec;
	td.tm_min = t.tm_min;
	td.tm_hour = t.tm_hour;
	td.tm_mday = t.tm_mday;
	td.tm_mon = t.tm_mon;
	td.tm_year = t.tm_year;
}

#define PSP_CONFIG_FILE "ms0:/scummvm.ini"

Common::SeekableReadStream *OSystem_PSP::createConfigReadStream() {
	Common::FSNode file(PSP_CONFIG_FILE);
	return file.createReadStream();
}

Common::WriteStream *OSystem_PSP::createConfigWriteStream() {
	Common::FSNode file(PSP_CONFIG_FILE);
	return file.createWriteStream();
}
