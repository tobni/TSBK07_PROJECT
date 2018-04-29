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
GLshort* volume16_array;
Model *m_quad;

GLuint grad_tex, vol_tex, shader, current_frag = 0;
GLfloat step_size, focal_length = 2.0, distance = -0.2,  angle_y, angle_x, alpha_val = 0.25;
mat4 rot_mat, mdl_mat;


GLfloat quad[] = { 
	-1.0f,-1.0f,0.0f,
	-1.0f,1.0f,0.0f,
	1.0f,1.0f,0.0f,
	1.0f,-1.0f,0.0f };
GLuint quad_ind[] = { 0,1,3,3,1,2 };


GLfloat quad_tex[] = { 
	0.0f, 0.0f,
	0.0f, 1.0f,
	1.0f, 1.0f,
	1.0f, 0.0f };

void initVolume()
{
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(0, &vol_tex);
	glBindTexture(GL_TEXTURE_3D, vol_tex);

	// set the volume texture parameters
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_INTENSITY, XDIM, YDIM, ZDIM, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, volume_array);
	printError("upload volume");

	step_size = 1.f / XDIM;
}


void init(void)
{
	// GL inits
	glClearColor(1.0,1.0,1.0,0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	printError("GL inits");

	// Load and compile shader
	shader = loadShaders("raycast.vert", "raycast.frag");
	glUseProgram(shader);
	printError("init shader");

	volume_array = readRaw2cArray("MRI-Head.raw", XDIM, YDIM, ZDIM);
	initVolume();
	// Upload the quad

	m_quad = LoadDataToModel(quad, NULL, quad_tex, NULL, quad_ind, 4, 6);

	
}

void display(void)
{
	// clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shader);

	rot_mat = Ry(M_PI * angle_y);
	rot_mat = Mult(Rx(M_PI * angle_x), rot_mat);
	rot_mat = Mult(rot_mat, T(-0.5, -0.5, -0.5));
	rot_mat = Mult(T(0.5, 0.5, 0.5), rot_mat);

	mdl_mat = Ry(M_PI * -angle_y);
	mdl_mat = Mult(mdl_mat, Rx(M_PI * -angle_x));
	mdl_mat = Mult(mdl_mat, T(-0.5, -0.5, -0.5));
	mdl_mat = Mult(T(0.5, 0.5, 0.5), mdl_mat);

	glUniform1i(glGetUniformLocation(shader, "texVol"), 0); // Texture unit 0
	glUniform1f(glGetUniformLocation(shader, "stepSize"), step_size);
	glUniform1f(glGetUniformLocation(shader, "focalLength"), focal_length);
	glUniform1f(glGetUniformLocation(shader, "distance"), distance);
	glUniform1f(glGetUniformLocation(shader, "alphaScale"), alpha_val);

	glUniformMatrix4fv(glGetUniformLocation(shader, "rotMat"), 1, GL_TRUE, rot_mat.m);
	glUniformMatrix4fv(glGetUniformLocation(shader, "mdlMat"), 1, GL_TRUE, mdl_mat.m);

	DrawModel(m_quad, shader, "inPos", NULL, "inTexCoord");

	glutSwapBuffers();
}

void keyboard(unsigned char c, int x, int y)
{
	switch (c)
	{
	case 27:
		exit(0);
		break;
	case 'w':
		distance += 0.01;
		glutPostRedisplay();
		break;
	case 's':
		distance -= 0.01;

		printf("%f", distance);
		glutPostRedisplay();
		break;
	case 'a':
		angle_y += 0.012;
		glutPostRedisplay();
		break;
	case 'd':
		angle_y -= 0.012;
		glutPostRedisplay();
		break;
	case ',':
		angle_x -= 0.012;
		glutPostRedisplay();
		break;
	case '.':
		angle_x += 0.012;
		glutPostRedisplay();
		break;
	case '-':
		alpha_val -= 0.001;
		glutPostRedisplay();
		break;
	case '+':
		alpha_val += 0.001;
		glutPostRedisplay();
		break;
	case 'f':
		if (current_frag == 1) {
			shader = loadShaders("raycast.vert", "raycast.frag");
			current_frag = 0;
		}
		else if (current_frag == 0) {
			shader = loadShaders("raycast.vert", "raycast_old.frag");
			current_frag = 1;
		};
		break;
	case 1:
		XDIM = 256;
		YDIM = 256;
		ZDIM = 256;

		volume_array = readRaw2cArray("MRI-Head.raw", XDIM, YDIM, ZDIM);
		initVolume();
		break;

	case 2:
		XDIM = 256;
		YDIM = 256;
		ZDIM = 44;

		volume_array = readRaw2cArray("frog.raw", XDIM, YDIM, ZDIM);
		initVolume();
		break;
	case 3:
		XDIM = 256;
		YDIM = 256;
		ZDIM = 256;

		volume_array = readRaw2cArray("foot.raw", XDIM, YDIM, ZDIM);
		initVolume();
		break;
	case 4:
		XDIM = 384;
		YDIM = 384;
		ZDIM = 240;

		volume_array = readRaw2cArray("CT-chest.raw", XDIM, YDIM, ZDIM);
		initVolume();
		break;
	}
}



int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 2);
	glutCreateWindow ("TSBK07");
#ifdef WIN32
	glewInit();
#endif
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(display); 
	init();
	glutMainLoop();
	exit(0);
}
