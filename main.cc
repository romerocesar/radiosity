#include <string>
#include <vector>
#include <map>

#include <algorithm>
#include <iterator>
#include <iostream>
#include <iomanip>

#include <sys/time.h>
#include <cmath>
#include <cassert>
#include <getopt.h>
#include <GL/glut.h>

#include "jitter.hpp"
#include "Shape.hpp"
#include "mdl.H"

using namespace std;

static int JITTER_SIZE = 8;
static jitter_point *JITTER_ARR = j8;
static int numpatches = 0;


static int PSIZE = 5000;
static bool TEST_VISIBILITY = true;
static float GAMMA = 1.5;
static int FACTOR = 25;
static bool WIREFRAME = false;
static bool SOFT = true;
static float eps_F = 0.1; 
static float eps_A = 0.5;
static bool ANTIALIAS = true;

// TODO: ver como hacer esteo mas elegante
static float rmax=-1000;
static float gmax=-1000;
static float bmax=-1000;

static float rmin=1000;
static float gmin=1000;
static float bmin=1000;
static float **formfactors;

static vector<Shape*> shapes;
static map<string, Material*> materials;
static Camera *camera;
static vector<Quad *> new_patches;
static vector<Vector> normals;
static vector<float> areas;

struct ThreadData {
  int start;
  int size;
  int *sum;
  pthread_mutex_t *mutexsum;
};


struct SVertex
{
  GLfloat x,y,z;
  GLfloat r,g,b;
};

SVertex *glvertices; // [shape][vx][vy]

float posz = 0;
float posy = 0;
float posx = 0;

inline float sigmoid(float x){
  return 1 / (1 + exp(-x));
}


void createVerticesAndColors(){
  int pos = 0;
  for(unsigned int i=0;i<shapes.size();++i){
    unsigned int n=shapes[i]->matrix.size();
    for(unsigned int j=0;j<n;++j){
      for(unsigned int k=0;k<n;++k, pos+=4){
        // punto 0
        int c=1;
        float r=shapes[i]->matrix[j][k].r;
        float g=shapes[i]->matrix[j][k].g;
        float b=shapes[i]->matrix[j][k].b;
        if(k>0){ 
          r+=shapes[i]->matrix[j][k-1].r;
          g+=shapes[i]->matrix[j][k-1].g;
          b+=shapes[i]->matrix[j][k-1].b;
          c++; 
        }
        if(j>0){ 
          r+=shapes[i]->matrix[j-1][k].r;
          g+=shapes[i]->matrix[j-1][k].g;
          b+=shapes[i]->matrix[j-1][k].b;
          c++;
        }
        if(j>0 && k>0){ 
          r+=shapes[i]->matrix[j-1][k-1].r;
          g+=shapes[i]->matrix[j-1][k-1].g;
          b+=shapes[i]->matrix[j-1][k-1].b;
          c++;
        }
        r=r/c; g=g/c; b=b/c;
        r=(powf(r*FACTOR,1.0/GAMMA)-rmin)/(rmax-rmin); 
        g=(powf(g*FACTOR,1.0/GAMMA)-gmin)/(gmax-gmin); b=(powf(b*FACTOR,1.0/GAMMA)-bmin)/(bmax-bmin);
        //glColor3d(r,g,b);
        glvertices[pos + 0].r = r;
        glvertices[pos + 0].g = g;
        glvertices[pos + 0].b = b;

        glvertices[pos + 0].x = shapes[i]->matrix[j][k][0].v[0];
        glvertices[pos + 0].y = shapes[i]->matrix[j][k][0].v[1];
        glvertices[pos + 0].z = shapes[i]->matrix[j][k][0].v[2];

        // punto 1
        c=1;
        r=shapes[i]->matrix[j][k].r;
        g=shapes[i]->matrix[j][k].g;
        b=shapes[i]->matrix[j][k].b;
        if(k+1<n){ 
          r+=shapes[i]->matrix[j][k+1].r;
          g+=shapes[i]->matrix[j][k+1].g;
          b+=shapes[i]->matrix[j][k+1].b;
          c++; 
        }
        if(j>0){ 
          r+=shapes[i]->matrix[j-1][k].r;
          g+=shapes[i]->matrix[j-1][k].g;
          b+=shapes[i]->matrix[j-1][k].b;
          c++;
        }
        if(j>0 && k+1<n){ 
          r+=shapes[i]->matrix[j-1][k+1].r;
          g+=shapes[i]->matrix[j-1][k+1].g;
          b+=shapes[i]->matrix[j-1][k+1].b;
          c++;
        }
        r=r/c; g=g/c; b=b/c;
        r=(powf(r*FACTOR,1.0/GAMMA)-rmin)/(rmax-rmin); 
        g=(powf(g*FACTOR,1.0/GAMMA)-gmin)/(gmax-gmin); b=(powf(b*FACTOR,1.0/GAMMA)-bmin)/(bmax-bmin);

        glvertices[pos + 1].r = r;
        glvertices[pos + 1].g = g;
        glvertices[pos + 1].b = b;

        glvertices[pos + 1].x = shapes[i]->matrix[j][k][1].v[0];
        glvertices[pos + 1].y = shapes[i]->matrix[j][k][1].v[1];
        glvertices[pos + 1].z = shapes[i]->matrix[j][k][1].v[2];

        // punto 2
        c=1;
        r=shapes[i]->matrix[j][k].r;
        g=shapes[i]->matrix[j][k].g;
        b=shapes[i]->matrix[j][k].b;
        if(k+1<n){ 
          r+=shapes[i]->matrix[j][k+1].r;
          g+=shapes[i]->matrix[j][k+1].g;
          b+=shapes[i]->matrix[j][k+1].b;
          c++; 
        }
        if(j+1<n){ 
          r+=shapes[i]->matrix[j+1][k].r;
          g+=shapes[i]->matrix[j+1][k].g;
          b+=shapes[i]->matrix[j+1][k].b;
          c++;
        }
        if(j+1<n && k+1<n){ 
          r+=shapes[i]->matrix[j+1][k+1].r;
          g+=shapes[i]->matrix[j+1][k+1].g;
          b+=shapes[i]->matrix[j+1][k+1].b;
          c++;
        }
        r=r/c; g=g/c; b=b/c;
        r=(powf(r*FACTOR,1.0/GAMMA)-rmin)/(rmax-rmin);
        g=(powf(g*FACTOR,1.0/GAMMA)-gmin)/(gmax-gmin); b=(powf(b*FACTOR,1.0/GAMMA)-bmin)/(bmax-bmin);
        glvertices[pos + 2].r = r;
        glvertices[pos + 2].g = g;
        glvertices[pos + 2].b = b;

        glvertices[pos + 2].x = shapes[i]->matrix[j][k][2].v[0];
        glvertices[pos + 2].y = shapes[i]->matrix[j][k][2].v[1];
        glvertices[pos + 2].z = shapes[i]->matrix[j][k][2].v[2];

        // punto 3
        c=1;
        r=shapes[i]->matrix[j][k].r;
        g=shapes[i]->matrix[j][k].g;
        b=shapes[i]->matrix[j][k].b;
        if(k>0){ 
          r+=shapes[i]->matrix[j][k-1].r;
          g+=shapes[i]->matrix[j][k-1].g;
          b+=shapes[i]->matrix[j][k-1].b;
          c++; 
        }
        if(j+1<n){ 
          r+=shapes[i]->matrix[j+1][k].r;
          g+=shapes[i]->matrix[j+1][k].g;
          b+=shapes[i]->matrix[j+1][k].b;
          c++;
        }
        if(j+1<n && k>0){ 
          r+=shapes[i]->matrix[j+1][k-1].r;
          g+=shapes[i]->matrix[j+1][k-1].g;
          b+=shapes[i]->matrix[j+1][k-1].b;
          c++;
        }
        r=r/c; g=g/c; b=b/c;
        r=(powf(r*FACTOR,1.0/GAMMA)-rmin)/(rmax-rmin);
        g=(powf(g*FACTOR,1.0/GAMMA)-gmin)/(gmax-gmin);b=(powf(b*FACTOR,1.0/GAMMA)-bmin)/(bmax-bmin);
        glvertices[pos + 3].r = r;
        glvertices[pos + 3].g = g;
        glvertices[pos + 3].b = b;

        glvertices[pos + 3].x = shapes[i]->matrix[j][k][3].v[0];
        glvertices[pos + 3].y = shapes[i]->matrix[j][k][3].v[1];
        glvertices[pos + 3].z = shapes[i]->matrix[j][k][3].v[2];
        
      }
    }
  }

}

void initGL(){
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClearAccum( 0.0, 0.0, 0.0, 0.0 );
  glShadeModel(GL_SMOOTH);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
}

void reshapeGL(int ww, int hh)
{
  glViewport(0,0, ww, hh);
}

void displayGL()
{

  GLint viewport[4];
  glGetIntegerv( GL_VIEWPORT, viewport );
  glLoadIdentity();

  static const GLfloat luz[4][3] =  {{343.0, 548.8, 227.0},
                                     {343.0, 548.8, 332.0},
                                     {213.0, 548.8, 332.0},
                                     {213.0, 548.8, 227.0}};


  if (ANTIALIAS) {
    glClear( GL_ACCUM_BUFFER_BIT );

    for( int jitter=0; jitter<JITTER_SIZE; jitter++ ){
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      accPerspective( 45, (GLdouble)viewport[2] / (GLdouble)viewport[3], 1.0, 10000.0,
                      JITTER_ARR[jitter].x, JITTER_ARR[jitter].y, 0.0, 0.0, 1.0 );
      gluLookAt(camera->eye[0]+posx,
                camera->eye[1],
                camera->eye[2]+posz,
                camera->eye[0]+posx,
                camera->eye[1],
                1,
                camera->up[0],
                camera->up[1],
                camera->up[2]);

      glDrawArrays(GL_QUADS, 0, numpatches*4);    


      glDepthFunc(GL_ALWAYS); // dejame dibujar la luz!
      glBegin(GL_QUADS);
      glColor3f(1.0, 1.0, 1.0);
      for (int i=0; i<4; i++){
        glVertex3fv(luz[i]);
      }
      glEnd();
      glDepthFunc(GL_LEQUAL);


      glAccum( GL_ACCUM, 1.0 / JITTER_SIZE);
    }
    glAccum( GL_RETURN, 1.0 );
  } else {
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    gluPerspective( 45, (GLdouble)viewport[2] / (GLdouble)viewport[3], 1.0, 10000.0);
    gluLookAt(camera->eye[0]+posx,
              camera->eye[1],
              camera->eye[2]+posz,
              camera->eye[0]+posx,
              camera->eye[1],
              1,
              camera->up[0],
              camera->up[1],
              camera->up[2]);
    
    glDrawArrays(GL_QUADS, 0, numpatches*4);

    glDepthFunc(GL_ALWAYS); // dejame dibujar la luz!
    glBegin(GL_QUADS);
    glColor3f(1.0, 1.0, 1.0);
    for (int i=0; i<4; i++){
      glVertex3fv(luz[i]);
    }
    glEnd();
    glDepthFunc(GL_LEQUAL);
  }
  glutSwapBuffers();
}


bool insideT(const Vector &A, const Vector &B, const Vector &C, const Vector &P){
  // Compute vectors        
  Vector v0 = C - A;
  Vector v1 = B - A;
  Vector v2 = P - A;

    // Compute dot products
  double dot00 = v0.dot(v0);
  double dot01 = v0.dot(v1);
  double dot02 = v0.dot(v2);
  double dot11 = v1.dot(v1);
  double dot12 = v1.dot(v2);

    // Compute barycentric coordinates
  double invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
  double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
  double v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    // Check if point is in triangle
  return (u > 0) && (v > 0) && (u + v < 1);
}


bool isVisible(const Point &x1, const Point &x2, Shape* x2parent){
  Vector dif = x2-x1;
  unsigned int ssize = shapes.size();


  for (unsigned int i=0; i<ssize; ++i){        

    if (shapes[i] == x2parent){
      continue;
    }

    Shape &S = *shapes[i];
    Vector &N = S.normal;

    //N.normalize();
    
    float den = dif.dot(N);
    if (den == 0)
      continue;

    float num = N.dot(S.vertices[1]-x1);
    
    float t = num/den;
    // TODO: hacer esto con un epsilon
    if (t <= 0.0 || t >= 1.0)
      continue;

    Vector P = x1 + (dif*t);  
    if( insideT(S.vertices[0], S.vertices[1], S.vertices[2],P) ||
        insideT(S.vertices[2], S.vertices[3], S.vertices[0],P) ){
      return false;
    }

  }

  return true;
}





float computeVisibility(const Point& Pr, const Quad& S)
{
  if (TEST_VISIBILITY){
    int hits = 0;
    for(int i=0; i<4; i++){       
      if (isVisible(Pr, S[i], S.parent))
        hits++;
    }
    return hits/4.0;
  } else {
    return 1.0;
  }
}

static float computeUnoccludedFormFactor(const Point& Pr, const Vector& Nr, const Quad& S)
{
  int i;
  float f, tmp;
  Vector s, t, sxt, s0;

  f = 0.0;
  s=S[3]-Pr;
  s.normalize();
  s0=s;
  for(i = 0; i < 4; i++) {
    if(i < 3) {
      t=S[i]-Pr;
      t.normalize();
    } else
      t=s0;
    sxt=s*t;
    sxt.normalize();
    tmp = max(-1.0f, s.dot(t));
    tmp = min(1.0f, tmp);
    f -= acosf(tmp) * sxt.dot(Nr);
    s=t;
  }
  return(f / (2.0 * M_PI));
}

static float computeFormFactor(const Point& Pr, const Vector& Nr, const Quad& S, const Vector& Ns)
{
  int j;
  Quad dS[4];
  float Frs, vis;

  // esto no funciona por un problema con las normales. ver como resolverlo
//       if(Nr.dot(Ns) >= 0.0)
//           /* the receiving point is oriented away from the source */
//           return 0.0;
    
  if(Ns.dot(Pr-S[0]) <= 0.0)
    /* the receiving point is behind the source */
    return 0.0;


  vis = computeVisibility(Pr, S);
  if(vis <= 0.0)
    Frs = 0.0;
  else if(vis >= 1.0)
    Frs = computeUnoccludedFormFactor(Pr, Nr, S);
  else {
    Frs = computeUnoccludedFormFactor(Pr, Nr, S);
    if(Frs <= eps_F || S.area() <= eps_A)
      return(Frs * vis);
    else {
      S.split(dS);
      Frs = 0.0;
      for(j = 0; j < 4; j++)
        Frs += computeFormFactor(Pr, Nr, dS[j], Ns);
    }
  }


  return(Frs);
}

// float computeContribution(const Point& Pr, const Vector& Nr, float rho, const Quad& S, const Vector& Ns, float Bs, float eps_B, float minA)
// {
//   Vector v;

//   if(Nr.dot(Ns) >= 0.0)
//     /* the receiving point is oriented away from the source */
//     return 0.0;
    
//   v=Pr-S[0];
//   if(Ns.dot(v) <= 0.0)
//     /* the receiving point is behind the source */
//     return 0.0;

//   eps_F = eps_B / (rho * Bs);

//   eps_A = minA;

//   return(rho * Bs * computeFormFactor(Pr, Nr, S, Ns));
// }

//     creo qeu los formfactors hay que carcularlos en cada
//     iteracion. si no, ver que significa la primera parte de cada
//     iteracion en el libro, sera que el del libro es gathering?
// TODO: revisar si es formfactor[i][j] o formfactor[j][i]
void radiosity_iterations(int iterations){
  unsigned int n = new_patches.size();
  for(int k=0;k<iterations;++k){
    for(unsigned int i=0;i<n;++i){
      for(unsigned int j=0;j<n;++j){

        if(i==j) continue;
		
        float* rho= new_patches[j]->parent->material->color;
        float areafactor = areas[i] / areas[j];
		
        float R = rho[0] * new_patches[i]->dr * formfactors[i][j] * areafactor;
        new_patches[j]->r += R;
        new_patches[j]->dr += R;

        R = rho[1] * new_patches[i]->dg * formfactors[i][j] * areafactor;
        new_patches[j]->g += R;
        new_patches[j]->dg += R;

        R = rho[2] * new_patches[i]->db * formfactors[i][j] * areafactor;
        new_patches[j]->b += R;
        new_patches[j]->db += R;
      }
      new_patches[i]->dr = new_patches[i]->dg = new_patches[i]->db = 0;
    }
  }

  for(unsigned int i=0;i<n;++i){
    rmin = min(new_patches[i]->r, rmin);
    rmax = max(new_patches[i]->r, rmax);
    gmin = min(new_patches[i]->g, gmin);
    gmax = max(new_patches[i]->g, gmax);
    bmin = min(new_patches[i]->b, bmin);
    bmax = max(new_patches[i]->b, bmax);
  }

  createVerticesAndColors();

//       cout<<"rmin:"<<rmin<<" rmax:"<<rmax<<endl;
//       cout<<"gmin:"<<gmin<<" gmax:"<<gmax<<endl;
//       cout<<"bmin:"<<bmin<<" bmax:"<<bmax<<endl;
//       cout<<endl;
}


void keyboardGL(unsigned char key, int x, int y){
  if (key=='n'){
    radiosity_iterations(1);
  } else if (key == 'w'){
    posz += 10;
  } else if (key == 's'){
    posz -= 10;
  } else if (key == 'a'){
    posx += 10;
  } else if (key == 'd'){
    posx -= 10;
  } else if (key == 'G'){
    GAMMA+=0.1;
    createVerticesAndColors();
  } else if (key == 'g'){
    if(GAMMA>0.1) GAMMA-=0.1;
    createVerticesAndColors();
  } else if (key == 'F'){
    FACTOR++;
    createVerticesAndColors();
  } else if (key == 'f'){
    FACTOR--;
    createVerticesAndColors();
  } else if (key == 'f'){
    FACTOR--;
    createVerticesAndColors();
  } else if (key == 'm'){
    SOFT = not SOFT;
    if (SOFT)
      glShadeModel(GL_SMOOTH);
    else
      glShadeModel(GL_FLAT);
  } else {
    return;
  }
  //   cout<<"GAMMA:"<<GAMMA<<endl;
  //   cout<<"FACTOR:"<<FACTOR<<endl;
  glutPostRedisplay();
}


void usage(){
  // TODO: poner aqui un mensajito con las opciones y sus posibles valores
  exit(1);
}

void *ffthread(void *arg){
  ThreadData *data = (ThreadData*)arg;
  int end = data->start+data->size;

  for (int i=data->start; i<end; ++i){
    Point center = new_patches[i]->center();
    for (int j=0; j<numpatches; ++j){
      formfactors[i][j] = computeFormFactor(center, normals[i], *new_patches[j], normals[j]);
      if (formfactors[i][j] < 0)
        formfactors[i][j] = 0;
    }

    pthread_mutex_lock (data->mutexsum);
    int tmp = ++*data->sum;
    pthread_mutex_unlock (data->mutexsum);
    if ((tmp % 100) == 0){
      cout << "\r" << tmp << "/" << numpatches;
      cout.flush();
    }
  }
    

  return 0;
}


int main(int argc, char *argv[])
{
  int NUM_ITER = 5;
  int c;
  // TODO: poner una opcion para FACTOR
  while ((c = getopt (argc, argv, "p:n:g:va")) != -1)
    switch (c)  {
    case 'p':      
      PSIZE = strtol(optarg, NULL, 0);
      if (PSIZE <= 0){
        fprintf(stderr, "Argumento invalido: `-p' requiere un numero positivo.\n");
        exit(1);
      }
      break;

    case 'n':      
      NUM_ITER = strtol(optarg, NULL, 0);
      if (NUM_ITER < 0){
        fprintf(stderr, "Argumento invalido: `-n' requiere un numero positivo.\n");
        exit(1);
      }
      break;

    case 'g':      
      GAMMA = strtof(optarg, NULL);
      if (GAMMA <= 0){
        fprintf(stderr, "Argumento invalido: `-g' requiere un numero positivo.\n");
        exit(1);
      }
      break;
      
    case 'v':
      TEST_VISIBILITY = false;
      break;

    case 'a':
      ANTIALIAS = false;
      break;
      
    case '?':
      fprintf(stderr, "Argumento invalido: `-%c'.\n", optopt);
      usage();
    default:
      abort ();
    }

  FILE *fp = fopen("data/box.mdl","r");
  mdlInput mdl(fp);
  while (mdl.NumRemain() > 0){
    mdlKey k = mdl.BeginChunk();
    if (k == mdlKey("cmr")){
      // descartar el nombre
      camera = new Camera;
      mdl.ReadString();
      mdl.ReadFloats(camera->eye.v, 3);
      mdl.ReadFloats(camera->direction.v, 3); // es la direccion de la camara pero con respecto al eje de coordenadas. Por ahora no se usa
      mdl.ReadFloats(camera->up.v, 3);
    } 
    else if (k == mdlKey("plnrMsh")){
      Shape *s = new Shape;
      // el nombre no tiene nada
      mdl.ReadString();

      // cargar el material
      mdl.BeginChunk();
      string mtrlname = mdl.ReadString();
      mdl.EndChunk();
      // 
      mdl.BeginChunk();
      mdl.ReadFloats(s->vertices[0].v, 3);
      mdl.ReadFloats(s->vertices[1].v, 3);
      mdl.ReadFloats(s->vertices[2].v, 3);
      mdl.ReadFloats(s->vertices[3].v, 3);
      mdl.EndChunk();
      s->material = materials[mtrlname];
      s->part(PSIZE); //partition
      numpatches += s->matrix.size()*s->matrix.size();
      shapes.push_back(s);
    } 
    else if (k == mdlKey("nmdMtrl")){
      Material *m = new Material;
      string name = mdl.ReadString();
      m->name = name;
      mdl.BeginChunk();
      mdl.BeginChunk();
      mdl.ReadFloats(m->color, 3);
      mdl.EndChunk();
      mdl.EndChunk();
      materials[name] = m;
    }
    mdl.EndChunk();
  }

  //shapes[atoi(argv[1])]->partition(2000.0f);

  //copy(shapes.begin(), shapes.end(), ostream_iterator<Shape*>(cout, "\n"));
  cout << shapes.size() << " shapes" << endl;
  cout << numpatches << " patches" << endl;
  cout << numpatches*4 << " vertices" << endl;
  fclose(fp);

  //return 0;
   
  //inicializo valores b (iluminacion). TODO: esto no deberia estar
  // cableado para que sea realmente generico

  // inicializo la luz en new_patches
  for(unsigned int i=0;i<shapes[1]->matrix.size();++i){
    for(unsigned int j=0;j<shapes[1]->matrix.size();++j){
      shapes[1]->matrix[i][j].r = shapes[1]->matrix[i][j].dr = 1;
      shapes[1]->matrix[i][j].g = shapes[1]->matrix[i][j].dg = 1;
      shapes[1]->matrix[i][j].b = shapes[1]->matrix[i][j].db = 1;
    }
  }


  // calcular las normales y crear un vector con todos los patches
  cout << "calculando normales..." << endl;
  normals.clear();
  areas.clear();
  new_patches.clear();
  normals.reserve(numpatches);
  areas.reserve(numpatches);
  new_patches.reserve(numpatches);
  for(vector<Shape*>::iterator i = shapes.begin(); i < shapes.end(); i++){
    Shape *s = *i;
    Vector u = s->vertices[0] - s->vertices[1];
    Vector v = s->vertices[0] - s->vertices[2];
    Vector normal = u*v;
    normal.normalize();
    s->normal = normal;

	// lleno la lista de new_patches con los apuntadores a los
	// Quads en la matriz del shape
	for(unsigned int i=0;i<s->matrix.size();i++){
	  for(unsigned int j=0;j<s->matrix[i].size();j++){
	    new_patches.push_back(&s->matrix[i][j]);
        normals.push_back(normal);
        areas.push_back(s->matrix[i][j].area());	    
	  }
	}
  }
         
  glvertices = new SVertex[numpatches*4];

  formfactors = new float*[numpatches];
  for (int i=0; i<numpatches; ++i){
    formfactors[i] = new float[numpatches];
  }

  cout << "Form factor calculation (2 threads)..." << endl;    
  int sum = 0;
  pthread_mutex_t mutexsum;
  pthread_mutex_init(&mutexsum, NULL);

  ThreadData td1, td2;
  td1.start = 0;
  td1.size = (numpatches+1)/2;
  td1.sum = &sum;
  td1.mutexsum = &mutexsum;

  td2.start = td1.size;
  td2.size = numpatches/2;
  td2.sum = &sum;   
  td2.mutexsum = &mutexsum;

  pthread_t tid1, tid2;

  pthread_create(&tid1, 0, ffthread, &td1);
  pthread_create(&tid2, 0, ffthread, &td2);

  pthread_join(tid1, 0);
  pthread_join(tid2, 0);

  pthread_mutex_destroy(&mutexsum);
     
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_ACCUM);
  glutInitWindowPosition(50, 50);
  glutInitWindowSize(700, 500);
  glutCreateWindow("Radiosity");
  initGL();

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  glVertexPointer(3, GL_FLOAT, sizeof(SVertex), glvertices);
  glColorPointer(3, GL_FLOAT, sizeof(SVertex), &glvertices[0].r);

  radiosity_iterations(NUM_ITER);


  glutDisplayFunc(displayGL);
  glutReshapeFunc(reshapeGL);
  glutKeyboardFunc(keyboardGL);

  glutMainLoop();


  return 0;
}
