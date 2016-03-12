#ifndef _shape_h
#define _shape_h

#include <vector>
#include <utility>
#include "Vector.hpp"
#include "Quad.hpp"
#include "Misc.hpp"

struct Shape{
  Quad vertices;   
    Material *material;
    Vector normal;
    std::vector<Quad> patches;
  std::vector<std::vector<Quad> > matrix;

    // particiona recursivamente hasta que el area de cada patch < area
  void part(float maxarea);
private:
  int parts(float maxarea, const Quad& Q);
};

std::ostream& operator << (std::ostream& os, Shape* s);

#endif //model_h
