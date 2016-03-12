#include "Vector.hpp"

/////////////////////////////////////////////////
///////////////// Constructores /////////////////
/////////////////////////////////////////////////
Vector::Vector(){
    v[0] = v[1] = v[2] = 0;
}

Vector::Vector(float v0, float v1, float v2){
    v[0] = v0;
    v[1] = v1;
    v[2] = v2;
}

Vector::Vector(const Vector& v2){
    v[0] = v2.v[0];
    v[1] = v2.v[1];
    v[2] = v2.v[2];
}

///////////////////////////////////////////
///////////////// Metodos /////////////////
///////////////////////////////////////////

void Vector::operator+=(const Vector& p){
    v[0] += p.v[0];
    v[1] += p.v[1];
    v[2] += p.v[2];
}        


void Vector::operator*=(float f){
    v[0] *= f;
    v[1] *= f;
    v[2] *= f;
}

void Vector::operator=(const Vector& p){
    v[0] = p.v[0];
    v[1] = p.v[1];
    v[2] = p.v[2];
}  



//////////////////////////////////////////////
///////////////// Operadores /////////////////
//////////////////////////////////////////////

Vector operator-(const Vector& v1, const Vector& v2){
    Vector v(v1[0]-v2[0],
             v1[1]-v2[1],
             v1[2]-v2[2]);
    return v;
}

Vector operator+(const Vector& v1, const Vector& v2){
    Vector v(v2[0]+v1[0],
             v2[1]+v1[1],
             v2[2]+v1[2]);
    return v;
}

// producto vectorial
Vector operator*(const Vector& s, const Vector& t){
    Vector v;
    v[X] = s[Y] * t[Z] - s[Z] * t[Y];
    v[Y] = s[Z] * t[X] - s[X] * t[Z];   
    v[Z] = s[X] * t[Y] - s[Y] * t[X];
    return v;
}


// producto escalar (por ambos lados)
Vector operator*(float f, const Vector& v){
    Vector v2(v[0]*f, v[1]*f, v[2]*f);
    return v2;
}
Vector operator*(const Vector& v, float f){
    return f*v;
}


std::ostream& operator << (std::ostream& os, const Vector& s) {
    os << "(" << s[0] << ", " << s[1] <<  ", " << s[2] << ")";
    return os;
}

