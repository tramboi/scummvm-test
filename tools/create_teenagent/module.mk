# $URL$
# $Id$

MODULE := tools/create_teenagent

MODULE_OBJS := \
	create_teenagent.o \
	md5.o

# Set the name of the executable
TOOL_EXECUTABLE := create_teenagent

# Include common rules
include $(srcdir)/rules.mk
