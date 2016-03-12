#include "Quad.hpp"

Quad::Quad():
  p(),parent(),r(),dr(),g(),dg(),b(),db()
{
    // nada por ahora
}

Quad::Quad(const Quad &q) {
    p[0] = q.p[0];
    p[1] = q.p[1];
    p[2] = q.p[2];
    p[3] = q.p[3];

    r=q.r;
    dr=q.dr;
    g=q.g;
    dg=q.dg;
    b=q.b;
    db=q.db;

    parent=q.parent;
}


void Quad::operator=(const Quad& q){
    p[0] = q.p[0];
    p[1] = q.p[1];
    p[2] = q.p[2];
    p[3] = q.p[3];

    r=q.r;
    dr=q.dr;
    g=q.g;
    dg=q.dg;
    b=q.b;
    db=q.db;

    parent=q.parent;
}

float Quad::area() const {
    Vector u = p[2] - p[1];
    Vector v = p[0] - p[1];
    Vector uxv = u * v;
    float area = uxv.norm();
        
    u = p[0] - p[3];
    v = p[2] - p[3];
    uxv = u * v;
    area += uxv.norm();

    return area*0.5;
}

void Quad::split(Quad dQ[4]) const
{
    int i, j;
    Point center, midpt[4];

    /* compute the center of 'Q' and the midpoint of its edges */
    for(i = 0; i < 4; i++) {
        j = (i + 1) % 4;
        center+=p[i];
        midpt[i]=0.5*(p[i]+p[j]);
    }
    center*=0.25;

    /* initialize the four new cuadrilaterals */
    dQ[0][0]=p[0];
    dQ[0][1]=midpt[0];
    dQ[0][2]=center;
    dQ[0][3]=midpt[3];
    dQ[0].parent = parent;

    dQ[1][1]=p[1];
    dQ[1][2]=midpt[1];
    dQ[1][3]=center;
    dQ[1][0]=midpt[0];
    dQ[1].parent = parent;

    dQ[0].parent = parent;
    dQ[2][2]=p[2];
    dQ[2][3]=midpt[2];
    dQ[2][0]=center;
    dQ[2][1]=midpt[1];
    dQ[2].parent = parent;

    dQ[3][3]=p[3];
    dQ[3][0]=midpt[3];
    dQ[3][1]=center;
    dQ[3][2]=midpt[2];
    dQ[3].parent = parent;
}
