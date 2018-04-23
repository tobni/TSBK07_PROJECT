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

	//clear first and last slice to 0
	memset(volumeArray, 0, sizeof(GLubyte)*x_dim*y_dim);
	GLubyte* temp = volumeArray;
	memset((volumeArray + (size - (sizeof(GLubyte)*x_dim*y_dim))), 0, sizeof(GLubyte)*x_dim*y_dim);
	volumeArray = temp;

	return volumeArray;
}