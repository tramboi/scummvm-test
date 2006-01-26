MODULE := simon

MODULE_OBJS := \
	simon/charset.o \
	simon/cursor.o \
	simon/debug.o \
	simon/debugger.o \
	simon/game.o \
	simon/icons.o \
	simon/items.o \
	simon/midi.o \
	simon/midiparser_s1d.o \
	simon/res.o \
	simon/saveload.o \
	simon/sound.o \
	simon/simon.o \
	simon/verb.o \
	simon/vga.o \

MODULE_DIRS += \
	simon

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
