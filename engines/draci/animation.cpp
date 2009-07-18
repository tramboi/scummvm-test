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

#include "draci/draci.h"
#include "draci/animation.h"

namespace Draci {

Animation::Animation(DraciEngine *vm) : _vm(vm) {
	_id = kUnused;
	_z = 0;
	_relX = 0;
	_relY = 0;
	setPlaying(false);
	_looping = false;
	_tick = _vm->_system->getMillis();
	_currentFrame = 0;
}	

Animation::~Animation() {
	deleteFrames();
}

bool Animation::isLooping() {
	return _looping;
}

void Animation::setRelative(int relx, int rely) {

	// Delete the previous frame
	Common::Rect frameRect = _frames[_currentFrame]->getRect();
	frameRect.translate(_relX, _relY);
	_vm->_screen->getSurface()->markDirtyRect(frameRect);

	_relX = relx;
	_relY = rely;
}

void Animation::setLooping(bool looping) {
	_looping = looping;
	debugC(7, kDraciAnimationDebugLevel, "Setting looping to %d on animation %d", 
		looping, _id);
}

void Animation::nextFrame(bool force) {

	// If there's only one or no frames, or if the animation is not playing, return
	if (getFramesNum() < 2 || !_playing)
		return;

	Drawable *frame = _frames[_currentFrame];

	Common::Rect frameRect = frame->getRect();
	
	// Translate rectangle to compensate for relative coordinates	
	frameRect.translate(_relX, _relY);

	if (force || (_tick + frame->getDelay() <= _vm->_system->getMillis())) {
		// If we are at the last frame and not looping, stop the animation
		// The animation is also restarted to frame zero
		if ((_currentFrame == getFramesNum() - 1) && !_looping) {
			_currentFrame = 0;
			setPlaying(false);
		} else {
			_vm->_screen->getSurface()->markDirtyRect(frameRect);
			_currentFrame = nextFrameNum();
			_tick += frame->getDelay();
		}
	}

	debugC(6, kDraciAnimationDebugLevel, 
	"anim=%d tick=%d delay=%d tick+delay=%d currenttime=%d frame=%d framenum=%d", 
	_id, _tick, frame->getDelay(), _tick + frame->getDelay(), _vm->_system->getMillis(), 
	_currentFrame, _frames.size());
}

uint Animation::nextFrameNum() {

	if ((_currentFrame == getFramesNum() - 1) && _looping)
		return 0;
	else
		return _currentFrame + 1;
}

void Animation::drawFrame(Surface *surface) {
	
	// If there are no frames or the animation is not playing, return
	if (_frames.size() == 0 || !_playing)
		return;

	if (_id == kOverlayImage) {			
		_frames[_currentFrame]->draw(surface, false);
	}
	else {
		Drawable *ptr = _frames[_currentFrame];
		int x = ptr->getX();
		int y = ptr->getY();
		int newX = x + _relX;
		int newY = y + _relY;

		ptr->setX(newX);
		ptr->setY(newY);
		ptr->draw(surface, true);

		ptr->setX(x);
		ptr->setY(y);
	}
}

void Animation::setID(int id) {
	_id = id;
}

int Animation::getID() {
	return _id;
}

void Animation::setZ(uint z) {
	_z = z;
}

uint Animation::getZ() {
	return _z;
}

bool Animation::isPlaying() {
	return _playing;
}

void Animation::setPlaying(bool playing) {
	_tick = _vm->_system->getMillis();
	_playing = playing;
}

void Animation::addFrame(Drawable *frame) {
	_frames.push_back(frame);	
}

Drawable *Animation::getFrame(int frameNum) {

	// If no argument is passed, return the current frame
	if (frameNum == kCurrentFrame) {
		return _frames[_currentFrame];
	} else {
		return _frames[frameNum];
	}
}

uint Animation::getFramesNum() {
	return _frames.size();
}

void Animation::deleteFrames() {
	
	for (int i = getFramesNum() - 1; i >= 0; --i) {		
		delete _frames[i];
		_frames.pop_back();	
	}
}

Animation *AnimationManager::addAnimation(int id, uint z, bool playing) {
	
	Animation *anim = new Animation(_vm);
	
	anim->setID(id);
	anim->setZ(z);
	anim->setPlaying(playing);
	anim->setLooping(false);

	insertAnimation(anim);

	return anim;
}

void AnimationManager::play(int id) {
	Animation *anim = getAnimation(id);

	if (anim) {
		anim->setPlaying(true);

		debugC(5, kDraciAnimationDebugLevel, "Playing animation %d...", id);
	}
}

void AnimationManager::stop(int id) {
	Animation *anim = getAnimation(id);
	
	// Clean up the last frame that was drawn before stopping
	Common::Rect frameRect = anim->getFrame()->getRect();
	frameRect.translate(anim->_relX, anim->_relY);
	_vm->_screen->getSurface()->markDirtyRect(frameRect);

	if (anim) {
		anim->setPlaying(false);
	
		debugC(5, kDraciAnimationDebugLevel, "Stopping animation %d...", id);
	}
}

Animation *AnimationManager::getAnimation(int id) {
	
	Common::List<Animation *>::iterator it;

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if ((*it)->getID() == id) {
			return *it;
		}
	}

	return NULL;
}

void AnimationManager::insertAnimation(Animation *animObj) {
	
	Common::List<Animation *>::iterator it;	

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if (animObj->getZ() < (*it)->getZ()) 
			break;
	}

	_animations.insert(it, animObj);
}

void AnimationManager::addOverlay(Drawable *overlay, uint z) {
	Animation *anim = new Animation(_vm);
	anim->setID(kOverlayImage);
	anim->setZ(z);
	anim->setPlaying(true);
	anim->addFrame(overlay);

	insertAnimation(anim);
}

void AnimationManager::drawScene(Surface *surf) {

	// Fill the screen with colour zero since some rooms may rely on the screen being black	
	_vm->_screen->getSurface()->fill(0);

	sortAnimations();

	Common::List<Animation *>::iterator it;

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if (! ((*it)->isPlaying()) ) {
			continue;
		}	
	
		(*it)->nextFrame();
		(*it)->drawFrame(surf);
	}
}

void AnimationManager::sortAnimations() {
	Common::List<Animation *>::iterator cur;
	Common::List<Animation *>::iterator next;

	cur = _animations.begin();

	// If the list is empty, we're done
	if (cur == _animations.end())
		return;	

	while(1) {
		next = cur;
		next++;

		// If we are at the last element, we're done
		if (next == _animations.end())
			break;

		// If we find an animation out of order, reinsert it
		if ((*next)->getZ() < (*cur)->getZ()) {

			Animation *anim = *cur;
			_animations.erase(cur);

			insertAnimation(anim);
		}

		// Advance to next animation
		cur = next;
	}
}

void AnimationManager::deleteAnimation(int id) {
	
	Common::List<Animation *>::iterator it;
  
	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if ((*it)->getID() == id) {
			(*it)->deleteFrames();
			_animations.erase(it);
			break;
		}
	}
}

void AnimationManager::deleteOverlays() {
	
	Common::List<Animation *>::iterator it;

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if ((*it)->getID() == kOverlayImage) {
			(*it)->deleteFrames();
			_animations.erase(it);
		}	
	}
}

void AnimationManager::deleteAll() {
	
	Common::List<Animation *>::iterator it;

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		(*it)->deleteFrames();	
	}

	_animations.clear();
}

}
