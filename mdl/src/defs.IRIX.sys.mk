#
# Various options for compilation.
#

CXX           := NCC
CXXFLAGS      := 
CXXOPTS_debug := -g
CXXOPTS_opt   := -O
CXXOPTS_prof  := -O -p

CCXX_DEP := g++ -MM

LPATH :=
IPATH := -I$(SRCDIR)
LIBS  := -lm

PRELINK := /usr/lib/DCC/edg_prelink

LIBSUFFIX := .a
