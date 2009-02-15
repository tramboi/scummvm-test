/***************************************************************************
 kernel.c Copyright (C) 1999 Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CJR) [jameson@linuxgames.com]

***************************************************************************/

#include "gc.h"
#include <sciresource.h>
#include <engine.h>
#ifdef _WIN32
#	include <windows.h>
#endif /* _WIN32 */

#include <gfx_operations.h>
#include "kernel_types.h"


/* New kernel functions */
reg_t kStrLen(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kGetFarText(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kReadNumber(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kStrCat(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kStrCmp(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kSetSynonyms(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kLock(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kPalette(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kNumCels(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kNumLoops(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kDrawCel(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kCoordPri(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kPriCoord(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kShakeScreen(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kSetCursor(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kMoveCursor(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kShow(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kPicNotValid(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kOnControl(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kDrawPic(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kGetPort(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kSetPort(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kNewWindow(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kDisposeWindow(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kCelWide(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kCelHigh(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kSetJump(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kDirLoop(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kDoAvoider(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kGetAngle(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kGetDistance(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kRandom(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kAbs(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kSqrt(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kTimesSin(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kTimesCos(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kCosMult(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kSinMult(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kTimesTan(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kTimesCot(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kCosDiv(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kSinDiv(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kValidPath(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kFOpen(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kFPuts(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kFGets(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kFClose(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kMapKeyToDir(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kGlobalToLocal(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kLocalToGlobal(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kWait(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kRestartGame(struct _state *s, int funct_nr, int argc, reg_t *argv);
#ifdef _WIN32
reg_t kDeviceInfo_Win32(struct _state *s, int funct_nr, int argc, reg_t *argv);
#else
reg_t kDeviceInfo_Unix(struct _state *s, int funct_nr, int argc, reg_t *argv);
#endif
reg_t kGetEvent(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kCheckFreeSpace(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kFlushResources(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kGetSaveFiles(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kSetDebug(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kCheckSaveGame(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kSaveGame(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kRestoreGame(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kFileIO(struct _state *s, int funct_nr, int argc, reg_t *argp);
reg_t kGetTime(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kHaveMouse(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kJoystick(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kGameIsRestarting(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kGetCWD(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kSort(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kStrEnd(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kMemory(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kAvoidPath(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kParse(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kSaid(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kStrCpy(struct _state *s, int funct_nr, int argc, reg_t *argp);
reg_t kStrAt(struct _state *s, int funct_nr, int argc, reg_t *argp);
reg_t kEditControl(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kDrawControl(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kHiliteControl(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kClone(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kDisposeClone(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kCanBeHere(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kSetNowSeen(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kInitBresen(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kDoBresen(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kBaseSetter(struct _state *s, int funct_nr, int argc, reg_t *argp);
reg_t kAddToPic(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kAnimate(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kDisplay(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kGraph(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kFormat(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kDoSound(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kAddMenu(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kSetMenu(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kGetMenu(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kDrawStatus(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kDrawMenuBar(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kMenuSelect(struct _state *s, int funct_nr, int argc, reg_t *argv);

reg_t kLoad(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kUnLoad(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kScriptID(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kDisposeScript(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kIsObject(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kRespondsTo(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kNewList(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kDisposeList(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kNewNode(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kFirstNode(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kLastNode(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kEmptyList(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kNextNode(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kPrevNode(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kNodeValue(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kAddAfter(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kAddToFront(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kAddToEnd(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kFindKey(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kDeleteKey(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kMemoryInfo(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kGetSaveDir(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kTextSize(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kIsItSkip(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kMessage(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t kDoAudio(struct _state *s, int funct_nr, int argc, reg_t *argv);
reg_t k_Unknown(struct _state *s, int funct_nr, int argc, reg_t *argv);

/* The Unknown/Unnamed kernel function */
reg_t kstub(struct _state *s, int funct_nr, int argc, reg_t *argv);
/* for unimplemented kernel functions */
reg_t kNOP(struct _state *s, int funct_nr, int argc, reg_t *argv);
/* for kernel functions that don't do anything */
reg_t kFsciEmu(struct _state *s, int funct_nr, int argc, reg_t *argv);
/* Emulating "old" kernel functions on the heap */


#define SCI_MAPPED_UNKNOWN_KFUNCTIONS_NR 0x75
/* kfunct_mappers below doubles for unknown kfunctions */

static int sci_max_allowed_unknown_kernel_functions[] = {
	0,
	0x72, /* SCI0 */
	0x7b, /* SCI01/EGA */
	0x7b, /* SCI01/VGA */
	0x7b, /* SCI1/EARLY */
	0x7b, /* SCI1/LATE */
	0x7b, /* SCI1.1 */
	0x0, /* SCI32 */
};

#define DEFUN(nm, cname, sig) {KF_NEW, nm, {cname, sig, NULL}}
#define NOFUN(nm) {KF_NONE, nm, {NULL, NULL, NULL}}

sci_kernel_function_t kfunct_mappers[] = {
/*00*/	DEFUN("Load", kLoad, "iii*"),
/*01*/	DEFUN("UnLoad", kUnLoad, "i.*"),
/*02*/	DEFUN("ScriptID", kScriptID, "Ioi*"),
/*03*/	DEFUN("DisposeScript", kDisposeScript, "Oi"), /* Work around QfG1 bug */
/*04*/	DEFUN("Clone", kClone, "o"),
/*05*/	DEFUN("DisposeClone", kDisposeClone, "o"),
/*06*/	DEFUN("IsObject", kIsObject, "."),
/*07*/	DEFUN("RespondsTo", kRespondsTo, ".i"),
/*08*/	DEFUN("DrawPic", kDrawPic, "i*"),
/*09*/	DEFUN("Show", kShow, "i"),
/*0a*/	DEFUN("PicNotValid", kPicNotValid, "i*"),
/*0b*/	DEFUN("Animate", kAnimate, "LI*"), /* More like (li?)? */
/*0c*/	DEFUN("SetNowSeen", kSetNowSeen, "oi*"), /* The second parameter is ignored */
/*0d*/	DEFUN("NumLoops", kNumLoops, "o"),
/*0e*/	DEFUN("NumCels", kNumCels, "o"),
/*0f*/	DEFUN("CelWide", kCelWide, "iOiOi"),
/*10*/	DEFUN("CelHigh", kCelHigh, "iOiOi"),
/*11*/	DEFUN("DrawCel", kDrawCel, "iiiiii*"),
/*12*/	DEFUN("AddToPic", kAddToPic, "Il*"),
/*13*/	DEFUN("NewWindow", kNewWindow, "iiiiZRi*"),
/*14*/	DEFUN("GetPort", kGetPort, ""),
/*15*/	DEFUN("SetPort", kSetPort, "ii*"),
/*16*/	DEFUN("DisposeWindow", kDisposeWindow, "ii*"),
/*17*/	DEFUN("DrawControl", kDrawControl, "o"),
/*18*/	DEFUN("HiliteControl", kHiliteControl, "o"),
/*19*/	DEFUN("EditControl", kEditControl, "ZoZo"),
/*1a*/	DEFUN("TextSize", kTextSize, "rZrii*r*"),
/*1b*/	DEFUN("Display", kDisplay, ".*"),
/*1c*/	DEFUN("GetEvent", kGetEvent, "io"),
/*1d*/	DEFUN("GlobalToLocal", kGlobalToLocal, "o"),
/*1e*/	DEFUN("LocalToGlobal", kLocalToGlobal, "o"),
/*1f*/	DEFUN("MapKeyToDir", kMapKeyToDir, "o"),
/*20*/	DEFUN("DrawMenuBar", kDrawMenuBar, "i"),
/*21*/	DEFUN("MenuSelect", kMenuSelect, "oi*"),
/*22*/	DEFUN("AddMenu", kAddMenu, "rr"),
/*23*/	DEFUN("DrawStatus", kDrawStatus, "Zri*"),
/*24*/	DEFUN("Parse", kParse, "ro"),
/*25*/	DEFUN("Said", kSaid, "Zr"),
/*26*/	DEFUN("SetSynonyms", kSetSynonyms, "o"),
/*27*/	DEFUN("HaveMouse", kHaveMouse, ""),
/*28*/	DEFUN("SetCursor", kSetCursor, "i*"),
/*28*/	DEFUN("MoveCursor", kMoveCursor, "ii*"),
/*29*/	DEFUN("FOpen", kFOpen, "ri"),
/*2a*/	DEFUN("FPuts", kFPuts, "ir"),
/*2b*/	DEFUN("FGets", kFGets, "rii"),
/*2c*/	DEFUN("FClose", kFClose, "i"),
/*2d*/	DEFUN("SaveGame", kSaveGame, "rirr*"),
/*2e*/	DEFUN("RestoreGame", kRestoreGame, "rir*"),
/*2f*/	DEFUN("RestartGame", kRestartGame, ""),
/*30*/	DEFUN("GameIsRestarting", kGameIsRestarting, "i*"),
/*31*/	DEFUN("DoSound", kDoSound, "iIo*"),
/*32*/	DEFUN("NewList", kNewList, ""),
/*33*/	DEFUN("DisposeList", kDisposeList, "l"),
/*34*/	DEFUN("NewNode", kNewNode, ".."),
/*35*/	DEFUN("FirstNode", kFirstNode, "Zl"),
/*36*/	DEFUN("LastNode", kLastNode, "l"),
/*37*/	DEFUN("EmptyList", kEmptyList, "l"),
/*38*/	DEFUN("NextNode", kNextNode, "n!"),
/*39*/	DEFUN("PrevNode", kPrevNode, "n"),
/*3a*/	DEFUN("NodeValue", kNodeValue, "Zn!"),
/*3b*/	DEFUN("AddAfter", kAddAfter, "lnn"),
/*3c*/	DEFUN("AddToFront", kAddToFront, "ln"),
/*3d*/	DEFUN("AddToEnd", kAddToEnd, "ln"),
/*3e*/	DEFUN("FindKey", kFindKey, "l."),
/*3f*/	DEFUN("DeleteKey", kDeleteKey, "l."),
/*40*/	DEFUN("Random", kRandom, "i*"),
/*41*/	DEFUN("Abs", kAbs, "Oi"),
/*42*/	DEFUN("Sqrt", kSqrt, "i"),
/*43*/	DEFUN("GetAngle", kGetAngle, "iiii"),
/*44*/	DEFUN("GetDistance", kGetDistance, "iiiii*"),
/*45*/	DEFUN("Wait", kWait, "i"),
/*46*/	DEFUN("GetTime", kGetTime, "i*"),
/*47*/	DEFUN("StrEnd", kStrEnd, "r"),
/*48*/	DEFUN("StrCat", kStrCat, "rr"),
/*49*/	DEFUN("StrCmp", kStrCmp, "rri*"),
/*4a*/	DEFUN("StrLen", kStrLen, "r"),
/*4b*/	DEFUN("StrCpy", kStrCpy, "rri*"),
/*4c*/	DEFUN("Format", kFormat, "r.*"),
/*4d*/	DEFUN("GetFarText", kGetFarText, "iir"),
/*4e*/	DEFUN("ReadNumber", kReadNumber, "r"),
/*4f*/	DEFUN("BaseSetter", kBaseSetter, "o"),
/*50*/	DEFUN("DirLoop", kDirLoop, "oi"),
/*51*/	DEFUN("CanBeHere", kCanBeHere, "ol*"),
/*51*/	DEFUN("CantBeHere", kCanBeHere, "ol*"),
/*52*/	DEFUN("OnControl", kOnControl, "i*"),
/*53*/	DEFUN("InitBresen", kInitBresen, "oi*"),
/*54*/	DEFUN("DoBresen", kDoBresen, "o"),
/*55*/	DEFUN("DoAvoider", kDoAvoider, "o"),
/*56*/	DEFUN("SetJump", kSetJump, "oiii"),
/*57*/	DEFUN("SetDebug", kSetDebug, "i*"),
/*58*/	NOFUN("InspectObj"),
/*59*/	NOFUN("ShowSends"),
/*5a*/	NOFUN("ShowObjs"),
/*5b*/	NOFUN("ShowFree"),
/*5c*/	DEFUN("MemoryInfo", kMemoryInfo, "i"),
/*5d*/	NOFUN("StackUsage"),
/*5e*/	NOFUN("Profiler"),
/*5f*/	DEFUN("GetMenu", kGetMenu, "i."),
/*60*/	DEFUN("SetMenu", kSetMenu, "i.*"),
/*61*/	DEFUN("GetSaveFiles", kGetSaveFiles, "rrr"),
/*62*/	DEFUN("GetCWD", kGetCWD, "r"),
/*63*/	DEFUN("CheckFreeSpace", kCheckFreeSpace, "r"),
/*64*/	DEFUN("ValidPath", kValidPath, "r"),
/*65*/	DEFUN("CoordPri", kCoordPri, "i"),
/*66*/	DEFUN("StrAt", kStrAt, "rii*"),
#ifdef _WIN32
/*67*/	DEFUN("DeviceInfo", kDeviceInfo_Win32, "i.*"),
#else /* !_WIN32 */
/*67*/	DEFUN("DeviceInfo", kDeviceInfo_Unix, "i.*"),
#endif
/*68*/	DEFUN("GetSaveDir", kGetSaveDir, ""),
/*69*/	DEFUN("CheckSaveGame", kCheckSaveGame, ".*"),
/*6a*/	DEFUN("ShakeScreen", kShakeScreen, "ii*"),
/*6b*/	DEFUN("FlushResources", kFlushResources, "i"),
/*6c*/	DEFUN("TimesSin", kTimesSin, "ii"),
/*6d*/	DEFUN("TimesCos", kTimesCos, "ii"),
#if 0
/*6e*/	NOFUN(NULL),
/*6f*/	NOFUN(NULL),
#else
/*6e*/	DEFUN("6e", kTimesSin, "ii"),
/*6f*/	DEFUN("6f", kTimesCos, "ii"),
#endif
/*70*/	DEFUN("Graph", kGraph, ".*"),
/*71*/	DEFUN("Joystick", kJoystick, ".*"),
/*72*/	NOFUN(NULL),
/*73*/	NOFUN(NULL),

  /* Experimental functions */
/*74*/	DEFUN("FileIO", kFileIO, "i.*"),
/*(?)*/	DEFUN("Memory", kMemory, "i.*"),
/*(?)*/	DEFUN("Sort", kSort, "ooo"),
/*(?)*/	DEFUN("AvoidPath", kAvoidPath, "ii.*"),
/*(?)*/	DEFUN("Lock", kLock, "iii*"),
/*(?)*/	DEFUN("Palette", kPalette, "i*"),
/*(?)*/	DEFUN("IsItSkip", kIsItSkip, "iiiii"),

  /* Non-experimental Functions without a fixed ID */

	DEFUN("CosMult", kTimesCos, "ii"),
	DEFUN("SinMult", kTimesSin, "ii"),
/*(?)*/	DEFUN("CosDiv", kCosDiv, "ii"),
/*(?)*/	DEFUN("PriCoord", kPriCoord, "i"),
/*(?)*/	DEFUN("SinDiv", kSinDiv, "ii"),
/*(?)*/	DEFUN("TimesCot", kTimesCot, "ii"),
/*(?)*/	DEFUN("TimesTan", kTimesTan, "ii"),
DEFUN("Message", kMessage, ".*"),
DEFUN("DoAudio", kDoAudio, ".*"),


  /* Special and NOP stuff */
	{KF_NEW, NULL, {k_Unknown, NULL, NULL}},

	{KF_TERMINATOR, NULL, {NULL, NULL, NULL}} /* Terminator */
};

static const char *argtype_description[] = {"Undetermined (WTF?)", "List","Node","Object","Reference","Arithmetic"};


/******************** Kernel Oops ********************/

int
kernel_oops(state_t *s, const char *file, int line, const char *reason)
{
	sciprintf("Kernel Oops in file %s, line %d: %s\n", file, line, reason);
	fprintf(stderr,"Kernel Oops in file %s, line %d: %s\n", file, line, reason);
	script_debug_flag = script_error_flag = 1;
	return 0;
}


/* Allocates a set amount of memory for a specified use and returns a handle to it. */
reg_t
kalloc(state_t *s, const char *type, int space)
{
	reg_t reg;

	sm_alloc_hunk_entry(&s->seg_manager, type, space, &reg);
	SCIkdebug(SCIkMEM, "Allocated %d at hunk "PREG" (%s)\n", space, PRINT_REG(reg), type);

	return reg;
}

int
has_kernel_function(state_t *s, const char *kname)
{
	int i = 0;

	while (s->kernel_names[i])
	{
		if (!strcmp(s->kernel_names[i], kname))
			return 1;
		i++;
	}

	return 0;
}

/* Returns a pointer to the memory indicated by the specified handle */
byte *
kmem(state_t *s, reg_t handle)
{
	mem_obj_t *mobj = GET_SEGMENT(s->seg_manager, handle.segment, MEM_OBJ_HUNK);
	hunk_table_t *ht = &(mobj->data.hunks);

	if (!mobj || !ENTRY_IS_VALID(ht, handle.offset)) {
		SCIkwarn(SCIkERROR, "Error: kmem() with invalid handle\n");
		return NULL;
	}

	return (byte *) ht->table[handle.offset].entry.mem;
}

/* Frees the specified handle. Returns 0 on success, 1 otherwise. */
int
kfree(state_t *s, reg_t handle)
{
	sm_free_hunk_entry(&s->seg_manager, handle);

	return 0;
}


/*****************************************/
/************* Kernel functions **********/
/*****************************************/

char *old_save_dir;

reg_t
kRestartGame(state_t *s, int funct_nr, int argc, reg_t *argv)
{
	char *deref_save_dir = (char*)kernel_dereference_bulk_pointer(s, s->save_dir_copy, 1);

	old_save_dir = strdup(deref_save_dir);
	s->restarting_flags |= SCI_GAME_IS_RESTARTING_NOW;
	s->restarting_flags &= ~SCI_GAME_WAS_RESTARTED_AT_LEAST_ONCE; /* This appears to help */
	s->execution_stack_pos = s->execution_stack_base;
	script_abort_flag = 1; /* Force vm to abort ASAP */
	return NULL_REG;
}


/* kGameIsRestarting():
** Returns the restarting_flag in acc
*/
reg_t
kGameIsRestarting(state_t *s, int funct_nr, int argc, reg_t *argv)
{
	char *deref_save_dir = (char*)kernel_dereference_bulk_pointer(s, s->save_dir_copy, 1);

	if (old_save_dir&&deref_save_dir)
	{
		strcpy(deref_save_dir, old_save_dir);
		free(old_save_dir);
		old_save_dir = NULL;
	}

	s->r_acc = make_reg(0, (s->restarting_flags & SCI_GAME_WAS_RESTARTED));

	if (argc) {/* Only happens during replay */
		if (!UKPV(0)) /* Set restarting flag */
			s->restarting_flags &= ~SCI_GAME_WAS_RESTARTED;
	}

	return s->r_acc;
}

reg_t
kHaveMouse(state_t *s, int funct_nr, int argc, reg_t *argv)
{

	return make_reg (0, (s->have_mouse_flag
		  && gfxop_have_mouse(s->gfx_state))? -1 : 0);
}



reg_t
kMemoryInfo(state_t *s, int funct_nr, int argc, reg_t *argv)
{
	switch (argv[0].offset) {
	case 0: /* Total free heap memory */
	case 1: /* Largest heap block available */
	case 2: /* Largest available hunk memory block */
	case 3: /* Total amount of hunk memory */
	case 4: /* Amount of free DOS paragraphs- SCI01 */
		return make_reg(0, 0x7fff); /* Must not be 0xffff, or some memory calculations will overflow */

	default: SCIkwarn(SCIkWARNING, "Unknown MemoryInfo operation: %04x\n", argv[0].offset);
	}
	return NULL_REG;
}


reg_t
k_Unknown(state_t *s, int funct_nr, int argc, reg_t *argv)
{
	if (funct_nr >= SCI_MAPPED_UNKNOWN_KFUNCTIONS_NR) {
		SCIkwarn(SCIkSTUB, "Unhandled Unknown function %04x\n", funct_nr);
		return NULL_REG;
	} else switch(kfunct_mappers[funct_nr].type) {

	case KF_NEW:
		return kfunct_mappers[funct_nr].new.fun(s, funct_nr, argc, argv);

	case KF_NONE:
	default:
		SCIkwarn(SCIkSTUB, "Unhandled Unknown function %04x\n", funct_nr);
		return NULL_REG;
	}
}


reg_t
kFlushResources(state_t *s, int funct_nr, int argc, reg_t *argv)
{
	run_gc(s);
	SCIkdebug(SCIkROOM, "Entering room number %d\n", UKPV(0));
	return s->r_acc;
}

reg_t
kSetDebug(state_t *s, int funct_nr, int argc, reg_t *argv)
{
	sciprintf("Debug mode activated\n");

	script_debug_flag = 1; /* Enter debug mode */
	_debug_seeking = _debug_step_running = 0;
	return s->r_acc;
}

#define _K_NEW_GETTIME_TICKS 0
#define _K_NEW_GETTIME_TIME_12HOUR 1
#define _K_NEW_GETTIME_TIME_24HOUR 2
#define _K_NEW_GETTIME_DATE 3

reg_t
kGetTime(state_t *s, int funct_nr, int argc, reg_t *argv)
{
	struct tm* loc_time;
	GTimeVal time_prec;
	time_t the_time;
	int retval = 0; /* Avoid spurious warning */

#if 0
	/* Reset optimization flags: If this function is called,
	** the game may be waiting for a timeout  */
	s->kernel_opt_flags &= ~(KERNEL_OPT_FLAG_GOT_EVENT
				 | KERNEL_OPT_FLAG_GOT_2NDEVENT);
#endif

#ifdef _WIN32
	if (TIMERR_NOERROR != timeBeginPeriod(1))
	{
		fprintf(stderr, "timeBeginPeriod(1) failed in kGetTime!\n");
	}
#endif /* _WIN32 */

	the_time = time(NULL);
	loc_time = localtime(&the_time);

#ifdef _WIN32
	if (TIMERR_NOERROR != timeEndPeriod(1))
	{
		fprintf(stderr, "timeEndPeriod(1) failed in kGetTime!\n");
	}
#endif /* _WIN32 */

	if (s->version<SCI_VERSION_FTU_NEW_GETTIME) { /* Use old semantics */
		if (argc) { /* Get seconds since last am/pm switch */
			retval = loc_time->tm_sec + loc_time->tm_min * 60 + (loc_time->tm_hour % 12) * 3600;
			SCIkdebug(SCIkTIME, "GetTime(timeofday) returns %d\n", retval);
		} else { /* Get time since game started */
			sci_get_current_time (&time_prec);
			retval = ((time_prec.tv_usec - s->game_start_time.tv_usec) * 60 / 1000000) +
				(time_prec.tv_sec - s->game_start_time.tv_sec) * 60;
			SCIkdebug(SCIkTIME, "GetTime(elapsed) returns %d\n", retval);
		}
	} else {
		int mode = UKPV_OR_ALT(0, 0); /* The same strange method is still used for distinguishing
					         mode 0 and the others. We assume that this is safe, though */
		switch (mode) {
		case _K_NEW_GETTIME_TICKS : {
			sci_get_current_time (&time_prec);
			retval = ((time_prec.tv_usec - s->game_start_time.tv_usec) * 60 / 1000000) +
				  (time_prec.tv_sec - s->game_start_time.tv_sec) * 60;
			SCIkdebug(SCIkTIME, "GetTime(elapsed) returns %d\n", retval);
			break;
		}
		case _K_NEW_GETTIME_TIME_12HOUR : {
			loc_time->tm_hour %= 12;
			retval=(loc_time->tm_min<<6)|(loc_time->tm_hour<<12)|(loc_time->tm_sec);
			SCIkdebug(SCIkTIME, "GetTime(12h) returns %d\n", retval);
			break;
		}
		case _K_NEW_GETTIME_TIME_24HOUR : {
			retval=(loc_time->tm_min<<5)|(loc_time->tm_sec>>1)|(loc_time->tm_hour<<11);
			SCIkdebug(SCIkTIME, "GetTime(24h) returns %d\n", retval);
			break;
		}
		case _K_NEW_GETTIME_DATE : {
			retval=(loc_time->tm_mon<<5)|loc_time->tm_mday|(loc_time->tm_year<<9);
			SCIkdebug(SCIkTIME, "GetTime(date) returns %d\n", retval);
			break;
		}
		default: {
			SCIkdebug(SCIkWARNING, "Attempt to use unknown GetTime mode %d\n", mode);
			break;
		}
		}
	}

	return make_reg(0, retval);
}

#define K_MEMORY_ALLOCATE_CRITICAL 	1
#define K_MEMORY_ALLOCATE_NONCRITICAL   2
#define K_MEMORY_FREE			3
#define	K_MEMORY_MEMCPY			4
#define K_MEMORY_PEEK			5
#define K_MEMORY_POKE			6

reg_t
kMemory(state_t *s, int funct_nr, int argc, reg_t *argv)
{

	switch (UKPV(0)) {

	case K_MEMORY_ALLOCATE_CRITICAL :

		if (!sm_alloc_dynmem(&s->seg_manager, UKPV(1), 
				     "kMemory() critical", &s->r_acc)) {
			SCIkwarn(SCIkERROR, "Critical heap allocation failed\n");
			script_error_flag = script_debug_flag = 1;
		}

		return s->r_acc;
		break;

	case K_MEMORY_ALLOCATE_NONCRITICAL :

		sm_alloc_dynmem(&s->seg_manager, UKPV(1),
				"kMemory() non-critical", &s->r_acc);
		break;

	case K_MEMORY_FREE :

		if (sm_free_dynmem(&s->seg_manager, argv[1])) {
			SCIkwarn(SCIkERROR, "Attempt to kMemory::free() non-dynmem pointer "PREG"!\n",
				 PRINT_REG(argv[1]));
		}
		break;

	case K_MEMORY_MEMCPY : {
		int size = UKPV(3);
		byte *dest = kernel_dereference_bulk_pointer(s, argv[1], size);
		byte *src = kernel_dereference_bulk_pointer(s, argv[2], size);

		if (dest && src)
			memcpy(dest, src, size);
		else {
			SCIkdebug(SCIkWARNING, "Warning: Could not execute kMemory:memcpy of %d bytes:\n", size);
			if (!dest) {
				SCIkdebug(SCIkWARNING, "  dest ptr ("PREG") invalid/memory region too small\n", PRINT_REG(argv[1]));
			}
			if (!src) {
				SCIkdebug(SCIkWARNING, "  src ptr ("PREG") invalid/memory region too small\n", PRINT_REG(argv[2]));
			}
		}

		break;
		}

	case K_MEMORY_PEEK : {
		byte *ref = kernel_dereference_bulk_pointer(s, argv[1], 2);

		if (!ref) {
			SCIkdebug(SCIkERROR, "Attempt to poke invalid memory at "PREG"!\n",
				  PRINT_REG(argv[1]));
			return s->r_acc;
		}
		if (s->seg_manager.heap[argv[1].segment]->type == MEM_OBJ_LOCALS)
			return *((reg_t *) ref); else
			return make_reg(0, getInt16(ref));
	}
		break;

	case K_MEMORY_POKE : {
		byte *ref = kernel_dereference_bulk_pointer(s, argv[1], 2);

		if (!ref) {
			SCIkdebug(SCIkERROR, "Attempt to poke invalid memory at "PREG"!\n",
				  PRINT_REG(argv[1]));
			return s->r_acc;
		}

		if (s->seg_manager.heap[argv[1].segment]->type == MEM_OBJ_LOCALS)
			*((reg_t *) ref) = argv[2]; else
			{
				if (argv[2].segment) {
					SCIkdebug(SCIkERROR, "Attempt to poke memory reference "PREG" to "PREG"!\n",
						  PRINT_REG(argv[2]), PRINT_REG(argv[1]));
					return s->r_acc;
					putInt16(ref, argv[2].offset);
				}
				
			}
		return s->r_acc;
		break;

	}
	}

	return s->r_acc;
}

reg_t
kstub(state_t *s, int funct_nr, int argc, reg_t *argv)
{
	int i;

	SCIkwarn(SCIkWARNING, "Unimplemented syscall: %s[%x](",
		 s->kernel_names[funct_nr], funct_nr);

	for (i = 0; i < argc; i++) {
		sciprintf(PREG, PRINT_REG(argv[i]));
		if (i+1 < argc) sciprintf(", ");
	}
	sciprintf(")\n");
	return NULL_REG;
}


reg_t
kNOP(state_t *s, int funct_nr, int argc, reg_t *argv)
{
	char *problem = (char*)(s->kfunct_table[funct_nr].orig_name ?
		"unmapped" : "NOP");

	SCIkwarn(SCIkWARNING, "Warning: Kernel function 0x%02x invoked: %s", funct_nr, problem);

	if (s->kfunct_table[funct_nr].orig_name &&
	    strcmp(s->kfunct_table[funct_nr].orig_name, SCRIPT_UNKNOWN_FUNCTION_STRING))
	  {
	    sciprintf(" (but its name is known to be %s)", s->kfunct_table[funct_nr].orig_name);
	  }

	sciprintf("\n");
	return NULL_REG;
}



void
kernel_compile_signature(const char **s)
{
	const char *src = *s;
	char *result;
	int ellipsis = 0;
	char v;
	int index = 0;

	if (!src)
		return; /* NULL signature: Nothing to do */

	result = (char*)sci_malloc(strlen(*s) + 1);

	while (*src) {
		char c;
		v = 0;

		if (ellipsis) {
			sciprintf("INTERNAL ERROR when compiling kernel"
				  " function signature '%s': non-terminal ellipsis\n", *s,
				  *src);
			exit(1);
		}

		do {
			char cc;
			cc = c = *src++;
			if (c >= 'A' || c <= 'Z')
				cc = c | KSIG_SPEC_SUM_DONE;

			switch (cc) {

			case KSIG_SPEC_LIST:
				v |= KSIG_LIST;
				break;

			case KSIG_SPEC_NODE:
				v |= KSIG_NODE;
				break;

			case KSIG_SPEC_REF:
				v |= KSIG_REF;
				break;

			case KSIG_SPEC_OBJECT:
				v |= KSIG_OBJECT;
				break;

			case KSIG_SPEC_ARITHMETIC:
				v |= KSIG_ARITHMETIC;
				break;

			case KSIG_SPEC_NULL:
				v |= KSIG_NULL;
				break;

			case KSIG_SPEC_ANY:
				v |= KSIG_ANY;
				break;

			case KSIG_SPEC_ALLOW_INV:
				v |= KSIG_ALLOW_INV;
				break;
			
			case KSIG_SPEC_ELLIPSIS:
				v |= KSIG_ELLIPSIS;
				ellipsis = 1;
				break;

			default: {
				sciprintf("INTERNAL ERROR when compiling kernel"
					  " function signature '%s': (%02x) not understood (aka"
					  " '%c')\n",
					  *s, c, c);
				exit(1);
			}

			}
		} while (*src && (*src == KSIG_SPEC_ALLOW_INV || 
				  *src == KSIG_SPEC_ELLIPSIS || 
				  (c < 'a' && c != KSIG_SPEC_ANY)));

		/* To handle sum types */
		result[index++] = v;
	}

	result[index] = 0;
	*s = result; /* Write back */
}

int
script_map_kernel(state_t *s)
{
	int functnr;
	int mapped = 0;
	int ignored = 0;
	int functions_nr = s->kernel_names_nr;
	int max_functions_nr
		= sci_max_allowed_unknown_kernel_functions[s->resmgr
							   ->sci_version];

	if (functions_nr < max_functions_nr) {
		sciprintf("Warning: SCI version believed to have %d kernel"
			  " functions, but only %d reported-- filling up"
			  " remaining %d\n",
			  max_functions_nr, functions_nr,
			  max_functions_nr - functions_nr);

		functions_nr = max_functions_nr;
	}

	s->kfunct_table = (kfunct_sig_pair_t*)sci_malloc(sizeof(kfunct_sig_pair_t) * functions_nr);


	s->kfunct_nr = functions_nr;

	for (functnr = 0; functnr < functions_nr; functnr++) {
		int seeker, found = -1;
		char *sought_name = NULL;

		if (functnr < s->kernel_names_nr)
			sought_name = s->kernel_names[functnr];

		if (sought_name)
			for (seeker = 0; (found == -1)
				     && kfunct_mappers[seeker].type != KF_TERMINATOR; seeker++)
				if (kfunct_mappers[seeker].name
				    && strcmp(kfunct_mappers[seeker].name, sought_name) == 0)
					found = seeker; /* Found a kernel function with the same name! */

		if (found == -1) {
			if (sought_name)
			{
				sciprintf("Warning: Kernel function %s[%x] unmapped\n",
					  s->kernel_names[functnr], functnr);
				s->kfunct_table[functnr].fun = kNOP;
			} else
			{
				sciprintf("Warning: Flagging kernel function %x as unknown\n",
					  functnr);
				s->kfunct_table[functnr].fun = k_Unknown;
			}

			s->kfunct_table[functnr].signature = NULL;
			s->kfunct_table[functnr].orig_name = sought_name;
		} else switch (kfunct_mappers[found].type) {

		case KF_OLD:
			sciprintf("Emulated kernel function found.\nThis shouldn't happen anymore.");
			return 1;

		case KF_NONE:
			s->kfunct_table[functnr].signature = NULL;
			++ignored;
			break;

		case KF_NEW:
			s->kfunct_table[functnr] = kfunct_mappers[found].new;
			kernel_compile_signature(&(s->kfunct_table[functnr].signature));
			++mapped;
			break;
		}

	} /* for all functions requesting to be mapped */

	sciprintf("Handled %d/%d kernel functions, mapping %d",
		  mapped+ignored, s->kernel_names_nr, mapped);
	if (ignored)
		sciprintf(" and ignoring %d", ignored);
	sciprintf(".\n");
	return 0;
}

void
free_kfunct_tables(state_t *s)
{
	sci_free(s->kfunct_table);
	s->kfunct_table = NULL;

}
int
determine_reg_type(state_t *s, reg_t reg, int allow_invalid)
{
	mem_obj_t *mobj;

	if (!reg.segment) {
		if (!reg.offset)
			return KSIG_ARITHMETIC | KSIG_NULL;
		return KSIG_ARITHMETIC;
	}

	if ((reg.segment >= s->seg_manager.heap_size)
	    || !s->seg_manager.heap[reg.segment])
		return 0; /* Invalid */

	mobj = s->seg_manager.heap[reg.segment];

	switch (mobj->type) {

	case MEM_OBJ_SCRIPT:
		if (reg.offset <= mobj->data.script.buf_size
		    && reg.offset >= -SCRIPT_OBJECT_MAGIC_OFFSET
		    && RAW_IS_OBJECT(mobj->data.script.buf + reg.offset)) {
			int idx = RAW_GET_CLASS_INDEX(&(mobj->data.script), reg);
			if (idx >= 0 && idx < mobj->data.script.objects_nr)
				return KSIG_OBJECT;
			else
				return KSIG_REF;
		} else
			return KSIG_REF;

	case MEM_OBJ_CLONES:
		if (allow_invalid || ENTRY_IS_VALID(&(mobj->data.clones), reg.offset))
			return KSIG_OBJECT;
		else
			return KSIG_OBJECT | KSIG_INVALID;

	case MEM_OBJ_LOCALS:
		if (allow_invalid || reg.offset < mobj->data.locals.nr * sizeof(reg_t))
			return KSIG_REF;
		else
			return KSIG_REF | KSIG_INVALID;

	case MEM_OBJ_STACK:
		if (allow_invalid || reg.offset < mobj->data.stack.nr * sizeof(reg_t))
			return KSIG_REF;
		else
			return KSIG_REF | KSIG_INVALID;

	case MEM_OBJ_SYS_STRINGS:
		if (allow_invalid || (reg.offset < SYS_STRINGS_MAX
		    && mobj->data.sys_strings.strings[reg.offset].name))
			return KSIG_REF;
		else
			return KSIG_REF | KSIG_INVALID;

	case MEM_OBJ_LISTS:
		if (allow_invalid || ENTRY_IS_VALID(&(mobj->data.lists), reg.offset))
			return KSIG_LIST;
		else
			return KSIG_LIST | KSIG_INVALID;

	case MEM_OBJ_NODES:
		if (allow_invalid || ENTRY_IS_VALID(&(mobj->data.nodes), reg.offset))
			return KSIG_NODE;
		else
			return KSIG_NODE | KSIG_INVALID;

	case MEM_OBJ_DYNMEM:
		if (allow_invalid || reg.offset < mobj->data.dynmem.size)
			return KSIG_REF;
		else
			return KSIG_REF | KSIG_INVALID;

	default:
		return 0;

	}
}

const char *
kernel_argtype_description(int type)
{
    type &= ~KSIG_INVALID;
    
    return argtype_description[sci_ffs(type)];
}

int
kernel_matches_signature(state_t *s, const char *sig, int argc, reg_t *argv)
{
	if (!sig)
		return 1;

	while (*sig && argc) {
		if ((*sig & KSIG_ANY) != KSIG_ANY) {
			int type = determine_reg_type(s, *argv, *sig & KSIG_ALLOW_INV);

			if (!type) {
				sciprintf("[KERN] Could not determine type of ref "PREG";"
					  " failing signature check\n",
					  PRINT_REG(*argv));
				return 0;
			}
			
			if (type & KSIG_INVALID) {
				sciprintf("[KERN] ref "PREG" was determined to be a %s, "
					  "but the reference itself is invalid\n",
					  PRINT_REG(*argv), kernel_argtype_description(type));
				return 0;
			}

			if (!(type & *sig))
				return 0;

		}
		if (!(*sig & KSIG_ELLIPSIS))
			++sig;
		++argv;
		--argc;
	}

	if (argc)
		return 0; /* Too many arguments */
	else
		return (*sig == 0 || (*sig & KSIG_ELLIPSIS));
}

static inline void *
_kernel_dereference_pointer(struct _state *s, reg_t pointer, int entries, int align)
{
	int maxsize;
	void *retval = sm_dereference(&s->seg_manager, pointer, &maxsize); 

	if (pointer.offset & (align-1)) {
		SCIkdebug(SCIkERROR, "Unaligned pointer read: "PREG" expected with %d alignment!\n",
			  PRINT_REG(pointer), align);
		return NULL;
	}

	if (entries > maxsize) {
		SCIkdebug(SCIkERROR, "Trying to dereference pointer "PREG" beyond end of segment!\n",
			  PRINT_REG(pointer));
		return NULL;
	}
		return retval;

}

byte *
kernel_dereference_bulk_pointer(struct _state *s, reg_t pointer, int entries)
{
	return (byte*)_kernel_dereference_pointer(s, pointer, entries, 1);
}


reg_t *
kernel_dereference_reg_pointer(struct _state *s, reg_t pointer, int entries)
{
	return (reg_t*)_kernel_dereference_pointer(s, pointer, entries, sizeof(reg_t));
}
