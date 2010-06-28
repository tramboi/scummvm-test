MODULE := engines/testbed
 
MODULE_OBJS := \
	detection.o \
	events.o \
	fs.o \
	graphics.o \
	misc.o \
	savegame.o \
	testbed.o \
	testsuite.o
 
MODULE_DIRS += \
	engines/testbed
 
# This module can be built as a plugin
ifeq ($(ENABLE_TESTBED), DYNAMIC_PLUGIN)
PLUGIN := 1
endif
 
# Include common rules 
include $(srcdir)/rules.mk
