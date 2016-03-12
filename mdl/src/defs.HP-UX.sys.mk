#
# Various options for compilation.
#

CXX           := CC
CXXFLAGS      := 
CXXOPTS_debug := -g
CXXOPTS_opt   := -O
CXXOPTS_prof  := -O -G

CCXX_DEP := g++ -MM

LPATH :=
IPATH := -I$(SRCDIR)
LIBS  := -lm

ifdef SHARED

CXXFLAGS += +z
PRELINK := echo >/dev/null   # Just so it won't do anything.
LIBSUFFIX := .sl
LDSHARED := $(CXX) -b
COMPSFX := -pic
INSTPFX := s

else

PRELINK := echo >/dev/null   # Just so it won't do anything.
LIBSUFFIX := .a

endif
