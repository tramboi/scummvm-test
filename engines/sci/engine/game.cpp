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

#include "common/system.h"
#include "common/file.h"

#include "sci/sci.h"
#include "sci/resource.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/kernel_types.h"
#include "sci/gui/gui.h"
#include "sci/gfx/gfx_widgets.h"
#include "sci/gfx/gfx_state_internal.h"	// required for GfxPort, GfxVisual
#include "sci/gfx/menubar.h"

namespace Sci {

struct OldNewIdTableEntry {
	const char *oldId;
	const char *newId;
	bool demo;
};

static const OldNewIdTableEntry s_oldNewTable[] = {
	{ "demo",		"christmas1988",	false },
	// iceman is the same
	{ "icedemo",	"iceman",			true },
	// longbow is the same
	{ "rh",			"longbow",			true },
	{ "eco2",		"ecoquest2",		true },		// EcoQuest 2 demo
	{ "rain",		"ecoquest2",		false },	// EcoQuest 2 full
	{ "fp",			"freddypharkas",	false },
	{ "emc",		"funseeker",		false },
	{ "gk",			"gk1",				false },
	{ "hoyledemo",	"hoyle1",			true },
	{ "cardgames",	"hoyle1",			false },
	{ "solitare",	"hoyle2",			false },
	// hoyle3 is the same
	// hoyle4 is the same
	{ "demo000",	"kq1sci",			true },
	{ "kq1",		"kq1sci",			false },
	{ "kq4",		"kq4sci",			false },
	{ "ll1",		"lsl1sci",			true },
	{ "lsl1",		"lsl1sci",			false },
	// lsl2 is the same
	{ "ll5",		"lsl5",				true },
	// lsl5 is the same
	// lsl6 is the same
	{ "mg",			"mothergoose",		false },
	{ "cb1",		"laurabow",			false },
	{ "lb2",		"laurabow2",		false },
	{ "twisty",		"pepper",			false },
	{ "pq",			"pq2",				false },
	{ "trial",		"qfg2",				false },
	{ "hq2demo",	"qfg2",				true },
	{ "thegame",	"slater",			false },
	{ "sq1demo",	"sq1sci",			true },
	{ "sq1",		"sq1sci",			false },
	// sq5 is the same

	{ 0, 0, 0 }
};

const char *convertSierraGameId(const char *gameName, uint32 *gameFlags) {
	// Convert the id to lower case, so that we match all upper/lower case variants.
	Common::String sierraId = gameName;
	sierraId.toLowercase();

	// TODO: SCI32 IDs

	for (const OldNewIdTableEntry *cur = s_oldNewTable; cur->oldId != 0; ++cur) {
		if (sierraId == cur->oldId) {
			if (cur->demo)
				*gameFlags |= ADGF_DEMO;
			return cur->newId;
		}
	}

	if (sierraId == "card") {
		// This could either be christmas1990 or christmas1992
		// christmas1990 has a "resource.001" file, whereas 
		// christmas1992 has a "resource.000" file
		return (Common::File::exists("resource.001")) ? "christmas1990" : "christmas1992";
	}
	if (sierraId == "arthur") {
		if (!Common::File::exists("resource.002"))
			*gameFlags |= ADGF_DEMO;
		return "camelot";
	}
	if (sierraId == "brain") {
		// This could either be The Castle of Dr. Brain, or The Island of Dr. Brain
		// castlebrain has resource.001, whereas islandbrain doesn't
		return (Common::File::exists("resource.001")) ? "castlebrain" : "islandbrain";
	}
	if (sierraId == "eco") {
		if (!Common::File::exists("resource.000"))
			*gameFlags |= ADGF_DEMO;
		return "ecoquest";
	}
	if (sierraId == "lsl3") {
		if (!Common::File::exists("resource.003"))
			*gameFlags |= ADGF_DEMO;
		return "lsl3";
	}
	// TODO: cnick-lsl, cnick-kq, cnick-laurabow, cnick-longbow and cnick-sq
	// (their resources can't be read)
	if (sierraId == "tales") {
		if (!Common::File::exists("resource.002"))
			*gameFlags |= ADGF_DEMO;
		return "fairytales";
	}
	// TODO: pq1sci (its resources can't be read)
	if (sierraId == "pq3") {
		// The pq3 demo comes with resource.000 and resource.001
		// The full version was released with several resource.* files,
		// or one big resource.000 file
		if (Common::File::exists("resource.000") && Common::File::exists("resource.001") &&
			!Common::File::exists("resource.002"))
			*gameFlags |= ADGF_DEMO;
		return "pq3";
	}
	if (sierraId == "glory" || sierraId == "hq") {
		// This could either be qfg1 or qfg3 or qfg4
		// qfg3 has resource.aud, qfg4 has resource.sfx
		if (Common::File::exists("resource.aud"))
			return "qfg3";
		else if (Common::File::exists("resource.sfx"))
			return "qfg4";
		else
			return "qfg1";
	}
	if (sierraId == "sq3") {
		// Both SQ3 and the separately released subgame, Astro Chicken,
		// have internal ID "sq3", but Astro Chicken only has "resource.map"
		// and "resource.001". Detect if it's SQ3 by the existence of
		// "resource.002"
		return (Common::File::exists("resource.002")) ? "sq3" : "astrochicken";
	}
	if (sierraId == "sq4") {
		// Both SQ4 and the separately released subgame, Ms. Astro Chicken,
		// have internal ID "sq4", but Astro Chicken only has "resource.map"
		// and "resource.001". Detect if it's SQ4 by the existence of
		// "resource.000" (which exists in both SQ4 floppy and CD, but not in
		// the subgame)
		return (Common::File::exists("resource.000")) ? "sq4" : "msastrochicken";
	}

	// FIXME: Evil use of strdup here (we are leaking that memory, too)
	return strdup(sierraId.c_str());
}

int _reset_graphics_input(EngineState *s) {
	Resource *resource;
	int font_nr;
	gfx_color_t transparent = { PaletteEntry(), 0, -1, -1, 0 };
	debug(2, "Initializing graphics");

	if (s->resMan->getViewType() == kViewEga) {
		for (int i = 0; i < 16; i++) {
			gfxop_set_color(s->gfx_state, &(s->ega_colors[i]), gfx_sci0_image_colors[sci0_palette][i].r,
					gfx_sci0_image_colors[sci0_palette][i].g, gfx_sci0_image_colors[sci0_palette][i].b, 0, -1, -1);
			gfxop_set_system_color(s->gfx_state, i, &(s->ega_colors[i]));
		}
	} else {
		// Allocate SCI1 system colors
		gfx_color_t black = { PaletteEntry(0, 0, 0), 0, 0, 0, GFX_MASK_VISUAL };
		gfxop_set_system_color(s->gfx_state, 0, &black);

		// Check for Amiga palette file.
		Common::File file;
		if (file.open("spal")) {
			s->gfx_state->gfxResMan->setStaticPalette(gfxr_read_pal1_amiga(file));
			file.close();
		} else {
			resource = s->resMan->findResource(ResourceId(kResourceTypePalette, 999), 1);
			if (resource) {
				if (getSciVersion() < SCI_VERSION_1_1)
					s->gfx_state->gfxResMan->setStaticPalette(gfxr_read_pal1(999, resource->data, resource->size));
				else
					s->gfx_state->gfxResMan->setStaticPalette(gfxr_read_pal11(999, resource->data, resource->size));
				s->resMan->unlockResource(resource);
			} else {
				debug(2, "Couldn't find the default palette!");
			}
		}
	}

	gfxop_fill_box(s->gfx_state, gfx_rect(0, 0, 320, 200), s->ega_colors[0]); // Fill screen black
	gfxop_update(s->gfx_state);

	s->gui->moveCursor(160, 150);

	s->pic_is_new = 0;
	s->pic_visible_map = GFX_MASK_NONE; // Other values only make sense for debugging
	s->dyn_views = NULL; // no DynViews
	s->drop_views = NULL; // And, consequently, no list for dropped views

	s->priority_first = 42; // Priority zone 0 ends here

	if (s->_kernel->usesOldGfxFunctions())
		s->priority_last = 200;
	else
		s->priority_last = 190;

	font_nr = -1;
	do {
		resource = s->resMan->testResource(ResourceId(kResourceTypeFont, ++font_nr));
	} while ((!resource) && (font_nr < 65536));

	if (!resource) {
		debug(2, "No text font was found.");
		return 1;
	}

	s->visual = new GfxVisual(s->gfx_state, font_nr);

	s->wm_port = new GfxPort(s->visual, s->gfx_state->pic_port_bounds, s->ega_colors[0], transparent);

	s->iconbar_port = new GfxPort(s->visual, gfx_rect(0, 0, 320, 200), s->ega_colors[0], transparent);
	s->iconbar_port->_flags |= GFXW_FLAG_NO_IMPLICIT_SWITCH;

	if (s->resMan->isVGA()) {
		// This bit sets the foreground and background colors in VGA SCI games
		gfx_color_t fgcolor;
		gfx_color_t bgcolor;
		memset(&fgcolor, 0, sizeof(gfx_color_t));
		memset(&bgcolor, 0, sizeof(gfx_color_t));

#if 0
		fgcolor.visual = s->gfx_state->resstate->static_palette[0];
		fgcolor.mask = GFX_MASK_VISUAL;
		bgcolor.visual = s->gfx_state->resstate->static_palette[255];
		bgcolor.mask = GFX_MASK_VISUAL;
#endif
		s->titlebar_port = new GfxPort(s->visual, gfx_rect(0, 0, 320, 10), fgcolor, bgcolor);
	} else {
		s->titlebar_port = new GfxPort(s->visual, gfx_rect(0, 0, 320, 10), s->ega_colors[0], s->ega_colors[15]);
	}
	s->titlebar_port->_color.mask |= GFX_MASK_PRIORITY;
	s->titlebar_port->_color.priority = 11;
	s->titlebar_port->_bgcolor.mask |= GFX_MASK_PRIORITY;
	s->titlebar_port->_bgcolor.priority = 11;
	s->titlebar_port->_flags |= GFXW_FLAG_NO_IMPLICIT_SWITCH;

	// but this is correct
	s->picture_port = new GfxPort(s->visual, s->gfx_state->pic_port_bounds, s->ega_colors[0], transparent);

	s->visual->add((GfxContainer *)s->visual, s->wm_port);
	s->visual->add((GfxContainer *)s->visual, s->titlebar_port);
	s->visual->add((GfxContainer *)s->visual, s->picture_port);
	s->visual->add((GfxContainer *)s->visual, s->iconbar_port);
	// Add ports to visual

	s->port = s->picture_port; // Currently using the picture port

#if 0
	s->titlebar_port->_bgcolor.mask |= GFX_MASK_PRIORITY;
	s->titlebar_port->_bgcolor.priority = 11; // Standard priority for the titlebar port
#endif

	return 0;
}

int game_init_graphics(EngineState *s) {
#ifdef CUSTOM_GRAPHICS_OPTIONS
#ifndef WITH_PIC_SCALING
	if (s->gfx_state->options->pic0_unscaled == 0)
		warning("Pic scaling was disabled; your version of ScummVM has no support for scaled pic drawing built in.");

	s->gfx_state->options->pic0_unscaled = 1;
#endif
#endif
	return _reset_graphics_input(s);
}

static void _free_graphics_input(EngineState *s) {
	debug(2, "Freeing graphics");

	delete s->visual;

	s->wm_port = s->titlebar_port = s->picture_port = NULL;
	s->visual = NULL;
	s->dyn_views = NULL;
	s->port = NULL;
}

int game_init_sound(EngineState *s, int sound_flags) {
	if (getSciVersion() > SCI_VERSION_0_LATE)
		sound_flags |= SFX_STATE_FLAG_MULTIPLAY;

	s->sfx_init_flags = sound_flags;
	s->_sound.sfx_init(s->resMan, sound_flags);

	return 0;
}

// Architectural stuff: Init/Unintialize engine
int script_init_engine(EngineState *s) {
	s->_segMan = new SegManager(s->resMan);
	s->gc_countdown = GC_INTERVAL - 1;

	SegmentId script_000_segment = s->_segMan->getScriptSegment(0, SCRIPT_GET_LOCK);

	if (script_000_segment <= 0) {
		debug(2, "Failed to instantiate script.000");
		return 1;
	}

	s->script_000 = s->_segMan->getScript(script_000_segment);

	s->sys_strings = s->_segMan->allocateSysStrings(&s->sys_strings_segment);
	s->string_frag_segment = s->_segMan->allocateStringFrags();

	// Allocate static buffer for savegame and CWD directories
	SystemString *str = &s->sys_strings->_strings[SYS_STRING_SAVEDIR];
	str->_name = "savedir";
	str->_maxSize = MAX_SAVE_DIR_SIZE;
	str->_value = (char *)calloc(MAX_SAVE_DIR_SIZE, sizeof(char));


	s->r_acc = s->r_prev = NULL_REG;
	s->restAdjust = 0;

	s->_executionStack.clear();    // Start without any execution stack
	s->execution_stack_base = -1; // No vm is running yet

	s->restarting_flags = SCI_GAME_IS_NOT_RESTARTING;

	s->bp_list = NULL; // No breakpoints defined
	s->have_bp = 0;

	if (s->detectLofsType() == SCI_VERSION_1_MIDDLE)
		s->_segMan->setExportAreWide(true);
	else
		s->_segMan->setExportAreWide(false);

	debug(2, "Engine initialized");

	s->pic_priority_table = NULL;

	return 0;
}

void script_free_vm_memory(EngineState *s) {
	debug(2, "Freeing VM memory");

	if (s->_segMan)
		s->_segMan->_classtable.clear();

	// Close all opened file handles
	s->_fileHandles.clear();
	s->_fileHandles.resize(5);
}

void script_free_engine(EngineState *s) {
	script_free_vm_memory(s);

	debug(2, "Freeing state-dependant data");
}

void script_free_breakpoints(EngineState *s) {
	Breakpoint *bp, *bp_next;

	// Free breakpoint list
	bp = s->bp_list;
	while (bp) {
		bp_next = bp->next;
		if (bp->type == BREAK_SELECTOR)
			free(bp->data.name);
		free(bp);
		bp = bp_next;
	}

	s->bp_list = NULL;
}

/*************************************************************/
/* Game instance stuff: Init/Unitialize state-dependant data */
/*************************************************************/

int game_init(EngineState *s) {
	// FIXME Use new VM instantiation code all over the place
	DataStack *stack;

	stack = s->_segMan->allocateStack(VM_STACK_SIZE, &s->stack_segment);
	s->stack_base = stack->_entries;
	s->stack_top = stack->_entries + stack->_capacity;

	if (!script_instantiate(s->resMan, s->_segMan, 0)) {
		warning("game_init(): Could not instantiate script 0");
		return 1;
	}

	s->parserIsValid = false; // Invalidate parser
	s->parser_event = NULL_REG; // Invalidate parser event
	if (s->gfx_state && _reset_graphics_input(s))
		return 1;

	s->successor = NULL; // No successor
	s->_statusBarText.clear(); // Status bar is blank
	s->status_bar_foreground = 0;
	s->status_bar_background = !s->resMan->isVGA() ? 15 : 255;

	SystemString *str = &s->sys_strings->_strings[SYS_STRING_PARSER_BASE];
	str->_name = "parser-base";
	str->_maxSize = MAX_PARSER_BASE;
	str->_value = (char *)calloc(MAX_PARSER_BASE, sizeof(char));

	s->parser_base = make_reg(s->sys_strings_segment, SYS_STRING_PARSER_BASE);

	s->game_start_time = g_system->getMillis();
	s->last_wait_time = s->game_start_time;

	srand(g_system->getMillis()); // Initialize random number generator

//	script_dissect(0, s->_selectorNames);
	// The first entry in the export table of script 0 points to the game object
	s->game_obj = s->_segMan->lookupScriptExport(0, 0);
	uint32 gameFlags = 0;	// unused
	s->_gameName = convertSierraGameId(s->_segMan->getObjectName(s->game_obj), &gameFlags);

	debug(2, " \"%s\" at %04x:%04x", s->_gameName.c_str(), PRINT_REG(s->game_obj));

	s->_menubar = new Menubar(); // Create menu bar

	if (s->sfx_init_flags & SFX_STATE_FLAG_NOSOUND)
		game_init_sound(s, 0);

	// Load game language into printLang property of game object
	s->getLanguage();

	return 0;
}

int game_exit(EngineState *s) {
	s->_executionStack.clear();

	if (!s->successor) {
		s->_sound.sfx_exit();
		// Reinit because some other code depends on having a valid state
		game_init_sound(s, SFX_STATE_FLAG_NOSOUND);
	}

	s->_segMan->_classtable.clear();
	delete s->_segMan;
	s->_segMan = 0;

	debug(2, "Freeing miscellaneous data...");

	// TODO Free parser segment here

	// TODO Free scripts here

	delete s->_menubar;

	_free_graphics_input(s);

	return 0;
}

} // End of namespace Sci
