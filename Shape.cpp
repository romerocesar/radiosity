#include "Shape.hpp"

// Particiona el Quad Q en NxM partes iguales con un area maxima
// maxarea. 
void Shape::part(float maxarea)
{
  int n=parts(maxarea,this->vertices);
  //  std::cout<<n<<std::endl;
  Vector dx=(this->vertices[1]-this->vertices[0])*(1.0/n);
  Vector dy=(this->vertices[3]-this->vertices[0])*(1.0/n);
  Point tmp;

  for(int i=0;i<n;i++){
    tmp=this->vertices[0]+dy*i;
    std::vector<Quad> v;
    for(int j=0;j<n;j++){
      Quad q;
      q[0]=tmp+dx*j;
      q[1]=q[0]+dx;
      q[2]=q[1]+dy;
      q[3]=q[0]+dy;

      q.parent=this;

      v.push_back(q);
    }
    matrix.push_back(v);
  }
}

// Retorna un entero n que indica el numero de patches en cada fila de
// la matriz de patches
int Shape::parts(float maxarea, const Quad& Q)
{
  int j;
  Point center, midpt[4];

  for(int i = 0; i < 4; i++) {
    j = (i + 1) % 4;
    center += Q[i];
    midpt[i] = (Q[i] + Q[j])*0.5;
  }
  center *= 0.25;

  Quad q;
  q[0] = Q[0];
  q[1] = midpt[0];
  q[2] = center;
  q[3] = midpt[3];

  if (q.area() > maxarea)
    return 2*parts(maxarea, q);

  return 2;
}

std::ostream& operator << (std::ostream& os, Shape* s) {
    os << "<Shape { ";
    for (int i=0; i<4; i++){
        os << "(";
        for (int j=0; j<3; j++){
            os << s->vertices[i][j] << ((j!=2)? ", ":"");
        }
        os << ")" << ((i!=3)? " ":"");
    }
    os << " } material="<< s->material->name <<">";
    return os;
}
