#include "MicroGlut.h"
#include "GL_utilities.h"
#include "VectorUtils3.h"
#include "loadobj.h"
#include "LoadTGA.h"
#include "stdlib.h"
#include "simplefont.h"

#include "readraw.h"
#include "math.h"

#define max(a,b) (((a)>(b))?(a):(b))
#define sgn(x) (x < 0) ? -1 : (x > 0)

int XDIM = 256;
int YDIM = 256;
int ZDIM = 256;

GLubyte* volume_array;
GLshort* volume16_array;
Model *m_quad;

GLuint vol_tex, shader, current_frag = 0, transfer_func;
GLfloat step_size, focal_length = 2.0, distance = -0.2,  angle_y = 0.0, angle_x = 0.0, alpha_val = 0.25;
mat4 rot_mat, mdl_mat;

GLint window_width = 700, window_height = 700;

GLuint font;

vec3 cube[] = {
	// Front
	{ 0.0f, 0.0f, 0.0f },
	{ 1.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, 0.0f },
	{ 0.0f, 1.0f, 0.0f },
	// Back
	{ 0.0f, 0.0f, 1.0f },
	{ 1.0f, 0.0f, 1.0f },
	{ 1.0f, 1.0f, 1.0f },
	{ 0.0f, 1.0f, 1.0f } };

GLfloat quad[] = { 
	-1.0f,-1.0f,0.0f,
	-1.0f,1.0f,0.0f,
	1.0f,1.0f,0.0f,
	1.0f,-1.0f,0.0f };

GLuint quad_ind[] = { 0,1,3,3,1,2 };

GLint cube_ind[] = {
	// front
	0, 1, 2,
	2, 3, 0,
	// right
	1, 5, 6,
	6, 2, 1,
	// back
	7, 6, 5,
	5, 4, 7,
	// left
	4, 0, 3,
	3, 7, 4,
	// bottom
	4, 5, 1,
	1, 0, 4,
	// top
	3, 2, 6,
	6, 7, 3,
};

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
	glUniform1i(glGetUniformLocation(shader, "texVol"), 0); // Texture unit 0
	printError("upload volume");

	step_size = 1.f / max(max(XDIM, YDIM), ZDIM);
	glUniform1f(glGetUniformLocation(shader, "stepSize"), step_size);
}

void init(void)
{
	// GL inits
	glClearColor(1.0,1.0,1.0,1.0);
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

	// Make model out of quad data
	m_quad = LoadDataToModel(quad, NULL, quad_tex, NULL, quad_ind, 4, 6);
	
	// Upload the cube to shader
	glUniform3fv(glGetUniformLocation(shader, "cubeVert"), 8, &cube[0].x);
	glUniform1iv(glGetUniformLocation(shader, "cubeInd"), 36, &cube_ind[0]);

	//Load Transfer Function
	glActiveTexture(GL_TEXTURE1);
	LoadTGATextureSimple("test.tga", &transfer_func);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glUniform1i(glGetUniformLocation(shader, "transferFunction"), 1); // Texture unit 1
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

	glUniform1f(glGetUniformLocation(shader, "focalLength"), focal_length);
	glUniform1f(glGetUniformLocation(shader, "distance"), distance);
	glUniform1f(glGetUniformLocation(shader, "alphaScale"), alpha_val);

	glUniformMatrix4fv(glGetUniformLocation(shader, "rotMat"), 1, GL_TRUE, rot_mat.m);
	glUniformMatrix4fv(glGetUniformLocation(shader, "mdlMat"), 1, GL_TRUE, mdl_mat.m);

	DrawModel(m_quad, shader, "inPos", NULL, "inTexCoord");

	// Print info
	char float_array[16];
	sfSetFont(font);
	sfSetFontColor(0, 0, 0);

	// Reset angles if full rotations
	float deg_x = (int)(roundf(angle_x * 180 * 100) / 100) % 360;
	float deg_y = (int)(roundf(angle_y * 180 * 100) / 100) % 360;
	if ((int)(roundf(angle_x * 180 * 100) / 100) % 360 == 0) {
		angle_x = 0;
	};
	if ((int)(roundf(angle_y * 180 * 100) / 100) % 360 == 0) {
		angle_y = 0;
	};

	// X
	sfDrawString(40, 60, "Rot(X): ");
	sprintf(float_array, "%g", deg_x);
	sfDrawString(110, 60, float_array);
	
	// Y
	sfDrawString(40, 80, "Rot(Y): ");
	sprintf(float_array, "%g", deg_y);
	sfDrawString(110, 80, float_array);

	// Distance
	sfDrawString(40, 100, "Distance: ");
	sprintf(float_array, "%g", roundf(distance * 100) / 100);
	sfDrawString(130, 100, float_array);

	// Opacity
	sfDrawString(40, 120, "Opacity: ");
	sprintf(float_array, "%g", roundf(alpha_val * 100) / 100);
	sfDrawString(120, 120, float_array);

	glutSwapBuffers();
}

void reshape(GLsizei w, GLsizei h)
{
	if (w < h) {
		glViewport(0, h / 2 - w / 2, w, w);
		sfSetRasterSize(w, w);
	}
	else {
		glViewport(w / 2 - h / 2, 0, h, h);
		sfSetRasterSize(h, h);
	}
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
		alpha_val -= 0.01;
		glutPostRedisplay();
		break;
	case '+':
		alpha_val += 0.01;
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
		glUniform3fv(glGetUniformLocation(shader, "cubeVert"), 8, &cube[0].x);
		glUniform1iv(glGetUniformLocation(shader, "cubeInd"), 36, &cube_ind[0]);
		glUniform1f(glGetUniformLocation(shader, "stepSize"), step_size);
		glutPostRedisplay();
		break;
	case 1:
		XDIM = 256;
		YDIM = 256;
		ZDIM = 256;
		delete volume_array;
		volume_array = readRaw2cArray("MRI-Head.raw", XDIM, YDIM, ZDIM);
		initVolume();
		glutPostRedisplay();
		break;

	case 2:
		XDIM = 256;
		YDIM = 256;
		ZDIM = 44;
		delete volume_array;
		volume_array = readRaw2cArray("frog.raw", XDIM, YDIM, ZDIM);
		initVolume();
		glutPostRedisplay();
		break;
	case 3:
		XDIM = 256;
		YDIM = 256;
		ZDIM = 256;
		delete volume_array;
		volume_array = readRaw2cArray("foot.raw", XDIM, YDIM, ZDIM);
		initVolume();
		glutPostRedisplay();
		break;
	case 4:
		XDIM = 384;
		YDIM = 384;
		ZDIM = 240;
		delete volume_array;
		volume_array = readRaw2cArray("CT-chest.raw", XDIM, YDIM, ZDIM);
		initVolume();
		glutPostRedisplay();
		break;

	case 5:
		XDIM = 128;
		YDIM = 256;
		ZDIM = 256;
		delete volume_array;
		volume_array = readRaw2cArray("CT-head.raw", XDIM, YDIM, ZDIM);
		initVolume();
		glutPostRedisplay();
		break;
	}
}



int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(50, 50);
	glutInitWindowSize(window_width, window_height);
	glutInitContextVersion(3, 2);
	glutCreateWindow ("TSBK07");
#ifdef WIN32
	glewInit();
#endif
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(display); 
	glutReshapeFunc(reshape);
	init();
	font = sfMakeRasterFont();	
	sfSetRasterSize(window_width, window_height);
	glutMainLoop();
	exit(0);
}
