#include "readraw.h"

GLubyte* readRaw2cArray(const char* filename, int x_dim, int y_dim, int z_dim)
{
	const int size = x_dim * y_dim * z_dim;

	FILE *file = fopen(filename, "rb");

	GLubyte* volumeArray = malloc(size*sizeof(*volumeArray));
	fread(volumeArray, sizeof(GLubyte), size, file);
	fclose(file);

	return volumeArray;
}