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

GLubyte* volume_array;

Model *m_quad;

GLuint vol_tex, shader;
GLfloat step_size;

mat4 proj_mat;


GLfloat quad[] = { 
	-1.0f,-1.0f,0.0f,
	-1.0f,1.0f,0.0f,
	1.0f,1.0f,0.0f,
	1.0f,-1.0f,0.0f };
GLuint quad_ind[] = { 0,1,3,3,1,2 };


GLfloat quad_tex[] = { 
	0.0f, 1.0f,
	0.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 1.0f };

void init(void)
{
	// GL inits
	glClearColor(0.0,0.0,0.0,0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	printError("GL inits");

	// Load and compile shader
	shader = loadShaders("raycast.vert", "raycast.frag");
	glUseProgram(shader);
	printError("init shader");

	//load data into a 3D texture
	volume_array = readRaw2cArray("MRI-Head.raw", XDIM, YDIM, ZDIM);

	glGenTextures(1, &vol_tex);

	glBindTexture(GL_TEXTURE_3D, vol_tex);

	// set the texture parameters
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_INTENSITY, XDIM, YDIM, ZDIM, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, volume_array);
	printError("upload volume");
	// Upload the quad

	m_quad = LoadDataToModel(quad, NULL, quad_tex, NULL, quad_ind, 4, 6);

	step_size = 1.f / ZDIM;
}


void display(void)
{
	// clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shader);
	
	glUniform1i(glGetUniformLocation(shader, "texVol"), 0); // Texture unit 0
	glUniform1f(glGetUniformLocation(shader, "stepSize"), step_size);

	DrawModel(m_quad, shader, "inPos", NULL, "inTexCoord");

	glutSwapBuffers();
}



int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 2);
	glutCreateWindow ("TSBK07");
#ifdef WIN32
	glewInit();
#endif
	glutDisplayFunc(display); 
	init();
	glutMainLoop();
	exit(0);
}
