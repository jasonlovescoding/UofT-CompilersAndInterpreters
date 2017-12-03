


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#include <GL/glut.h>
#include <GL/glext.h>

#include "utility.h"
#include "glUtils.h"
#include "textfile.h"

#define Near		 1
#define Far		20

#define cmdTogglePhong	 1
#define cmdToggleGeom	 2
#define cmdAnimate	 4
#define cmdExit		99


#define FRAG_LOC "frag.txt"

typedef struct {
	RGBA	color;
	float	shininess;
	} Material;


static int	Menu;

static int	UsePhong, ShowTea;
static int	Animate;
static GLfloat	Spin;
static GLfloat	LightPos[4];
static RGBA	Ambient = { 0.2, 0.2, 0.2, 1.0 };
static RGBA	Background = { 0, 0, 0, 1 };
static Material Surface;
static GLUquadricObj *	QState;

static int	Frames;
static int	NShapes;

#define NProgs		2
static GLuint		Shader[NProgs];

static GLuint VertShader, FragShader;

static char FragLoc[64];
static char * VertSrc = 

"!!ARBvp1.0					\n\
# Setup						\n\
ATTRIB	vPos	= vertex.position;		\n\
ATTRIB	vNorm	= vertex.normal;		\n\
ATTRIB  vCol	= vertex.color;			\n\
OUTPUT	oPos	= result.position;		\n\
OUTPUT	oCol	= result.color;			\n\
OUTPUT  oTex	= result.texcoord;		\n\
PARAM	CTM[4]	= { state.matrix.mvp };		\n\
PARAM	IMV[4]	= { state.matrix.modelview.invtrans };\n\
TEMP	eyeNorm;				\n\
## Standard transform				\n\
DP4  oPos.x, CTM[0], vPos;			\n\
DP4  oPos.y, CTM[1], vPos;			\n\
DP4  oPos.z, CTM[2], vPos;			\n\
DP4  oPos.w, CTM[3], vPos;			\n\
## Transform surface normal			\n\
DP3  eyeNorm.x, IMV[0], vNorm;			\n\
DP3  eyeNorm.y, IMV[1], vNorm; 			\n\
DP3  eyeNorm.z, IMV[2], vNorm;			\n\
# ...normalise					\n\
DP3  eyeNorm.w, eyeNorm, eyeNorm;		\n\
RSQ  eyeNorm.w, eyeNorm.w;			\n\
MUL  eyeNorm, eyeNorm, eyeNorm.w;		\n\
# ...pass to fragment as texture coords		\n\
MOV  oTex, eyeNorm;				\n\
## Lighting done in fragment shader		\n\
MOV  oCol, vCol;				\n\
END\n"
;

/* Phong shader for fragments */
/*
"!!ARBfp1.0					\n\
# Setup						\n\
ATTRIB	fCol	= fragment.color;		\n\
ATTRIB	fTex	= fragment.texcoord;		\n\
TEMP	eyeNorm, coeff, shade;			\n\
PARAM	lVec	= program.env[1];		\n\
PARAM	lHalf	= state.light[0].half;		\n\
PARAM	red = { 1, 0, 0, 1 };			\n\
## Interpolated surface normal			\n\
MOV  eyeNorm, fTex;				\n\
# May not be unit length any more		\n\
DP3  eyeNorm.w, eyeNorm, eyeNorm;		\n\
RSQ  eyeNorm.w, eyeNorm.w;			\n\
MUL  eyeNorm, eyeNorm, eyeNorm.w;		\n\
## Lighting					\n\
## Ambient: no setup required			\n\
MUL  shade, state.lightmodel.ambient, fCol;	\n\
## Diffuse: light dot normal			\n\
DP3  coeff.x, lVec, eyeNorm;			\n\
## Specular					\n\
DP3  coeff.y, lHalf, eyeNorm;			\n\
MOV  coeff.w, state.material.shininess.x;	\n\
# Eval coefficients and sum			\n\
LIT  coeff, coeff;				\n\
MAD  shade, coeff.y, fCol, shade;		\n\
MAD  shade, coeff.z, fCol, shade;		\n\
MOV  result.color, shade;			\n\
END\n\
"
};
*/

/* Simple pyramid shape, normals + vertices */

static GLfloat Vertices[36] = {
  0, 0, 1,	0, 0, 1,		/* Top */
  -1, 1, 0,	-1, 1, 0,		/* Left rear */
  -1, -1, 0,	-1, -1, 0,		/* Left front */
  1, -1, 0,	1, -1, 0,		/* Right front */
  1, 1, 0,	1, 1, 0,		/* Right rear */
  -1, 1, 0,	-1, 1, 0,		/* Left rear again */
};

/****		Drawing the world		****/

static void lookFrom (float x, float y, float z)
{
  gluLookAt(x, y, z, 0, 0, 0, 0, 1, 0);
}

static void positionLight ()
{
  GLfloat vLight[4];

  glLightfv(GL_LIGHT0, GL_POSITION, LightPos);
  if (UsePhong) {
    /* Pass light vector to vertex shader. Not strictly necessary
       as the position is part of the program state, but this way
       the vertex shader doesn't have to normalize it each time. */
    glGetLightfv(GL_LIGHT0, GL_POSITION, vLight);
    NormVec(vLight);
    glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 1, vLight);
  }
}

static void apply (Material * mat)
{
  glColor4fv(mat->color);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat->color);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat->color);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat->shininess);
}

static void drawShape ()
{
  glPushMatrix();
  glRotatef(Spin, 0, 1, 1);
  
  apply(&Surface);
  
  if (ShowTea) {
    glPushMatrix();
    glTranslatef(0, 0, -2);
    //gluCylinder(QState, 1, 1, 4, 8, 4);
    glutSolidTeapot(1.5);
    glPopMatrix();
  } else {
    glEnableClientState(GL_VERTEX_ARRAY);
    glInterleavedArrays(GL_N3F_V3F, 0, Vertices);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
    glDisableClientState(GL_VERTEX_ARRAY);
  }
  
  glPopMatrix();
}

static void display ()

{
  int i;
  
  glClearColor(Background[0], Background[1], Background[2], Background[3]);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glPushMatrix();
  glLoadIdentity();
  
  if (UsePhong) {
    glEnable(GL_VERTEX_PROGRAM_ARB);
    glEnable(GL_FRAGMENT_PROGRAM_ARB);
  }
    
  lookFrom(0, 0, 6);
  positionLight();
  for (i = 0; i < NShapes; i++)
    drawShape();
  
  glPopMatrix();  
  CheckGL();
  glutSwapBuffers();
  
  glDisable(GL_VERTEX_PROGRAM_ARB);
  glDisable(GL_FRAGMENT_PROGRAM_ARB);
  Frames ++;
}

static void resize (int width, int height)
{
  GLfloat aspect;

  glMatrixMode(GL_PROJECTION);
  glViewport(0, 0, width, height);
  glLoadIdentity();
  aspect = (float)width / (float)height;
  gluPerspective(60.0, aspect, Near, Far);
  glMatrixMode(GL_MODELVIEW);
}


/****		Input handlers		****/

static void menuChoice (int item)
{
  switch (item) {
    case cmdTogglePhong:
      UsePhong = ! UsePhong;
      if (UsePhong)
        printf("Phong shader\n");
      else
        printf("Standard OpenGL\n");
      break;
    case cmdToggleGeom:
      ShowTea = ! ShowTea;
      break;
    case cmdAnimate:
      Animate = ! Animate;
      break;
    case cmdExit:
      exit(0);
      break;
    default:
      break;
  }
}

static void asciiKey (unsigned char key, int x, int y)
{
  key = toupper(key);
  
  if (key == 27) /* ESC */
    exit(0);
  else if (key == 'F')
    menuChoice(cmdTogglePhong);
  else if (key == 'G')
    menuChoice(cmdToggleGeom);
  else if (key == ' ')
    menuChoice(cmdAnimate);
  else if (key == 'W')
    SetColor(Background, 1, 1, 1);
  else if (key == 'B')
    SetColor(Background, 0, 0, 0);
}


/****		Main control		****/


static void initGraphics ()
{
  glEnable(GL_DEPTH_TEST);
  
  glShadeModel(GL_SMOOTH);
  glEnable(GL_NORMALIZE);
  
  glEnable(GL_LIGHTING);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, Ambient);
  glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
  
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_AMBIENT, Black);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, White);
  glLightfv(GL_LIGHT0, GL_SPECULAR, White);
}

static void initQuadric ()
{
  QState = gluNewQuadric();
  FailNull(QState, "Cannot allocate quadric object");
  
  gluQuadricDrawStyle(QState, GLU_FILL);
  gluQuadricOrientation(QState, GLU_OUTSIDE);
  gluQuadricNormals(QState, GLU_SMOOTH);
  gluQuadricTexture(QState, GL_FALSE);
}


static void getErrorLine (char * src, int err, char line[], int maxLine)
{
  int start, finish, n;
  
  start = err;
  while (start >= 0 && isprint(src[start]))
    start --;
  start ++;
  finish = err;
  while (isprint(src[finish]))
    finish ++;
  n = finish - start;
  if (n >= maxLine)
    n = maxLine;
  strncpy(line, src + start, n);
  line[n] = 0;
}


static void initShader ()
{
  if (! glutExtensionSupported("GL_ARB_vertex_program"))
    Fail("GL_ARB_vertex_program not available on this machine");
  if (! glutExtensionSupported("GL_ARB_fragment_program"))
    Fail("GL_ARB_fragment_program not available on this machine");
  char* FragSrc = NULL;
  FragSrc = textFileRead(FragLoc);
  //if (FragSrc == NULL)
  printf("Invalid frag input file");

  glEnable(GL_VERTEX_PROGRAM_ARB);
  glEnable(GL_FRAGMENT_PROGRAM_ARB);
  
  glGenProgramsARB(NProgs, VertShader);
    
  glBindProgramARB(GL_VERTEX_PROGRAM_ARB, VertShader);
  glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
  			strlen(VertSrc), VertSrc);
  	
  glGenProgramsARB(NProgs, FragShader);			
  glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, FragShader);
  glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
  			strlen(FragSrc), FragSrc);
  int err = 0;
  glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &err);

  if (err >= 0)
	printf("Error in fragment shader %s\n",glGetString(GL_PROGRAM_ERROR_STRING_ARB));

  free(FragSrc);
  				
  glDisable(GL_VERTEX_PROGRAM_ARB);
  glDisable(GL_FRAGMENT_PROGRAM_ARB); 
}

static void initApp ()
{
  initGraphics();
  initQuadric();
  initShader();
  /* Make sure we did everything right */
  CheckGL();
  
  LightPos[0] = 0;
  LightPos[1] = -5;
  LightPos[2] = 4;
  LightPos[3] = 0;
   
  SetColor(Surface.color, 0.25, 0.25, 0.4);
  Surface.shininess = 2;

  Animate = TRUE;
  Frames  = 0;
  NShapes = 1;
  
  Menu = glutCreateMenu(menuChoice);
  glutSetMenu(Menu);
  glutAddMenuEntry("Phong shader", cmdTogglePhong);
  glutAddMenuEntry("Change shape", cmdToggleGeom);
  glutAddMenuEntry("Animate", cmdAnimate);
  glutAddMenuEntry("----", 0);
  glutAddMenuEntry("Exit", cmdExit);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
}

static void animate ()
{
  if (Animate)
    Spin += 0.25;
  if (Spin > 360)
    Spin -= 360;
  
  glutPostRedisplay();
}

int main (int argc, char * argv[])
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
  if (argc > 1){
     sprintf(FragLoc,"%s",argv[1]);
  }
  else{
     sprintf(FragLoc,"%s",FRAG_LOC);
  }
  glutInitWindowSize(800, 600);
  glutInitWindowPosition(100, 75);
  glutCreateWindow("ARB Phong Shading");

  initApp();
  
  printf("\n");
  printf("F toggles Phong shading\n");
  printf("G changes shape\n");
  printf("'Space' Start/Stop animation\n");
  printf("W change background to white\n");
  printf("B change background to black\n");
  printf("\n");
  
  glutDisplayFunc(display);
  glutReshapeFunc(resize);
  glutIdleFunc(animate);
  glutKeyboardFunc(asciiKey);
  
  glutMainLoop();
  return 0;	/* Keeps compiler happy */
}
