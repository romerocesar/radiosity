CXX=g++
CXXFLAGS=-Wall -Wno-deprecated  -O3 -fomit-frame-pointer -march=nocona
#CXXFLAGS=-Wall -Wno-deprecated  -O3 -fomit-frame-pointer -march=nocona -ftracer -fprofile-use
#CXXFLAGS=-Wall -Wno-deprecated -O2 -ggdb
IPATH=-Imdl/src
LIBS=-lm mdl/src/HP-UX.gnu.opt/libmdl.a -lglut -lGL -lGLU -lSDL -lpthread

main: main.o Vector.o Quad.o Shape.o jitter.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

main.o: main.cc Misc.hpp
	$(CXX) $(CXXFLAGS) $(IPATH) -c -o $@ $<

Vector.o: Vector.cpp Vector.hpp
	$(CXX) $(CXXFLAGS) $(IPATH) -c -o $@ $<

Shape.o: Shape.cpp Shape.hpp
	$(CXX) $(CXXFLAGS) $(IPATH) -c -o $@ $<

Quad.o: Quad.cpp Quad.hpp Vector.hpp
	$(CXX) $(CXXFLAGS) $(IPATH) -c -o $@ $<

jitter.o: jitter.cpp jitter.hpp
	$(CXX) $(CXXFLAGS) $(IPATH) -c -o $@ $<

clean:
	rm -rf *.o main
