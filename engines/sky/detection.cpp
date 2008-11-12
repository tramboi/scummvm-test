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

#include "sky/control.h"
#include "sky/sky.h"

#include "base/plugins.h"

#include "common/config-manager.h"
#include "common/advancedDetector.h"
#include "common/system.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/savefile.h"

#include "engines/metaengine.h"

static const PlainGameDescriptor skySetting =
	{"sky", "Beneath a Steel Sky" };

struct SkyVersion {
	int dinnerTableEntries;
	int dataDiskSize;
	const char *extraDesc;
	int version;
};

// TODO: Would be nice if Disk::determineGameVersion() used this table, too.
static const SkyVersion skyVersions[] = {
	{  243, -1, "pc gamer demo", 109 },
	{  247, -1, "floppy demo", 267 },
	{ 1404, -1, "floppy", 288 },
	{ 1413, -1, "floppy", 303 },
	{ 1445, 8830435, "floppy", 348 },
	{ 1445, -1, "floppy", 331 },
	{ 1711, -1, "cd demo", 365 },
	{ 5099, -1, "cd", 368 },
	{ 5097, -1, "cd", 372 },
	{ 0, 0, 0, 0 }
};

class SkyMetaEngine : public MetaEngine {
public:
	virtual const char *getName() const;
	virtual const char *getCopyright() const;

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual GameList getSupportedGames() const;
	virtual GameDescriptor findGame(const char *gameid) const;
	virtual GameList detectGames(const Common::FSList &fslist) const;	

	virtual Common::Error createInstance(OSystem *syst, Engine **engine) const;

	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
};

const char *SkyMetaEngine::getName() const {
	return "Beneath a Steel Sky";
}

const char *SkyMetaEngine::getCopyright() const {
	return "Beneath a Steel Sky (C) Revolution";
}

bool SkyMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup);
}

bool Sky::SkyEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL);
}

GameList SkyMetaEngine::getSupportedGames() const {
	GameList games;
	games.push_back(skySetting);
	return games;
}

GameDescriptor SkyMetaEngine::findGame(const char *gameid) const {
	if (0 == scumm_stricmp(gameid, skySetting.gameid))
		return skySetting;
	return GameDescriptor();
}

GameList SkyMetaEngine::detectGames(const Common::FSList &fslist) const {
	GameList detectedGames;
	bool hasSkyDsk = false;
	bool hasSkyDnr = false;
	int dinnerTableEntries = -1;
	int dataDiskSize = -1;

	// Iterate over all files in the given directory
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (!file->isDirectory()) {
			const char *fileName = file->getName().c_str();

			if (0 == scumm_stricmp("sky.dsk", fileName)) {
				Common::File dataDisk;
				if (dataDisk.open(*file)) {
					hasSkyDsk = true;
					dataDiskSize = dataDisk.size();
				}
			}

			if (0 == scumm_stricmp("sky.dnr", fileName)) {
				Common::File dinner;
				if (dinner.open(*file)) {
					hasSkyDnr = true;
					dinnerTableEntries = dinner.readUint32LE();
				}
			}
		}
	}

	if (hasSkyDsk && hasSkyDnr) {
		// Match found, add to list of candidates, then abort inner loop.
		// The game detector uses US English by default. We want British
		// English to match the recorded voices better.
		GameDescriptor dg(skySetting.gameid, skySetting.description, Common::UNK_LANG, Common::kPlatformUnknown);
		const SkyVersion *sv = skyVersions;
		while (sv->dinnerTableEntries) {
			if (dinnerTableEntries == sv->dinnerTableEntries &&
				(sv->dataDiskSize == dataDiskSize || sv->dataDiskSize == -1)) {
				char buf[32];
				snprintf(buf, sizeof(buf), "v0.0%d %s", sv->version, sv->extraDesc);
				dg.updateDesc(buf);
				break;
			}
			++sv;
		}
		detectedGames.push_back(dg);
	}

	return detectedGames;
}

Common::Error SkyMetaEngine::createInstance(OSystem *syst, Engine **engine) const {
	assert(engine);
	*engine = new Sky::SkyEngine(syst);
	return Common::kNoError;
}

SaveStateList SkyMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	SaveStateList saveList;

	// Load the descriptions
	Common::StringList savenames;
	savenames.resize(MAX_SAVE_GAMES+1);

	Common::InSaveFile *inf;
	inf = saveFileMan->openForLoading("SKY-VM.SAV");
	if (inf != NULL) {
		char *tmpBuf =  new char[MAX_SAVE_GAMES * MAX_TEXT_LEN];
		char *tmpPtr = tmpBuf;
		inf->read(tmpBuf, MAX_SAVE_GAMES * MAX_TEXT_LEN);
		for (int i = 0; i < MAX_SAVE_GAMES; ++i) {
			savenames[i] = tmpPtr;
			tmpPtr += savenames[i].size() + 1;
		}
		delete inf;
		delete[] tmpBuf;
	}

	// Find all saves
	Common::StringList filenames;
	filenames = saveFileMan->listSavefiles("SKY-VM.???");
	sort(filenames.begin(), filenames.end());	// Sort (hopefully ensuring we are sorted numerically..)

	// Slot 0 is the autosave, if it exists.
	// TODO: Check for the existence of the autosave -- but this require us
	// to know which SKY variant we are looking at.
	saveList.insert_at(0, SaveStateDescriptor(0, "*AUTOSAVE*"));

	// Prepare the list of savestates by looping over all matching savefiles
	for (Common::StringList::const_iterator file = filenames.begin(); file != filenames.end(); file++) {
		// Extract the extension
		Common::String ext = file->c_str() + file->size() - 3;
		ext.toUppercase();
		if (isdigit(ext[0]) && isdigit(ext[1]) && isdigit(ext[2])){
			int slotNum = atoi(ext.c_str());
			Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
			if (in) {
				saveList.push_back(SaveStateDescriptor(slotNum+1, savenames[slotNum]));
				delete in;
			}
		}
	}

	return saveList;
}

int SkyMetaEngine::getMaximumSaveSlot() const { return MAX_SAVE_GAMES; }

#if PLUGIN_ENABLED_DYNAMIC(SKY)
	REGISTER_PLUGIN_DYNAMIC(SKY, PLUGIN_TYPE_ENGINE, SkyMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(SKY, PLUGIN_TYPE_ENGINE, SkyMetaEngine);
#endif
