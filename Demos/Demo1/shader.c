

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glext.h>

#include "utility.h"
#include "glUtils.h"
#include "textfile.h"

#define Near		-1
#define Far		 1

#define cmdToggleFragment	2
#define cmdSphere		3
#define cmdTeapot		4
#define cmdExit			99

static int	Menu;

static int	UseShader;	
static int	Teapot;		

static GLUquadricObj * QState;

static GLuint VertShader, FragShader;
static char FragLoc[64];

#define FRAG_LOC "frag.txt"

static void chooseShader (int status)
{
  UseShader = status;
  glDisable(GL_FRAGMENT_PROGRAM_ARB);
  switch (UseShader) {
    case 0:
      printf("Standard OpenGL\n");
      break;
    case 2:
      printf("Fragment shader\n");
      glEnable(GL_FRAGMENT_PROGRAM_ARB);
      break;
  }
}

static void display ()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glPushMatrix();
  glLoadIdentity();
  
  gluLookAt(0, 2, 8,
  		0, 0, 0,
		0, 1, 0);
  float red = 0.002;
  float green = 0.002;
  float blue = 0.2;
  glColor3f(red, green, blue);
  if (Teapot)
    glutWireTeapot(3);
  else
    gluSphere(QState, 3, 32, 8);

  glPopMatrix();  
  CheckGL();
  glutSwapBuffers();
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
    case cmdToggleFragment:
      if (UseShader == 2)
        chooseShader(0);
      else
        chooseShader(2);
      break;
    case cmdSphere:
      Teapot = FALSE;
      break;
    case cmdTeapot:
      Teapot = TRUE;
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
    menuChoice(cmdToggleFragment);
  else if (key == 'S')
    menuChoice(cmdSphere);
  else if (key == 'T')
    menuChoice(cmdTeapot);
}


/****		Main control		****/


static void initGraphics ()
{
  glClearColor(0, 0, 0, 0); 
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glDisable(GL_LIGHTING);
  glShadeModel(GL_SMOOTH);
  
  QState = gluNewQuadric();
  FailNull(QState, "Cannot allocate quadric object");
  
  gluQuadricDrawStyle(QState, GLU_LINE);
}

static void initShaders ()
{
  int err;	
  char* FragSrc = textFileRead(FragLoc);
  if (FragSrc == NULL){
	  printf("ERROR:Src Prog is %s\n",FragSrc);
   }
  if (! glutExtensionSupported("GL_ARB_vertex_program"))
    Fail("GL_ARB_vertex_program not available on this machine");
  if (! glutExtensionSupported("GL_ARB_fragment_program"))
    Fail("GL_ARB_fragment_program not available on this machine");
  
  
  /* Fragment shader */
  glEnable(GL_FRAGMENT_PROGRAM_ARB);
  glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, &FragShader);


  glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
  			strlen(FragSrc), FragSrc);
  glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &err);
  if (err >= 0)
    printf("Error in fragment shader %s\n",
    		glGetString(GL_PROGRAM_ERROR_STRING_ARB));

  free(FragSrc);
}

static void initApp ()
{
printf("initApp \n");
  initGraphics();
  initShaders();
  /* Make sure we did everything right */
  CheckGL();
  chooseShader(0);
  
   Menu = glutCreateMenu(menuChoice);
  glutSetMenu(Menu);
  glutAddMenuEntry("Fragment shader", cmdToggleFragment);
  glutAddMenuEntry("Sphere", cmdSphere);
  glutAddMenuEntry("Teapot", cmdTeapot);
  glutAddMenuEntry("----", 0);
  glutAddMenuEntry("Exit", cmdExit);
  glutAttachMenu(GLUT_RIGHT_BUTTON);  
}

static void animate ()
{
  glutPostRedisplay();
}

int main (int argc, char * argv[])
{
  int err;	
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
  glutCreateWindow("ARB shaders");

  initApp();
  
  printf("\n");
  printf("F toggles use of fragment shader\n");
  printf("T draws teapot\n");
  printf("S draws sphere\n");
  printf("\n");
  
  glutDisplayFunc(display);
  glutReshapeFunc(resize);
  glutIdleFunc(animate);
  glutKeyboardFunc(asciiKey);
  
  glutMainLoop();
  return 0;	/* Keeps compiler happy */
}
