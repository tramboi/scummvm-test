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

#include "lastexpress/game/menu.h"

// Data
#include "lastexpress/data/animation.h"
#include "lastexpress/data/cursor.h"
#include "lastexpress/data/snd.h"
#include "lastexpress/data/scene.h"

#include "lastexpress/game/fight.h"
#include "lastexpress/game/inventory.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/savegame.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/sound.h"
#include "lastexpress/game/state.h"

#include "lastexpress/graphics.h"
#include "lastexpress/helpers.h"
#include "lastexpress/lastexpress.h"
#include "lastexpress/resource.h"

#define getNextGameId() (GameId)((_gameId + 1) % 6)

namespace LastExpress {

// Bottom-left buttons (quit.seq)
enum StartMenuButtons {
	kButtonVolumeDownPushed,
	kButtonVolumeDown,
	kButtonVolume,
	kButtonVolumeUp,
	kButtonVolumeUpPushed,
	kButtonBrightnessDownPushed,    // 5
	kButtonBrightnessDown,
	kButtonBrightness,
	kButtonBrightnessUp,
	kButtonBrightnessUpPushed,
	kButtonQuit,                    // 10
	kButtonQuitPushed
};

// Egg buttons (buttns.seq)
enum StartMenuEggButtons {
	kButtonShield,
	kButtonRewind,
	kButtonRewindPushed,
	kButtonForward,
	kButtonForwardPushed,
	kButtonCredits,                // 5
	kButtonCreditsPushed,
	kButtonContinue
};

// Tooltips sequence (helpnewr.seq)
enum StartMenuTooltips {
	kTooltipInsertCd1,
	kTooltipInsertCd2,
	kTooltipInsertCd3,
	kTooltipContinueGame,
	kTooltipReplayGame,
	kTooltipContinueRewoundGame,    // 5
	kTooltipViewGameEnding,
	kTooltipStartAnotherGame,
	kTooltipVolumeUp,
	kTooltipVolumeDown,
	kTooltipBrightnessUp,           // 10
	kTooltipBrightnessDown,
	kTooltipQuit,
	kTooltipRewindParis,
	kTooltipForwardStrasbourg,
	kTooltipRewindStrasbourg,      // 15
	kTooltipRewindMunich,
	kTooltipForwardMunich,
	kTooltipForwardVienna,
	kTooltipRewindVienna,
	kTooltipRewindBudapest,        // 20
	kTooltipForwardBudapest,
	kTooltipForwardBelgrade,
	kTooltipRewindBelgrade,
	kTooltipForwardConstantinople,
	kTooltipSwitchBlueGame,        // 25
	kTooltipSwitchRedGame,
	kTooltipSwitchGoldGame,
	kTooltipSwitchGreenGame,
	kTooltipSwitchTealGame,
	kTooltipSwitchPurpleGame,      // 30
	kTooltipPlayNewGame,
	kTooltipCredits,
	kTooltipFastForward,
	kTooltipRewind
};

//////////////////////////////////////////////////////////////////////////
// DATA
//////////////////////////////////////////////////////////////////////////

// Information about the cities on the train line
static const struct {
	uint8 frame;
	TimeValue time;
} _trainCities[31] = {
	{0, kTimeCityParis},
	{9, kTimeCityEpernay},
	{11, kTimeCityChalons},
	{16, kTimeCityBarLeDuc},
	{21, kTimeCityNancy},
	{25, kTimeCityLuneville},
	{35, kTimeCityAvricourt},
	{37, kTimeCityDeutschAvricourt},
	{40, kTimeCityStrasbourg},
	{53, kTimeCityBadenOos},
	{56, kTimeCityKarlsruhe},
	{60, kTimeCityStuttgart},
	{63, kTimeCityGeislingen},
	{66, kTimeCityUlm},
	{68, kTimeCityAugsburg},
	{73, kTimeCityMunich},
	{84, kTimeCitySalzbourg},
	{89, kTimeCityAttnangPuchheim},
	{97, kTimeCityWels},
	{100, kTimeCityLinz},
	{104, kTimeCityAmstetten},
	{111, kTimeCityVienna},
	{120, kTimeCityPoszony},
	{124, kTimeCityGalanta},
	{132, kTimeCityBudapest},
	{148, kTimeCityBelgrade},
	/* Line 1 ends at 150 - line 2 begins at 0 */
	{157, kTimeCityNish},
	{165, kTimeCityTzaribrod},
	{174, kTimeCitySofia},
	{198, kTimeCityAdrianople},
	{210, kTimeCityConstantinople}};

static const struct {
	TimeValue time;
	uint index;
	StartMenuTooltips rewind;
	StartMenuTooltips forward;
} _cityButtonsInfo[7] = {
	{kTimeCityParis, 64, kTooltipRewindParis, kTooltipRewindParis},
	{kTimeCityStrasbourg, 128, kTooltipRewindStrasbourg, kTooltipForwardStrasbourg},
	{kTimeCityMunich, 129, kTooltipRewindMunich, kTooltipForwardMunich},
	{kTimeCityVienna, 130, kTooltipRewindVienna, kTooltipForwardVienna},
	{kTimeCityBudapest, 131, kTooltipRewindBudapest, kTooltipForwardBudapest},
	{kTimeCityBelgrade, 132, kTooltipRewindBelgrade, kTooltipForwardBelgrade},
	{kTimeCityConstantinople, 192, kTooltipForwardConstantinople, kTooltipForwardConstantinople}
};

//////////////////////////////////////////////////////////////////////////
// Clock
//////////////////////////////////////////////////////////////////////////
class Clock {
public:
	Clock(LastExpressEngine *engine);
	~Clock();

	void draw(uint32 time);
	void clear();

private:
	LastExpressEngine *_engine;

	// Frames
	SequenceFrame *_frameMinutes;
	SequenceFrame *_frameHour;
	SequenceFrame *_frameSun;
	SequenceFrame *_frameDate;
};

Clock::Clock(LastExpressEngine *engine) : _engine(engine), _frameMinutes(NULL), _frameHour(NULL), _frameSun(NULL), _frameDate(NULL) {
	_frameMinutes = new SequenceFrame(loadSequence("eggmin.seq"), 0, true);
	_frameHour = new SequenceFrame(loadSequence("egghour.seq"), 0, true);
	_frameSun = new SequenceFrame(loadSequence("sun.seq"), 0, true);
	_frameDate = new SequenceFrame(loadSequence("datenew.seq"), 0, true);
}

Clock::~Clock() {
	delete _frameMinutes;
	delete _frameHour;
	delete _frameSun;
	delete _frameDate;

	// Zero passed pointers
	_engine = NULL;
}

void Clock::clear() {
	getScenes()->removeAndRedraw(&_frameMinutes, false);
	getScenes()->removeAndRedraw(&_frameHour, false);
	getScenes()->removeAndRedraw(&_frameSun, false);
	getScenes()->removeAndRedraw(&_frameDate, false);
}

void Clock::draw(uint32 time) {
	assert(time >= kTimeCityParis && time <= kTimeCityConstantinople);

	// Check that sequences have been loaded
	if (!_frameMinutes || !_frameHour || !_frameSun || !_frameDate)
		error("Clock::process: clock sequences have not been loaded correctly!");

	// Clear existing frames
	clear();

	// Game starts at: 1037700 = 7:13 p.m. on July 24, 1914
	// Game ends at:   4941000 = 7:30 p.m. on July 26, 1914
	// Game lasts for: 3903300 = 2 days + 17 mins = 2897 mins

	// 15 = 1 second
	// 15 * 60 = 900 = 1 minute
	// 900 * 60 = 54000 = 1 hour
	// 54000 * 24 = 1296000 = 1 day

	// Calculate each sequence index from the current time
	uint8 hour = (uint8)((time % 1296000) / 54000);
	uint8 minute =  (uint8)((time % 54000) / 900);
	uint32 index_date = 18 * time / 1296000;
	if (hour == 23)
		index_date += 18 * minute / 60;

	// Set sequences frames
	_frameMinutes->setFrame(minute);
	_frameHour->setFrame((5 * hour + minute / 12) % 60);
	_frameSun->setFrame((5 * hour + minute / 12) % 120);
	_frameDate->setFrame((uint16)index_date);

	// Adjust z-order and queue
	_frameMinutes->getInfo()->location = 1;
	_frameHour->getInfo()->location = 1;
	_frameSun->getInfo()->location = 1;
	_frameDate->getInfo()->location = 1;

	getScenes()->addToQueue(_frameMinutes);
	getScenes()->addToQueue(_frameHour);
	getScenes()->addToQueue(_frameSun);
	getScenes()->addToQueue(_frameDate);
}

//////////////////////////////////////////////////////////////////////////
// TrainLine
//////////////////////////////////////////////////////////////////////////
class TrainLine {
public:
	TrainLine(LastExpressEngine *engine);
	~TrainLine();

	void draw(uint32 time);
	void clear();

private:
	LastExpressEngine *_engine;

	// Frames
	SequenceFrame *_frameLine1;
	SequenceFrame *_frameLine2;
};

TrainLine::TrainLine(LastExpressEngine *engine) : _engine(engine), _frameLine1(NULL), _frameLine2(NULL) {
	_frameLine1 = new SequenceFrame(loadSequence("line1.seq"), 0, true);
	_frameLine2 = new SequenceFrame(loadSequence("line2.seq"), 0, true);
}

TrainLine::~TrainLine() {
	delete _frameLine1;
	delete _frameLine2;

	// Zero passed pointers
	_engine = NULL;
}

void TrainLine::clear() {
	getScenes()->removeAndRedraw(&_frameLine1, false);
	getScenes()->removeAndRedraw(&_frameLine2, false);
}

// Draw the train line at the time
//  line1: 150 frames (=> Belgrade)
//  line2: 61 frames (=> Constantinople)
void TrainLine::draw(uint32 time) {
	assert(time >= kTimeCityParis && time <= kTimeCityConstantinople);

	// Check that sequences have been loaded
	if (!_frameLine1 || !_frameLine2)
		error("TrainLine::process: Line sequences have not been loaded correctly!");

	// Clear existing frames
	clear();

	// Get the index of the last city the train has visited
	uint index = 0;
	for (uint i = 0; i < ARRAYSIZE(_trainCities); i++)
		if ((uint32)_trainCities[i].time <= time)
			index = i;

	uint16 frame;
	if (time > (uint32)_trainCities[index].time) {
		// Interpolate linearly to use a frame between the cities
		uint8 diffFrames = _trainCities[index + 1].frame - _trainCities[index].frame;
		uint diffTimeCities = (uint)(_trainCities[index + 1].time - _trainCities[index].time);
		uint traveledTime = (time - (uint)_trainCities[index].time);
		frame = (uint16)(_trainCities[index].frame + (traveledTime * diffFrames) / diffTimeCities);
	} else {
		// Exactly on the city
		frame = _trainCities[index].frame;
	}

	// Set frame, z-order and queue
	if (frame < 150) {
		_frameLine1->setFrame(frame);

		_frameLine1->getInfo()->location = 1;
		getScenes()->addToQueue(_frameLine1);
	} else {
		// We passed Belgrade
		_frameLine1->setFrame(149);
		_frameLine2->setFrame(frame - 150);

		_frameLine1->getInfo()->location = 1;
		_frameLine2->getInfo()->location = 1;

		getScenes()->addToQueue(_frameLine1);
		getScenes()->addToQueue(_frameLine2);
	}
}


//////////////////////////////////////////////////////////////////////////
// Menu
//////////////////////////////////////////////////////////////////////////
Menu::Menu(LastExpressEngine *engine) : _engine(engine),
	_seqTooltips(NULL), _seqEggButtons(NULL), _seqButtons(NULL), _seqAcorn(NULL), _seqCity1(NULL), _seqCity2(NULL), _seqCity3(NULL), _seqCredits(NULL),
	_gameId(kGameBlue), _hasShownStartScreen(false), _hasShownIntro(false),
	_isShowingCredits(false), _isGameStarted(false), _isShowingMenu(false),
	_creditsSequenceIndex(0), _checkHotspotsTicks(15),  _mouseFlags(Common::EVENT_INVALID), _lastHotspot(NULL),
	_currentIndex(0), _currentTime(0), _lowerTime(0), _index(0), _index2(0), _time(0), _delta(0), _handleTimeDelta(false) {

	_clock = new Clock(_engine);
	_trainLine = new TrainLine(_engine);
}

Menu::~Menu() {
	delete _clock;
	delete _trainLine;

	SAFE_DELETE(_seqTooltips);
	SAFE_DELETE(_seqEggButtons);
	SAFE_DELETE(_seqButtons);
	SAFE_DELETE(_seqAcorn);
	SAFE_DELETE(_seqCity1);
	SAFE_DELETE(_seqCity2);
	SAFE_DELETE(_seqCity3);
	SAFE_DELETE(_seqCredits);

	_lastHotspot = NULL;

	// Zero passed pointers
	_engine = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Setup
void Menu::setup() {

	// Clear drawing queue
	getScenes()->removeAndRedraw(&_frames[kOverlayAcorn], false);
	SAFE_DELETE(_seqAcorn);

	// Load Menu scene
	// + 1 = normal menu with open egg / clock
	// + 2 = shield menu, when no savegame exists (no game has been started)
	_isGameStarted = _lowerTime >= kTimeStartGame;
	getScenes()->loadScene((SceneIndex)(_isGameStarted ? _gameId * 5 + 1 : _gameId * 5 + 2));
	getFlags()->shouldRedraw = true;
	getLogic()->updateCursor();

	//////////////////////////////////////////////////////////////////////////
	// Load Acorn sequence
	_seqAcorn = loadSequence(getAcornSequenceName(_isGameStarted ? getNextGameId() : kGameBlue));

	//////////////////////////////////////////////////////////////////////////
	// Check if we loaded sequences before
	if (_seqTooltips && _seqTooltips->count() > 0)
		return;

	// Load all static data
	_seqTooltips = loadSequence("helpnewr.seq");
	_seqEggButtons = loadSequence("buttns.seq");
	_seqButtons = loadSequence("quit.seq");
	_seqCity1 = loadSequence("jlinetl.seq");
	_seqCity2 = loadSequence("jlinecen.seq");
	_seqCity3 = loadSequence("jlinebr.seq");
	_seqCredits = loadSequence("credits.seq");

	_frames[kOverlayTooltip] = new SequenceFrame(_seqTooltips);
	_frames[kOverlayEggButtons] = new SequenceFrame(_seqEggButtons);
	_frames[kOverlayButtons] = new SequenceFrame(_seqButtons);
	_frames[kOverlayAcorn] = new SequenceFrame(_seqAcorn);
	_frames[kOverlayCity1] = new SequenceFrame(_seqCity1);
	_frames[kOverlayCity2] = new SequenceFrame(_seqCity2);
	_frames[kOverlayCity3] = new SequenceFrame(_seqCity3);
	_frames[kOverlayCredits] = new SequenceFrame(_seqCredits);
}

//////////////////////////////////////////////////////////////////////////
// Handle events
void Menu::eventMouse(const Common::Event &ev) {
	if (!getFlags()->shouldRedraw)
		return;

	bool redraw = true;
	getFlags()->shouldRedraw = false;

	// Update coordinates
	setCoords(ev.mouse);
	//_mouseFlags = (Common::EventType)(ev.type & Common::EVENT_LBUTTONUP);

	if (_isShowingCredits) {
		if (ev.type == Common::EVENT_RBUTTONUP) {
			showFrame(kOverlayCredits, -1, true);
			_isShowingCredits = false;
		}

		if (ev.type == Common::EVENT_LBUTTONUP) {
			// Last frame of the credits
			if (_seqCredits && _creditsSequenceIndex == _seqCredits->count() - 1) {
				showFrame(kOverlayCredits, -1, true);
				_isShowingCredits = false;
			} else {
				++_creditsSequenceIndex;
				showFrame(kOverlayCredits, _creditsSequenceIndex, true);
			}
		}
	} else {
		// Check for hotspots
		SceneHotspot *hotspot = NULL;
		getScenes()->get(getState()->scene)->checkHotSpot(ev.mouse, &hotspot);

		if (_lastHotspot != hotspot || ev.type == Common::EVENT_LBUTTONUP) {
			_lastHotspot = hotspot;

			if (ev.type == Common::EVENT_MOUSEMOVE) { /* todo check event type */
				if (!_handleTimeDelta && hasTimeDelta())
					setTime();
			}

			if (hotspot) {
				redraw = handleEvent((StartMenuAction)hotspot->action, ev.type);
				getFlags()->mouseRightClick = false;
				getFlags()->mouseLeftClick = false;
			} else {
				hideOverlays();
			}
		}
	}

	if (redraw) {
		getFlags()->shouldRedraw = true;
		askForRedraw();
	}
}

void Menu::eventTick(const Common::Event&) {
	if (hasTimeDelta())
		adjustTime();
	else if (_handleTimeDelta)
		_handleTimeDelta = false;

	// Check hotspots
	if (!--_checkHotspotsTicks) {
		checkHotspots();
		_checkHotspotsTicks = 15;
	}
}

//////////////////////////////////////////////////////////////////////////
// Show the intro and load the main menu scene
void Menu::show(bool doSavegame, SavegameType type, uint32 value) {

	if (_isShowingMenu)
		return;

	_isShowingMenu = true;
	getEntities()->reset();

	// If no blue savegame exists, this might be the first time we start the game, so we show the full intro
	if (!getFlags()->mouseRightClick) {
		if (!SaveLoad::isSavegameValid(kGameBlue) && _engine->getResourceManager()->loadArchive(kArchiveCd1)) {

			if (!_hasShownIntro) {
				// Show Broderbrund logo
				Animation animation;
				if (animation.load(getArchive("1930.nis")))
					animation.play();

				getFlags()->mouseRightClick = false;

				// Play intro music
				getSound()->playSoundWithSubtitles("MUS001.SND", SoundManager::kFlagMusic, kEntityPlayer);

				// Show The Smoking Car logo
				if (animation.load(getArchive("1931.nis")))
					animation.play();

				_hasShownIntro = true;
			}
		} else {
			// Only show the quick intro
			if (!_hasShownStartScreen) {
				getSound()->playSoundWithSubtitles("MUS018.SND", SoundManager::kFlagMusic, kEntityPlayer);
				getScenes()->loadScene(kSceneStartScreen);

				// Original game waits 60 frames and loops Sound::unknownFunction1 unless the right button is pressed
				uint32 nextFrameCount = getFrameCount() + 60;
				while (getFrameCount() < nextFrameCount) {
					_engine->pollEvents();

					if (getFlags()->mouseRightClick)
						break;

					getSound()->updateQueue();
				}
			}
		}
	}

	_hasShownStartScreen = true;

	// Init Menu
	init(doSavegame, type, value);

	// Setup sound
	getSound()->unknownFunction4();
	getSound()->resetQueue(SoundManager::kSoundType11, SoundManager::kSoundType13);
	if (getSound()->isBuffered("TIMER"))
		getSound()->removeFromQueue("TIMER");

	// Init flags & misc
	_isShowingCredits = false;
	_handleTimeDelta = hasTimeDelta();
	getInventory()->unselectItem();

	// Set Cursor type
	_engine->getCursor()->setStyle(kCursorNormal);
	_engine->getCursor()->show(true);

	setup();
	checkHotspots();

	// Set event handlers
	SET_EVENT_HANDLERS(Menu, this);
}

bool Menu::handleEvent(StartMenuAction action, Common::EventType type) {
	bool clicked = (type == Common::EVENT_LBUTTONUP);

	switch(action) {
	default:
		hideOverlays();
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuCredits:
		if (hasTimeDelta()) {
			hideOverlays();
			break;
		}

		if (clicked) {
			showFrame(kOverlayEggButtons, kButtonCreditsPushed, true);
			showFrame(kOverlayTooltip, -1, true);

			getSound()->playSound(kEntityPlayer, "LIB046");

			hideOverlays();

			_isShowingCredits = true;
			_creditsSequenceIndex = 0;

			showFrame(kOverlayCredits, 0, true);
		} else {
			// TODO check flags ?

			showFrame(kOverlayEggButtons, kButtonCredits, true);
			showFrame(kOverlayTooltip, kTooltipCredits, true);
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuQuitGame:
		showFrame(kOverlayTooltip, kTooltipQuit, true);

		if (clicked) {
			showFrame(kOverlayButtons, kButtonQuitPushed, true);

			getSound()->clearStatus();
			getSound()->updateQueue();
			getSound()->playSound(kEntityPlayer, "LIB046");

			// FIXME uncomment when sound queue is properly implemented
			/*while (getSound()->isBuffered("LIB046"))
				getSound()->updateQueue();*/

			getFlags()->shouldRedraw = false;

			Engine::quitGame();

			return false;
		} else {
			showFrame(kOverlayButtons, kButtonQuit, true);
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuCase4:
		if (clicked)
			_index = 0;
		// fall down to kMenuContinue

	//////////////////////////////////////////////////////////////////////////
	case kMenuContinue: {
		if (hasTimeDelta()) {
			hideOverlays();
			break;
		}

		// Determine the proper CD archive
		ArchiveIndex cd = kArchiveCd1;
		if (getProgress().chapter > kChapter1)
			cd = (getProgress().chapter > kChapter3) ? kArchiveCd3 : kArchiveCd2;

		// Show tooltips & buttons to start a game, continue a game or load the proper cd
		if (ResourceManager::isArchivePresent(cd)) {
			if (_isGameStarted) {
				showFrame(kOverlayEggButtons, kButtonContinue, true);

				if (_index2 == _index) {
					showFrame(kOverlayTooltip, isGameFinished() ? kTooltipViewGameEnding : kTooltipContinueGame, true);
				} else {
					showFrame(kOverlayTooltip, kTooltipContinueRewoundGame, true);
				}

			} else {
				showFrame(kOverlayEggButtons, kButtonShield, true);
				showFrame(kOverlayTooltip, kTooltipPlayNewGame, true);
			}
		} else {
			showFrame(kOverlayEggButtons, -1, true);
			showFrame(kOverlayTooltip, cd - 1, true);
		}

		if (!clicked)
			break;

		// Try loading the archive file
		if (!_engine->getResourceManager()->loadArchive(cd))
			break;

		// Load the train data file and setup game
		getScenes()->loadSceneDataFile(cd);
		showFrame(kOverlayTooltip, -1, true);
		getSound()->playSound(kEntityPlayer, "LIB046");

		// Setup new game
		getSavePoints()->reset();
		setLogicEventHandlers();

		getSound()->processEntry(SoundManager::kSoundType11);

		if (!getFlags()->mouseRightClick) {
			getScenes()->loadScene((SceneIndex)(5 * _gameId + 3));

			if (!getFlags()->mouseRightClick) {
				getScenes()->loadScene((SceneIndex)(5 * _gameId + 4));

				if (!getFlags()->mouseRightClick) {
					getScenes()->loadScene((SceneIndex)(5 * _gameId + 5));

					if (!getFlags()->mouseRightClick) {
						getSound()->processEntry(SoundManager::kSoundType11);

						// Show intro
						Animation animation;
						if (animation.load(getArchive("1601.nis")))
							animation.play();

						getEvent(kEventIntro) = 1;
					}
				}
			}
		}

		if (!getEvent(kEventIntro))	{
			getEvent(kEventIntro) = 1;

			getSound()->processEntry(SoundManager::kSoundType11);
		}

		// Setup game
		getFlags()->isGameRunning = true;
		startGame();

		if (!_isShowingMenu)
			getInventory()->show();

		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	case kMenuSwitchSaveGame:
		if (hasTimeDelta()) {
			hideOverlays();
			break;
		}

		if (clicked) {
			showFrame(kOverlayAcorn, 1, true);
			showFrame(kOverlayTooltip, -1, true);
			getSound()->playSound(kEntityPlayer, "LIB047");

			// Setup new menu screen
			switchGame();
			setup();

			// Set fight state to 0
			getFight()->resetState();

			return true;
		}

		// TODO Check for flag

		showFrame(kOverlayAcorn, 0, true);

		if (_isGameStarted) {
			showFrame(kOverlayTooltip, kTooltipSwitchBlueGame, true);
			break;
		}

		if (_gameId == kGameGold) {
			showFrame(kOverlayTooltip, kTooltipSwitchBlueGame, true);
			break;
		}

		if (!SaveLoad::isSavegameValid(getNextGameId())) {
			showFrame(kOverlayTooltip, kTooltipStartAnotherGame, true);
			break;
		}

		// Stupid tooltips ids are not in order, so we can't just increment them...
		switch(_gameId) {
		default:
			break;

		case kGameBlue:
			showFrame(kOverlayTooltip, kTooltipSwitchRedGame, true);
			break;

		case kGameRed:
			showFrame(kOverlayTooltip, kTooltipSwitchGreenGame, true);
			break;

		case kGameGreen:
			showFrame(kOverlayTooltip, kTooltipSwitchPurpleGame, true);
			break;

		case kGamePurple:
			showFrame(kOverlayTooltip, kTooltipSwitchTealGame, true);
			break;

		case kGameTeal:
			showFrame(kOverlayTooltip, kTooltipSwitchGoldGame, true);
			break;
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuRewindGame:
		if (!_index || _currentTime < _time) {
			hideOverlays();
			break;
		}

		if (clicked) {
			if (hasTimeDelta())
				_handleTimeDelta = false;

			showFrame(kOverlayEggButtons, kButtonRewindPushed, true);
			showFrame(kOverlayTooltip, -1, true);

			getSound()->playSound(kEntityPlayer, "LIB046");

			rewindTime();

			_handleTimeDelta = false;
		} else {
			showFrame(kOverlayEggButtons, kButtonRewind, true);
			showFrame(kOverlayTooltip, kTooltipRewind, true);
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuForwardGame:
		if (_index2 <= _index || _currentTime > _time) {
			hideOverlays();
			break;
		}

		if (clicked) {
			if (hasTimeDelta())
				_handleTimeDelta = false;

			showFrame(kOverlayEggButtons, kButtonForwardPushed, true);
			showFrame(kOverlayTooltip, -1, true);

			getSound()->playSound(kEntityPlayer, "LIB046");

			forwardTime();

			_handleTimeDelta = false;
		} else {
			showFrame(kOverlayEggButtons, kButtonForward, true);
			showFrame(kOverlayTooltip, kTooltipFastForward, true);
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuParis:
		moveToCity(kParis, clicked);
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuStrasBourg:
		moveToCity(kStrasbourg, clicked);
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuMunich:
		moveToCity(kMunich, clicked);
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuVienna:
		moveToCity(kVienna, clicked);
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuBudapest:
		moveToCity(kBudapest, clicked);
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuBelgrade:
		moveToCity(kBelgrade, clicked);
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuConstantinople:
		moveToCity(kConstantinople, clicked);
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuDecreaseVolume:
		if (hasTimeDelta()) {
			hideOverlays();
			break;
		}

		// Cannot decrease volume further
		if (getVolume() == 0) {
			showFrame(kOverlayButtons, kButtonVolume, true);
			showFrame(kOverlayTooltip, -1, true);
			break;
		}

		showFrame(kOverlayTooltip, kTooltipVolumeDown, true);

		// Show highlight on button & adjust volume if needed
		if (clicked) {
			showFrame(kOverlayButtons, kButtonVolumeDownPushed, true);
			getSound()->playSound(kEntityPlayer, "LIB046");
			setVolume(getVolume() - 1);

			getSaveLoad()->saveVolumeBrightness();

			uint32 nextFrameCount = getFrameCount() + 15;
			while (nextFrameCount > getFrameCount()) {
				_engine->pollEvents();

				getSound()->updateQueue();
			}
		} else {
			showFrame(kOverlayButtons, kButtonVolumeDown, true);
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuIncreaseVolume:
		if (hasTimeDelta()) {
			hideOverlays();
			break;
		}

		// Cannot increase volume further
		if (getVolume() >= 7) {
			showFrame(kOverlayButtons, kButtonVolume, true);
			showFrame(kOverlayTooltip, -1, true);
			break;
		}

		showFrame(kOverlayTooltip, kTooltipVolumeUp, true);

		// Show highlight on button & adjust volume if needed
		if (clicked) {
			showFrame(kOverlayButtons, kButtonVolumeUpPushed, true);
			getSound()->playSound(kEntityPlayer, "LIB046");
			setVolume(getVolume() + 1);

			getSaveLoad()->saveVolumeBrightness();

			uint32 nextFrameCount = getFrameCount() + 15;
			while (nextFrameCount > getFrameCount()) {
				_engine->pollEvents();

				getSound()->updateQueue();
			}
		} else {
			showFrame(kOverlayButtons, kButtonVolumeUp, true);
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuDecreaseBrightness:
		if (hasTimeDelta()) {
			hideOverlays();
			break;
		}

		// Cannot increase brightness further
		if (getBrightness() == 0) {
			showFrame(kOverlayButtons, kButtonBrightness, true);
			showFrame(kOverlayTooltip, -1, true);
			break;
		}

		showFrame(kOverlayTooltip, kTooltipBrightnessDown, true);

		// Show highlight on button & adjust brightness if needed
		if (clicked) {
			showFrame(kOverlayButtons, kButtonBrightnessDownPushed, true);
			getSound()->playSound(kEntityPlayer, "LIB046");
			setBrightness(getBrightness() - 1);

			getSaveLoad()->saveVolumeBrightness();

			// Reshow the background and frames (they will pick up the new brightness through the GraphicsManager)
			_engine->getGraphicsManager()->draw(getScenes()->get((SceneIndex)(_isGameStarted ? _gameId * 5 + 1 : _gameId * 5 + 2)), GraphicsManager::kBackgroundC, true);
			showFrame(kOverlayTooltip, kTooltipBrightnessDown, false);
			showFrame(kOverlayButtons, kButtonBrightnessDownPushed, false);
		} else {
			showFrame(kOverlayButtons, kButtonBrightnessDown, true);
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuIncreaseBrightness:
		if (hasTimeDelta()) {
			hideOverlays();
			break;
		}

		// Cannot increase brightness further
		if (getBrightness() >= 6) {
			showFrame(kOverlayButtons, kButtonBrightness, true);
			showFrame(kOverlayTooltip, -1, true);
			break;
		}

		showFrame(kOverlayTooltip, kTooltipBrightnessUp, true);

		// Show highlight on button & adjust brightness if needed
		if (clicked) {
			showFrame(kOverlayButtons, kButtonBrightnessUpPushed, true);
			getSound()->playSound(kEntityPlayer, "LIB046");
			setBrightness(getBrightness() + 1);

			getSaveLoad()->saveVolumeBrightness();

			// Reshow the background and frames (they will pick up the new brightness through the GraphicsManager)
			_engine->getGraphicsManager()->draw(getScenes()->get((SceneIndex)(_isGameStarted ? _gameId * 5 + 1 : _gameId * 5 + 2)), GraphicsManager::kBackgroundC, true);
			showFrame(kOverlayTooltip, kTooltipBrightnessUp, false);
			showFrame(kOverlayButtons, kButtonBrightnessUpPushed, false);
		} else {
			showFrame(kOverlayButtons, kButtonBrightnessUp, true);
		}
		break;
	}

	return true;
}

void Menu::setLogicEventHandlers() {
	SET_EVENT_HANDLERS(Logic, getLogic());
	clear();
	_isShowingMenu = false;
}

//////////////////////////////////////////////////////////////////////////
// Game-related
//////////////////////////////////////////////////////////////////////////
void Menu::init(bool doSavegame, SavegameType type, uint32 value) {

	bool useSameIndex = true;

	if (getGlobalTimer()) {
		value = 0;

		// Check if the CD file is present
		ArchiveIndex index = kArchiveCd1;
		switch (getProgress().chapter) {
		default:
		case kChapter1:
			break;

		case kChapter2:
		case kChapter3:
			index = kArchiveCd2;
			break;

		case kChapter4:
		case kChapter5:
			index = kArchiveCd3;
			break;
		}

		if (ResourceManager::isArchivePresent(index)) {
			setGlobalTimer(0);
			useSameIndex = false;

			// TODO remove existing savegame and reset index & savegame name
			warning("Menu::initGame: not implemented!");
		}

		doSavegame = false;
	} else {
		// TODO rename saves?
	}

	// Create a new savegame if needed
	if (!SaveLoad::isSavegamePresent(_gameId))
		SaveLoad::writeMainHeader(_gameId);

	if (doSavegame)
		getSaveLoad()->saveGame(kSavegameTypeEvent2, kEntityPlayer, kEventNone);

	if (!getGlobalTimer()) {
		// TODO: remove existing savegame temp file
	}

	// Init savegame and get the header data
	getSaveLoad()->initSavegame(_gameId, true);
	SaveLoad::SavegameMainHeader header;
	if (!SaveLoad::loadMainHeader(_gameId, &header))
		error("Menu::init: Corrupted savegame - Recovery path not implemented!");

	// Init Menu values
	_index2 = header.index;
	_lowerTime = getSaveLoad()->getEntry(_index2)->time;

	if (useSameIndex)
		_index = _index2;

	//if (!getGlobalTimer())
	//	_index3 = 0;

	if (!getProgress().chapter)
		getProgress().chapter = kChapter1;

	getState()->time = getSaveLoad()->getEntry(_index)->time;
	getProgress().chapter = getSaveLoad()->getEntry(_index)->chapter;

	if (_lowerTime >= kTimeStartGame) {
		_currentTime = getState()->time;
		_time = getState()->time;
		_clock->draw(_time);
		_trainLine->draw(_time);

		initTime(type, value);
	}
}

void Menu::startGame() {
	// TODO: we need to reset the current scene
	getState()->scene = kSceneDefault;

	getEntities()->setup(true, kEntityPlayer);
	warning("Menu::startGame: not implemented!");
}

// Switch to the next savegame
void Menu::switchGame() {

	// Switch back to blue game is the current game is not started
	_gameId = SaveLoad::isSavegameValid(_gameId) ? getNextGameId() : kGameBlue;

	// Initialize savegame if needed
	if (!SaveLoad::isSavegamePresent(_gameId))
		SaveLoad::writeMainHeader(_gameId);

	getState()->time = 0;

	// Clear menu elements
	_clock->clear();
	_trainLine->clear();

	// Clear loaded savegame data
	getSaveLoad()->clearEntries();

	init(false, kSavegameTypeIndex, 0);
}

bool Menu::isGameFinished() const {
	SaveLoad::SavegameEntryHeader *data = getSaveLoad()->getEntry(_index);

	if (_index2 != _index)
		return false;

	if (data->type != SaveLoad::kHeaderType2)
		return false;

	return (data->event == kEventAnnaKilled
		 || data->event == kEventKronosHostageAnnaNoFirebird
		 || data->event == kEventKahinaPunchBaggageCarEntrance
		 || data->event == kEventKahinaPunchBlue
		 || data->event == kEventKahinaPunchYellow
		 || data->event == kEventKahinaPunchSalon
		 || data->event == kEventKahinaPunchKitchen
		 || data->event == kEventKahinaPunchBaggageCar
		 || data->event == kEventKahinaPunchCar
		 || data->event == kEventKahinaPunchSuite4
		 || data->event == kEventKahinaPunchRestaurant
		 || data->event == kEventKahinaPunch
		 || data->event == kEventKronosGiveFirebird
		 || data->event == kEventAugustFindCorpse
		 || data->event == kEventMertensBloodJacket
		 || data->event == kEventMertensCorpseFloor
		 || data->event == kEventMertensCorpseBed
		 || data->event == kEventCoudertBloodJacket
		 || data->event == kEventGendarmesArrestation
		 || data->event == kEventAbbotDrinkGiveDetonator
		 || data->event == kEventMilosCorpseFloor
		 || data->event == kEventLocomotiveAnnaStopsTrain
		 || data->event == kEventTrainStopped
		 || data->event == kEventCathVesnaRestaurantKilled
		 || data->event == kEventCathVesnaTrainTopKilled
		 || data->event == kEventLocomotiveConductorsDiscovered
		 || data->event == kEventViennaAugustUnloadGuns
		 || data->event == kEventViennaKronosFirebird
		 || data->event == kEventVergesAnnaDead
		 || data->event == kEventTrainExplosionBridge
		 || data->event == kEventKronosBringNothing);
}

//////////////////////////////////////////////////////////////////////////
// Overlays & elements
//////////////////////////////////////////////////////////////////////////
void Menu::checkHotspots() {
	if (!_isShowingMenu)
		return;

	if (!getFlags()->shouldRedraw)
		return;

	if (_isShowingCredits)
		return;

	SceneHotspot *hotspot = NULL;
	getScenes()->get(getState()->scene)->checkHotSpot(getCoords(), &hotspot);

	if (hotspot)
		handleEvent((StartMenuAction)hotspot->action, _mouseFlags);
	else
		hideOverlays();
}

void Menu::hideOverlays() {
	_lastHotspot = NULL;

	// Hide all menu overlays
	for (MenuFrames::iterator it = _frames.begin(); it != _frames.end(); it++)
		showFrame(it->_key, -1, false);

	getScenes()->drawFrames(true);
}

void Menu::showFrame(StartMenuOverlay overlayType, int index, bool redraw) {
	if (index == -1) {
		getScenes()->removeFromQueue(_frames[overlayType]);
	} else {
		// Check that the overlay is valid
		if (!_frames[overlayType])
			return;

		// Remove the frame and add a new one with the proper index
		getScenes()->removeFromQueue(_frames[overlayType]);
		_frames[overlayType]->setFrame((uint16)index);
		getScenes()->addToQueue(_frames[overlayType]);
	}

	if (redraw)
		getScenes()->drawFrames(true);
}

// Remove all frames from the queue
void Menu::clear() {
	for (MenuFrames::iterator it = _frames.begin(); it != _frames.end(); it++)
		getScenes()->removeAndRedraw(&it->_value, false);

	clearBg(GraphicsManager::kBackgroundOverlay);
}

// Get the sequence name to use for the acorn highlight, depending of the currently loaded savegame
Common::String Menu::getAcornSequenceName(GameId id) const {
	Common::String name = "";
	switch (id) {
	default:
	case kGameBlue:
		name = "aconblu3.seq";
		break;

	case kGameRed:
		name = "aconred.seq";
		break;

	case kGameGreen:
		name = "acongren.seq";
		break;

	case kGamePurple:
		name = "aconpurp.seq";
		break;

	case kGameTeal:
		name = "aconteal.seq";
		break;

	case kGameGold:
		name = "acongold.seq";
		break;
	}

	return name;
}

//////////////////////////////////////////////////////////////////////////
// Time
//////////////////////////////////////////////////////////////////////////
void Menu::initTime(SavegameType type, uint32 value) {
	if (!value)
		return;

	// The savegame entry index
	uint32 entryIndex = 0;

	switch (type) {
	default:
		break;

	case kSavegameTypeIndex:
		entryIndex = (_index <= value) ? 1 : _index - value;
		break;

	case kSavegameTypeTime:
		if (value < kTimeStartGame)
			break;

		entryIndex = _index;
		if (!entryIndex)
			break;

		// Iterate through existing entries
		do {
			if (getSaveLoad()->getEntry(entryIndex)->time <= value)
				break;

			entryIndex--;
		} while (entryIndex);
		break;

	case kSavegameTypeEvent:
		entryIndex = _index;
		if (!entryIndex)
			break;

		do {
			if (getSaveLoad()->getEntry(entryIndex)->event == (EventIndex)value)
				break;

			entryIndex--;
		} while (entryIndex);
		break;

	case kSavegameTypeEvent2:
		// TODO rewrite in a more legible way
		if (_index > 1) {
			uint32 index = _index;
			do {
				if (getSaveLoad()->getEntry(index)->event == (EventIndex)value)
					break;

				index--;
			} while (index > 1);

			entryIndex = index - 1;
		} else {
			entryIndex = _index - 1;
		}
		break;
	}

	if (entryIndex) {
		_currentIndex = entryIndex;
		updateTime(getSaveLoad()->getEntry(entryIndex)->time);
	}
}

void Menu::updateTime(uint32 time) {
	if (_currentTime == _time)
		_delta = 0;

	_currentTime = time;

	if (_time != time) {
		if (getSound()->isBuffered(kEntityChapters))
			getSound()->removeFromQueue(kEntityChapters);

		getSound()->playSoundWithSubtitles((_currentTime >= _time) ? "LIB042" : "LIB041", SoundManager::kFlagMenuClock, kEntityChapters);
		adjustIndex(_currentTime, _time, false);
	}
}

void Menu::adjustIndex(uint32 time1, uint32 time2, bool searchEntry) {
	uint32 index = 0;
	int32 timeDelta = -1;

	if (time1 != time2) {

		index = _index;

		if (time2 >= time1) {
			if (searchEntry) {
				uint32 currentIndex = _index;

				if ((int32)_index >= 0) {
					do {
						// Calculate new delta
						int32 newDelta = time1 - getSaveLoad()->getEntry(currentIndex)->time;

						if (newDelta >= 0 && timeDelta >= newDelta) {
							timeDelta = newDelta;
							index = currentIndex;
						}

						--currentIndex;
					} while ((int32)currentIndex >= 0);
				}
			} else {
				index = _index - 1;
			}
		} else {
			if (searchEntry) {
				uint32 currentIndex = _index;

				if (_index2 >= _index) {
					do {
						// Calculate new delta
						int32 newDelta = getSaveLoad()->getEntry(currentIndex)->time - time1;

						if (newDelta >= 0 && timeDelta > newDelta) {
							timeDelta = newDelta;
							index = currentIndex;
						}

						++currentIndex;
					} while (currentIndex >= _index2);
				}
			} else {
				index = _index + 1;
			}
		}

		_index = index;
		checkHotspots();
	}

	if (_index == _currentIndex) {
		if (getProgress().chapter != getSaveLoad()->getEntry(index)->chapter)
			getProgress().chapter = getSaveLoad()->getEntry(_index)->chapter;
	}
}

void Menu::goToTime(uint32 time) {

	uint32 entryIndex = 0;
	uint32 deltaTime = (uint32)ABS((int32)(getSaveLoad()->getEntry(0)->time - time));
	uint32 index = 0;

	do {
		uint32 deltaTime2 = (uint32)ABS((int32)(getSaveLoad()->getEntry(index)->time - time));
		if (deltaTime2 < deltaTime) {
			deltaTime = deltaTime2;
			entryIndex = index;
		}

		++index;
	} while (_index2 >= index);

	_currentIndex = entryIndex;
	updateTime(getSaveLoad()->getEntry(entryIndex)->time);
}

void Menu::setTime() {
	_currentIndex = _index;
	_currentTime = getSaveLoad()->getEntry(_currentIndex)->time;

	if (_time == _currentTime)
		adjustTime();
}

void Menu::forwardTime() {
	if (_index2 <= _index)
		return;

	_currentIndex = _index2;
	updateTime(getSaveLoad()->getEntry(_currentIndex)->time);
}

void Menu::rewindTime() {
	if (!_index)
		return;

	_currentIndex = 0;
	updateTime(getSaveLoad()->getEntry(_currentIndex)->time);
}

void Menu::adjustTime() {
	uint32 originalTime = _time;

	// Adjust time delta
	uint32 timeDelta = (_delta >= 90) ? 9 : (9 * _delta + 89) / 90;

	if (_currentTime < _time) {
		_time -= 900 * timeDelta;

		if (_time >= _currentTime)
			_time = _currentTime;
	} else {
		_time += 900 * timeDelta;

		if (_time < _currentTime)
			_time = _currentTime;
	}

	if (_currentTime == _time && getSound()->isBuffered(kEntityChapters))
		getSound()->removeFromQueue(kEntityChapters);

	_clock->draw(_time);
	_trainLine->draw(_time);
	getScenes()->drawFrames(true);

	adjustIndex(_time, originalTime, true);

	++_delta;
}

void Menu::moveToCity(CityButton city, bool clicked) {
	uint32 time = (uint32)_cityButtonsInfo[city].time;

	// TODO Check if we have access (there seems to be more checks on some internal times) - probably : current_time (menu only) / game time / some other?
	if (_lowerTime < time || _time == time || _currentTime == time) {
		hideOverlays();
		return;
	}

	// Show city overlay
	showFrame((StartMenuOverlay)((_cityButtonsInfo[city].index >> 6) + 3), _cityButtonsInfo[city].index & 63, true);

	if (clicked) {
		showFrame(kOverlayTooltip, -1, true);
		getSound()->playSound(kEntityPlayer, "LIB046");
		goToTime(time);

		_handleTimeDelta = true;

		return;
	}

	// Special case of first and last cities
	if (city == kParis || city == kConstantinople) {
		showFrame(kOverlayTooltip, (city == kParis) ? kTooltipRewindParis : kTooltipForwardConstantinople, true);
		return;
	}

	showFrame(kOverlayTooltip, (_time <= time) ? _cityButtonsInfo[city].forward : _cityButtonsInfo[city].rewind, true);
}

//////////////////////////////////////////////////////////////////////////
// Sound / Brightness
//////////////////////////////////////////////////////////////////////////

// Get current volume (converted internal ScummVM value)
uint32 Menu::getVolume() const {
	return getState()->volume;
}

// Set the volume (converts to ScummVM values)
void Menu::setVolume(uint32 volume) const {
	getState()->volume = volume;

	// Clamp volume
	uint32 value = volume * Audio::Mixer::kMaxMixerVolume / 7;

	if (value > Audio::Mixer::kMaxMixerVolume)
		value = Audio::Mixer::kMaxMixerVolume;

	_engine->_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, (int32)value);
}

uint32 Menu::getBrightness() const {
	return getState()->brightness;
}

void Menu::setBrightness(uint32 brightness) const {
	getState()->brightness = brightness;

	// TODO reload cursor & font with adjusted brightness
}

} // End of namespace LastExpress
