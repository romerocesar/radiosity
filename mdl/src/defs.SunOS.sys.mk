#
# Various options for compilation.
#

CXX           := CC
CXXFLAGS      := 
CXXOPTS_debug := -g
CXXOPTS_opt   := -O
CXXOPTS_prof  := -O -p # is this right?

CCXX_DEP := g++ -MM

LPATH :=
IPATH := -I$(SRCDIR)
LIBS  := -lm

export PRELINK := echo >/dev/null   # Just so it won't do anything.
