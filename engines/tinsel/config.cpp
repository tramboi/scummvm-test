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
 * This file contains configuration functionality
 */

#include "tinsel/config.h"
#include "tinsel/dw.h"
#include "tinsel/sound.h"
#include "tinsel/music.h"

#include "common/file.h"
#include "common/config-manager.h"

#include "sound/mixer.h"

namespace Tinsel {

//----------------- GLOBAL GLOBAL DATA --------------------

int dclickSpeed = DOUBLE_CLICK_TIME;
int volMidi = Audio::Mixer::kMaxChannelVolume;
int volSound = Audio::Mixer::kMaxChannelVolume;
int volVoice = Audio::Mixer::kMaxChannelVolume;
int speedText = DEFTEXTSPEED;
int bSubtitles = false;
int bSwapButtons = 0;
LANGUAGE g_language = TXT_ENGLISH;
int bAmerica = 0;


// Shouldn't really be here, but time is short...
bool bNoBlocking;

/**
 * Write settings to config manager and flush the config file to disk.
 */
void WriteConfig(void) {
	ConfMan.setInt("dclick_speed", dclickSpeed);
	ConfMan.setInt("music_volume", volMidi);
	ConfMan.setInt("sfx_volume", volSound);
	ConfMan.setInt("speech_volume", volVoice);
	ConfMan.setInt("talkspeed", (speedText * 255) / 100);
	ConfMan.setBool("subtitles", bSubtitles);
	//ConfMan.setBool("swap_buttons", bSwapButtons ? 1 : 0);
	//ConfigData.bAmerica = bAmerica;		// EN_USA / EN_GRB

	// Store language for multilingual versions
	if ((_vm->getFeatures() & GF_USE_3FLAGS) || (_vm->getFeatures() & GF_USE_4FLAGS) || (_vm->getFeatures() & GF_USE_5FLAGS)) {
		Common::Language lang;
		switch (g_language) {
		case TXT_FRENCH:
			lang = Common::FR_FRA;
			break;
		case TXT_GERMAN:
			lang = Common::DE_DEU;
			break;
		case TXT_SPANISH:
			lang = Common::ES_ESP;
			break;
		case TXT_ITALIAN:
			lang = Common::IT_ITA;
			break;
		default:
			lang = Common::EN_ANY;
		}
		
		ConfMan.set("language", Common::getLanguageCode(lang));
	}
	
	// Write to disk
	ConfMan.flushToDisk();
}

/*---------------------------------------------------------------------*\
|	ReadConfig()							|
|-----------------------------------------------------------------------|
|
\*---------------------------------------------------------------------*/
void ReadConfig(void) {
	if (ConfMan.hasKey("dclick_speed"))
		dclickSpeed = ConfMan.getInt("dclick_speed");

	volMidi = ConfMan.getInt("music_volume");
	volSound = ConfMan.getInt("sfx_volume");
	volVoice = ConfMan.getInt("speech_volume");

	if (ConfMan.hasKey("talkspeed"))
		speedText = (ConfMan.getInt("talkspeed") * 100) / 255;
	if (ConfMan.hasKey("subtitles"))
		bSubtitles = ConfMan.getBool("subtitles");

	// FIXME: If JAPAN is set, subtitles are forced OFF in the original engine

	//bSwapButtons = ConfMan.getBool("swap_buttons") == 1 ? true : false;
	//ConfigData.language = language;	// not necessary, as language has been set in the launcher
	//ConfigData.bAmerica = bAmerica;		// EN_USA / EN_GRB

	// Set language - we'll be clever here and use the ScummVM language setting
	g_language = TXT_ENGLISH;
	Common::Language lang = _vm->getLanguage();
	if (lang == Common::UNK_LANG && ConfMan.hasKey("language"))
		lang = Common::parseLanguage(ConfMan.get("language"));	// For multi-lingual versions, fall back to user settings
	switch (lang) {
	case Common::FR_FRA:
		g_language = TXT_FRENCH;
		break;
	case Common::DE_DEU:
		g_language = TXT_GERMAN;
		break;
	case Common::ES_ESP:
		g_language = TXT_SPANISH;
		break;
	case Common::IT_ITA:
		g_language = TXT_ITALIAN;
		break;
	default:
		g_language = TXT_ENGLISH;
	}

	if (lang == Common::JA_JPN) {
		// TODO: Add support for JAPAN version
	} else if (lang == Common::HB_ISR) {
		// TODO: Add support for HEBREW version

		// The Hebrew version appears to the software as being English
		// but it needs to have subtitles on...
		g_language = TXT_ENGLISH;
		bSubtitles = true;
	} else if (_vm->getFeatures() & GF_USE_3FLAGS) {
		// 3 FLAGS version supports French, German, Spanish
		// Fall back to German if necessary
		if (g_language != TXT_FRENCH && g_language != TXT_GERMAN && g_language != TXT_SPANISH) {
			g_language = TXT_GERMAN;
			bSubtitles = true;
		}
	} else if (_vm->getFeatures() & GF_USE_4FLAGS) {
		// 4 FLAGS version supports French, German, Spanish, Italian
		// Fall back to German if necessary
		if (g_language != TXT_FRENCH && g_language != TXT_GERMAN &&
				g_language != TXT_SPANISH && g_language != TXT_ITALIAN) {
			g_language = TXT_GERMAN;
			bSubtitles = true;
		}
	}
}

bool isJapanMode() {
#ifdef JAPAN
	return true;
#else
	return false;
#endif
}

} // end of namespace Tinsel
