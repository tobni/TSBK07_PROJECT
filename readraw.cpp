#include "readraw.h"


std::vector<GLubyte> readRaw2Vec(const std::string & filename, int x_dim, int y_dim, int z_dim)
{
	std::ifstream infile;
	std::vector<GLubyte> volumeArray;
	GLuint bytes = x_dim * y_dim * z_dim;

	infile.open(filename, std::ios::binary);
	infile.unsetf(std::ios::skipws);

	volumeArray.reserve(bytes);

	volumeArray.insert(volumeArray.begin(),
		std::istream_iterator<GLubyte>(infile),
		std::istream_iterator<GLubyte>());
	infile.close();

	return volumeArray;
}

GLubyte* readRaw2cArray(const char* filename, int x_dim, int y_dim, int z_dim)
{
	const int size = x_dim * y_dim * z_dim;

	FILE *file = fopen(filename, "rb");

	GLubyte* volumeArray = new GLubyte[size];
	fread(volumeArray, sizeof(GLubyte), size, file);
	fclose(file);

	// Clear first and last slices to 0
	for (int z = 0; z < z_dim; z++)
	{
		for (int y = 0; y < y_dim; y++)
		{
			for (int x = 0; x < x_dim; x++)
			{
				if ((x == 0) || (x == x_dim - 1) ||
					(y == 0) || (y == y_dim - 1) ||
					(z == 0) || (z == z_dim - 1)) {
				volumeArray[x + y*x_dim + z*x_dim*y_dim] = GLubyte(0);
				}
			}
		}
	}

	return volumeArray;
}

GLushort* readRaw2cArray16bit(const char* filename, int x_dim, int y_dim, int z_dim)
{
	const int size = x_dim * y_dim * z_dim;

	FILE *file = fopen(filename, "rb");

	GLushort* volumeArray = new GLushort[size];
	fread(volumeArray, sizeof(GLushort), size, file);
	fclose(file);

	return volumeArray;
}