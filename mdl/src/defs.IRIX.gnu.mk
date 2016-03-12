#
# Various options for compilation.
#

CXX           := g++
CXXFLAGS      := 
CXXOPTS_debug := -g
CXXOPTS_opt   := -O
CXXOPTS_prof  := -O -pg

CCXX_DEP := g++ -MM

LPATH :=
IPATH := -I$(SRCDIR)
LIBS  := -lm

PRELINK := echo >/dev/null   # Just so it won't do anything.

LIBSUFFIX := .a
