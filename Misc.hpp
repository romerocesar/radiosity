#ifndef _misc_h
#define _misc_h

#include <string>
#include "Vector.hpp"

typedef float Color[3];

struct Camera {
    Point eye;
    Vector direction;
    Vector up;
};

struct Material{
  Color color;
  std::string name;
};



#endif
