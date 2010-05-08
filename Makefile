# $URL$
# $Id$

#######################################################################
# Default compilation parameters. Normally don't edit these           #
#######################################################################

srcdir      ?= .

DEFINES     := -DHAVE_CONFIG_H
LDFLAGS     :=
INCLUDES    := -I. -I$(srcdir) -I$(srcdir)/engines
LIBS        :=
OBJS        :=
DEPDIR      := .deps

MODULES     :=
MODULE_DIRS :=

# Load the make rules generated by configure
-include config.mk

ifeq "$(HAVE_GCC)" "1"
	CXXFLAGS:= -Wall $(CXXFLAGS)
	# Turn off some annoying and not-so-useful warnings
	CXXFLAGS+= -Wno-long-long -Wno-multichar -Wno-unknown-pragmas -Wno-reorder
	# Enable even more warnings...
	CXXFLAGS+= -Wpointer-arith -Wcast-qual -Wcast-align
	CXXFLAGS+= -Wshadow -Wimplicit -Wnon-virtual-dtor -Wwrite-strings

	# Currently we disable this gcc flag, since it will also warn in cases,
	# where using GCC_PRINTF (means: __attribute__((format(printf, x, y))))
	# is not possible, thus it would fail compiliation with -Werror without
	# being helpful.
	#CXXFLAGS+= -Wmissing-format-attribute

	# Disable RTTI and exceptions, and enable checking of pointers returned by "new"
	CXXFLAGS+= -fno-rtti -fno-exceptions -fcheck-new
endif

ifeq "$(HAVE_CLANG)" "1"
	CXXFLAGS+= -Wno-conversion -Wno-shorten-64-to-32 -Wno-sign-compare -Wno-four-char-constants
endif

#######################################################################
# Default commands - put the necessary replacements in config.mk      #
#######################################################################

CAT     ?= cat
CP      ?= cp
ECHO    ?= printf
INSTALL ?= install
MKDIR   ?= mkdir -p
RM      ?= rm -f
RM_REC  ?= $(RM) -r
ZIP     ?= zip -q

#######################################################################
# Misc stuff - you should never have to edit this                     #
#######################################################################

EXECUTABLE  := scummvm$(EXEEXT)

include $(srcdir)/Makefile.common

# check if configure has been run or has been changed since last run
config.h config.mk: $(srcdir)/configure
ifeq "$(findstring config.mk,$(MAKEFILE_LIST))" "config.mk"
	@echo "Running $(srcdir)/configure with the last specified parameters"
	@sleep 2
	LDFLAGS="$(SAVED_LDFLAGS)" CXX="$(SAVED_CXX)" \
			CXXFLAGS="$(SAVED_CXXFLAGS)" CPPFLAGS="$(SAVED_CPPFLAGS)" \
			ASFLAGS="$(SAVED_ASFLAGS)" WINDRESFLAGS="$(SAVED_WINDRESFLAGS)" \
			$(srcdir)/configure $(SAVED_CONFIGFLAGS)
else
	$(error You need to run $(srcdir)/configure before you can run make. Check $(srcdir)/configure --help for a list of parameters)
endif

ifneq ($(origin port_mk), undefined)
include $(srcdir)/$(port_mk)
endif

