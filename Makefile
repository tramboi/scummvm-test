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

CXXFLAGS:= -Wall $(CXXFLAGS)
# Turn off some annoying and not-so-useful warnings
CXXFLAGS+= -Wno-long-long -Wno-multichar -Wno-unknown-pragmas -Wno-reorder
# Enable even more warnings...
CXXFLAGS+= -pedantic -Wpointer-arith -Wcast-qual -Wcast-align
CXXFLAGS+= -Wshadow -Wimplicit -Wnon-virtual-dtor -Wwrite-strings

# Disable RTTI and exceptions, and enabled checking of pointers returned by "new"
CXXFLAGS+= -fno-rtti -fno-exceptions -fcheck-new

ifneq "$(SCUMMVM_SVN_REVISION)" ""
CXXFLAGS+= -DSCUMMVM_SVN_REVISION=\"$(SCUMMVM_SVN_REVISION)\"
endif

# There is a nice extra warning that flags variables that are potentially
# used before being initialized. Very handy to catch a certain kind of
# bugs. Unfortunately, it only works when optimizations are turned on,
# which is why we normally don't use it.
#CXXFLAGS+= -O -Wuninitialized

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
	@sleep 2s
	LDFLAGS="$(SAVED_LDFLAGS)" CXX="$(SAVED_CXX)" CXXFLAGS="$(SAVED_CXXFLAGS)" CPPFLAGS="$(SAVED_CPPFLAGS)" \
		$(srcdir)/configure $(SAVED_CONFIGFLAGS)
else
	$(error You need to run $(srcdir)/configure before you can run make. Check $(srcdir)/configure --help for a list of parameters)
endif

include $(srcdir)/ports.mk
