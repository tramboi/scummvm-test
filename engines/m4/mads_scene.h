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

#ifndef M4_MADS_SCENE_H
#define M4_MADS_SCENE_H

#include "m4/scene.h"
#include "m4/mads_logic.h"
#include "m4/mads_views.h"

namespace M4 {

#define INTERFACE_HEIGHT 106
class MadsInterfaceView;

#define DEPTH_BANDS_SIZE 15

class SceneNode {
public:
	Common::Point pt;

	void load(Common::SeekableReadStream *stream);
};

class MadsSceneResources: public SceneResources {
public:
	int _sceneId;
	int _artFileNum;
	int _depthStyle;
	int _width;
	int _height;
	Common::Array<SceneNode> _nodes;
	Common::Array<Common::String> _setNames;
	int _yBandsStart, _yBandsEnd;
	int _maxScale, _minScale;
	int _depthBands[DEPTH_BANDS_SIZE];

	MadsSceneResources() {}
	~MadsSceneResources() {}
	void load(int sceneId, const char *resName, int v0, M4Surface *depthSurface, M4Surface *surface);
	int bandsRange() const { return _yBandsEnd - _yBandsStart; }
	int scaleRange() const { return _maxScale - _minScale; }
	void setRouteNode(int nodeIndex, const Common::Point &pt, M4Surface *depthSurface);
};

enum MadsActionMode {ACTMODE_NONE = 0, ACTMODE_VERB = 1, ACTMODE_OBJECT = 3, ACTMODE_TALK = 6};
enum MAdsActionMode2 {ACTMODE2_0 = 0, ACTMODE2_2 = 2, ACTMODE2_5 = 5};

class MadsAction {
private:
	char _statusText[100];

	void appendVocab(int vocabId, bool capitalise = false);
public:
	int _currentHotspot;
	int _objectNameId;
	int _objectDescId;
	int _currentAction;
	int8 _flags1, _flags2;
	MadsActionMode _actionMode;
	MAdsActionMode2 _actionMode2;
	int _articleNumber;
	bool _lookFlag;
	int _selectedRow;
	// Unknown fields
	int16 _word_86F3A;
	int16 _word_86F42;
	int16 _word_86F4E;
	int16 _word_86F4A;
	int16 _word_83334;
	int16 _word_86F4C;

public:
	MadsAction();

	void clear();
	void set();
	const char *statusText() const { return _statusText; }
};

class MadsScene : public Scene, public MadsView {
private:
	MadsEngine *_vm;
	MadsSceneResources _sceneResources;
	MadsAction _action;
	Animation *_activeAnimation;

	MadsSceneLogic _sceneLogic;
	SpriteAsset *_playerSprites;

	void drawElements();
	void loadScene2(const char *aaName, int sceneNumber);
	void loadSceneTemporary();
	void loadSceneHotspots(int sceneNumber);
	void clearAction();
	void appendActionVocab(int vocabId, bool capitalise);
	void setAction();
public:
	char _aaName[100];
public:
	MadsScene(MadsEngine *vm);
	virtual ~MadsScene();

	// Methods that differ between engines
	virtual void loadScene(int sceneNumber);
	virtual void leaveScene();
	virtual void loadSceneCodes(int sceneNumber, int index = 0);
	virtual void show();
	virtual void checkHotspotAtMousePos(int x, int y);
	virtual void leftClick(int x, int y);
	virtual void rightClick(int x, int y);
	virtual void setAction(int action, int objectId = -1);
	virtual void update();

	virtual void updateState();

	int loadSceneSpriteSet(const char *setName);
	void showMADSV2TextBox(char *text, int x, int y, char *faceName);
	void loadAnimation(const Common::String &animName, int abortTimers);
	Animation *activeAnimation() const { return _activeAnimation; }
	void freeAnimation();

	MadsInterfaceView *getInterface() { return (MadsInterfaceView *)_interfaceSurface; }
	MadsSceneResources &getSceneResources() { return _sceneResources; }
	MadsAction &getAction() { return _action; }
	void setStatusText(const char *text) {}//***DEPRECATED***
	bool getDepthHighBit(const Common::Point &pt);
	bool getDepthHighBits(const Common::Point &pt);
};

#define CHEAT_SEQUENCE_MAX 8

class IntegerList : public Common::Array<int> {
public:
	int indexOf(int v) {
		for (uint i = 0; i < size(); ++i)
			if (operator [](i) == v)
				return i;
		return -1;
	}
};

enum InterfaceFontMode {ITEM_NORMAL, ITEM_HIGHLIGHTED, ITEM_SELECTED};

enum InterfaceObjects {ACTIONS_START = 0, SCROLL_UP = 10, SCROLL_SCROLLER = 11, SCROLL_DOWN = 12,
		INVLIST_START = 13, VOCAB_START = 18};

class MadsInterfaceView : public GameInterfaceView {
private:
	IntegerList _inventoryList;
	RectList _screenObjects;
	int _highlightedElement;
	int _topIndex;
	uint32 _nextScrollerTicks;
	int _cheatKeyCtr;

	// Object display fields
	int _selectedObject;
	SpriteAsset *_objectSprites;
	RGBList *_objectPalData;
	int _objectFrameNumber;

	void setFontMode(InterfaceFontMode newMode);
	bool handleCheatKey(int32 keycode);
	bool handleKeypress(int32 keycode);
	void leaveScene();
public:
	MadsInterfaceView(MadsM4Engine *vm);
	~MadsInterfaceView();

	virtual void initialise();
	virtual void setSelectedObject(int objectNumber);
	virtual void addObjectToInventory(int objectNumber);
	int getSelectedObject() { return _selectedObject; }
	int getInventoryObject(int objectIndex) { return _inventoryList[objectIndex]; }

	void onRefresh(RectList *rects, M4Surface *destSurface);
	bool onEvent(M4EventType eventType, int32 param1, int x, int y, bool &captureEvents);
};

} // End of namespace M4

#endif
