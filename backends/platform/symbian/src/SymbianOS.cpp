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

#include <sdlapp.h> // for CSDLApp::GetExecutablePathCStr() @ Symbian::GetExecutablePath()
#include <bautils.h>
#include <eikenv.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_fclose
#define FORBIDDEN_SYMBOL_EXCEPTION_fopen

#include "backends/fs/symbian/symbian-fs-factory.h"
#include "backends/platform/symbian/src/SymbianOS.h"
#include "backends/platform/symbian/src/SymbianActions.h"
#include "backends/saves/default/default-saves.h"

#include "base/main.h"

#include "common/config-manager.h"
#include "common/scummsys.h"
#include "common/translation.h"

#include "gui/message.h"

#include "sound/mixer_intern.h"

#ifdef SAMPLES_PER_SEC_8000 // the GreanSymbianMMP format cannot handle values for defines :(
  #define SAMPLES_PER_SEC 8000
#else
  #define SAMPLES_PER_SEC 16000
#endif

#define DEFAULT_CONFIG_FILE "scummvm.ini"
#define DEFAULT_SAVE_PATH "Savegames"

////////// extern "C" ///////////////////////////////////////////////////
namespace Symbian {

// make this easily available everywhere
char* GetExecutablePath() {
	return CSDLApp::GetExecutablePathCStr();
}

} // namespace Symbian {

////////// OSystem_SDL_Symbian //////////////////////////////////////////

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
	{"1x", "Fullscreen", GFX_NORMAL},
	{0, 0, 0}
};

bool OSystem_SDL_Symbian::hasFeature(Feature f) {
	switch (f) {
	case kFeatureFullscreenMode:
	case kFeatureAspectRatioCorrection:
	case kFeatureCursorHasPalette:
#ifdef  USE_VIBRA_SE_PXXX
	case kFeatureVibration:
#endif
		return true;

	default:
		return false;
	}
}

void OSystem_SDL_Symbian::setFeatureState(Feature f, bool enable) {
	switch (f) {
	case kFeatureVirtualKeyboard:
		if (enable) {
		}
		else {

		}
		break;
	case kFeatureDisableKeyFiltering:
		GUI::Actions::Instance()->beginMapping(enable);
		break;
	default:
		OSystem_SDL::setFeatureState(f, enable);
	}
}

static Common::String getDefaultConfigFileName() {
	char configFile[MAXPATHLEN];
	strcpy(configFile, Symbian::GetExecutablePath());
	strcat(configFile, DEFAULT_CONFIG_FILE);
	return configFile;
}

Common::SeekableReadStream *OSystem_SDL_Symbian::createConfigReadStream() {
	Common::FSNode file(getDefaultConfigFileName());
	return file.createReadStream();
}

Common::WriteStream *OSystem_SDL_Symbian::createConfigWriteStream() {
	Common::FSNode file(getDefaultConfigFileName());
	return file.createWriteStream();
}

OSystem_SDL_Symbian::zoneDesc OSystem_SDL_Symbian::_zones[TOTAL_ZONES] = {
        { 0, 0, 320, 145 },
        { 0, 145, 150, 55 },
        { 150, 145, 170, 55 }
};
OSystem_SDL_Symbian::OSystem_SDL_Symbian() :_channels(0),_stereo_mix_buffer(0) {
	_RFs = &CEikonEnv::Static()->FsSession();
	_fsFactory = new SymbianFilesystemFactory();
}

void OSystem_SDL_Symbian::initBackend() {
	// Calculate the default savepath
	Common::String savePath;
	savePath = Symbian::GetExecutablePath();
	savePath += DEFAULT_SAVE_PATH "\\";
	_savefile = new DefaultSaveFileManager(savePath);

	// If savepath has not already been set then set it
	if (!ConfMan.hasKey("savepath")) {
		ConfMan.set("savepath", savePath);

	}

	// Ensure that the current set path (might have been altered by the user) exists
	Common::String currentPath = ConfMan.get("savepath");
	TFileName fname;
	TPtrC8 ptr((const unsigned char*)currentPath.c_str(),currentPath.size());
	fname.Copy(ptr);
	BaflUtils::EnsurePathExistsL(static_cast<OSystem_SDL_Symbian*>(g_system)->FsSession(), fname);

	ConfMan.setBool("FM_high_quality", false);
#if !defined(S60) || defined(S60V3) // S60 has low quality as default
	ConfMan.setBool("FM_medium_quality", true);
#else
	ConfMan.setBool("FM_medium_quality", false);
#endif
	ConfMan.setInt("joystick_num", 0); // Symbian OS  should have joystick_num set to 0 in the ini file , but uiq devices might refuse opening the joystick
	ConfMan.flushToDisk();

	GUI::Actions::init();

	OSystem_SDL::initBackend();

	// Initialize global key mapping for Smartphones
	GUI::Actions* actions = GUI::Actions::Instance();

	actions->initInstanceMain(this);
	actions->loadMapping();
	initZones();
}

void OSystem_SDL_Symbian::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {
	Common::FSNode pluginsNode(Symbian::GetExecutablePath());
	if (pluginsNode.exists() && pluginsNode.isDirectory()) {
			s.add("SYMBIAN_DATAFOLDER", new Common::FSDirectory(Symbian::GetExecutablePath()), priority);
		}
}

OSystem_SDL_Symbian::~OSystem_SDL_Symbian() {
	delete[] _stereo_mix_buffer;
}

int OSystem_SDL_Symbian::getDefaultGraphicsMode() const {
	return GFX_NORMAL;
}

const OSystem::GraphicsMode *OSystem_SDL_Symbian::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}

// make sure we always go to normal, even if the string might be set wrong!
bool OSystem_SDL_Symbian::setGraphicsMode(const char * /*name*/) {
	// let parent OSystem_SDL handle it
	return OSystem_SDL::setGraphicsMode(getDefaultGraphicsMode());
}

void OSystem_SDL_Symbian::quitWithErrorMsg(const char * /*aMsg*/) {

	CEikonEnv::Static()->AlertWin(_L("quitWithErrorMsg()")) ;

	if (g_system)
		g_system->quit();
}

// Overloaded from SDL_Commmon
void OSystem_SDL_Symbian::quit() {
	delete GUI_Actions::Instance();
	OSystem_SDL::quit();
}

void OSystem_SDL_Symbian::setupMixer() {

	SDL_AudioSpec desired;
	SDL_AudioSpec obtained;

	// Determine the desired output sampling frequency.
	uint32 samplesPerSec = 0;
	if (ConfMan.hasKey("output_rate"))
		samplesPerSec = ConfMan.getInt("output_rate");
	if (samplesPerSec <= 0)
		samplesPerSec = SAMPLES_PER_SEC;

	// Determine the sample buffer size. We want it to store enough data for
	// at least 1/16th of a second (though at most 8192 samples). Note
	// that it must be a power of two. So e.g. at 22050 Hz, we request a
	// sample buffer size of 2048.
	uint32 samples = 8192;
	while (samples * 16 > samplesPerSec * 2)
		samples >>= 1;

	memset(&desired, 0, sizeof(desired));
	desired.freq = samplesPerSec;
	desired.format = AUDIO_S16SYS;
	desired.channels = 2;
	desired.samples = (uint16)samples;
	desired.callback = symbianMixCallback;
	desired.userdata = this;

	assert(!_mixer);
	if (SDL_OpenAudio(&desired, &obtained) != 0) {
		warning("Could not open audio device: %s", SDL_GetError());
		_mixer = new Audio::MixerImpl(this, samplesPerSec);
		assert(_mixer);
		_mixer->setReady(false);
	} else {
		// Note: This should be the obtained output rate, but it seems that at
		// least on some platforms SDL will lie and claim it did get the rate
		// even if it didn't. Probably only happens for "weird" rates, though.
		samplesPerSec = obtained.freq;
		_channels = obtained.channels;

		// Need to create mixbuffer for stereo mix to downmix
		if (_channels != 2) {
			_stereo_mix_buffer = new byte [obtained.size*2];//*2 for stereo values
		}

		// Create the mixer instance and start the sound processing
		_mixer = new Audio::MixerImpl(this, samplesPerSec);
		assert(_mixer);
		_mixer->setReady(true);
		SDL_PauseAudio(0);
	}
}

/**
 * The mixer callback function.
 */
void OSystem_SDL_Symbian::symbianMixCallback(void *sys, byte *samples, int len) {
	OSystem_SDL_Symbian *this_ = (OSystem_SDL_Symbian *)sys;
	assert(this_);

	if (!this_->_mixer)
		return;

#if defined (S60) && !defined(S60V3)
	// If not stereo then we need to downmix
	if (this_->_mixer->_channels != 2) {
		this_->_mixer->mixCallback(_stereo_mix_buffer, len * 2);

		int16 *bitmixDst = (int16 *)samples;
		int16 *bitmixSrc = (int16 *)_stereo_mix_buffer;

		for (int loop = len / 2; loop >= 0; loop --) {
			*bitmixDst = (*bitmixSrc + *(bitmixSrc + 1)) >> 1;
			bitmixDst++;
			bitmixSrc += 2;
		}
	} else
#else
	this_->_mixer->mixCallback(samples, len);
#endif
}


/**
 * This is an implementation by the remapKey function
 * @param SDL_Event to remap
 * @param ScumVM event to modify if special result is requested
 * @return true if Common::Event has a valid return status
 */
bool OSystem_SDL_Symbian::remapKey(SDL_Event &ev, Common::Event &event) {
	if (GUI::Actions::Instance()->mappingActive() || ev.key.keysym.sym <= SDLK_UNKNOWN)
		return false;

	for (TInt loop = 0; loop < GUI::ACTION_LAST; loop++) {
		if (GUI::Actions::Instance()->getMapping(loop) == ev.key.keysym.sym &&
			GUI::Actions::Instance()->isEnabled(loop)) {
			// Create proper event instead
			switch (loop) {
			case GUI::ACTION_UP:
				if (ev.type == SDL_KEYDOWN) {
					_km.y_vel = -1;
					_km.y_down_count = 1;
				} else {
					_km.y_vel = 0;
					_km.y_down_count = 0;
				}
				event.type = Common::EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);

				return true;

			case GUI::ACTION_DOWN:
				if (ev.type == SDL_KEYDOWN) {
					_km.y_vel = 1;
					_km.y_down_count = 1;
				} else {
					_km.y_vel = 0;
					_km.y_down_count = 0;
				}
				event.type = Common::EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);

				return true;

			case GUI::ACTION_LEFT:
				if (ev.type == SDL_KEYDOWN) {
					_km.x_vel = -1;
					_km.x_down_count = 1;
				} else {
					_km.x_vel = 0;
					_km.x_down_count = 0;
				}
				event.type = Common::EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);

				return true;

			case GUI::ACTION_RIGHT:
				if (ev.type == SDL_KEYDOWN) {
					_km.x_vel = 1;
					_km.x_down_count = 1;
				} else {
					_km.x_vel = 0;
					_km.x_down_count = 0;
				}
				event.type = Common::EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);

				return true;

			case GUI::ACTION_LEFTCLICK:
				event.type = (ev.type == SDL_KEYDOWN ? Common::EVENT_LBUTTONDOWN : Common::EVENT_LBUTTONUP);
				fillMouseEvent(event, _km.x, _km.y);

				return true;

			case GUI::ACTION_RIGHTCLICK:
				event.type = (ev.type == SDL_KEYDOWN ? Common::EVENT_RBUTTONDOWN : Common::EVENT_RBUTTONUP);
				fillMouseEvent(event, _km.x, _km.y);

				return true;

			case GUI::ACTION_ZONE:
				if (ev.type == SDL_KEYDOWN) {
					int i;

					for (i=0; i < TOTAL_ZONES; i++)
						if (_km.x >= _zones[i].x && _km.y >= _zones[i].y &&
							_km.x <= _zones[i].x + _zones[i].width && _km.y <= _zones[i].y + _zones[i].height
							) {
							_mouseXZone[i] = _km.x;
							_mouseYZone[i] = _km.y;
							break;
						}
						_currentZone++;
						if (_currentZone >= TOTAL_ZONES)
							_currentZone = 0;
						event.type = Common::EVENT_MOUSEMOVE;
						fillMouseEvent(event, _mouseXZone[_currentZone], _mouseYZone[_currentZone]);
						SDL_WarpMouse(event.mouse.x, event.mouse.y);
				}

				return true;
			case GUI::ACTION_MULTI: {
				GUI::Key &key = GUI::Actions::Instance()->getKeyAction(loop);
				// if key code is pause, then change event to interactive or just fall through
				if (key.keycode() == SDLK_PAUSE) {
					event.type = Common::EVENT_PREDICTIVE_DIALOG;
					return true;
					}
				}
			case GUI::ACTION_SAVE:
			case GUI::ACTION_SKIP:
			case GUI::ACTION_SKIP_TEXT:
			case GUI::ACTION_PAUSE:
			case GUI::ACTION_SWAPCHAR:
			case GUI::ACTION_FASTMODE:
			case GUI::ACTION_DEBUGGER:
			case GUI::ACTION_MAINMENU:
			case GUI::ACTION_VKB:
			case GUI::ACTION_KEYMAPPER:{
					GUI::Key &key = GUI::Actions::Instance()->getKeyAction(loop);
					ev.key.keysym.sym = (SDLKey) key.keycode();
					ev.key.keysym.scancode = 0;
					ev.key.keysym.mod = (SDLMod) key.flags();

					// Translate from SDL keymod event to Scummvm Key Mod Common::Event.
					// This codes is also present in GP32 backend and in SDL backend as a static function
					// Perhaps it should be shared.
					if (key.flags() != 0) {
						event.kbd.flags = 0;

						if (ev.key.keysym.mod & KMOD_SHIFT)
							event.kbd.flags |= Common::KBD_SHIFT;

						if (ev.key.keysym.mod & KMOD_ALT)
							event.kbd.flags |= Common::KBD_ALT;

						if (ev.key.keysym.mod & KMOD_CTRL)
							event.kbd.flags |= Common::KBD_CTRL;
					}

					return false;
				}

			case GUI::ACTION_QUIT:
				{
					GUI::MessageDialog alert(_("Do you want to quit ?"), _("Yes"), _("No"));
					if (alert.runModal() == GUI::kMessageOK)
						quit();

					return true;
				}
			}
		}
	}

	return false;
}

void OSystem_SDL_Symbian::setWindowCaption(const char *caption) {
	OSystem_SDL::setWindowCaption(caption);
}

void OSystem_SDL_Symbian::engineInit() {
	// Check mappings for the engine just started
	check_mappings();
}

void OSystem_SDL_Symbian::engineDone() {
	// Need to reset engine to basic state after an engine has been running
	GUI::Actions::Instance()->initInstanceMain(this);
}

void OSystem_SDL_Symbian::check_mappings() {
	if (ConfMan.get("gameid").empty() || GUI::Actions::Instance()->initialized())
		return;

	GUI::Actions::Instance()->initInstanceGame();
}

void OSystem_SDL_Symbian::initZones() {
	int i;

	_currentZone = 0;

	for (i = 0; i < TOTAL_ZONES; i++) {
		_mouseXZone[i] = (_zones[i].x + (_zones[i].width / 2));
		_mouseYZone[i] = (_zones[i].y + (_zones[i].height / 2));
	}
}

RFs& OSystem_SDL_Symbian::FsSession() {
	return *_RFs;
}

// Symbian bsearch implementation is flawed
void* scumm_bsearch(const void *key, const void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *)) {
	// Perform binary search
	size_t lo = 0;
	size_t hi = nmemb;
	while (lo < hi) {
		size_t mid = (lo + hi) / 2;
		const void *p = ((const char *)base) + mid * size;
		int tmp = (*compar)(key, p);
		if (tmp < 0)
			hi = mid;
		else if (tmp > 0)
			lo = mid + 1;
		else
			return (void *)p;
	}

	return NULL;
}

extern "C"
{
// Include the snprintf and vsnprintf implementations as 'C' code
#include "vsnprintf.h"
}

// Symbian SDL_Main implementation
// Redirects standard io, creates Symbian specific SDL backend (inherited from main SDL)
int main(int argc, char *argv[]) {
	//
	// Set up redirects for stdout/stderr under Symbian.
	// Code copied from SDL_main.
	//

	// Symbian does not like any output to the console through any *print* function
	char STDOUT_FILE[256], STDERR_FILE[256]; // shhh, don't tell anybody :)
	strcpy(STDOUT_FILE, Symbian::GetExecutablePath());
	strcpy(STDERR_FILE, Symbian::GetExecutablePath());
	strcat(STDOUT_FILE, "scummvm.stdout.txt");
	strcat(STDERR_FILE, "scummvm.stderr.txt");

	/* Flush the output in case anything is queued */
	fclose(stdout);
	fclose(stderr);

	/* Redirect standard input and standard output */
	FILE *newfp = freopen(STDOUT_FILE, "w", stdout);
	if (newfp == NULL) {	/* This happens on NT */
#if !defined(stdout)
		stdout = fopen(STDOUT_FILE, "w");
#else
		newfp = fopen(STDOUT_FILE, "w");
		if (newfp) {
			*stdout = *newfp;
		}
#endif
	}
	newfp = freopen(STDERR_FILE, "w", stderr);
	if (newfp == NULL) {	/* This happens on NT */
#if !defined(stderr)
		stderr = fopen(STDERR_FILE, "w");
#else
		newfp = fopen(STDERR_FILE, "w");
		if (newfp) {
			*stderr = *newfp;
		}
#endif
	}
	setbuf(stderr, NULL);			/* No buffering */

	// Create our OSystem instance
	g_system = new OSystem_SDL_Symbian();
	assert(g_system);

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new SDLPluginProvider());
#endif

	// Invoke the actual ScummVM main entry point:
	int res = scummvm_main(argc, argv);
	g_system->quit();	// TODO: Consider removing / replacing this!
	return res;
}
