#include "MicroGlut.h"

#include "GL_utilities.h"
#include "VectorUtils3.h"
#include "loadobj.h"
#include "LoadTGA.h"
#include "stdlib.h"

#include "readraw.h"



int XDIM = 256;
int YDIM = 256;
int ZDIM = 256;

std::vector<GLubyte> volumeArray;

GLuint volTex, shader;

mat4 projectionMatrix;

void init(void)
{
	// GL inits
	glClearColor(1.0,1.0,1.0,0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_TRUE);
	printError("GL inits");

	projectionMatrix = frustum(-0.1, 0.1, -0.1, 0.1, 0.2, 50.0);

	// Load and compile shader
	shader = loadShaders("raycast.vert", "raycast.frag");
	glUseProgram(shader);
	printError("init shader");

	//load data into a 3D texture
	volumeArray = readRaw2Vec("MRI-Head.raw", XDIM, YDIM, ZDIM);

	glGenTextures(1, &volTex);
	glBindTexture(GL_TEXTURE_3D, volTex);

	// set the texture parameters
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_INTENSITY, XDIM, YDIM, ZDIM, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, &volumeArray.front());
}


void display(void)
{

	// clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glutSwapBuffers();
}



int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
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
