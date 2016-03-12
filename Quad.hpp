#ifndef _quad_h
#define _quad_h

#include "Vector.hpp"

struct Shape;

/* Quad - Cuadrilatero formado por 4 puntos en R3 */
// Cada Quad necesita un B y \delta B, por cada banda de color
struct Quad{    
  Point p[4];
  Shape *parent;
  float r,dr,g,dg,b,db;

    Quad();
    Quad(const Quad &q);

    void operator=(const Quad& q);

    float area() const;
    void split(Quad dQ[4]) const;
    Point center() const;

    Point& operator[](int i);
    const Point& operator[](int i) const;

};



inline Point Quad::center() const {
    return (p[0] + p[1] + p[2] + p[3]) * 0.25;
}

inline Point& Quad::operator[](int i) {
    return p[i];
}

inline const Point& Quad::operator[](int i) const {
    return p[i];
}


#endif //quad_h
