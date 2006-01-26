/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
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
 * $Header$
 *
 */

#include "common/stdafx.h"
#include <kernel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <malloc.h>
#include <assert.h>
#include <iopcontrol.h>
#include <iopheap.h>
#include "common/scummsys.h"
#include "../intern.h"
#include "base/engine.h"
#include "backends/ps2/systemps2.h"
#include "backends/ps2/Gs2dScreen.h"
#include "backends/ps2/ps2input.h"
#include <sjpcm.h>
#include <libhdd.h>
#include "backends/ps2/savefile.h"
#include "common/file.h"
#include "backends/ps2/sysdefs.h"
#include <libmc.h>
#include <libpad.h>
#include "backends/ps2/cd.h"
#include <sio.h>
#include <fileXio_rpc.h>
#include "eecodyvdfs.h"
#include "graphics/surface.h"
#include "graphics/font.h"

extern void *_gp;

#define TIMER_STACK_SIZE (1024 * 32)
#define SOUND_STACK_SIZE (1024 * 32)
#define SMP_PER_BLOCK 800
#define BUS_CLOCK 147456000 // bus clock, a little less than 150 mhz
#define CLK_DIVIS 5760	// the timer IRQ handler gets called (BUS_CLOCK / 256) / CLK_DIVIS times per second (100 times)

#ifdef USE_PS2LINK
#define IRX_PREFIX "host:"
#define IRX_SUFFIX
#else
#define IRX_PREFIX "cdrom0:\\"
#define IRX_SUFFIX ";1"
#endif

static int g_TimerThreadSema = -1, g_SoundThreadSema = -1;
static int g_MainWaitSema = -1, g_TimerWaitSema = -1;
static volatile int32 g_MainWakeUp = 0, g_TimerWakeUp = 0;
volatile uint32 msecCount = 0;

OSystem_PS2 *g_systemPs2 = NULL;

int gBitFormat = 555;

#define FOREVER 2147483647

namespace Graphics {
	extern const NewFont g_sysfont;
};
#include "asyncfio.h"
extern AsyncFio fio;

void sioprintf(const char *zFormat, ...) {
	va_list ap;
	char resStr[2048];

	va_start(ap,zFormat);
	vsnprintf(resStr, 2048, zFormat, ap);
	va_end(ap);

	sio_puts(resStr);
}

OSystem *OSystem_PS2_create(void) {
	if (!g_systemPs2)
		g_systemPs2 = new OSystem_PS2();
	return g_systemPs2;
}

extern "C" int scummvm_main(int argc, char *argv[]);

extern "C" int main(int argc, char *argv[]) {
	SifInitRpc(0);
#ifndef USE_PS2LINK // reset the IOP if this is a CD build
	cdvdInit(CDVD_EXIT);
	cdvdExit();
	SifExitIopHeap();
	SifLoadFileExit();
	SifExitRpc();
	sio_puts("Resetting IOP.");
	SifIopReset("rom0:UDNL rom0:EELOADCNF", 0);
	while (!SifIopSync())
		;
	sio_puts("IOP synced.");
	SifInitRpc(0);
	SifLoadFileInit();
	cdvdInit(CDVD_INIT_WAIT);
#endif

	ee_thread_t thisThread;
	int tid = GetThreadId();
	ReferThreadStatus(tid, &thisThread);

	sioprintf("Thread Start Priority = %d\n", thisThread.current_priority);
	if ((thisThread.current_priority < 5) || (thisThread.current_priority > 80)) {
		/* Depending on the way ScummVM is run, we may get here with different
		   thread priorities.
		   The PS2 BIOS executes it with priority = 0, ps2link uses priority 64.
		   Don't know about NapLink, etc.
		   The priority doesn't matter too much, but we need to be at least at prio 3,
		   so we can have the timer thread run at prio 2 and the sound thread at prio 1	*/
		sioprintf("Changing thread priority");
		int res = ChangeThreadPriority(tid, 20);
		sioprintf("Result = %d", res);
	}

	sioprintf("Creating system");
	/* The OSystem has to be created before we enter ScummVM's main.
	   It sets up the memory card, etc. */
	OSystem_PS2_create();

	sioprintf("init done. starting ScummVM.");
	return scummvm_main(argc, argv);
}

s32 timerInterruptHandler(s32 cause) {
	T0_MODE = 0xDC2; // same value as in initialization.
	msecCount += 10;

	iSignalSema(g_SoundThreadSema);
	iSignalSema(g_TimerThreadSema);

	if (g_MainWakeUp) {
		g_MainWakeUp -= 10;
		if (g_MainWakeUp <= 0) {
			iSignalSema(g_MainWaitSema);
			g_MainWakeUp = 0;
		}
	}
	if (g_TimerWakeUp) {
		g_TimerWakeUp -= 10;
		if (g_TimerWakeUp <= 0) {
			iSignalSema(g_TimerWaitSema);
			g_TimerWakeUp = 0;
		}
	}
	return 0;
}

void systemTimerThread(OSystem_PS2 *system) {
	system->timerThread();
}

void systemSoundThread(OSystem_PS2 *system) {
	system->soundThread();
}

void gluePowerOffCallback(void *system) {
	((OSystem_PS2*)system)->powerOffCallback();
}

void mass_connect_cb(void *pkt, void *system) {
	((OSystem_PS2*)system)->setUsbMassConnected(true);
}

void mass_disconnect_cb(void *pkt, void *system) {
	((OSystem_PS2*)system)->setUsbMassConnected(false);
}

OSystem_PS2::OSystem_PS2(void) {
	_soundStack = _timerStack = NULL;
	_scummTimerProc = NULL;
	_scummSoundProc = NULL;
	_scummSoundParam = NULL;
	_printY = 0;
	_msgClearTime = 0;
	_systemQuit = false;

	_screen = new Gs2dScreen(320, 200, TV_DONT_CARE);

	sioprintf("Initializing system...");
	initTimer();

	_screen->wantAnim(true);

	sioprintf("Loading IOP modules...");
	loadModules();

	int res;
	if ((res = initCdvdFs()) < 0) {
		msgPrintf(FOREVER, "CoDyVDfs bind failed: %d", res);
		quit();
	}

	if ((res = SjPCM_Init(0)) < 0) {
		msgPrintf(FOREVER, "SjPCM Bind failed: %d\n", res);
		quit();
	}

	hddPreparePoweroff();
	hddSetUserPoweroffCallback(gluePowerOffCallback, this);

	fileXioSetBlockMode(FXIO_NOWAIT);

	_mouseVisible = false;

	sioprintf("reading RTC");
	readRtcTime();

	if (_useHdd) {
		if (fio.mount("pfs0:", "hdd0:+ScummVM", 0) >= 0)
			printf("Successfully mounted!\n");
		else
			_useHdd = false;
	}

	sioprintf("Starting SavefileManager");
	_saveManager = new Ps2SaveFileManager(this, _screen);

	sioprintf("Initializing ps2Input");
	_input = new Ps2Input(this, _useMouse, _useKbd);

	ee_sema_t newSema;
	newSema.init_count = 1;
	newSema.max_count = 1;
	_mutexSema = CreateSema(&newSema);
	for (int i = 0; i < MAX_MUTEXES; i++) {
		_mutex[i].sema = -1;
		_mutex[i].count = _mutex[i].owner = 0;
	}

	_screen->wantAnim(false);
	_screen->clearScreen();
}

OSystem_PS2::~OSystem_PS2(void) {
}

void OSystem_PS2::initTimer(void) {
	// first setup the two threads that get activated by the timer:
	// the timerthread and the soundthread
	ee_sema_t threadSema;
	threadSema.init_count = 0;
	threadSema.max_count = 255;
	g_TimerThreadSema = CreateSema(&threadSema);
	g_SoundThreadSema = CreateSema(&threadSema);
	assert((g_TimerThreadSema >= 0) && (g_SoundThreadSema >= 0));

	ee_thread_t timerThread, soundThread, thisThread;
	ReferThreadStatus(GetThreadId(), &thisThread);

	_timerStack = (uint8*)malloc(TIMER_STACK_SIZE);
	_soundStack = (uint8*)malloc(SOUND_STACK_SIZE);

	// give timer thread a higher priority than main thread
	timerThread.initial_priority = thisThread.current_priority - 1;
	timerThread.stack            = _timerStack;
	timerThread.stack_size       = TIMER_STACK_SIZE;
	timerThread.func             = (void *)systemTimerThread;
	timerThread.gp_reg			 = &_gp;

	// soundthread's priority is higher than main- and timerthread
	soundThread.initial_priority = thisThread.current_priority - 2;
	soundThread.stack            = _soundStack;
	soundThread.stack_size       = SOUND_STACK_SIZE;
	soundThread.func             = (void *)systemSoundThread;
	soundThread.gp_reg			 = &_gp;

	_timerTid = CreateThread(&timerThread);
	_soundTid = CreateThread(&soundThread);

	assert((_timerTid >= 0) && (_soundTid >= 0));

	StartThread(_timerTid, this);
	StartThread(_soundTid, this);

	// these semaphores are used for OSystem::delayMillis()
	threadSema.init_count = 0;
	threadSema.max_count = 1;
	g_MainWaitSema = CreateSema(&threadSema);
	g_TimerWaitSema = CreateSema(&threadSema);
	assert((g_MainWaitSema >= 0) && (g_TimerWaitSema >= 0));

	// threads done, start the interrupt handler
	_intrId = AddIntcHandler( INT_TIMER0, timerInterruptHandler, 0); // 0=first handler
	assert(_intrId >= 0);
	EnableIntc(INT_TIMER0);
	T0_HOLD = 0;
	T0_COUNT = 0;
	T0_COMP = CLK_DIVIS; // (BUS_CLOCK / 256) / CLK_DIVIS = 100
	T0_MODE = TIMER_MODE( 2, 0, 0, 0, 1, 1, 1, 0, 1, 1);

	DI();
	SifAddCmdHandler(0x12, mass_connect_cb, this);
	SifAddCmdHandler(0x13, mass_disconnect_cb, this);
	EI();
}

void OSystem_PS2::timerThread(void) {
	while (!_systemQuit) {
		WaitSema(g_TimerThreadSema);
		if (_scummTimerProc)
			_scummTimerProc(0);
	}
	ExitThread();
}

void OSystem_PS2::soundThread(void) {
	ee_sema_t soundSema;
	soundSema.init_count = 1;
	soundSema.max_count = 1;
	_soundSema = CreateSema(&soundSema);
	assert(_soundSema >= 0);
	int16 *soundBufL = (int16*)memalign(64, SMP_PER_BLOCK * sizeof(int16) * 2);
	int16 *soundBufR = soundBufL + SMP_PER_BLOCK;

	int bufferedSamples = 0;
	int cycles = 0;

	while (!_systemQuit) {
		WaitSema(g_SoundThreadSema);

		if (!(cycles & 31))
			bufferedSamples = SjPCM_Buffered();
		else
			bufferedSamples -= 480;
		cycles++;

		WaitSema(_soundSema);
		if (_scummSoundProc) {
			if (bufferedSamples <= 4 * SMP_PER_BLOCK) {
				// we have to produce more samples, call sound mixer
				// the scratchpad at 0x70000000 is used as temporary soundbuffer
				_scummSoundProc(_scummSoundParam, (uint8*)0x70000000, SMP_PER_BLOCK * 2 * sizeof(int16));

				// demux data into 2 buffers, L and R
				 __asm__ (
					"move  $t2, %1\n\t"			// dest buffer right
					"move  $t3, %0\n\t"			// dest buffer left
					"lui   $t8, 0x7000\n\t"		// muxed buffer, fixed at 0x70000000
					"addiu $t9, $0, 100\n\t"	// number of loops
					"mtsab $0, 2\n\t"			// set qword shift = 2 byte

					"loop:\n\t"
					"  lq $t4,  0($t8)\n\t"		// load 8 muxed samples
					"  lq $t5, 16($t8)\n\t"		// load 8 more muxed samples

					"  qfsrv $t6, $0, $t4\n\t"	// shift right for second
					"  qfsrv $t7, $0, $t5\n\t"	// packing step (right channel)

					"  ppach $t4, $t5, $t4\n\t"	// combine left channel data
					"  ppach $t6, $t7, $t6\n\t"	// right channel data

					"  sq $t4, 0($t3)\n\t"		// write back
					"  sq $t6, 0($t2)\n\t"		//

					"  addiu $t9, -1\n\t"		// decrement loop counter
					"  addiu $t2, 16\n\t"		// increment pointers
					"  addiu $t3, 16\n\t"
					"  addiu $t8, 32\n\t"
					"  bnez  $t9, loop\n\t"		// loop
						:  // outputs
				 		: "r"(soundBufL), "r"(soundBufR)  // inputs
					//  : "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9"  // destroyed
						: "$10", "$11", "$12", "$13", "$14", "$15", "$24", "$25"  // destroyed
				);
				// and feed it into the SPU
				// non-blocking call, the function will return before the buffer's content
				// was transferred.
				SjPCM_Enqueue((short int*)soundBufL, (short int*)soundBufR, SMP_PER_BLOCK, 0);
				bufferedSamples += SMP_PER_BLOCK;
			}
		} 
		SignalSema(_soundSema);
	}
	free(soundBufL);
	DeleteSema(_soundSema);
	ExitThread();
}

const char *irxModules[] = {
	"rom0:SIO2MAN",
	"rom0:MCMAN",
	"rom0:MCSERV",
	"rom0:PADMAN",
	"rom0:LIBSD",
#ifndef USE_PS2LINK
	IRX_PREFIX "IOMANX.IRX" IRX_SUFFIX,
#endif
	IRX_PREFIX "FILEXIO.IRX" IRX_SUFFIX,
	IRX_PREFIX "CODYVDFS.IRX" IRX_SUFFIX,
	IRX_PREFIX "SJPCM.IRX" IRX_SUFFIX,
};

void OSystem_PS2::loadModules(void) {

	_usbMassConnected = _usbMassLoaded = _useMouse = _useKbd = _useHdd = false;
	static char hddarg[] = "-o" "\0" "4" "\0" "-n" "\0" "20";
	static char pfsarg[] = "-m" "\0" "2" "\0" "-o" "\0" "16" "\0" "-n" "\0" "40" /*"\0" "-debug"*/;

	int res;
	for (int i = 0; i < ARRAYSIZE(irxModules); i++) {
		if ((res = SifLoadModule(irxModules[i], 0, NULL)) < 0) {
			msgPrintf(FOREVER, "Unable to load module %s, Error %d", irxModules[i], res);
			_screen->wantAnim(false);
			updateScreen();
			SleepThread();
		}
	}

	// now try to load optional IRXs
	if ((res = SifLoadModule(IRX_PREFIX "USBD.IRX" IRX_SUFFIX, 0, NULL)) >= 0) {
		if ((res = SifLoadModule(IRX_PREFIX "PS2MOUSE.IRX" IRX_SUFFIX, 0, NULL)) < 0)
			sioprintf("Cannot load module: PS2MOUSE.IRX (%d)", res);
		else
			_useMouse = true;
		if ((res = SifLoadModule(IRX_PREFIX "RPCKBD.IRX" IRX_SUFFIX, 0, NULL)) < 0)
			sioprintf("Cannot load module: RPCKBD.IRX (%d)", res);
		else
			_useKbd = true;
		if ((res = SifLoadModule(IRX_PREFIX "USB_MASS.IRX" IRX_SUFFIX, 0, NULL)) < 0)
			sioprintf("Cannot load module: USB_MASS.IRX (%d)", res);
		else
			_usbMassLoaded = true;
	}

	if ((res = fileXioInit()) < 0) {
		msgPrintf(FOREVER, "FXIO Init failed: %d", res);
		quit();
	}
	
	if ((res = SifLoadModule(IRX_PREFIX "PS2DEV9.IRX" IRX_SUFFIX, 0, NULL)) < 0)
		sioprintf("Cannot load module: PS2DEV9.IRX (%d)", res);
	else if ((res = SifLoadModule(IRX_PREFIX "PS2ATAD.IRX" IRX_SUFFIX, 0, NULL)) < 0)
		sioprintf("Cannot load module: PS2ATAD.IRX (%d)", res);
	else if ((res = SifLoadModule(IRX_PREFIX "PS2HDD.IRX" IRX_SUFFIX, sizeof(hddarg), hddarg)) < 0)
		sioprintf("Cannot load module: PS2HDD.IRX (%d)", res);
	else if ((res = SifLoadModule(IRX_PREFIX "PS2FS.IRX" IRX_SUFFIX, sizeof(pfsarg), pfsarg)) < 0)
		sioprintf("Cannot load module: PS2FS.IRX (%d)", res);
	else {
		if ((res = SifLoadModule(IRX_PREFIX "POWEROFF.IRX" IRX_SUFFIX, 0, NULL)) < 0)
			sioprintf("Cannot load module: POWEROFF.IRX (%d)", res);
		if ((hddCheckPresent() >= 0) && (hddCheckFormatted() >= 0))
			_useHdd = true;
	}

	sioprintf("Modules: UsbMouse %sloaded, UsbKbd %sloaded, UsbMass %sloaded, HDD %sloaded.", _useMouse ? "" : "not ", _useKbd ? "" : "not ", _usbMassLoaded ? "" : "not ", _useHdd ? "" : "not ");
}

bool OSystem_PS2::hddPresent(void) {
	return _useHdd;
}

void OSystem_PS2::setUsbMassConnected(bool stat) {
	_usbMassConnected = stat;
}

bool OSystem_PS2::usbMassPresent(void) {
	return _usbMassConnected;
}

void OSystem_PS2::initSize(uint width, uint height, int overscale) {
	printf("initializing new size: (%d/%d)...", width, height);
	_screen->newScreenSize(width, height);
	_screen->setMouseXy(width / 2, height / 2);
	_input->newRange(0, 0, width - 1, height - 1);
	_input->warpTo(width / 2, height / 2);

	_oldMouseX = width / 2;
	_oldMouseY = height / 2;
	printf("done\n");
}

void OSystem_PS2::setPalette(const byte *colors, uint start, uint num) {
	_screen->setPalette((const uint32*)colors, (uint8)start, (uint16)num);
}

void OSystem_PS2::grabPalette(byte *colors, uint start, uint num) {
	_screen->grabPalette((uint32*)colors, (uint8)start, (uint16)num);
}

void OSystem_PS2::copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h) {
	_screen->copyScreenRect((const uint8*)buf, pitch, x, y, w, h);
}

bool OSystem_PS2::grabRawScreen(Graphics::Surface *surf) {
	_screen->grabScreen(surf);
	return true;
}

void OSystem_PS2::updateScreen(void) {
	if (_msgClearTime && (_msgClearTime < getMillis())) {
		_screen->clearPrintfOverlay();
		_msgClearTime = 0;
	}
	_screen->updateScreen();
}

uint32 OSystem_PS2::getMillis(void) {
	return msecCount;
}

void OSystem_PS2::delayMillis(uint msecs) {
	if (msecs == 0)
		return;

	int tid = GetThreadId();
	if (tid == _soundTid) {
		sioprintf("ERROR: delayMillis() from sound thread!");
		return;
	}

	if (tid == _timerTid) {
		g_TimerWakeUp = (int32)msecs;
		WaitSema(g_TimerWaitSema);
	} else {
		g_MainWakeUp = (int32)msecs;
		WaitSema(g_MainWaitSema);
	}
}

void OSystem_PS2::setTimerCallback(OSystem::TimerProc callback, int interval) {
	if (callback && (interval != 10))
		sioprintf("unhandled timer interval: %d\n", interval);
	_scummTimerProc = callback;
}

int OSystem_PS2::getOutputSampleRate(void) const {
	return 48000;
}

bool OSystem_PS2::setSoundCallback(SoundProc proc, void *param) {
	assert(proc != NULL);

	WaitSema(_soundSema);
    _scummSoundProc = proc;
	_scummSoundParam = param;
	SignalSema(_soundSema);
	return true;
}

void OSystem_PS2::clearSoundCallback(void) {
	WaitSema(_soundSema);
	_scummSoundProc = NULL;
	_scummSoundParam = NULL;
	SignalSema(_soundSema);
}

Common::SaveFileManager *OSystem_PS2::getSavefileManager(void) {
	return _saveManager;
}

void OSystem_PS2::setShakePos(int shakeOffset) {
	_screen->setShakePos(shakeOffset);
}

bool OSystem_PS2::showMouse(bool visible) {
	bool retVal = _mouseVisible;
	_screen->showMouse(visible);
	_mouseVisible = visible;
	return retVal;
}

void OSystem_PS2::warpMouse(int x, int y) {
	_input->warpTo((uint16)x, (uint16)y);
	_screen->setMouseXy(x, y);
}

void OSystem_PS2::setMouseCursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y, byte keycolor, int cursorTargetScale) {
	_screen->setMouseOverlay(buf, w, h, hotspot_x, hotspot_y, keycolor);
}

bool OSystem_PS2::openCD(int drive) {
	return false;
}

bool OSystem_PS2::pollCD(void) {
	return false;
}

void OSystem_PS2::playCD(int track, int num_loops, int start_frame, int duration) {
}

void OSystem_PS2::stopCD(void) {
}

void OSystem_PS2::updateCD(void) {
}

void OSystem_PS2::showOverlay(void) {
	_screen->showOverlay();
}

void OSystem_PS2::hideOverlay(void) {
	_screen->hideOverlay();
}

void OSystem_PS2::clearOverlay(void) {
	_screen->clearOverlay();
}

void OSystem_PS2::grabOverlay(OverlayColor *buf, int pitch) {
	_screen->grabOverlay((uint16*)buf, (uint16)pitch);
}

void OSystem_PS2::copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h) {
	_screen->copyOverlayRect((uint16*)buf, (uint16)pitch, (uint16)x, (uint16)y, (uint16)w, (uint16)h);
}

const OSystem::GraphicsMode OSystem_PS2::_graphicsMode = { NULL, NULL, 0 };

const OSystem::GraphicsMode *OSystem_PS2::getSupportedGraphicsModes(void) const {
    return &_graphicsMode;
}

bool OSystem_PS2::setGraphicsMode(int mode) {
	return (mode == 0);
}

int OSystem_PS2::getGraphicsMode(void) const {
	return 0;
}

int OSystem_PS2::getDefaultGraphicsMode(void) const {
	return 0;
}

bool OSystem_PS2::pollEvent(Event &event) {
	bool res = _input->pollEvent(&event);
	if (res && (event.type == EVENT_MOUSEMOVE))
		_screen->setMouseXy(event.mouse.x, event.mouse.y);
	return res;
}

OverlayColor OSystem_PS2::RGBToColor(uint8 r, uint8 g, uint8 b) {
	return (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10);
}

void OSystem_PS2::colorToRGB(OverlayColor color, uint8 &r, uint8 &g, uint8 &b) {
	r = (color & 0x1F) << 3;
	g = ((color >> 5) & 0x1F) << 3;
	b = ((color >> 10) & 0x1F) << 3;
}

int16 OSystem_PS2::getHeight(void) {
	return _screen->getHeight();
}

int16 OSystem_PS2::getWidth(void) {
	return _screen->getWidth();
}

void OSystem_PS2::msgPrintf(int millis, char *format, ...) {
	va_list ap;
	char resStr[1024];
	memset(resStr, 0, 1024);

	va_start(ap, format);
	vsnprintf(resStr, 1023, format, ap);
	va_end(ap);

	uint16 posY = 2;
	int maxWidth = 0;

	Graphics::Surface surf;
	surf.create(300, 200, 1);

	char *lnSta = resStr;
	while (*lnSta && (posY < 180)) {
		char *lnEnd = lnSta;
		while ((*lnEnd) && (*lnEnd != '\n'))
			lnEnd++;
		*lnEnd = '\0';
		
		Common::String str(lnSta);
		int width = Graphics::g_sysfont.getStringWidth(str);
		if (width > maxWidth)
			maxWidth = width;
		int posX = (300 - width) / 2;
		Graphics::g_sysfont.drawString(&surf, str, posX, posY, 300 - posX, 1);
		posY += 14;

        lnSta = lnEnd + 1;
	}

	uint8 *scrBuf = (uint8*)memalign(64, 320 * 200);
	memset(scrBuf, 4, 320 * 200);

	uint8 *dstPos = scrBuf + ((200 - posY) >> 1) * 320 + (320 - maxWidth) / 2;
	for (int y = 0; y < posY; y++) {
		uint8 *srcPos = (uint8*)surf.getBasePtr((300 - maxWidth) / 2, y);
		for (int x = 0; x < maxWidth; x++)
			dstPos[x] = srcPos[x] + 5;
		dstPos += 320;
	}
	surf.free();
	_screen->copyPrintfOverlay(scrBuf);
	free(scrBuf);
	_msgClearTime = millis + getMillis();
}

void OSystem_PS2::powerOffCallback(void) {
	sioprintf("powerOffCallback");
	_saveManager->quit();
	if (_useHdd) {
		sioprintf("umount");
		fio.umount("pfs0:");
	}
	sioprintf("fxio");
	// enable blocking FXIO so libhdd will correctly close drive, etc.
	fileXioSetBlockMode(FXIO_WAIT);
	sioprintf("done");
}

void OSystem_PS2::quit(void) {
#ifdef USE_PS2LINK
	printf("OSystem_PS2::quit\n");
	SleepThread();
#endif
	sioprintf("OSystem_PS2::quit");
	if (_useHdd) {
		driveStandby();
		fio.umount("pfs0:");
	}
	clearSoundCallback();
	setTimerCallback(NULL, 0);
	_screen->wantAnim(false);
	_systemQuit = true;
	ee_thread_t statSound, statTimer;
	do {	// wait until both threads called ExitThread()
		ReferThreadStatus(_timerTid, &statTimer);
		ReferThreadStatus(_soundTid, &statSound);
	} while ((statSound.status != 0x10) || (statTimer.status != 0x10));
	DeleteThread(_timerTid);
	DeleteThread(_soundTid);
	free(_timerStack);
	free(_soundStack);
	DisableIntc(INT_TIMER0);
	RemoveIntcHandler(INT_TIMER0, _intrId);

	_saveManager->quit();
	_screen->quit();

	padEnd(); // stop pad library
	cdvdInit(CDVD_EXIT);
	sioprintf("resetting iop");
	SifIopReset(NULL, 0);
	SifExitRpc();
	while (!SifIopSync());
    SifInitRpc(0);
	cdvdExit();
	SifExitRpc();
	FlushCache(0);
	SifLoadFileExit();
	sioprintf("Restarting ScummVM");
	LoadExecPS2("cdrom0:\\SCUMMVM.ELF", 0, NULL); // resets the console and executes the ELF
}

void OSystem_PS2::makeConfigPath(char *dest) {
	FILE *handle;
	strcpy(dest, "cdfs:/ScummVM.ini");
	handle = ps2_fopen(dest, "r");
	if (_usbMassConnected && !handle) {
		strcpy(dest, "mass:/ScummVM.ini");
		handle = ps2_fopen(dest, "r");
	}
	if (handle)
		ps2_fclose(handle);
	else
		strcpy(dest, "mc0:ScummVM/scummvm.ini");
}


