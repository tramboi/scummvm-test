MODULE := engines/sci

MODULE_OBJS := \
	console.o \
	decompressor.o \
	detection.o \
	event.o \
	resource.o \
	sci.o \
	engine/features.o \
	engine/game.o \
	engine/gc.o \
	engine/kernel.o \
	engine/kevent.o \
	engine/kfile.o \
	engine/kgraphics.o \
	engine/klists.o \
	engine/kmath.o \
	engine/kmenu.o \
	engine/kmisc.o \
	engine/kmovement.o \
	engine/kparse.o \
	engine/kpathing.o \
	engine/kscripts.o \
	engine/ksound.o \
	engine/kstring.o \
	engine/message.o \
	engine/savegame.o \
	engine/script.o \
	engine/scriptdebug.o \
	engine/selector.o \
	engine/seg_manager.o \
	engine/segment.o \
	engine/state.o \
	engine/static_selectors.o \
	engine/vm.o \
	graphics/animate.o \
	graphics/cache.o \
	graphics/compare.o \
	graphics/controls.o \
	graphics/cursor.o \
	graphics/font.o \
	graphics/gui.o \
	graphics/menu.o \
	graphics/paint16.o \
	graphics/palette.o \
	graphics/picture.o \
	graphics/portrait.o \
	graphics/ports.o \
	graphics/screen.o \
	graphics/text16.o \
	graphics/transitions.o \
	graphics/view.o \
	parser/grammar.o \
	parser/said.o \
	parser/vocabulary.o \
	sound/audio.o \
	sound/midiparser_sci.o \
	sound/music.o \
	sound/soundcmd.o \
	sound/drivers/adlib.o \
	sound/drivers/amiga.o \
	sound/drivers/fb01.o \
	sound/drivers/midi.o \
	sound/drivers/pcjr.o \
	sound/iterator/core.o \
	sound/iterator/iterator.o \
	sound/iterator/songlib.o \
	video/seq_decoder.o
	
	
ifdef ENABLE_SCI32
MODULE_OBJS += \
	engine/kernel32.o \
	graphics/frameout.o \
	graphics/gui32.o \
	graphics/robot.o \
	video/vmd_decoder.o
endif

# This module can be built as a plugin
ifeq ($(ENABLE_SCI), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
