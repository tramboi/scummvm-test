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
#include "sci/engine/state.h"
#include "sci/engine/kernel_types.h"
#include "sci/engine/kernel.h"

namespace Sci {

// Loads arbitrary resources of type 'restype' with resource numbers 'resnrs'
// This implementation ignores all resource numbers except the first one.
reg_t kLoad(EngineState *s, int argc, reg_t *argv) {
	int restype = argv[0].toUint16();
	int resnr = argv[1].toUint16();

	// Request to dynamically allocate hunk memory for later use
	if (restype == kResourceTypeMemory)
		return kalloc(s->_segMan, "kLoad()", resnr);

	return make_reg(0, ((restype << 11) | resnr)); // Return the resource identifier as handle
}

reg_t kLock(EngineState *s, int argc, reg_t *argv) {
	int state = argc > 2 ? argv[2].toUint16() : 1;
	ResourceType type = (ResourceType)(argv[0].toUint16() & 0x7f);
	ResourceId id = ResourceId(type, argv[1].toUint16());

	Resource *which;

	switch (state) {
	case 1 :
		s->resMan->findResource(id, 1);
		break;
	case 0 :
		which = s->resMan->findResource(id, 0);

		if (which)
			s->resMan->unlockResource(which);
		else {
			if (id.type == kResourceTypeInvalid)
				warning("[resMan] Attempt to unlock resource %i of invalid type %i", id.number, type);
			else
				warning("[resMan] Attempt to unlock non-existant resource %s", id.toString().c_str());
		}
		break;
	}
	return s->r_acc;
}

// Unloads an arbitrary resource of type 'restype' with resource numbber 'resnr'
reg_t kUnLoad(EngineState *s, int argc, reg_t *argv) {
	int restype = argv[0].toUint16();
	reg_t resnr = argv[1];

	if (restype == kResourceTypeMemory)
		kfree(s->_segMan, resnr);

	return s->r_acc;
}

reg_t kResCheck(EngineState *s, int argc, reg_t *argv) {
	Resource *res = NULL;
	ResourceType restype = (ResourceType)(argv[0].toUint16() & 0x7f);

	if ((restype == kResourceTypeAudio36) || (restype == kResourceTypeSync36)) {
		if (argc >= 6) {
			uint noun = argv[2].toUint16() & 0xff;
			uint verb = argv[3].toUint16() & 0xff;
			uint cond = argv[4].toUint16() & 0xff;
			uint seq = argv[5].toUint16() & 0xff;

			res = s->resMan->testResource(ResourceId(restype, argv[1].toUint16(), noun, verb, cond, seq));
		}
	} else {
		res = s->resMan->testResource(ResourceId(restype, argv[1].toUint16()));
	}

	return make_reg(0, res != NULL);
}

reg_t kClone(EngineState *s, int argc, reg_t *argv) {
	reg_t parent_addr = argv[0];
	Object *parent_obj = s->_segMan->getObject(parent_addr);
	reg_t clone_addr;
	Clone *clone_obj; // same as Object*

	if (!parent_obj) {
		error("Attempt to clone non-object/class at %04x:%04x failed", PRINT_REG(parent_addr));
		return NULL_REG;
	}

	debugC(2, kDebugLevelMemory, "Attempting to clone from %04x:%04x\n", PRINT_REG(parent_addr));

	clone_obj = s->_segMan->allocateClone(&clone_addr);

	if (!clone_obj) {
		error("Cloning %04x:%04x failed-- internal error", PRINT_REG(parent_addr));
		return NULL_REG;
	}

	*clone_obj = *parent_obj;

	// Mark as clone
	clone_obj->setInfoSelector(make_reg(0, SCRIPT_INFO_CLONE));
	clone_obj->setSpeciesSelector(clone_obj->getPos());
	if (parent_obj->isClass())
		clone_obj->setSuperClassSelector(parent_obj->getPos());
	s->_segMan->getScript(parent_obj->getPos().segment)->incrementLockers();
	s->_segMan->getScript(clone_obj->getPos().segment)->incrementLockers();

	return clone_addr;
}

extern void _k_view_list_mark_free(EngineState *s, reg_t off);

reg_t kDisposeClone(EngineState *s, int argc, reg_t *argv) {
	reg_t victim_addr = argv[0];
	Clone *victim_obj = s->_segMan->getObject(victim_addr);

	if (!victim_obj) {
		error("Attempt to dispose non-class/object at %04x:%04x",
		         PRINT_REG(victim_addr));
		return s->r_acc;
	}

	if (victim_obj->getInfoSelector().offset != SCRIPT_INFO_CLONE) {
		//warning("Attempt to dispose something other than a clone at %04x", offset);
		// SCI silently ignores this behaviour; some games actually depend on it
		return s->r_acc;
	}

	// QFG3 clears clones with underbits set
	//if (GET_SEL32V(victim_addr, underBits))
	//	warning("Clone %04x:%04x was cleared with underBits set", PRINT_REG(victim_addr));

#if 0
	if (s->dyn_views) {  // Free any widget associated with the clone
		GfxWidget *widget = gfxw_set_id(gfxw_remove_ID(s->dyn_views, offset), GFXW_NO_ID);

		if (widget && s->bg_widgets)
			s->bg_widgets->add(GFXWC(s->bg_widgets), widget);
	}
#endif

	victim_obj->markAsFreed();

	return s->r_acc;
}

// Returns script dispatch address index in the supplied script
reg_t kScriptID(EngineState *s, int argc, reg_t *argv) {
	int script = argv[0].toUint16();
	int index = (argc > 1) ? argv[1].toUint16() : 0;

	if (argv[0].segment)
		return argv[0];

	SegmentId scriptSeg = s->_segMan->getScriptSegment(script, SCRIPT_GET_LOAD);
	Script *scr;

	if (!scriptSeg)
		return NULL_REG;

	scr = s->_segMan->getScript(scriptSeg);

	if (!scr->_numExports) {
		// FIXME: Is this fatal? This occurs in SQ4CD
		warning("Script 0x%x does not have a dispatch table", script);
		return NULL_REG;
	}

	if (index > scr->_numExports) {
		error("Dispatch index too big: %d > %d", index, scr->_numExports);
		return NULL_REG;
	}

	return make_reg(scriptSeg, s->_segMan->validateExportFunc(index, scriptSeg));
}

reg_t kDisposeScript(EngineState *s, int argc, reg_t *argv) {
	int script = argv[0].offset;

	// Work around QfG1 graveyard bug
	if (argv[0].segment)
		return s->r_acc;

	int id = s->_segMan->getScriptSegment(script);
	Script *scr = s->_segMan->getScriptIfLoaded(id);
	if (scr) {
		if (s->_executionStack.back().addr.pc.segment != id)
			scr->setLockers(1);
	}

	script_uninstantiate(s->_segMan, script);
	s->_executionStackPosChanged = true;

	if (argc != 2) {
		return s->r_acc;
	} else {
		// This exists in the KQ5CD and GK1 interpreter. We know it is used when GK1 starts
		// up, before the Sierra logo.
		warning("kDisposeScript called with 2 parameters, still untested");
		return argv[1];
	}
}

reg_t kIsObject(EngineState *s, int argc, reg_t *argv) {
	if (argv[0].offset == SIGNAL_OFFSET) // Treated specially
		return NULL_REG;
	else
		return make_reg(0, s->_segMan->isHeapObject(argv[0]));
}

reg_t kRespondsTo(EngineState *s, int argc, reg_t *argv) {
	reg_t obj = argv[0];
	int selector = argv[1].toUint16();

	return make_reg(0, s->_segMan->isHeapObject(obj) && lookup_selector(s->_segMan, obj, selector, NULL, NULL) != kSelectorNone);
}

} // End of namespace Sci
