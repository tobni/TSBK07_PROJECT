#include "readraw.h"


std::vector<GLuint> readRaw2Vec(const std::string & filename, int x_dim, int y_dim, int z_dim)
{
	std::ifstream infile;
	std::vector<GLuint> volumeArray;
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