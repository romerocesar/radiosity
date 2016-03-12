##############################################################################
#####
#####    This part decides what to make and calls make again
#####
##############################################################################

ifeq ($(MAKELEVEL),0)

#
# Gather info about the type of compilation
#   OPSYS -- which operating system (HP-UX, IRIX, etc.)
#   COMP  -- what compiler (sys or gnu)
#   TYPE  -- what type of compilation (debug, opt, or prof)
#
# Note that COMP or TYPE can be overridden on the command line.
#

OPSYS := HP-UX
COMP := gnu
TYPE := opt

include defs.$(OPSYS).$(COMP).mk

export OPSYS COMP TYPE


#
# Where things should be installed
#

MDLROOT := /afs/graphics.cornell.edu/projects/mdl


#
# High-level instructions about how to do things
#

default: library


install: install_debug install_opt install_prof

debug:
	@$(MAKE) --no-print-directory TYPE=debug library

opt:
	@$(MAKE) --no-print-directory TYPE=opt library

prof:
	@$(MAKE) --no-print-directory TYPE=prof library

install_% : %
ifdef SHARED
	cp $(OPSYS).$(COMP)$(COMPSFX).$*/libmdl.sl \
           $(MDLROOT)/lib/$(OPSYS).$(COMP).$*/libs_mdl.sl
else
	cp $(OPSYS).$(COMP)$(COMPSFX).$*/libmdl.a \
           $(MDLROOT)/lib/$(OPSYS).$(COMP).$*/libmdl.a
endif
	cp mdl.H mdlScan.H $(MDLROOT)/include

library:
	@$(MAKE) --no-print-directory $@

test-high:
	@$(MAKE) --no-print-directory $@

new-test:
	@$(MAKE) --no-print-directory $@

depend:
	@$(MAKE) --no-print-directory $@

clean:
	\rm -rf $(OPSYS).$(COMP).debug \
                $(OPSYS).$(COMP).opt \
                $(OPSYS).$(COMP).prof




##############################################################################
#####
#####    This part is used in the source directory to create the compile
#####    directory and call make again
#####
##############################################################################

else
ifeq ($(MAKELEVEL),1)


#
# OBJECTDIR is the directory where the compilation will happen
# These three variables (OPSYS, COMP, TYPE) should come from the enclosing
# 'make'.  We mkdir it if it's not there.
#

include defs.$(OPSYS).$(COMP).mk

OBJECTDIR := $(OPSYS).$(COMP)$(COMPSFX).$(TYPE)
SRCDIR := $(PWD)

export TYPE OBJECTDIR SRCDIR

ifeq "$(wildcard $(OBJECTDIR))" ""
X := $(shell mkdir $(OBJECTDIR))
endif


#
# To actually do real work, descend into OBJECTDIR and run make yet again
# (note that exported variables are carried over). 
#

.SUFFIXES:

%:
	@echo; echo Building $@ in $(OBJECTDIR); echo
	@$(MAKE) --no-print-directory -C $(OBJECTDIR) \
                 -f $(SRCDIR)/Makefile $@




##############################################################################
#####
#####    This part is used in the compile directory
#####
##############################################################################

else


#
# This tells make that it should look in the src directory when it can't find
# files in the current directory.
#

VPATH := $(SRCDIR)


#
# Include the relevant compiler defs file, which contains definitions of
# various variables that specify the name of the compiler, etc.
#

include $(SRCDIR)/defs.$(OPSYS).$(COMP).mk

CXXFLAGS += $(CXXAUX) $(CXXOPTS_$(TYPE))

FLEX = flex



#
# Pattern matching rules for C++ file and lex files
#

.SUFFIXES:

%.o : %.C
	$(CXX) $(CXXFLAGS) $(IPATH) -c -o $@ $<

%.C : %.l %.lopts
	$(FLEX) `cat $(SRCDIR)/$*.lopts` -t $< > $@


#
# The commands to build the mdl library and some test programs
#

library: libmdl$(LIBSUFFIX)

libmdl.a : mdlCompile.o mdlScan.o mdlInput.o mdlOutput.o \
	   mdl_common.o mdl_chunks.o
	$(AR) ruv $@ $?

libmdl.sl : mdlCompile.o mdlScan.o mdlInput.o mdlOutput.o \
	   mdl_common.o mdl_chunks.o
	$(LDSHARED) -o $@ $^

test-high : test-high.o libmdl.a
	$(CXX) $(CXXFLAGS) $(IPATH) $(LPATH) -o $@ $^ $(LIBS)

new-test : new-test.o libmdl.a
	$(CXX) $(CXXFLAGS) $(IPATH) $(LPATH) -o $@ $^ $(LIBS)


#
# How to build depend.mk, which has hidden dependency info and is rebuilt only
# on an explicit "make depend".
#

depend :
	$(CCXX_DEP) $(CPPFLAGS) $(IPATH) $(wildcard $(SRCDIR)/*.C) \
	     > $(SRCDIR)/depend.mk



# Complain if depend.mk does not exist.

ifeq "$(wildcard $(SRCDIR)/depend.mk)" ""
X := $(shell sh -c "echo Warning: depend.mk is missing\; do make depend.")
endif

#
# Get the hidden dependencies
#

ifneq "$(wildcard $(SRCDIR)/depend.mk)" ""
include $(SRCDIR)/depend.mk
endif



endif
endif
