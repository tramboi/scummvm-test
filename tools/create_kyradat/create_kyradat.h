/* ScummVM Tools
 * Copyright (C) 2007 The ScummVM project
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

#ifndef CREATE_KYRADAT_CREATE_KYRADAT_H
#define CREATE_KYRADAT_CREATE_KYRADAT_H

#include "util.h"

struct Language {
	int lang;
	const char *ext;
};

extern const Language languageTable[];

struct PlatformExtension {
	int platform;
	const char *ext;
};

extern const PlatformExtension platformTable[];

enum kExtractID {
	kForestSeq = 1,
	kKallakWritingSeq,
	kKyrandiaLogoSeq,
	kKallakMalcolmSeq,
	kMalcolmTreeSeq,
	kWestwoodLogoSeq,

	kDemo1Seq,
	kDemo2Seq,
	kDemo3Seq,
	kDemo4Seq,

	kAmuleteAnimSeq,

	kOutroReunionSeq,

	kIntroCPSStrings,
	kIntroCOLStrings,
	kIntroWSAStrings,
	kIntroStrings,

	kOutroHomeString,

	kRoomFilenames,
	kRoomList,

	kCharacterImageFilenames,

	kAudioTracks,
	kAudioTracksIntro,

	kItemNames,
	kTakenStrings,
	kPlacedStrings,
	kDroppedStrings,
	kNoDropStrings,

	kPutDownString,
	kWaitAmuletString,
	kBlackJewelString,
	kPoisonGoneString,
	kHealingTipString,
	kWispJewelStrings,
	kMagicJewelStrings,

	kThePoisonStrings,
	kFluteStrings,

	kFlaskFullString,
	kFullFlaskString,

	kVeryCleverString,
	kNewGameString,

	kDefaultShapes,
	kHealing1Shapes,
	kHealing2Shapes,
	kPoisonDeathShapes,
	kFluteShapes,
	kWinter1Shapes,
	kWinter2Shapes,
	kWinter3Shapes,
	kDrinkShapes,
	kWispShapes,
	kMagicAnimShapes,
	kBranStoneShapes,

	kPaletteList1,
	kPaletteList2,
	kPaletteList3,
	kPaletteList4,
	kPaletteList5,
	kPaletteList6,
	kPaletteList7,
	kPaletteList8,
	kPaletteList9,
	kPaletteList10,
	kPaletteList11,
	kPaletteList12,
	kPaletteList13,
	kPaletteList14,
	kPaletteList15,
	kPaletteList16,
	kPaletteList17,
	kPaletteList18,
	kPaletteList19,
	kPaletteList20,
	kPaletteList21,
	kPaletteList22,
	kPaletteList23,
	kPaletteList24,
	kPaletteList25,
	kPaletteList26,
	kPaletteList27,
	kPaletteList28,
	kPaletteList29,
	kPaletteList30,
	kPaletteList31,
	kPaletteList32,
	kPaletteList33,

	kGUIStrings,
	kConfigStrings,

	kKyra1TownsSFXwdTable,
	kKyra1TownsSFXbtTable,
	kKyra1TownsCDATable,
	kKyra1PC98StoryStrings,
	kKyra1PC98IntroSfx,
	kCreditsStrings,

	kAmigaIntroSFXTable,
	kAmigaGameSFXTable,

	k2SeqplayPakFiles,
	k2SeqplayStrings,
	k2SeqplaySfxFiles,
	k2SeqplayTlkFiles,
	k2SeqplaySeqData,
	k2SeqplayCredits,
	k2SeqplayCreditsSpecial,
	k2SeqplayIntroTracks,
	k2SeqplayFinaleTracks,
	k2SeqplayIntroCDA,
	k2SeqplayFinaleCDA,
	k2SeqplayShapeAnimData,

	k2IngamePakFiles,
	k2IngameSfxFiles,
	k2IngameSfxFilesTns,
	k2IngameSfxIndex,
	k2IngameTracks,
	k2IngameCDA,
	k2IngameTalkObjIndex,
	k2IngameTimJpStrings,
	k2IngameItemAnimData,
	k2IngameTlkDemoStrings,

	k3MainMenuStrings,
	k3MusicFiles,
	k3ScoreTable,
	k3SfxFiles,
	k3SfxMap,
	k3ItemAnimData,
	k3ItemMagicTable,
	k3ItemStringMap,

	kLolSeqplayIntroTracks,

	kLolIngamePakFiles,

	kLolCharacterDefs,
	kLolIngameSfxFiles,
	kLolIngameSfxIndex,
	kLolMusicTrackMap,
	kLolGMSfxIndex,
	kLolMT32SfxIndex,
	//lolADLSfxIndex,
	kLolSpellProperties,
	kLolGameShapeMap,
	kLolSceneItemOffs,
	kLolCharInvIndex,
	kLolCharInvDefs,
	kLolCharDefsMan,
	kLolCharDefsWoman,
	kLolCharDefsKieran,
	//lolCharDefsUnk,
	kLolCharDefsAkshel,
	kLolExpRequirements,
	kLolMonsterModifiers,
	kLolMonsterLevelOffsets,
	kLolMonsterDirFlags,
	kLolMonsterScaleY,
	kLolMonsterScaleX,
	kLolMonsterScaleWH,
	kLolFlyingItemShp,
	kLolInventoryDesc,

	kLolLevelShpList,
	kLolLevelDatList,
	kLolCompassDefs,
	kLolItemPrices,
	kLolStashSetup,

	kLolDscUnk1,
	kLolDscShapeIndex1,
	kLolDscShapeIndex2,
	kLolDscScaleWidthData,
	kLolDscScaleHeightData,
	kLolDscX,
	kLolDscY,
	kLolDscTileIndex,
	kLolDscUnk2,
	kLolDscDoorShapeIndex,
	kLolDscDimData1,
	kLolDscDimData2,
	kLolDscBlockMap,
	kLolDscDimMap,
	kLolDscShapeOvlIndex,
	kLolDscBlockIndex,
	kLolDscDoor1,
	kLolDscDoorScale,
	kLolDscDoor4,
	kLolDscDoorX,
	kLolDscDoorY,

	kLolScrollXTop,
	kLolScrollYTop,
	kLolScrollXBottom,
	kLolScrollYBottom,

	kLolButtonDefs,
	kLolButtonList1,
	kLolButtonList2,
	kLolButtonList3,
	kLolButtonList4,
	kLolButtonList5,
	kLolButtonList6,
	kLolButtonList7,
	kLolButtonList8,

	kLolLegendData,
	kLolMapCursorOvl,
	kLolMapStringId,
	//lolMapPal,

	kLolSpellbookAnim,
	kLolSpellbookCoords,
	kLolHealShapeFrames,
	kLolLightningDefs,
	kLolFireballCoords,

	kLolCredits,

	kLolHistory,

	kMaxResIDs
};

struct ExtractFilename {
	int id;
	int type;
	const char *filename;
};

enum kSpecial {
	kTalkieVersion = 0,
	kDemoVersion,

	kFile1,
	kFile2,
	kTalkieFile1,
	kTalkieFile2,

	k2CDDemo,

	// special case for Kyrandia 1 CD demo
	kDemoCDVersion
};

struct SpecialExtension {
	int special;
	const char *ext;
};

extern const SpecialExtension specialTable[];

enum kGame {
	kKyra1 = 0,
	kKyra2 = 1,
	kKyra3 = 2,
	kLol = 4
};

struct Game {
	int game;
	int lang;
	int platform;
	int special;

	const char *md5;
};

#define GAME_DUMMY_ENTRY { -1, -1, -1, -1, 0 }

extern const Game *gameDescs[];

const int *getNeedList(const Game *g);

#endif
