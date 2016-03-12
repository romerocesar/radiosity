#include <GL/glut.h>
#include <cmath>

// TODO: poner la referencia al red book

void accFrustum( GLdouble left, GLdouble right, GLdouble bottom,
                 GLdouble top, GLdouble near, GLdouble far, GLdouble pixdx,
                 GLdouble pixdy, GLdouble eyedx, GLdouble eyedy, GLdouble focus )
{
  // make a frustum for use with the accumulation buffer
  GLdouble xwsize, ywsize;
  GLdouble dx, dy;
  GLint viewport[4];

  glGetIntegerv( GL_VIEWPORT, viewport );
  xwsize = right - left;
  ywsize = top - bottom;
  dx = -(pixdx*xwsize/(GLdouble)viewport[2] + eyedx*near/focus );
  dy = -(pixdx*ywsize/(GLdouble)viewport[3] + eyedy*near/focus );

  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();

  glFrustum( left+dx, right+dx, bottom+dy, top+dy, near, far );
  glMatrixMode ( GL_MODELVIEW );
  glLoadIdentity();
  glTranslatef( -eyedx, -eyedy, 0.0 );
}


void accPerspective( GLdouble fovy, GLdouble aspect,
                     GLdouble near, GLdouble far, GLdouble pixdx, GLdouble pixdy,
                     GLdouble eyedx, GLdouble eyedy, GLdouble focus )
{
  GLdouble fov2, left, right, bottom, top;
  fov2 = ((fovy*M_PI) / 180.0) / 2.0;

  top = near / (cos( fov2 ) / sin( fov2 ));
  bottom = -top;
  right = top * aspect;
  left = -right;

  accFrustum( left, right, bottom, top, near, far,
              pixdx, pixdy, eyedx, eyedy, focus );
}
