# Requires GNU Make.
#
# Note that this makefile executes in two passes, one in the source directory
# and one in the compile directory.  I use the variable MAKELEVEL to decide
# which half of the makefile to execute.


##############################################################################
#####
#####    This part is executed in the source dir
#####
##############################################################################

ifeq ($(MAKELEVEL),0)

#
# Change these to reflect the location packages like mdl
#

export BASEDIR := /afs/graphics.cornell.edu/projects/mdl
export SRCDIR := $(shell pwd)
export MDLDIR := /afs/graphics.cornell.edu/projects/mdl

#
# Gather info about the type of compilation
#   OPSYS -- which operating system (HP-UX, IRIX, etc.)
#   COMP  -- what compiler (sys or gnu)
#   TYPE  -- what type of compilation (debug, opt, or prof)
#
# Note that COMP and TYPE can be overridden on the command line.
#
# TYPEDIR is the extension we use to search for libraries
# COMPILEDIR is the directory where the compilation will happen
#

OPSYS := $(shell uname)
COMP := gnu
TYPE := opt

TYPEDIR := $(OPSYS).$(COMP).$(TYPE)
COMPILEDIR := $(SRCDIR)/$(TYPEDIR)
BASEEXEDIR := $(BASEDIR)/bin
EXEDIR := $(BASEEXEDIR)/$(OPSYS)

export OPSYS COMP TYPE TYPEDIR SRCDIR COMPILEDIR EXEDIR BASEEXEDIR

#
# We will build everything in COMPILEDIR, but first it has to exist, with a
# link from TYPEDIR.
#

ifeq "$(wildcard $(COMPILEDIR))" ""
X := $(shell mkdir $(COMPILEDIR))
endif
ifeq "$(wildcard $(EXEDIR))" ""
X := $(shell mkdir $(EXEDIR))
endif

#
# To actually do real work, move to the compile directory and
# make again (note that exported variables are carried over).
#

.PHONY: default
default : all

.SUFFIXES:

%:
	@echo; echo Building $@ in $(COMPILEDIR); echo
	@$(MAKE) --no-print-directory -C $(COMPILEDIR) \
                 -f $(SRCDIR)/Makefile $@

#
# How to clean up
#

clean :
	rm -rf *.[gs][ny][us].opt 




##############################################################################
#####
#####    This part is executed recursively in the compile dir
#####
##############################################################################


else

#
# This tells make that it should look in the source directory when it can't 
# find files in the current directory.
#

VPATH := $(SRCDIR) $(SRCDIR)/prettyprint $(SRCDIR)/filters $(SRCDIR)/converters

#
# A pattern matching rule to make object files from C++ source files
#

.SUFFIXES:
.SUFFIXES: .o .C .c

.C.o:
	$(CPP) $(FLAGS) -c $< -o $@

.c.o:
	$(CPP) $(FLAGS) -c $< -o $@


#
# Add some options to the defaults
#

CPP = g++
FLAGS = -Wall -O3 -I$(SRCDIR)/prettyprint -I$(SRCDIR)/filters -I$(SRCDIR)/converters -I$(MDLDIR)/include

MDLLIB = -L$(MDLDIR)/lib/$(TYPEDIR) -lmdl

LINKFLAGS = $(MDLLIB) -lm

PURIFY = purify -collector=/usr/local/gnu/lib/gcc-lib/hppa1.1-hp-hpux9.05/2.7.2/ld

QUANTIFY = quantify -collector=/usr/local/gnu/lib/gcc-lib/hppa1.1-hp-hpux9.05/2.7.2/ld -collection-granularity=line

all: mdlprettyprint mdlcat mdlchunkcount mdlcopy mdlhistory mdltrianglestrip \
	oldmdl2mdl mdlstrip ply2mdl

mdlprettyprint: mdlprettyprint.c printtoken.c
	$(CPP) $(FLAGS) -o $@ $+ $(LINKFLAGS)
	cp $@ $(EXEDIR)
	ln -sf $(BASEEXEDIR)/arch_redirect_script $(BASEEXEDIR)/$@

mdlcat: mdlcat.c
	$(CPP) $(FLAGS) -o $@ $< $(LINKFLAGS)
	cp $@ $(EXEDIR)
	ln -sf $(BASEEXEDIR)/arch_redirect_script $(BASEEXEDIR)/$@

mdlchunkcount: mdlchunkcount.c
	$(CPP) $(FLAGS) -o $@ $< $(LINKFLAGS)
	cp $@ $(EXEDIR)
	ln -sf $(BASEEXEDIR)/arch_redirect_script $(BASEEXEDIR)/$@

mdlcopy: mdlcopy.c
	$(CPP) $(FLAGS) -o $@ $< $(LINKFLAGS)
	cp $@ $(EXEDIR)
	ln -sf $(BASEEXEDIR)/arch_redirect_script $(BASEEXEDIR)/$@

mdlhistory: mdlhistory.c
	$(CPP) $(FLAGS) -o $@ $< $(LINKFLAGS)
	cp $@ $(EXEDIR)
	ln -sf $(BASEEXEDIR)/arch_redirect_script $(BASEEXEDIR)/$@

mdltrianglestrip: mdltrianglestrip.c
	$(CPP) $(FLAGS) -o $@ $< $(LINKFLAGS)
	cp $@ $(EXEDIR)
	ln -sf $(BASEEXEDIR)/arch_redirect_script $(BASEEXEDIR)/$@

oldmdl2mdl: oldmdl2mdl.c
	$(CPP) $(FLAGS) -o $@ $< $(LINKFLAGS)
	cp $@ $(EXEDIR)
	ln -sf $(BASEEXEDIR)/arch_redirect_script $(BASEEXEDIR)/$@

mdlstrip: mdlstrip.C
	$(CPP) $(FLAGS) -o $@ $< $(LINKFLAGS)
	cp $@ $(EXEDIR)
	ln -sf $(BASEEXEDIR)/arch_redirect_script $(BASEEXEDIR)/$@

ply2mdl: ply2mdl.o plyfile.o
	$(CPP) $(FLAGS) -o $@ ply2mdl.o plyfile.o $(LINKFLAGS)
	cp $@ $(EXEDIR)
	ln -sf $(BASEEXEDIR)/arch_redirect_script $(BASEEXEDIR)/$@


endif

