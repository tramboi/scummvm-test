const ExtractEntry extractProviders[] = {
	{ kForestSeq, kForestSeqProvider },
	{ kKallakWritingSeq, kKallakWritingSeqProvider },
	{ kKyrandiaLogoSeq, kKyrandiaLogoSeqProvider },
	{ kKallakMalcolmSeq, kKallakMalcolmSeqProvider },
	{ kMalcolmTreeSeq, kMalcolmTreeSeqProvider },
	{ kWestwoodLogoSeq, kWestwoodLogoSeqProvider },
	{ kDemo1Seq, kDemo1SeqProvider },
	{ kDemo2Seq, kDemo2SeqProvider },
	{ kDemo3Seq, kDemo3SeqProvider },
	{ kDemo4Seq, kDemo4SeqProvider },
	{ kAmuleteAnimSeq, kAmuleteAnimSeqProvider },
	{ kOutroReunionSeq, kOutroReunionSeqProvider },
	{ kIntroCPSStrings, kIntroCPSStringsProvider },
	{ kIntroCOLStrings, kIntroCOLStringsProvider },
	{ kIntroWSAStrings, kIntroWSAStringsProvider },
	{ kIntroStrings, kIntroStringsProvider },
	{ kOutroHomeString, kOutroHomeStringProvider },
	{ kRoomFilenames, kRoomFilenamesProvider },
	{ kRoomList, kRoomListProvider },
	{ kCharacterImageFilenames, kCharacterImageFilenamesProvider },
	{ kAudioTracks, kAudioTracksProvider },
	{ kAudioTracksIntro, kAudioTracksIntroProvider },
	{ kItemNames, kItemNamesProvider },
	{ kTakenStrings, kTakenStringsProvider },
	{ kPlacedStrings, kPlacedStringsProvider },
	{ kDroppedStrings, kDroppedStringsProvider },
	{ kNoDropStrings, kNoDropStringsProvider },
	{ kPutDownString, kPutDownStringProvider },
	{ kWaitAmuletString, kWaitAmuletStringProvider },
	{ kBlackJewelString, kBlackJewelStringProvider },
	{ kPoisonGoneString, kPoisonGoneStringProvider },
	{ kHealingTipString, kHealingTipStringProvider },
	{ kWispJewelStrings, kWispJewelStringsProvider },
	{ kMagicJewelStrings, kMagicJewelStringsProvider },
	{ kThePoisonStrings, kThePoisonStringsProvider },
	{ kFluteStrings, kFluteStringsProvider },
	{ kFlaskFullString, kFlaskFullStringProvider },
	{ kFullFlaskString, kFullFlaskStringProvider },
	{ kVeryCleverString, kVeryCleverStringProvider },
	{ kNewGameString, kNewGameStringProvider },
	{ kDefaultShapes, kDefaultShapesProvider },
	{ kHealing1Shapes, kHealing1ShapesProvider },
	{ kHealing2Shapes, kHealing2ShapesProvider },
	{ kPoisonDeathShapes, kPoisonDeathShapesProvider },
	{ kFluteShapes, kFluteShapesProvider },
	{ kWinter1Shapes, kWinter1ShapesProvider },
	{ kWinter2Shapes, kWinter2ShapesProvider },
	{ kWinter3Shapes, kWinter3ShapesProvider },
	{ kDrinkShapes, kDrinkShapesProvider },
	{ kWispShapes, kWispShapesProvider },
	{ kMagicAnimShapes, kMagicAnimShapesProvider },
	{ kBranStoneShapes, kBranStoneShapesProvider },
	{ kPaletteList1, kPaletteList1Provider },
	{ kPaletteList2, kPaletteList2Provider },
	{ kPaletteList3, kPaletteList3Provider },
	{ kPaletteList4, kPaletteList4Provider },
	{ kPaletteList5, kPaletteList5Provider },
	{ kPaletteList6, kPaletteList6Provider },
	{ kPaletteList7, kPaletteList7Provider },
	{ kPaletteList8, kPaletteList8Provider },
	{ kPaletteList9, kPaletteList9Provider },
	{ kPaletteList10, kPaletteList10Provider },
	{ kPaletteList11, kPaletteList11Provider },
	{ kPaletteList12, kPaletteList12Provider },
	{ kPaletteList13, kPaletteList13Provider },
	{ kPaletteList14, kPaletteList14Provider },
	{ kPaletteList15, kPaletteList15Provider },
	{ kPaletteList16, kPaletteList16Provider },
	{ kPaletteList17, kPaletteList17Provider },
	{ kPaletteList18, kPaletteList18Provider },
	{ kPaletteList19, kPaletteList19Provider },
	{ kPaletteList20, kPaletteList20Provider },
	{ kPaletteList21, kPaletteList21Provider },
	{ kPaletteList22, kPaletteList22Provider },
	{ kPaletteList23, kPaletteList23Provider },
	{ kPaletteList24, kPaletteList24Provider },
	{ kPaletteList25, kPaletteList25Provider },
	{ kPaletteList26, kPaletteList26Provider },
	{ kPaletteList27, kPaletteList27Provider },
	{ kPaletteList28, kPaletteList28Provider },
	{ kPaletteList29, kPaletteList29Provider },
	{ kPaletteList30, kPaletteList30Provider },
	{ kPaletteList31, kPaletteList31Provider },
	{ kPaletteList32, kPaletteList32Provider },
	{ kPaletteList33, kPaletteList33Provider },
	{ kGUIStrings, kGUIStringsProvider },
	{ kConfigStrings, kConfigStringsProvider },
	{ kKyra1TownsSFXwdTable, kKyra1TownsSFXwdTableProvider },
	{ kKyra1TownsSFXbtTable, kKyra1TownsSFXbtTableProvider },
	{ kKyra1TownsCDATable, kKyra1TownsCDATableProvider },
	{ kCreditsStrings, kCreditsStringsProvider },
	{ kAmigaIntroSFXTable, kAmigaIntroSFXTableProvider },
	{ kAmigaGameSFXTable, kAmigaGameSFXTableProvider },
	{ k2SeqplayPakFiles, k2SeqplayPakFilesProvider },
	{ k2SeqplayStrings, k2SeqplayStringsProvider },
	{ k2SeqplaySfxFiles, k2SeqplaySfxFilesProvider },
	{ k2SeqplayTlkFiles, k2SeqplayTlkFilesProvider },
	{ k2SeqplaySeqData, k2SeqplaySeqDataProvider },
	{ k2SeqplayCredits, k2SeqplayCreditsProvider },
	{ k2SeqplayCreditsSpecial, k2SeqplayCreditsSpecialProvider },
	{ k2SeqplayIntroTracks, k2SeqplayIntroTracksProvider },
	{ k2SeqplayFinaleTracks, k2SeqplayFinaleTracksProvider },
	{ k2SeqplayIntroCDA, k2SeqplayIntroCDAProvider },
	{ k2SeqplayFinaleCDA, k2SeqplayFinaleCDAProvider },
	{ k2SeqplayShapeAnimData, k2SeqplayShapeAnimDataProvider },
	{ k2IngamePakFiles, k2IngamePakFilesProvider },
	{ k2IngameSfxFiles, k2IngameSfxFilesProvider },
	{ k2IngameSfxFilesTns, k2IngameSfxFilesTnsProvider },
	{ k2IngameSfxIndex, k2IngameSfxIndexProvider },
	{ k2IngameTracks, k2IngameTracksProvider },
	{ k2IngameCDA, k2IngameCDAProvider },
	{ k2IngameTalkObjIndex, k2IngameTalkObjIndexProvider },
	{ k2IngameTimJpStrings, k2IngameTimJpStringsProvider },
	{ k2IngameItemAnimData, k2IngameItemAnimDataProvider },
	{ k2IngameTlkDemoStrings, k2IngameTlkDemoStringsProvider },
	{ k3MainMenuStrings, k3MainMenuStringsProvider },
	{ k3MusicFiles, k3MusicFilesProvider },
	{ k3ScoreTable, k3ScoreTableProvider },
	{ k3SfxFiles, k3SfxFilesProvider },
	{ k3SfxMap, k3SfxMapProvider },
	{ k3ItemAnimData, k3ItemAnimDataProvider },
	{ k3ItemMagicTable, k3ItemMagicTableProvider },
	{ k3ItemStringMap, k3ItemStringMapProvider },
	{ kLolSeqplayIntroTracks, kLolSeqplayIntroTracksProvider },
	{ kLolIngamePakFiles, kLolIngamePakFilesProvider },
	{ kLolCharacterDefs, kLolCharacterDefsProvider },
	{ kLolIngameSfxFiles, kLolIngameSfxFilesProvider },
	{ kLolIngameSfxIndex, kLolIngameSfxIndexProvider },
	{ kLolMusicTrackMap, kLolMusicTrackMapProvider },
	{ kLolGMSfxIndex, kLolGMSfxIndexProvider },
	{ kLolMT32SfxIndex, kLolMT32SfxIndexProvider },
	{ kLolSpellProperties, kLolSpellPropertiesProvider },
	{ kLolGameShapeMap, kLolGameShapeMapProvider },
	{ kLolSceneItemOffs, kLolSceneItemOffsProvider },
	{ kLolCharInvIndex, kLolCharInvIndexProvider },
	{ kLolCharInvDefs, kLolCharInvDefsProvider },
	{ kLolCharDefsMan, kLolCharDefsManProvider },
	{ kLolCharDefsWoman, kLolCharDefsWomanProvider },
	{ kLolCharDefsKieran, kLolCharDefsKieranProvider },
	{ kLolCharDefsAkshel, kLolCharDefsAkshelProvider },
	{ kLolExpRequirements, kLolExpRequirementsProvider },
	{ kLolMonsterModifiers, kLolMonsterModifiersProvider },
	{ kLolMonsterLevelOffsets, kLolMonsterLevelOffsetsProvider },
	{ kLolMonsterDirFlags, kLolMonsterDirFlagsProvider },
	{ kLolMonsterScaleY, kLolMonsterScaleYProvider },
	{ kLolMonsterScaleX, kLolMonsterScaleXProvider },
	{ kLolMonsterScaleWH, kLolMonsterScaleWHProvider },
	{ kLolFlyingItemShp, kLolFlyingItemShpProvider },
	{ kLolInventoryDesc, kLolInventoryDescProvider },
	{ kLolLevelShpList, kLolLevelShpListProvider },
	{ kLolLevelDatList, kLolLevelDatListProvider },
	{ kLolCompassDefs, kLolCompassDefsProvider },
	{ kLolItemPrices, kLolItemPricesProvider },
	{ kLolStashSetup, kLolStashSetupProvider },
	{ kLolDscUnk1, kLolDscUnk1Provider },
	{ kLolDscShapeIndex1, kLolDscShapeIndex1Provider },
	{ kLolDscShapeIndex2, kLolDscShapeIndex2Provider },
	{ kLolDscScaleWidthData, kLolDscScaleWidthDataProvider },
	{ kLolDscScaleHeightData, kLolDscScaleHeightDataProvider },
	{ kLolDscX, kLolDscXProvider },
	{ kLolDscY, kLolDscYProvider },
	{ kLolDscTileIndex, kLolDscTileIndexProvider },
	{ kLolDscUnk2, kLolDscUnk2Provider },
	{ kLolDscDoorShapeIndex, kLolDscDoorShapeIndexProvider },
	{ kLolDscDimData1, kLolDscDimData1Provider },
	{ kLolDscDimData2, kLolDscDimData2Provider },
	{ kLolDscBlockMap, kLolDscBlockMapProvider },
	{ kLolDscDimMap, kLolDscDimMapProvider },
	{ kLolDscShapeOvlIndex, kLolDscShapeOvlIndexProvider },
	{ kLolDscBlockIndex, kLolDscBlockIndexProvider },
	{ kLolDscDoor1, kLolDscDoor1Provider },
	{ kLolDscDoorScale, kLolDscDoorScaleProvider },
	{ kLolDscDoor4, kLolDscDoor4Provider },
	{ kLolDscDoorX, kLolDscDoorXProvider },
	{ kLolDscDoorY, kLolDscDoorYProvider },
	{ kLolScrollXTop, kLolScrollXTopProvider },
	{ kLolScrollYTop, kLolScrollYTopProvider },
	{ kLolScrollXBottom, kLolScrollXBottomProvider },
	{ kLolScrollYBottom, kLolScrollYBottomProvider },
	{ kLolButtonDefs, kLolButtonDefsProvider },
	{ kLolButtonList1, kLolButtonList1Provider },
	{ kLolButtonList2, kLolButtonList2Provider },
	{ kLolButtonList3, kLolButtonList3Provider },
	{ kLolButtonList4, kLolButtonList4Provider },
	{ kLolButtonList5, kLolButtonList5Provider },
	{ kLolButtonList6, kLolButtonList6Provider },
	{ kLolButtonList7, kLolButtonList7Provider },
	{ kLolButtonList8, kLolButtonList8Provider },
	{ kLolLegendData, kLolLegendDataProvider },
	{ kLolMapCursorOvl, kLolMapCursorOvlProvider },
	{ kLolMapStringId, kLolMapStringIdProvider },
	{ kLolSpellbookAnim, kLolSpellbookAnimProvider },
	{ kLolSpellbookCoords, kLolSpellbookCoordsProvider },
	{ kLolHealShapeFrames, kLolHealShapeFramesProvider },
	{ kLolLightningDefs, kLolLightningDefsProvider },
	{ kLolFireballCoords, kLolFireballCoordsProvider },
	{ kLolHistory, kLolHistoryProvider },
	{ -1, NULL }
};

