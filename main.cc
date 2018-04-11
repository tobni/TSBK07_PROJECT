#include "MicroGlut.h"

#include "GL_utilities.h"
#include "VectorUtils3.h"
#include "loadobj.h"
#include "LoadTGA.h"
#include "stdlib.h"

#include "readraw.h"

void init(void)
{
	// GL inits
	glClearColor(1.0,1.0,1.0,0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_TRUE);

}


void display(void)
{

	// clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glutSwapBuffers();
}

int XDIM = 256;
int YDIM = 256;
int ZDIM = 256;


int main(int argc, char *argv[])
{
	std::vector<GLuint> volumeArray = readRaw2Vec("MRI-Head.raw", XDIM, YDIM, ZDIM);
	std::cout << volumeArray.size();
	glutInit(&argc, argv);
	glutInitContextVersion(3, 2);
	glutCreateWindow ("TSBK07");
#ifdef WIN32
	glewInit();
#endif
	glutDisplayFunc(display); 

	init ();
	glutMainLoop();
	exit(0);
}
