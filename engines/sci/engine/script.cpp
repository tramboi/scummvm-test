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

#include "sci/sci.h"
#include "sci/resource.h"
#include "sci/util.h"
#include "sci/engine/features.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/script.h"

#include "common/util.h"

namespace Sci {

#define END Script_None

opcode_format g_opcode_formats[128][4] = {
	/*00*/
	{Script_None}, {Script_None}, {Script_None}, {Script_None},
	/*04*/
	{Script_None}, {Script_None}, {Script_None}, {Script_None},
	/*08*/
	{Script_None}, {Script_None}, {Script_None}, {Script_None},
	/*0C*/
	{Script_None}, {Script_None}, {Script_None}, {Script_None},
	/*10*/
	{Script_None}, {Script_None}, {Script_None}, {Script_None},
	/*14*/
	{Script_None}, {Script_None}, {Script_None}, {Script_SRelative, END},
	/*18*/
	{Script_SRelative, END}, {Script_SRelative, END}, {Script_SVariable, END}, {Script_None},
	/*1C*/
	{Script_SVariable, END}, {Script_None}, {Script_None}, {Script_Variable, END},
	/*20*/
	{Script_SRelative, Script_Byte, END}, {Script_Variable, Script_Byte, END}, {Script_Variable, Script_Byte, END}, {Script_Variable, Script_SVariable, Script_Byte, END},
	/*24 (24=ret)*/
	{Script_End}, {Script_Byte, END}, {Script_Invalid}, {Script_Invalid},
	/*28*/
	{Script_Variable, END}, {Script_Invalid}, {Script_Byte, END}, {Script_Variable, Script_Byte, END},
	/*2C*/
	{Script_SVariable, END}, {Script_SVariable, Script_Variable, END}, {Script_None}, {Script_Invalid},
	/*30*/
	{Script_None}, {Script_Property, END}, {Script_Property, END}, {Script_Property, END},
	/*34*/
	{Script_Property, END}, {Script_Property, END}, {Script_Property, END}, {Script_Property, END},
	/*38*/
	{Script_Property, END}, {Script_SRelative, END}, {Script_SRelative, END}, {Script_None},
	/*3C*/
	{Script_None}, {Script_None}, {Script_None}, {Script_Word},
	/*40-4F*/
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	/*50-5F*/
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	/*60-6F*/
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	/*70-7F*/
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END}
};
#undef END

// TODO: script_adjust_opcode_formats should probably be part of the
// constructor (?) of a VirtualMachine or a ScriptManager class.
void script_adjust_opcode_formats() {
	if (g_sci->_features->detectLofsType() != SCI_VERSION_0_EARLY) {
		g_opcode_formats[op_lofsa][0] = Script_Offset;
		g_opcode_formats[op_lofss][0] = Script_Offset;
	}

#ifdef ENABLE_SCI32
	// In SCI32, some arguments are now words instead of bytes
	if (getSciVersion() >= SCI_VERSION_2) {
		g_opcode_formats[op_calle][2] = Script_Word;
		g_opcode_formats[op_callk][1] = Script_Word;
		g_opcode_formats[op_super][1] = Script_Word;
		g_opcode_formats[op_send][0] = Script_Word;
		g_opcode_formats[op_self][0] = Script_Word;
		g_opcode_formats[op_call][1] = Script_Word;
		g_opcode_formats[op_callb][1] = Script_Word;
	}
#endif
}

void SegManager::createClassTable() {
	Resource *vocab996 = _resMan->findResource(ResourceId(kResourceTypeVocab, 996), 1);

	if (!vocab996)
		error("SegManager: failed to open vocab 996");

	int totalClasses = vocab996->size >> 2;
	_classTable.resize(totalClasses);

	for (uint16 classNr = 0; classNr < totalClasses; classNr++) {
		uint16 scriptNr = READ_SCI11ENDIAN_UINT16(vocab996->data + classNr * 4 + 2);

		_classTable[classNr].reg = NULL_REG;
		_classTable[classNr].script = scriptNr;
	}

	_resMan->unlockResource(vocab996);
}

reg_t SegManager::getClassAddress(int classnr, ScriptLoadType lock, reg_t caller) {
	if (classnr == 0xffff)
		return NULL_REG;

	if (classnr < 0 || (int)_classTable.size() <= classnr || _classTable[classnr].script < 0) {
		error("[VM] Attempt to dereference class %x, which doesn't exist (max %x)", classnr, _classTable.size());
		return NULL_REG;
	} else {
		Class *the_class = &_classTable[classnr];
		if (!the_class->reg.segment) {
			getScriptSegment(the_class->script, lock);

			if (!the_class->reg.segment) {
				error("[VM] Trying to instantiate class %x by instantiating script 0x%x (%03d) failed;", classnr, the_class->script, the_class->script);
				return NULL_REG;
			}
		} else
			if (caller.segment != the_class->reg.segment)
				getScript(the_class->reg.segment)->incrementLockers();

		return the_class->reg;
	}
}

void SegManager::scriptInitialiseLocals(SegmentId segmentId) {
	Script *scr = getScript(segmentId);

	LocalVariables *locals = allocLocalsSegment(scr);
	if (locals) {
		if (getSciVersion() > SCI_VERSION_0_EARLY) {
			const byte *base = (const byte *)(scr->_buf + scr->getLocalsOffset());

			for (uint16 i = 0; i < scr->getLocalsCount(); i++)
				locals->_locals[i] = make_reg(0, READ_SCI11ENDIAN_UINT16(base + i * 2));
		} else {
			// In SCI0 early, locals are set at run time, thus zero them all here
			for (uint16 i = 0; i < scr->getLocalsCount(); i++)
				locals->_locals[i] = NULL_REG;
		}
	}
}

void SegManager::scriptInitialiseObjectsSci0(SegmentId seg) {
	Script *scr = getScript(seg);
	int objType;
	reg_t addr;
	bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);

	// The script is initialized in 2 passes. 
	// Pass 1: creates a lookup table of all used classes
	// Pass 2: loads classes and objects

	for (uint16 pass = 0; pass <= 1; pass++) {
		uint16 objLength = 0;
		uint16 curOffset = oldScriptHeader ? 2 : 0;

		do {
			objType = READ_SCI11ENDIAN_UINT16(scr->_buf + curOffset);
			if (!objType)
				break;

			objLength = READ_SCI11ENDIAN_UINT16(scr->_buf + curOffset + 2);
			curOffset += 4;		// skip header
			addr = make_reg(seg, curOffset);;

			switch (objType) {
			case SCI_OBJ_OBJECT:
			case SCI_OBJ_CLASS:
				if (pass == 0 && objType == SCI_OBJ_CLASS) {
					int classpos = curOffset + 8;	// SCRIPT_OBJECT_MAGIC_OFFSET
					int species = READ_SCI11ENDIAN_UINT16(scr->_buf + classpos);

					if (species == (int)classTableSize()) {
						// Happens in the LSL2 demo
						warning("Applying workaround for an off-by-one invalid species access");
						resizeClassTable(classTableSize() + 1);
					} else if (species < 0 || species > (int)classTableSize()) {
						error("Invalid species %d(0x%x) not in interval "
								  "[0,%d) while instantiating script at segment %d\n",
								  species, species, classTableSize(),
								  seg);
						return;
					}

					setClassOffset(species, make_reg(seg, classpos));
				} else if (pass == 1) {
					Object *obj = scr->scriptObjInit(addr);
					obj->initSpecies(this, addr);

					if (!obj->initBaseObject(this, addr)) {
						error("Failed to locate base object for object at %04X:%04X; skipping", PRINT_REG(addr));
						//scr->scriptObjRemove(addr);
					}
				}
				break;

			default:
				break;
			}

			curOffset += objLength - 4;
		} while (objType != 0 && curOffset < scr->getScriptSize() - 2);
	}	// for
}

void SegManager::scriptInitialiseObjectsSci11(SegmentId seg) {
	Script *scr = getScript(seg);
	const byte *seeker = scr->_heapStart;;
	uint16 entrySize = READ_SCI11ENDIAN_UINT16(seeker + 2) * 2;
	seeker += entrySize;	// skip first entry
	seeker += 4;			// skip header

	while (READ_SCI11ENDIAN_UINT16(seeker) == SCRIPT_OBJECT_MAGIC_NUMBER) {
		if (READ_SCI11ENDIAN_UINT16(seeker + 14) & kInfoFlagClass) {	// -info- selector
			int classpos = seeker - scr->_buf;
			int species = READ_SCI11ENDIAN_UINT16(seeker + 10);

			if (species < 0 || species >= (int)_classTable.size()) {
				error("Invalid species %d(0x%x) not in interval [0,%d) while instantiating script %d",
				          species, species, _classTable.size(), scr->_nr);
				return;
			}

			setClassOffset(species, make_reg(seg, classpos));
		}
		seeker += READ_SCI11ENDIAN_UINT16(seeker + 2) * 2;
	}

	seeker = scr->_heapStart + 4 + READ_SCI11ENDIAN_UINT16(scr->_heapStart + 2) * 2;
	while (READ_SCI11ENDIAN_UINT16(seeker) == SCRIPT_OBJECT_MAGIC_NUMBER) {
		reg_t reg = make_reg(seg, seeker - scr->_buf);
		Object *obj = scr->scriptObjInit(reg);

		// Copy base from species class, as we need its selector IDs
		obj->setSuperClassSelector(
			getClassAddress(obj->getSuperClassSelector().offset, SCRIPT_GET_LOCK, NULL_REG));

		// If object is instance, get -propDict- from class and set it for this object
		//  This is needed for ::isMemberOf() to work.
		// Example testcase - room 381 of sq4cd - if isMemberOf() doesn't work, talk-clicks on the robot will act like
		//                     clicking on ego
		if (!obj->isClass()) {
			reg_t classObject = obj->getSuperClassSelector();
			Object *classObj = getObject(classObject);
			obj->setPropDictSelector(classObj->getPropDictSelector());
		}

		// Set the -classScript- selector to the script number.
		// FIXME: As this selector is filled in at run-time, it is likely
		// that it is supposed to hold a pointer. The Obj::isKindOf method
		// uses this selector together with -propDict- to compare classes.
		// For the purpose of Obj::isKindOf, using the script number appears
		// to be sufficient.
		obj->setClassScriptSelector(make_reg(0, scr->_nr));

		seeker += READ_SCI11ENDIAN_UINT16(seeker + 2) * 2;
	}
}

int script_instantiate(ResourceManager *resMan, SegManager *segMan, int scriptNum) {
	SegmentId segmentId = segMan->getScriptSegment(scriptNum);
	Script *scr = segMan->getScriptIfLoaded(segmentId);
	if (scr) {
		if (!scr->isMarkedAsDeleted()) {
			scr->incrementLockers();
			return segmentId;
		} else {
			scr->freeScript();
		}
	} else {
		scr = segMan->allocateScript(scriptNum, &segmentId);
	}

	scr->init(scriptNum, resMan);
	scr->load(resMan);
	segMan->scriptInitialiseLocals(segmentId);

	if (getSciVersion() >= SCI_VERSION_1_1) {
		segMan->scriptInitialiseObjectsSci11(segmentId);
		scr->relocate(make_reg(segmentId, READ_SCI11ENDIAN_UINT16(scr->_heapStart)));
	} else {
		segMan->scriptInitialiseObjectsSci0(segmentId);
		byte *relocationBlock = scr->findBlock(SCI_OBJ_POINTERS);
		if (relocationBlock)
			scr->relocate(make_reg(segmentId, relocationBlock - scr->_buf + 4));
	}

	return segmentId;
}

void script_uninstantiate_sci0(SegManager *segMan, int script_nr, SegmentId seg) {
	bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);
	reg_t reg = make_reg(seg, oldScriptHeader ? 2 : 0);
	int objType, objLength = 0;
	Script *scr = segMan->getScript(seg);

	// Make a pass over the object in order uninstantiate all superclasses

	do {
		reg.offset += objLength; // Step over the last checked object

		objType = READ_SCI11ENDIAN_UINT16(scr->_buf + reg.offset);
		if (!objType)
			break;
		objLength = READ_SCI11ENDIAN_UINT16(scr->_buf + reg.offset + 2);

		reg.offset += 4; // Step over header

		if ((objType == SCI_OBJ_OBJECT) || (objType == SCI_OBJ_CLASS)) { // object or class?
			reg.offset += 8;	// magic offset (SCRIPT_OBJECT_MAGIC_OFFSET)
			int superclass = READ_SCI11ENDIAN_UINT16(scr->_buf + reg.offset + 2);

			if (superclass >= 0) {
				int superclass_script = segMan->getClass(superclass).script;

				if (superclass_script == script_nr) {
					if (scr->getLockers())
						scr->decrementLockers();  // Decrease lockers if this is us ourselves
				} else
					script_uninstantiate(segMan, superclass_script);
				// Recurse to assure that the superclass lockers number gets decreased
			}

			reg.offset += SCRIPT_OBJECT_MAGIC_OFFSET;
		} // if object or class

		reg.offset -= 4; // Step back on header

	} while (objType != 0);
}

void script_uninstantiate(SegManager *segMan, int script_nr) {
	SegmentId segment = segMan->getScriptSegment(script_nr);
	Script *scr = segMan->getScriptIfLoaded(segment);

	if (!scr) {   // Is it already loaded?
		//warning("unloading script 0x%x requested although not loaded", script_nr);
		// This is perfectly valid SCI behaviour
		return;
	}

	scr->decrementLockers();   // One less locker

	if (scr->getLockers() > 0)
		return;

	// Free all classtable references to this script
	for (uint i = 0; i < segMan->classTableSize(); i++)
		if (segMan->getClass(i).reg.segment == segment)
			segMan->setClassOffset(i, NULL_REG);

	if (getSciVersion() < SCI_VERSION_1_1)
		script_uninstantiate_sci0(segMan, script_nr, segment);
	// FIXME: Add proper script uninstantiation for SCI 1.1

	if (!scr->getLockers()) {
		// The actual script deletion seems to be done by SCI scripts themselves
		scr->markDeleted();
		debugC(kDebugLevelScripts, "Unloaded script 0x%x.", script_nr);
	}
}


} // End of namespace Sci
