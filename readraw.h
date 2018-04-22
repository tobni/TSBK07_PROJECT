
#ifndef readraw_h
#define readraw_h

#include "GL_utilities.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <vector>
#include <iterator>

std::vector<GLubyte> readRaw2Vec(const std::string & filename, int x_dim, int y_dim, int z_dim);

GLubyte* readRaw2cArray(const char* filename, int x_dim, int y_dim, int z_dim);
#endif // readraw_h