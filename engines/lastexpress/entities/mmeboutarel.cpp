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

#include "lastexpress/entities/mmeboutarel.h"

#include "lastexpress/game/entities.h"
#include "lastexpress/game/inventory.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/sound.h"
#include "lastexpress/game/state.h"

#include "lastexpress/lastexpress.h"
#include "lastexpress/helpers.h"

namespace LastExpress {

MmeBoutarel::MmeBoutarel(LastExpressEngine *engine) : Entity(engine, kEntityMmeBoutarel) {
	ADD_CALLBACK_FUNCTION(MmeBoutarel, reset);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, playSound);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, draw);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, updateFromTime);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, enterExitCompartment);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, enterExitCompartment2);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, updateEntity);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, function8);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, function9);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, chapter1);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, function11);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, chapter1Handler);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, function13);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, function14);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, function15);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, function16);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, chapter2);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, chapter2Handler);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, function19);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, chapter3);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, chapter3Handler);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, chapter4);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, chapter4Handler);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, function24);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, function25);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, chapter5);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, chapter5Handler);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, function28);
	ADD_NULL_FUNCTION();
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, MmeBoutarel, reset)
	Entity::reset(savepoint);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(2, MmeBoutarel, playSound)
	Entity::playSound(savepoint);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(3, MmeBoutarel, draw)
	Entity::draw(savepoint);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(4, MmeBoutarel, updateFromTime, uint32)
	Entity::updateFromTime(savepoint);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(5, MmeBoutarel, enterExitCompartment, ObjectIndex)
	Entity::enterExitCompartment(savepoint);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(6, MmeBoutarel, enterExitCompartment2, ObjectIndex)
	Entity::enterExitCompartment(savepoint, kPosition_5790, kPosition_6130, kCarRedSleeping, kObjectCompartmentD, true);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(7, MmeBoutarel, updateEntity, CarIndex, EntityPosition)
	if (savepoint.action == kActionExcuseMeCath) {
		getInventory()->hasItem(kItemPassengerList) ? getSound()->playSound(kEntityPlayer, "CAT1021") : getSound()->excuseMeCath();

		return;
	}

	Entity::updateEntity(savepoint, true);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(8, MmeBoutarel, function8)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param4 && params->param5) {
			getSavePoints()->push(kEntityMmeBoutarel, kEntityCoudert, kAction125499160);

			if (!getEntities()->isPlayerPosition(kCarRedSleeping, 2))
				getData()->entityPosition = kPosition_2088;

			CALLBACK_ACTION();
		}
		break;

	case kActionEndSound:
		params->param5 = 1;
		break;

	case kActionDefault:
		getEntities()->drawSequenceLeft(kEntityMmeBoutarel, "606U");
		getSavePoints()->push(kEntityMmeBoutarel, kEntityCoudert, kAction169557824);
		break;

	case kAction155853632:
		params->param4 = 1;
		break;

	case kAction202558662:
		getEntities()->drawSequenceLeft(kEntityMmeBoutarel, "606L");
		getSound()->playSound(kEntityMmeBoutarel, (char *)&params->seq1);

		if (getEntities()->hasValidFrame(kEntityMmeBoutarel) || getEntities()->isDistanceBetweenEntities(kEntityMmeBoutarel, kEntityPlayer, 2000)) {
			if (getProgress().chapter == kChapter1)
				getProgress().field_A8 = 1;
			else if (getProgress().chapter == kChapter3)
				getProgress().field_A4 = 1;
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(9, MmeBoutarel, function9)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!params->param1) {
			getData()->entityPosition = getEntityData(kEntityBoutarel)->entityPosition;
			getData()->location = getEntityData(kEntityBoutarel)->location;
		}
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartmentD, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject51, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

		setCallback(1);
		setup_enterExitCompartment("606Rd", kObjectCompartmentD);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->location = kLocationOutsideCompartment;

			setCallback(2);
			setup_updateEntity(kCarRestaurant, kPosition_850);
			break;

		case 2:
			getEntities()->clearSequences(kEntityMmeBoutarel);
			getSavePoints()->push(kEntityMmeBoutarel, kEntityBoutarel, kAction203520448);
			break;

		case 3:
			if (getEntities()->isInsideCompartment(kEntityFrancois, kCarRedSleeping, kPosition_5790)) {
				getObjects()->update(kObjectCompartmentD, kEntityPlayer, kObjectLocationNone, kCursorNormal, kCursorNormal);

				setCallback(4);
				setup_enterExitCompartment2("606Ad", kObjectCompartmentD);
			} else {
				params->param1 = 1;
				getEntities()->drawSequenceLeft(kEntityMmeBoutarel, "606Md");
				getEntities()->enterCompartment(kEntityMmeBoutarel, kObjectCompartmentD, true);
			}
			break;

		case 4:
			getObjects()->update(kObjectCompartmentD, kEntityPlayer, kObjectLocation2, kCursorNormal, kCursorNormal);
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityMmeBoutarel);

			CALLBACK_ACTION();
			break;

		case 5:
			getEntities()->exitCompartment(kEntityMmeBoutarel, kObjectCompartmentD, true);
			getObjects()->update(kObjectCompartmentD, kEntityPlayer, kObjectLocation2, kCursorNormal, kCursorNormal);
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityMmeBoutarel);

			CALLBACK_ACTION();
			break;
		}
		break;

	case kAction100901266:
		setCallback(3);
		setup_updateEntity(kCarRedSleeping, kPosition_5790);
		break;

	case kAction100957716:
		getObjects()->update(kObjectCompartmentD, kEntityPlayer, kObjectLocationNone, kCursorNormal, kCursorNormal);

		setCallback(5);
		setup_enterExitCompartment2("606Ad", kObjectCompartmentD);
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(10, MmeBoutarel, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		TIME_CHECK_CHAPTER1(setup_chapter1Handler);
		break;

	case kActionDefault:
		getSavePoints()->addData(kEntityMmeBoutarel, kAction242526416, 0);

		getObjects()->update(kObjectCompartmentD, kEntityPlayer, kObjectLocation2, kCursorNormal, kCursorNormal);
		getObjects()->update(kObject51, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

		getData()->entityPosition = kPosition_5790;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(11, MmeBoutarel, function11)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param2 == kTimeInvalid)
			break;

		if (params->param1 >= getState()->time) {
			if (!getEntities()->isDistanceBetweenEntities(kEntityMmeBoutarel, kEntityPlayer, 1000) || !params->param2)
				params->param2 = getState()->time + 150;

			if (params->param2 >= getState()->time)
				break;
		}

		params->param2 = kTimeInvalid;

		setCallback(1);
		setup_playSound("MME1040");
		break;

	case kActionDefault:
		params->param1 = getState()->time + 1800;
		getObjects()->update(kObjectCompartmentD, kEntityMmeBoutarel, kObjectLocation1, kCursorNormal, kCursorNormal);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_playSound("MME1040A");
			break;

		case 2:
			setCallback(3);
			setup_playSound("MME1041");
			break;

		case 3:
			setCallback(4);
			setup_updateFromTime(900);
			break;

		case 4:
			CALLBACK_ACTION();
			break;
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(12, MmeBoutarel, chapter1Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		params->param1 = 1;
		getEntities()->drawSequenceLeft(kEntityMmeBoutarel, "501");
		break;

	case kActionDrawScene:
		if (getEntities()->isPlayerPosition(kCarRedSleeping, 44)) {
			setCallback(1);
			setup_draw("502B");
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->drawSequenceLeft(kEntityMmeBoutarel, "502A");
			break;

		case 2:
			getEntities()->drawSequenceLeft(kEntityMmeBoutarel, "606Qd");
			getEntities()->enterCompartment(kEntityMmeBoutarel, kObjectCompartmentD, true);
			break;

		case 3:
			getData()->location = kLocationInsideCompartment;
			params->param1 = 1;
			getEntities()->clearSequences(kEntityMmeBoutarel);
			setup_function13();
			break;
		}
		break;

	case kAction102484312:
		getEntities()->drawSequenceLeft(kEntityMmeBoutarel, "501");
		params->param1 = 1;
		break;

	case kAction134289824:
		getEntities()->drawSequenceLeft(kEntityMmeBoutarel, "502A");
		params->param1 = 0;
		break;

	case kAction168986720:
		getSavePoints()->push(kEntityMmeBoutarel, kEntityFrancois, kAction102752636);
		getSound()->playSound(kEntityMmeBoutarel, "MME1036");
		getEntities()->exitCompartment(kEntityMmeBoutarel, kObjectCompartmentD, true);

		setCallback(3);
		setup_enterExitCompartment("606Fd", kObjectCompartmentD);
		break;

	case kAction202221040:
		getObjects()->update(kObjectCompartmentD, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
		getData()->location = kLocationOutsideCompartment;

		getSound()->playSound(kEntityMmeBoutarel, "MME1035A");

		if (getEntities()->hasValidFrame(kEntityMmeBoutarel) || getEntities()->isDistanceBetweenEntities(kEntityMmeBoutarel, kEntityPlayer, 2000) )
			getProgress().field_AC = 1;

		setCallback(2);
		setup_enterExitCompartment("606Ed", kObjectCompartmentD);
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, MmeBoutarel, function13)
	error("MmeBoutarel: callback function 13 not implemented!");
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, MmeBoutarel, function14)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_enterExitCompartment("606Dd", kObjectCompartmentD);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getObjects()->update(kObjectCompartmentD, kEntityPlayer, kObjectLocation2, kCursorNormal, kCursorNormal);
            getObjects()->update(kObject51, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getEntities()->drawSequenceLeft(kEntityMmeBoutarel, "503");
			break;

		case 2:
			getEntities()->drawSequenceLeft(kEntityMmeBoutarel, "503");

			setCallback(3);
			setup_playSound("MRB1080");
			break;

		case 3:
			getObjects()->update(kObjectCompartmentD, kEntityPlayer, kObjectLocation1, kCursorKeepValue, kCursorKeepValue);

			setCallback(4);
			setup_enterExitCompartment("606Cd", kObjectCompartmentD);
			break;

		case 4:
			getEntities()->clearSequences(kEntityMmeBoutarel);

			setup_function15();
			break;
		}
		break;

	case kAction101107728:
		setCallback(2);
		setup_function9();
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(15, MmeBoutarel, function15)
	error("MmeBoutarel: callback function 15 not implemented!");
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(16, MmeBoutarel, function16)
	if (savepoint.action == kActionDefault) {
		getData()->entityPosition = kPosition_5790;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getObjects()->update(kObjectCompartmentD, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject51, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);

		getEntities()->clearSequences(kEntityMmeBoutarel);
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(17, MmeBoutarel, chapter2)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter2Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityMmeBoutarel);

		getData()->entityPosition = kPosition_4689;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObjectCompartmentD, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject51, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject43, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);

		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, MmeBoutarel, chapter2Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			if (getEntities()->isInsideCompartment(kEntityFrancois, kCarRedSleeping, kPosition_5790)) {
				getObjects()->update(kObjectCompartmentD, kEntityPlayer, kObjectLocationNone, kCursorNormal, kCursorNormal);

				setCallback(2);
				setup_enterExitCompartment2("606Ad", kObjectCompartmentD);
			} else {
				getEntities()->drawSequenceLeft(kEntityMmeBoutarel, "606Md");
				getEntities()->enterCompartment(kEntityMmeBoutarel, kObjectCompartmentD, true);
			}
			break;

		case 2:
		case 3:
			getObjects()->update(kObjectCompartmentD, kEntityPlayer, kObjectLocation2, kCursorNormal, kCursorNormal);
			getData()->location = kLocationInsideCompartment;
			setup_function19();
			break;
		}
		break;

	case kAction100901266:
		setCallback(1);
		setup_updateEntity(kCarRedSleeping, kPosition_5790);
		break;

	case kAction100957716:
		getEntities()->exitCompartment(kEntityMmeBoutarel, kObjectCompartmentD, true);
		getObjects()->update(kObjectCompartmentD, kEntityPlayer, kObjectLocationNone, kCursorNormal, kCursorNormal);

		setCallback(3);
		setup_enterExitCompartment2("606Ad", kObjectCompartmentD);
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(19, MmeBoutarel, function19)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getEntities()->isPlayerPosition(kCarRedSleeping, 44) && !params->param2) {
			if (params->param1) {
				setCallback(1);
				setup_draw("502B");
			} else {
				params->param1 = 1;
			}
		}
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartmentD, kEntityPlayer, kObjectLocation2, kCursorNormal, kCursorNormal);
		getObjects()->update(kObject51, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

		params->param2 = 1;
		getEntities()->drawSequenceLeft(kEntityMmeBoutarel, "501");
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			if (getEntities()->isPlayerPosition(kCarRedSleeping , 44))
				getScenes()->loadSceneFromPosition(kCarRedSleeping, 11);
		}
		break;

	case kAction102484312:
		getEntities()->drawSequenceLeft(kEntityMmeBoutarel, "501");
		params->param2 = 1;
		break;

	case kAction134289824:
		getEntities()->drawSequenceLeft(kEntityMmeBoutarel, "502A");
		params->param2 = 0;
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(20, MmeBoutarel, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter3Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityMmeBoutarel);

		getData()->entityPosition = kPosition_5790;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(21, MmeBoutarel, chapter3Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (ENTITY_PARAM(0, 1) && params->param2 != kTimeInvalid) {

			if (getState()->time <= kTime2038500) {
				if (!getEntities()->isPlayerInCar(kCarRedSleeping)
				 || !params->param1
				 || getSound()->isBuffered("FRA2012")
				 || getSound()->isBuffered("FRA2010")
				 ||!params->param2)
					params->param2 = getState()->time;

				if (params->param2 >= getState()->time)
					break;
			}

			params->param2 = kTimeInvalid;

			getSavePoints()->push(kEntityMmeBoutarel, kEntityFrancois, kAction189872836);
			getObjects()->update(kObjectCompartmentD, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);

			setCallback(1);
			setup_enterExitCompartment("606Cd", kObjectCompartmentD);
		}
		break;

	case kActionDefault:
		params->param1 = 1;

		getObjects()->update(kObjectCompartmentD, kEntityPlayer, kObjectLocation2, kCursorNormal, kCursorNormal);
		getObjects()->update(kObject51, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject43, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
		getEntities()->drawSequenceLeft(kEntityMmeBoutarel, "501");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterExitCompartment("606Rd", kObjectCompartmentD);
			break;

		case 2:
			getData()->location = kLocationOutsideCompartment;

			setCallback(3);
			setup_updateEntity(kCarRedSleeping, kPosition_2000);
			break;

		case 3:
			setCallback(4);
			setup_function8("MME3001");
			break;

		case 4:
			setCallback(5);
			setup_updateEntity(kCarRedSleeping, kPosition_5790);
			break;

		case 5:
			setCallback(6);
			setup_enterExitCompartment2("606Td", kObjectCompartmentD);
			break;

		case 6:
			getEntities()->clearSequences(kEntityMmeBoutarel);
			getObjects()->update(kObjectCompartmentD, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);

			setCallback(7);
			setup_updateFromTime(150);
			break;

		case 7:
			setCallback(8);
			setup_enterExitCompartment("606Dd", kObjectCompartmentD);
			break;

		case 8:
			getObjects()->update(kObjectCompartmentD, kEntityPlayer, kObjectLocation2, kCursorNormal, kCursorNormal);
			getEntities()->drawSequenceLeft(kEntityMmeBoutarel, "501");
			getSavePoints()->push(kEntityMmeBoutarel, kEntityFrancois, kAction190390860);
			break;

		case 9:
			getEntities()->drawSequenceLeft(kEntityMmeBoutarel, "501");
			params->param1 = 1;
			break;
		}
		break;

	case kAction101107728:
		setCallback(9);
		setup_function9();
		break;

	case kAction102484312:
		getEntities()->drawSequenceLeft(kEntityMmeBoutarel, "501");
		params->param1 = 1;
		break;

	case kAction134289824:
		getEntities()->drawSequenceLeft(kEntityMmeBoutarel, "502A");
		params->param1 = 0;
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(22, MmeBoutarel, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter4Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityMmeBoutarel);

		getData()->entityPosition = kPosition_5790;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(23, MmeBoutarel, chapter4Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1) {
			UPDATE_PARAM(params->param2, getState()->time, 900);

			getObjects()->update(kObjectCompartmentD, kEntityPlayer, kObjectLocation1, kCursorKeepValue, kCursorKeepValue);

			setCallback(1);
			setup_enterExitCompartment("606Cd", kObjectCompartmentD);
		}
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartmentD, kEntityPlayer, kObjectLocation2, kCursorNormal, kCursorNormal);
		getObjects()->update(kObject51, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getEntities()->drawSequenceLeft(kEntityMmeBoutarel, "501");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->clearSequences(kEntityMmeBoutarel);
			setup_function24();
			break;

		case 2:
			getEntities()->drawSequenceLeft(kEntityMmeBoutarel, "501");
			params->param1 = 1;
			break;
		}
		break;

	case kAction101107728:
		setCallback(2);
		setup_function9();
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(24, MmeBoutarel, function24)
	error("MmeBoutarel: callback function 24 not implemented!");
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(25, MmeBoutarel, function25)
	if (savepoint.action == kActionDefault) {
		getEntities()->clearSequences(kEntityMmeBoutarel);

		getData()->entityPosition = kPosition_5790;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getObjects()->update(kObjectCompartmentD, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject51, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(26, MmeBoutarel, chapter5)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter5Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityMmeBoutarel);

		getData()->entityPosition = kPosition_3969;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(27, MmeBoutarel, chapter5Handler)
	if (savepoint.action == kActionProceedChapter5)
		setup_function28();
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(28, MmeBoutarel, function28)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1) {
			UPDATE_PARAM(params->param3, getState()->timeTicks, 75);

			params->param1 = 0;
			params->param2 = 1;

			getObjects()->update(kObjectCompartmentD, kEntityMmeBoutarel, kObjectLocation1, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject51, kEntityMmeBoutarel, kObjectLocation1, kCursorNormal, kCursorNormal);
		}

		params->param3 = 0;
		break;

	case kActionKnock:
	case kActionOpenDoor:
		if (params->param1) {
			getObjects()->update(kObjectCompartmentD, kEntityMmeBoutarel, kObjectLocation1, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject51, kEntityMmeBoutarel, kObjectLocation1, kCursorNormal, kCursorNormal);
			params->param1 = 0;

			setCallback(1);
			setup_playSound(getSound()->justCheckingCath());
			break;
		}

		setCallback(savepoint.action == kActionKnock ? 2 : 3);
		setup_playSound(savepoint.action == kActionKnock ? "LIB012" : "LIB013");
		break;

	case kActionDefault:
		getData()->car = kCarRedSleeping;
		getData()->entityPosition = kPosition_5790;
		getData()->location = kLocationInsideCompartment;

		getEntities()->clearSequences(kEntityMmeBoutarel);
		break;

	case kActionDrawScene:
		if (params->param1 || params->param2) {
			params->param1 = 0;
			params->param2 = 0;

			getObjects()->update(kObjectCompartmentD, kEntityMmeBoutarel, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject51, kEntityMmeBoutarel, kObjectLocation1, kCursorHandKnock, kCursorHand);
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getObjects()->update(kObjectCompartmentD, kEntityMmeBoutarel, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject51, kEntityMmeBoutarel, kObjectLocation1, kCursorHandKnock, kCursorHand);
			break;

		case 2:
		case 3:
			getObjects()->update(kObjectCompartmentD, kEntityMmeBoutarel, kObjectLocation1, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject51, kEntityMmeBoutarel, kObjectLocation1, kCursorNormal, kCursorNormal);

			setCallback(4);
			setup_playSound("Mme5001");
			break;

		case 4:
			params->param1 = 1;
			getObjects()->update(kObjectCompartmentD, kEntityMmeBoutarel, kObjectLocation1, kCursorTalk, kCursorNormal);
			getObjects()->update(kObject51, kEntityMmeBoutarel, kObjectLocation1, kCursorTalk, kCursorNormal);
			break;
		}
		break;

	case kAction135800432:
		setup_nullfunction();
		break;

	case kAction155604840:
		getObjects()->update(kObjectCompartmentD, kEntityMmeBoutarel, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject51, kEntityMmeBoutarel, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_NULL_FUNCTION(29, MmeBoutarel)

} // End of namespace LastExpress
