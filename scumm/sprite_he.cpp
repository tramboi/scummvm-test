/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#include "scumm/intern.h"
#include "scumm/resource.h"
#include "scumm/saveload.h"
#include "scumm/scumm.h"
#include "scumm/sprite_he.h"
#include "scumm/usage_bits.h"
#include "scumm/util.h"
#include "scumm/wiz_he.h"

namespace Scumm {

Sprite::Sprite(ScummEngine_v90he *vm) : _vm(vm) {
}

void ScummEngine_v90he::allocateArrays() {
	ScummEngine::allocateArrays();
	_sprite->allocTables(_numSprites, MAX(64, _numSprites / 4), 64);
}

void Sprite::getSpriteBounds(int spriteId, bool checkGroup, Common::Rect &bound) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");
	int32 spr_wiz_x, spr_wiz_y;
	int angle, scale, x1, y1;
	int32 w, h;

	SpriteInfo *spi = &_spriteTable[spriteId];

	_vm->_wiz->loadImgSpot(spi->image, spi->imageState, spr_wiz_x, spr_wiz_y);
	if (checkGroup && spi->group) {
		SpriteGroup *spg = &_spriteGroups[spi->group];

		if (spg->scaling) {
			x1 = spi->tx * spg->scale_x_ratio_mul / spg->scale_x_ratio_div - spr_wiz_x + spg->tx;
			y1 = spi->ty * spg->scale_y_ratio_mul / spg->scale_y_ratio_div - spr_wiz_y + spg->ty;
		} else {
			x1 = spi->tx - spr_wiz_x + spg->tx;
			y1 = spi->ty - spr_wiz_y + spg->ty;
		}
	} else {
		x1 = spi->tx - spr_wiz_x;
		y1 = spi->ty - spr_wiz_y;
	}

	if (spi->image) {
		angle = spi->angle;
		scale = spi->scale;
		_vm->_wiz->getWizImageDim(spi->image, spi->imageState, w, h);
		if (!(spi->flags & (kSFScaled | kSFRotated))) {
			bound.left = x1;
			bound.top = y1;
			bound.right = x1 + w;
			bound.bottom = y1 + h;
		} else {
			Common::Point pts[4];

			pts[1].x = pts[2].x = w / 2 - 1;
			pts[0].x = pts[0].y = pts[1].y = pts[3].x = -w / 2;
			pts[2].y = pts[3].y = h / 2 - 1;

			if ((spi->flags & kSFScaled) && scale) {
				for (int j = 0; j < 4; ++j) {
					pts[j].x = pts[j].x * scale / 256;
					pts[j].y = pts[j].y * scale / 256;
				}
			}
			if ((spi->flags & kSFRotated) && angle)
				_vm->_wiz->polygonRotatePoints(pts, 4, angle);

			for (int j = 0; j < 4; ++j) {
				pts[j].x += x1;
				pts[j].y += y1;
			}

			_vm->_wiz->polygonCalcBoundBox(pts, 4, bound);
		}
	} else {
		bound.left = 1234;
		bound.top = 1234;
		bound.right = -1234;
		bound.bottom = -1234;
	}
}

//
// spriteInfoGet functions
//
int Sprite::findSpriteWithClassOf(int x_pos, int y_pos, int spriteGroupId, int type, int num, int *args) {
	bool cond;
	int code, classId, x, y;
	debug(1, "findSprite: x %d, y %d, spriteGroup %d, type %d, num %d", x_pos, y_pos, spriteGroupId, type, num);

	for (int i = (_numSpritesToProcess - 1); i >= 0; i--) {
		SpriteInfo *spi = _activeSpritesTable[i];
		if (!spi->curImage)
			continue;

		if (spriteGroupId && spi->group != spriteGroupId)
			continue;

		cond = true;
		for (int j = 0; j < num; j++) {
			code = classId = args[j];
			classId &= 0x7F;
			checkRange(32, 1, classId, "class %d out of range in statement");
			if (code & 0x80) {
				if (!(spi->classFlags & (1 << (classId - 1))))
					cond = 0;
			} else {
				if ((spi->classFlags & (1 << (classId - 1))))
					cond = 0;
			}
		}
		if (!cond)
			continue;

		if (type) {
			if (spi->bbox.left > spi->bbox.right)
				continue;
			if (spi->bbox.top > spi->bbox.bottom)
				continue;
			if (spi->bbox.left > x_pos)
				continue;
			if (spi->bbox.top > y_pos)
				continue;
			if (spi->bbox.right < x_pos)
				continue;
			if (spi->bbox.bottom < y_pos)
				continue;
			return spi->id;
		} else {
			int image, imageState, angle, scale;
			int32 w, h;

			image = spi->curImage;
			if (spi->maskImage) {
				int32 x1, x2, y1, y2;

				imageState = spi->curImageState % _vm->_wiz->getWizImageStates(spi->maskImage);

				x = x_pos - spi->pos.x;
				y = y_pos - spi->pos.y;

				_vm->_wiz->loadImgSpot(spi->curImage, imageState, x1, y1);
				_vm->_wiz->loadImgSpot(spi->maskImage, imageState, x2, y2);

				x += (x2 - x1);
				y += (y2 - y1);
			} else {
				if (spi->bbox.left > spi->bbox.right)
					continue;
				if (spi->bbox.top > spi->bbox.bottom)
					continue;
				if (spi->bbox.left > x_pos)
					continue;
				if (spi->bbox.top > y_pos)
					continue;
				if (spi->bbox.right < x_pos)
					continue;
				if (spi->bbox.bottom < y_pos)
					continue;

				x = x_pos - spi->pos.x;
				y = y_pos - spi->pos.y;
				imageState = spi->curImageState;
			}

			angle = spi->curAngle;
			scale = spi->curScale;
			if ((spi->flags & kSFScaled) || (spi->flags & kSFRotated)) {
				if (spi->flags & kSFScaled && scale) {
					x = x * 256 / scale;
					y = y * 256 / scale;
				}
				if (spi->flags & kSFRotated && angle) {
					angle = (360 - angle) % 360;
					Common::Point pts[1];
					_vm->_wiz->polygonRotatePoints(pts, 1, angle);
				}

				_vm->_wiz->getWizImageDim(image, imageState, w, h);
				x += w / 2;
				y += h / 2;
			}

			if (_vm->_wiz->isWizPixelNonTransparent(image, imageState, x, y, spi->curImgFlags))
				return spi->id;
		}
	}

	return 0;
}

int Sprite::getSpriteClass(int spriteId, int num, int *args) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");
	int code, classId;

	if (num == 0)
		return _spriteTable[spriteId].classFlags;

	for (int i = 0; i < num; i++) {
		code = classId = args[i];
		classId &= 0x7F;
		checkRange(32, 1, classId, "class %d out of range in statement");
		if (code & 0x80) {
			if (!(_spriteTable[spriteId].classFlags & (1 << (classId - 1))))
				return 0;
		} else {
			if ((_spriteTable[spriteId].classFlags & (1 << (classId - 1))))
				return 0;
		}
	}

	return 1;
}

int Sprite::getSpriteFlagDoubleBuffered(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return ((_spriteTable[spriteId].flags & kSFDoubleBuffered) != 0) ? 1 : 0;
}

int Sprite::getSpriteFlagYFlipped(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return ((_spriteTable[spriteId].flags & kSFYFlipped) != 0) ? 1 : 0;
}

int Sprite::getSpriteFlagXFlipped(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return ((_spriteTable[spriteId].flags & kSFXFlipped) != 0) ? 1 : 0;
}

int Sprite::getSpriteFlagActive(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return ((_spriteTable[spriteId].flags & kSFActive) != 0) ? 1 : 0;
}

int Sprite::getSpriteFlagRemapPalette(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return ((_spriteTable[spriteId].flags & kSFRemapPalette) != 0) ? 1 : 0;
}

int Sprite::getSpriteFlagAutoAnim(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return ((_spriteTable[spriteId].flags & kSFAutoAnim) != 0) ? 1 : 0;
}

int Sprite::getSpriteFlagUpdateType(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return ((_spriteTable[spriteId].flags & kSFMarkDirty) != 0) ? 1 : 0;
}

int Sprite::getSpriteFlagEraseType(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return ((_spriteTable[spriteId].flags & kSFImageless) != 0) ? 1 : 0;
}

int Sprite::getSpriteImage(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].image;
}

int Sprite::getSpriteImageState(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].imageState;
}

int Sprite::getSpriteGroup(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].group;
}

int Sprite::getSpritePalette(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].palette;
}

int Sprite::getSpritePriority(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].priority;
}

int Sprite::getSpriteDisplayX(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (_spriteTable[spriteId].group)
		return _spriteTable[spriteId].tx + _spriteGroups[_spriteTable[spriteId].group].tx;
	else
		return _spriteTable[spriteId].tx;
}

int Sprite::getSpriteDisplayY(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (_spriteTable[spriteId].group)
		return _spriteTable[spriteId].ty + _spriteGroups[_spriteTable[spriteId].group].ty;
	else
		return _spriteTable[spriteId].ty;
}

int Sprite::getSpriteUserValue(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].userValue;
}

int Sprite::getSpriteShadow(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].shadow;
}

int Sprite::getSpriteImageStateCount(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].imageStateCount;
}

int Sprite::getSpriteScale(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].scale;
}

int Sprite::getSpriteAnimSpeed(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].animSpeed;
}

int Sprite::getSpriteSourceImage(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].sourceImage;
}

int Sprite::getSpriteMaskImage(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].maskImage;
}

int Sprite::getSpriteGeneralProperty(int spriteId, int type) {
	debug(0, "getSpriteGeneralProperty: spriteId %d type 0x%x", spriteId, type);
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	// XXX U32 related check

	switch(type) {
	case 0x7B:
		return _spriteTable[spriteId].imgFlags;
	case 0x7D:
		return _spriteTable[spriteId].field_90;
	case 0x7E:
		return _spriteTable[spriteId].animProgress;
	default:
		error("getSpriteGeneralProperty: Invalid type %d", type);
	}
}

void Sprite::getSpriteImageDim(int spriteId, int32 &w, int32 &h) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (_spriteTable[spriteId].image) {
		_vm->_wiz->getWizImageDim(_spriteTable[spriteId].image, _spriteTable[spriteId].imageState, w, h);
	} else {
		w = 0;
		h = 0;
	}
}

void Sprite::getSpritePosition(int spriteId, int32 &tx, int32 &ty) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	tx = _spriteTable[spriteId].tx;
	ty = _spriteTable[spriteId].ty;
}

void Sprite::getSpriteDist(int spriteId, int32 &dx, int32 &dy) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	dx = _spriteTable[spriteId].dx;
	dy = _spriteTable[spriteId].dy;
}

//
// spriteGroupGet functions
//
int ScummEngine_v90he::getGroupSpriteArray(int spriteGroupId) {
	int i, numSprites = 0;

	checkRange(_sprite->_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (i = (_sprite->_varNumSprites - 1); i > 0; i--) {
		if (_sprite->_spriteTable[i].group == spriteGroupId)
			numSprites++;
	}

	if (!numSprites)
		return 0;

	writeVar(0, 0);
	defineArray(0, kDwordArray, 0, 0, 0, numSprites);
	writeArray(0, 0, 0, numSprites);

	numSprites = 1;
	for (i = (_sprite->_varNumSprites - 1); i > 0; i--) {
		if (_sprite->_spriteTable[i].group == spriteGroupId) {
			writeArray(0, 0, numSprites, i);
			numSprites++;
		}
	}

	return readVar(0);
}

int Sprite::getGroupPriority(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	return _spriteGroups[spriteGroupId].priority;
}

int Sprite::getGroupDstResNum(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	return _spriteGroups[spriteGroupId].image;
}

int Sprite::getGroupScale_x_ratio_mul(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	return _spriteGroups[spriteGroupId].scale_x_ratio_mul;
}

int Sprite::getGroupScale_x_ratio_div(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	return _spriteGroups[spriteGroupId].scale_x_ratio_div;
}

int Sprite::getGroupScale_y_ratio_mul(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	return _spriteGroups[spriteGroupId].scale_y_ratio_mul;
}

int Sprite::getGroupScale_y_ratio_div(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	return _spriteGroups[spriteGroupId].scale_y_ratio_div;
}

void Sprite::getGrouptx_ty(int spriteGroupId, int32 &tx, int32 &ty) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	tx = _spriteGroups[spriteGroupId].tx;
	ty = _spriteGroups[spriteGroupId].ty;
}

//
// spriteInfoSet functions
//
void Sprite::setSpritePalette(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (_spriteTable[spriteId].palette != value) {
		_spriteTable[spriteId].palette = value;
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
	}
}

void Sprite::setSpriteSourceImage(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (_spriteTable[spriteId].sourceImage != value) {
		_spriteTable[spriteId].sourceImage = value;
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
	}
}

void Sprite::setSpriteMaskImage(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].maskImage = value;
}

void Sprite::setSpriteImageState(int spriteId, int state) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (_spriteTable[spriteId].image) {
		int imageStateCount = _spriteTable[spriteId].imageStateCount - 1;
		state = MAX(0, state);
		state = MIN(state, imageStateCount);

		if (_spriteTable[spriteId].imageState != state) {
			_spriteTable[spriteId].imageState = state;
			_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
		}
	}
}

void Sprite::setSpritePosition(int spriteId, int tx, int ty) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (_spriteTable[spriteId].tx != tx || _spriteTable[spriteId].ty != ty) {
		_spriteTable[spriteId].tx = tx;
		_spriteTable[spriteId].ty = ty;
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
	}
}

void Sprite::setSpriteGroup(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");
	checkRange(_varNumSpriteGroups, 0, value, "Invalid sprite group %d");

	_spriteTable[spriteId].group = value;
	_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
}

void Sprite::setSpriteDist(int spriteId, int value1, int value2) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].dx = value1;
	_spriteTable[spriteId].dy = value2;
}

void Sprite::setSpriteShadow(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].shadow = value;
	if (_spriteTable[spriteId].image)
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
}

void Sprite::setSpriteUserValue(int spriteId, int value1, int value2) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].userValue = value2;
}

void Sprite::setSpritePriority(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].priority = value;
}

void Sprite::moveSprite(int spriteId, int value1, int value2) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].tx += value1;
	_spriteTable[spriteId].ty += value2;

	if  (value1 || value2)
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
}

void Sprite::setSpriteScale(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].flags |= kSFScaled;

	if (_spriteTable[spriteId].scale != value) {
		_spriteTable[spriteId].scale = value;

		if (_spriteTable[spriteId].image)
			_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
	}
}

void Sprite::setSpriteAngle(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].flags |= kSFRotated;

	if (_spriteTable[spriteId].angle != value) {
		_spriteTable[spriteId].angle = value;

		if (_spriteTable[spriteId].image)
			_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
	}
}

void Sprite::setSpriteFlagDoubleBuffered(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	int oldFlags = _spriteTable[spriteId].flags;
	if (value)
		_spriteTable[spriteId].flags |= kSFDoubleBuffered;
	else
		_spriteTable[spriteId].flags &= ~kSFDoubleBuffered;

	if (_spriteTable[spriteId].image && _spriteTable[spriteId].flags != oldFlags)
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
}

void Sprite::setSpriteFlagYFlipped(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	int oldFlags = _spriteTable[spriteId].flags;
	if (value)
		_spriteTable[spriteId].flags |= kSFYFlipped;
	else
		_spriteTable[spriteId].flags &= ~kSFYFlipped;

	if (_spriteTable[spriteId].image && _spriteTable[spriteId].flags != oldFlags)
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
}

void Sprite::setSpriteFlagXFlipped(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	int oldFlags = _spriteTable[spriteId].flags;
	if (value)
		_spriteTable[spriteId].flags |= kSFXFlipped;
	else
		_spriteTable[spriteId].flags &= ~kSFXFlipped;

	if (_spriteTable[spriteId].image && _spriteTable[spriteId].flags != oldFlags)
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
}

void Sprite::setSpriteFlagActive(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (value)
		_spriteTable[spriteId].flags |= kSFActive;
	else
		_spriteTable[spriteId].flags &= ~kSFActive;
}

void Sprite::setSpriteFlagRemapPalette(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	int oldFlags = _spriteTable[spriteId].flags;
	if (value)
		_spriteTable[spriteId].flags |= kSFRemapPalette;
	else
		_spriteTable[spriteId].flags &= ~kSFRemapPalette;

	if (_spriteTable[spriteId].image && _spriteTable[spriteId].flags != oldFlags)
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
}

void Sprite::setSpriteFlagAutoAnim(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (value)
		_spriteTable[spriteId].flags |= kSFAutoAnim;
	else
		_spriteTable[spriteId].flags &= ~kSFAutoAnim;
}

void Sprite::setSpriteFlagUpdateType(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	switch(value) {
	case 2:
		_spriteTable[spriteId].flags &= ~(kSFMarkDirty);
		_spriteTable[spriteId].flags |= kSFBlitDirectly;
		break;
	case 1:
		_spriteTable[spriteId].flags |= kSFMarkDirty | kSFBlitDirectly;
		break;
	case 0:
		_spriteTable[spriteId].flags &= ~(kSFMarkDirty | kSFBlitDirectly);
		break;
	default:
		error("setSpriteFlagUpdateType: Invalid value %d", value);
	}
}

void Sprite::setSpriteFlagEraseType(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	// Note that condition is inverted
	if (!value)
		_spriteTable[spriteId].flags |= kSFImageless;
	else
		_spriteTable[spriteId].flags &= ~kSFImageless;
}

void Sprite::setSpriteAnimSpeed(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].animSpeed = value;
	_spriteTable[spriteId].animProgress = value;
}

void Sprite::setSpriteSetClass(int spriteId, int classId, int toggle) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");
	checkRange(32, 1, classId, "class %d out of range in statement");

	if (toggle) {
		_spriteTable[spriteId].classFlags |= (1 << (classId - 1));
	} else {
		_spriteTable[spriteId].classFlags &= ~(1 << (classId - 1));
	}
}

void Sprite::setSpriteResetClass(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].classFlags = 0;
}

void Sprite::setSpriteField84(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].field_84 = value;
}

void Sprite::setSpriteGeneralProperty(int spriteId, int type, int value) {
	debug(0, "setSpriteGeneralProperty: spriteId %d type 0x%x", spriteId, type);
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");
	int32 delay;

	// XXX U32 related check

	switch(type) {
	case 0x7B:
		_spriteTable[spriteId].imgFlags = value;
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
		break;
	case 0x7D:
		_spriteTable[spriteId].field_90 = value;
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
		break;
	case 0x7E:
		delay = MAX(0, value);
		delay = MIN(delay, _spriteTable[spriteId].animSpeed);

		_spriteTable[spriteId].animProgress = delay;
		break;
	default:
		error("setSpriteGeneralProperty: Invalid value %d", type);
	}
}

void Sprite::resetSprite(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].angle = 0;
	_spriteTable[spriteId].scale = 0;

	setSpriteImage(spriteId, 0);

	_spriteTable[spriteId].shadow = 0;
	_spriteTable[spriteId].tx = 0;
	_spriteTable[spriteId].ty = 0;

	_spriteTable[spriteId].flags &= ~(kSFYFlipped | kSFXFlipped);
	_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
	_spriteTable[spriteId].dx = 0;
	_spriteTable[spriteId].dy = 0;
	_spriteTable[spriteId].userValue = 0;
	_spriteTable[spriteId].group = 0;
	_spriteTable[spriteId].animSpeed = 0;
	_spriteTable[spriteId].animProgress = 0;
	_spriteTable[spriteId].classFlags = 0;
	_spriteTable[spriteId].palette = 0;
	_spriteTable[spriteId].sourceImage = 0;
	_spriteTable[spriteId].maskImage = 0;
	_spriteTable[spriteId].priority = 0;
	_spriteTable[spriteId].field_84 = 0;
	_spriteTable[spriteId].imgFlags = 0;
	_spriteTable[spriteId].field_90 = 0;
}

void Sprite::setSpriteImage(int spriteId, int imageNum) {
	int origResId, origResWizStates;

	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	origResId = _spriteTable[spriteId].image;
	origResWizStates = _spriteTable[spriteId].imageStateCount;

	_spriteTable[spriteId].image = imageNum;
	_spriteTable[spriteId].field_74 = 0;
	_spriteTable[spriteId].imageState = 0;

	if (_spriteTable[spriteId].image) {
		_spriteTable[spriteId].imageStateCount = _vm->_wiz->getWizImageStates(_spriteTable[spriteId].image);
		_spriteTable[spriteId].flags |= kSFActive | kSFAutoAnim | kSFMarkDirty | kSFBlitDirectly;

		if (_spriteTable[spriteId].image != origResId || _spriteTable[spriteId].imageStateCount != origResWizStates)
			_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
	} else {
		if (_spriteTable[spriteId].flags & kSFImageless)
			_spriteTable[spriteId].flags = 0;
		else
			_spriteTable[spriteId].flags = kSFChanged | kSFBlitDirectly;
		_spriteTable[spriteId].curImage = 0;
		_spriteTable[spriteId].curImageState = 0;
		_spriteTable[spriteId].imageStateCount = 0;
	}
}

//
// spriteGroupSet functions
//
void Sprite::redrawSpriteGroup(int spriteGroupId) {
	for (int i = 0; i < _numSpritesToProcess; ++i) {
		SpriteInfo *spi = _activeSpritesTable[i];
		if (spi->group == spriteGroupId) {
			spi->flags |= kSFChanged | kSFNeedRedraw;
		}
	}
}

void Sprite::moveGroupMembers(int spriteGroupId, int value1, int value2) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group == spriteGroupId) {
			_spriteTable[i].tx += value1;
			_spriteTable[i].ty += value2;

			if (value1 || value2)
				_spriteTable[i].flags |= kSFChanged | kSFNeedRedraw;
		}
	}
}

void Sprite::setGroupMembersPriority(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group == spriteGroupId)
			_spriteTable[i].priority = value;
	}
}

void Sprite::setGroupMembersGroup(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group == spriteGroupId) {
			_spriteTable[i].group = value;
			_spriteTable[i].flags |= kSFChanged | kSFNeedRedraw;
		}
	}
}

void Sprite::setGroupMembersUpdateType(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group == spriteGroupId)
			setSpriteFlagUpdateType(i, value);
	}
}

void Sprite::setGroupMembersResetSprite(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group == spriteGroupId)
			resetSprite(i);
	}
}

void Sprite::setGroupMembersAnimationSpeed(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group == spriteGroupId) {
			_spriteTable[i].animSpeed = value;
			_spriteTable[i].animProgress = value;
		}
	}
}

void Sprite::setGroupMembersAutoAnimFlag(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group == spriteGroupId) {
			if (value)
				_spriteTable[i].flags |= kSFAutoAnim;
			else
				_spriteTable[i].flags &= ~kSFAutoAnim;
		}
	}
}

void Sprite::setGroupMembersShadow(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group == spriteGroupId) {
			_spriteTable[i].shadow = value;
			if (_spriteTable[i].image)
				_spriteTable[i].flags |= kSFChanged | kSFNeedRedraw;
		}
	}
}

void Sprite::setGroupBounds(int spriteGroupId, int x1, int y1, int x2, int y2) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	_spriteGroups[spriteGroupId].flags |= kSGFClipBox;
	_spriteGroups[spriteGroupId].bbox.left = x1;
	_spriteGroups[spriteGroupId].bbox.top = y1;
	_spriteGroups[spriteGroupId].bbox.right = x2;
	_spriteGroups[spriteGroupId].bbox.bottom = y2;

	redrawSpriteGroup(spriteGroupId);
}

void Sprite::setGroupPriority(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	if (_spriteGroups[spriteGroupId].priority != value) {
		_spriteGroups[spriteGroupId].priority = value;
		redrawSpriteGroup(spriteGroupId);
	}
}

void Sprite::setGroupPosition(int spriteGroupId, int value1, int value2) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	if (_spriteGroups[spriteGroupId].tx != value1 || _spriteGroups[spriteGroupId].ty != value2) {
		_spriteGroups[spriteGroupId].tx = value1;
		_spriteGroups[spriteGroupId].ty = value2;
		redrawSpriteGroup(spriteGroupId);
	}
}

void Sprite::moveGroup(int spriteGroupId, int value1, int value2) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	if (value1 || value2) {
		_spriteGroups[spriteGroupId].tx += value1;
		_spriteGroups[spriteGroupId].ty += value2;
		redrawSpriteGroup(spriteGroupId);
	}
}

void Sprite::setGroupImage(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	if (_spriteGroups[spriteGroupId].image != value) {
		_spriteGroups[spriteGroupId].image = value;
		redrawSpriteGroup(spriteGroupId);
	}
}

void Sprite::setGroupScaling(int spriteGroupId) {
	if ((_spriteGroups[spriteGroupId].scale_x_ratio_mul != _spriteGroups[spriteGroupId].scale_x_ratio_div) || (_spriteGroups[spriteGroupId].scale_y_ratio_mul != _spriteGroups[spriteGroupId].scale_y_ratio_div))
		_spriteGroups[spriteGroupId].scaling = 1;
	else
		_spriteGroups[spriteGroupId].scaling = 0;

}

void Sprite::setGroupScale_x_ratio_mul(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	if (_spriteGroups[spriteGroupId].scale_x_ratio_mul != value) {
		_spriteGroups[spriteGroupId].scale_x_ratio_mul = value;
		setGroupScaling(spriteGroupId);
		redrawSpriteGroup(spriteGroupId);
	}
}

void Sprite::setGroupScale_x_ratio_div(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	if (value == 0)
		error("setGroupScale_x_ratio_div: Divisor must not be 0");

	if (_spriteGroups[spriteGroupId].scale_x_ratio_div != value) {
		_spriteGroups[spriteGroupId].scale_x_ratio_div = value;
		setGroupScaling(spriteGroupId);
		redrawSpriteGroup(spriteGroupId);
	}
}

void Sprite::setGroupScale_y_ratio_mul(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	if (_spriteGroups[spriteGroupId].scale_y_ratio_mul != value) {
		_spriteGroups[spriteGroupId].scale_y_ratio_mul = value;
		setGroupScaling(spriteGroupId);
		redrawSpriteGroup(spriteGroupId);
	}
}

void Sprite::setGroupScale_y_ratio_div(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	if (value == 0)
		error("setGroupScale_y_ratio_div: Divisor must not be 0");

	if (_spriteGroups[spriteGroupId].scale_y_ratio_div != value) {
		_spriteGroups[spriteGroupId].scale_y_ratio_div = value;
		setGroupScaling(spriteGroupId);
		redrawSpriteGroup(spriteGroupId);
	}
}

void Sprite::resetGroupBounds(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	_spriteGroups[spriteGroupId].flags &= ~(kSGFClipBox);
	redrawSpriteGroup(spriteGroupId);
}

void Sprite::allocTables(int numSprites, int numGroups, int numMaxSprites) {
	_varNumSpriteGroups = numGroups;
	_numSpritesToProcess = 0;
	_varNumSprites = numSprites;
	_varMaxSprites = numMaxSprites;
	_spriteGroups = (SpriteGroup *)malloc((_varNumSpriteGroups + 1) * sizeof(SpriteGroup));
	_spriteTable = (SpriteInfo *)malloc((_varNumSprites + 1) * sizeof(SpriteInfo));
	_activeSpritesTable = (SpriteInfo **)malloc((_varNumSprites + 1) * sizeof(SpriteInfo *));
}

void Sprite::resetGroup(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");
	SpriteGroup *spg = &_spriteGroups[spriteGroupId];

	spg->priority = 0;
	spg->tx = spg->ty = 0;

	spg->flags &= ~kSGFClipBox;
	redrawSpriteGroup(spriteGroupId);

	spg->image = 0;
	spg->scaling = 0;
	spg->scale_x_ratio_mul = 1;
	spg->scale_x_ratio_div = 1;
	spg->scale_y_ratio_mul = 1;
	spg->scale_y_ratio_div = 1;
}

void Sprite::resetTables(bool refreshScreen) {
	memset(_spriteTable, 0, (_varNumSprites + 1) * sizeof(SpriteInfo));
	memset(_spriteGroups, 0, (_varNumSpriteGroups + 1) * sizeof(SpriteGroup));
	for (int curGrp = 1; curGrp < _varNumSpriteGroups; ++curGrp)
		resetGroup(curGrp);

	if (refreshScreen) {
		_vm->gdi.copyVirtScreenBuffers(Common::Rect(_vm->_screenWidth, _vm->_screenHeight));
	}
	_numSpritesToProcess = 0;
}

void Sprite::resetBackground() {
	int xmin, xmax, ymin, ymax;
	xmin = ymin = 1234;
	xmax = ymax = -1234;
	bool firstLoop = true;
	bool refreshScreen = false;

	for (int i = 0; i < _numSpritesToProcess; ++i) {
		SpriteInfo *spi = _activeSpritesTable[i];
		if (!(spi->flags & kSFImageless) && (spi->flags & kSFChanged)) {
			spi->flags &= ~kSFChanged;
			if (spi->bbox.left <= spi->bbox.right && spi->bbox.top <= spi->bbox.bottom) {
				if (spi->flags & kSFBlitDirectly) {
					_vm->gdi.copyVirtScreenBuffers(spi->bbox, USAGE_BIT_RESTORED);
				} else if (firstLoop) {
					xmin = spi->bbox.left;
					ymin = spi->bbox.top;
					xmax = spi->bbox.right;
					ymax = spi->bbox.bottom;
					firstLoop = false;
					refreshScreen = true;
				} else {
					if (xmin > spi->bbox.left) {
						xmin = spi->bbox.left;
					}
					if (ymin > spi->bbox.top) {
						ymin = spi->bbox.top;
					}
					if (xmax < spi->bbox.right) {
						xmax = spi->bbox.right;
					}
					if (ymax < spi->bbox.bottom) {
						ymax = spi->bbox.bottom;
					}
					refreshScreen = true;
				}
				if (!(spi->flags & (kSFNeedRedraw | kSF30)) && spi->image)
					spi->flags |= kSFNeedRedraw;
			}
		}
	}
	if (refreshScreen) {
		_vm->gdi.copyVirtScreenBuffers(Common::Rect(xmin, ymin, xmax, ymax), USAGE_BIT_RESTORED);
	}
}

void Sprite::setRedrawFlags(bool checkZOrder) {
	VirtScreen *vs = &_vm->virtscr[kMainVirtScreen];
	for (int i = 0; i < _numSpritesToProcess; ++i) {
		SpriteInfo *spi = _activeSpritesTable[i];
		if (!(spi->flags & (kSFNeedRedraw | kSF30))) {
			if ((!checkZOrder || spi->priority >= 0) && (spi->flags & kSFMarkDirty)) {
				int lp = spi->bbox.left / 8;
				lp = MAX(0, lp);
				lp = MIN(lp, 79);
				int rp = (spi->bbox.right + 7) / 8;
				rp = MAX(0, rp);
				rp = MIN(rp, 79);
				for (; lp <= rp; ++lp) {
					if (vs->tdirty[lp] < vs->h && spi->bbox.bottom >= vs->tdirty[lp] && spi->bbox.top <= vs->bdirty[lp]) {
						spi->flags |= kSFNeedRedraw;
						break;
					}
				}
			}
		}
	}
}

void Sprite::updateImages() {
	for (int i = 0; i < _numSpritesToProcess; ++i) {
		SpriteInfo *spi = _activeSpritesTable[i];
		if (spi->dx || spi->dy) {
			int tx = spi->tx;
			int ty = spi->ty;
			spi->tx += spi->dx;
			spi->ty += spi->dy;
			if (tx != spi->tx || ty != spi->ty) {
				spi->flags |= kSFChanged | kSFNeedRedraw;
			}
		}
		if (spi->flags & kSFAutoAnim) {
			if (spi->animSpeed) {
				--spi->animProgress;
				if (spi->animProgress)
					continue;

				spi->animProgress = spi->animSpeed;
			}
			int imageState = spi->imageState;
			++spi->imageState;
			if (spi->imageState >= spi->imageStateCount) {
				spi->imageState = 0;
				if (imageState == 0)
					continue;
			}
			spi->flags |= kSFChanged | kSFNeedRedraw;
		}
	}
}

static int compareSprTable(const void *a, const void *b) {
	const SpriteInfo *spr1 = *(const SpriteInfo *const*)a;
	const SpriteInfo *spr2 = *(const SpriteInfo *const*)b;

	if (spr1->zorder > spr2->zorder)
		return 1;

	if (spr1->zorder < spr2->zorder)
		return -1;

	return 0;
}

void Sprite::sortActiveSprites() {
	int groupZorder;

	_numSpritesToProcess = 0;

	if (_varNumSprites <= 1)
		return;

	for (int i = 1; i < _varNumSprites; i++) {
		SpriteInfo *spi = &_spriteTable[i];

		if (spi->flags & kSFActive) {
			if (!(spi->flags & kSFMarkDirty)) {
				if (!(spi->flags & kSF30))
					spi->flags |= kSFNeedRedraw;
				if (!(spi->flags & kSFImageless))
					spi->flags |= kSFChanged;
			}
			if (spi->group)
				groupZorder = _spriteGroups[spi->group].priority;
			else
				groupZorder = 0;

			spi->id = i;
			spi->zorder = spi->priority + groupZorder;

			_activeSpritesTable[_numSpritesToProcess++] = spi;
		}
	}

	if (_numSpritesToProcess < 2)
		return;

	qsort(_activeSpritesTable, _numSpritesToProcess, sizeof(SpriteInfo *), compareSprTable);
}

void Sprite::processImages(bool arg) {
	int spr_flags;
	int32 spr_wiz_x, spr_wiz_y;
	int image, imageState;
	Common::Rect *bboxPtr;
	int angle, scale;
	int32 w, h;
	WizParameters wiz;

	for (int i = 0; i < _numSpritesToProcess; i++) {
		SpriteInfo *spi = _activeSpritesTable[i];

		if (!(spi->flags & kSFNeedRedraw))
			continue;

		spr_flags = spi->flags;

		if (arg) {
			if (spi->zorder >= 0)
				return;
		} else {
			if (spi->zorder < 0)
				continue;
		}

		spi->flags &= ~kSFNeedRedraw;
		image = spi->image;
		imageState = spi->imageState;
		_vm->_wiz->loadImgSpot(spi->image, spi->imageState, spr_wiz_x, spr_wiz_y);

		if (spi->group) {
			SpriteGroup *spg = &_spriteGroups[spi->group];

			if (spg->scaling) {
				wiz.img.x1 = spi->tx * spg->scale_x_ratio_mul / spg->scale_x_ratio_div - spr_wiz_x + spg->tx;
				wiz.img.y1 = spi->ty * spg->scale_y_ratio_mul / spg->scale_y_ratio_div - spr_wiz_y + spg->ty;
			} else {
				wiz.img.x1 = spi->tx - spr_wiz_x + spg->tx;
				wiz.img.y1 = spi->ty - spr_wiz_y + spg->ty;
			}
		} else {
			wiz.img.x1 = spi->tx - spr_wiz_x;
			wiz.img.y1 = spi->ty - spr_wiz_y;
		}

		wiz.spriteId = spi->id;
		wiz.spriteGroup = spi->group;
		wiz.field_23EA = spi->field_90;
		spi->curImageState = wiz.img.state = imageState;
		spi->curImage = wiz.img.resNum = image;
		wiz.processFlags = kWPFNewState | kWPFSetPos;
		spi->curAngle = spi->angle;
		spi->curScale = spi->scale;
		spi->pos.x = wiz.img.x1;
		spi->pos.y = wiz.img.y1;
		bboxPtr = &spi->bbox;
		if (image) {
			angle = spi->angle;
			scale = spi->scale;
			_vm->_wiz->getWizImageDim(image, imageState, w, h);
			if (!(spi->flags & (kSFScaled | kSFRotated))) {
				bboxPtr->left = wiz.img.x1;
				bboxPtr->top = wiz.img.y1;
				bboxPtr->right = wiz.img.x1 + w;
				bboxPtr->bottom = wiz.img.y1 + h;
			} else {
				Common::Point pts[4];

				pts[1].x = pts[2].x = w / 2 - 1;
				pts[0].x = pts[0].y = pts[1].y = pts[3].x = -w / 2;
				pts[2].y = pts[3].y = h / 2 - 1;

				if ((spi->flags & kSFScaled) && scale) {
					for (int j = 0; j < 4; ++j) {
						pts[j].x = pts[j].x * scale / 256;
						pts[j].y = pts[j].y * scale / 256;
					}
				}
				if ((spi->flags & kSFRotated) && angle)
					_vm->_wiz->polygonRotatePoints(pts, 4, angle);

				for (int j = 0; j < 4; ++j) {
					pts[j].x += wiz.img.x1;
					pts[j].y += wiz.img.y1;
				}

				_vm->_wiz->polygonCalcBoundBox(pts, 4, spi->bbox);
			}
		} else {
			bboxPtr->left = 1234;
			bboxPtr->top = 1234;
			bboxPtr->right = -1234;
			bboxPtr->bottom = -1234;
		}

		wiz.img.flags = kWIFMarkBufferDirty;
		wiz.img.zorder = 0;
		if (spr_flags & kSFXFlipped)
			wiz.img.flags |= kWIFFlipX;
		if (spr_flags & kSFYFlipped)
			wiz.img.flags |= kWIFFlipY;
		if (spr_flags & kSFDoubleBuffered) {
			wiz.img.flags &= ~kWIFMarkBufferDirty;
			wiz.img.flags |= kWIFBlitToFrontVideoBuffer;
		}
		if (spi->shadow) {
			wiz.img.flags |= 0x200;
			wiz.processFlags |= kWPFShadow;
			wiz.img.shadow = spi->shadow;
		}
		if (spr_flags & kSFRemapPalette)
			wiz.img.flags |= kWIFRemapPalette;
		if (spi->field_84) {
			wiz.processFlags |= 0x200000;
			wiz.img.field_390 = spi->field_84;
			wiz.img.zorder = spi->priority;
		}
		if (spi->sourceImage) {
			wiz.processFlags |= kWPFMaskImg;
			wiz.sourceImage = spi->sourceImage;
		}
		wiz.processFlags |= kWPFNewFlags;
		wiz.img.flags |= spi->imgFlags;

		if (spr_flags & kSFRotated) {
			wiz.processFlags |= kWPFRotate;
			wiz.angle = spi->angle;
		}
		if (spr_flags & kSFScaled) {
			wiz.processFlags |= kWPFScaled;
			wiz.scale = spi->scale;
		}
		spi->curImgFlags = wiz.img.flags;

		if (spi->group && (_spriteGroups[spi->group].flags & kSGFClipBox)) {
			Common::Rect &spgBbox = _spriteGroups[spi->group].bbox;
			if (spgBbox.isValidRect() && spi->bbox.intersects(spgBbox)) {
				spi->bbox.clip(spgBbox);
				wiz.processFlags |= kWPFClipBox;
				wiz.box = spi->bbox;
			} else {
				bboxPtr->left = 1234;
				bboxPtr->top = 1234;
				bboxPtr->right = -1234;
				bboxPtr->bottom = -1234;
				continue;
			}
		}
		if (spi->palette) {
			wiz.processFlags |= kWPFPaletteNum;
			wiz.img.palette = spi->palette;
		}
		if (spi->image && spi->group && _spriteGroups[spi->group].image) {
			wiz.processFlags |= kWPFDstResNum;
			wiz.dstResNum = _spriteGroups[spi->group].image;
		}
		_vm->_wiz->displayWizComplexImage(&wiz);
	}
}

void Sprite::saveOrLoadSpriteData(Serializer *s, uint32 savegameVersion) {
	static const SaveLoadEntry spriteEntries[] = {
		MKLINE(SpriteInfo, id, sleInt32, VER(48)),
		MKLINE(SpriteInfo, zorder, sleInt32, VER(48)),
		MKLINE(SpriteInfo, flags, sleInt32, VER(48)),
		MKLINE(SpriteInfo, image, sleInt32, VER(48)),
		MKLINE(SpriteInfo, imageState, sleInt32, VER(48)),
		MKLINE(SpriteInfo, group, sleInt32, VER(48)),
		MKLINE(SpriteInfo, palette, sleInt32, VER(48)),
		MKLINE(SpriteInfo, priority, sleInt32, VER(48)),
		MKLINE(SpriteInfo, bbox.left, sleInt32, VER(48)),
		MKLINE(SpriteInfo, bbox.top, sleInt32, VER(48)),
		MKLINE(SpriteInfo, bbox.right, sleInt32, VER(48)),
		MKLINE(SpriteInfo, bbox.bottom, sleInt32, VER(48)),
		MKLINE(SpriteInfo, dx, sleInt32, VER(48)),
		MKLINE(SpriteInfo, dy, sleInt32, VER(48)),
		MKLINE(SpriteInfo, pos.x, sleInt32, VER(48)),
		MKLINE(SpriteInfo, pos.y, sleInt32, VER(48)),
		MKLINE(SpriteInfo, tx, sleInt32, VER(48)),
		MKLINE(SpriteInfo, ty, sleInt32, VER(48)),
		MKLINE(SpriteInfo, userValue, sleInt32, VER(48)),
		MKLINE(SpriteInfo, curImageState, sleInt32, VER(48)),
		MKLINE(SpriteInfo, curImage, sleInt32, VER(48)),
		MKLINE(SpriteInfo, imglistNum, sleInt32, VER(48)),
		MKLINE(SpriteInfo, shadow, sleInt32, VER(48)),
		MKLINE(SpriteInfo, imageStateCount, sleInt32, VER(48)),
		MKLINE(SpriteInfo, angle, sleInt32, VER(48)),
		MKLINE(SpriteInfo, scale, sleInt32, VER(48)),
		MKLINE(SpriteInfo, animProgress, sleInt32, VER(48)),
		MKLINE(SpriteInfo, curAngle, sleInt32, VER(48)),
		MKLINE(SpriteInfo, curScale, sleInt32, VER(48)),
		MKLINE(SpriteInfo, curImgFlags, sleInt32, VER(48)),
		MKLINE(SpriteInfo, field_74, sleInt32, VER(48)),
		MKLINE(SpriteInfo, animSpeed, sleInt32, VER(48)),
		MKLINE(SpriteInfo, sourceImage, sleInt32, VER(48)),
		MKLINE(SpriteInfo, maskImage, sleInt32, VER(48)),
		MKLINE(SpriteInfo, field_84, sleInt32, VER(48)),
		MKLINE(SpriteInfo, classFlags, sleInt32, VER(48)),
		MKLINE(SpriteInfo, imgFlags, sleInt32, VER(48)),
		MKLINE(SpriteInfo, field_90, sleInt32, VER(48)),
		MKEND()
	};

	static const SaveLoadEntry spriteGroupEntries[] = {
		MKLINE(SpriteGroup, bbox.left, sleInt32, VER(48)),
		MKLINE(SpriteGroup, bbox.top, sleInt32, VER(48)),
		MKLINE(SpriteGroup, bbox.right, sleInt32, VER(48)),
		MKLINE(SpriteGroup, bbox.bottom, sleInt32, VER(48)),
		MKLINE(SpriteGroup, priority, sleInt32, VER(48)),
		MKLINE(SpriteGroup, flags, sleInt32, VER(48)),
		MKLINE(SpriteGroup, tx, sleInt32, VER(48)),
		MKLINE(SpriteGroup, ty, sleInt32, VER(48)),
		MKLINE(SpriteGroup, image, sleInt32, VER(48)),
		MKLINE(SpriteGroup, scaling, sleInt32, VER(48)),
		MKLINE(SpriteGroup, scale_x_ratio_mul, sleInt32, VER(48)),
		MKLINE(SpriteGroup, scale_x_ratio_div, sleInt32, VER(48)),
		MKLINE(SpriteGroup, scale_y_ratio_mul, sleInt32, VER(48)),
		MKLINE(SpriteGroup, scale_y_ratio_div, sleInt32, VER(48)),
		MKEND()
	};

	s->saveLoadArrayOf(_activeSpritesTable, _varNumSprites, sizeof(_activeSpritesTable[0]), spriteEntries);
	s->saveLoadArrayOf(_spriteTable, _varNumSprites, sizeof(_spriteTable[0]), spriteEntries);
	s->saveLoadArrayOf(_spriteGroups, _varNumSpriteGroups, sizeof(_spriteGroups[0]), spriteGroupEntries);
}

} // End of namespace Scumm
