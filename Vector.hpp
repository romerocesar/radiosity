#ifndef _myvector_h
#define _myvector_h

#include <iostream>
#include <cmath>

static const int X=0;
static const int Y=1;
static const int Z=2;

struct Vector{
    float v[3];
    
    Vector();
    Vector(float v0, float v1, float v2);
    Vector(const Vector& v2);

    float norm() const;
    float& operator[](int i);
    const float& operator[](int i) const;
    float dot(const Vector& s) const;

    void operator+=(const Vector& p);
    void operator*=(float f);
    void operator=(const Vector& p);

  void normalize();

};

bool operator==(const Vector&v1, const Vector& v2);

typedef Vector Point;

//////////////////////////////////////////////////
///////////////// Metodos inline /////////////////
//////////////////////////////////////////////////

inline void Vector::normalize(){
  float n = norm();
  v[0] /= n;
  v[1] /= n;
  v[2] /= n;
}

inline float Vector::norm() const {
    return sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

inline float& Vector::operator[](int i) {
    return v[i];
}

inline const float& Vector::operator[](int i) const {
    return v[i];
}

// producto escalar
inline float Vector::dot(const Vector& s) const {
    return v[0]*s[0] + v[1]*s[1] + v[2]*s[2];
}

//////////////////////////////////////////////
///////////////// Operadores /////////////////
//////////////////////////////////////////////

Vector operator-(const Vector& v1, const Vector& v2);
Vector operator+(const Vector& v1, const Vector& v2);

// producto vectorial
Vector operator*(const Vector& s, const Vector& t);

// producto escalar (por ambos lados)
Vector operator*(float f, const Vector& v);
Vector operator*(const Vector& v, float f);

std::ostream& operator << (std::ostream& os, const Vector& s);


inline bool operator==(const Vector&v1, const Vector& v2){
  return v1[0]==v2[0] && v1[1]==v2[1] && v1[2]==v2[2];
}


#endif //myvector_h
