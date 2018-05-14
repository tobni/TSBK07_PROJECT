
#ifndef readraw_h
#define readraw_h

#include "GL_utilities.h"
#include "stdio.h"

GLubyte* readRaw2cArray(const char* filename, int x_dim, int y_dim, int z_dim);

#endif // readraw_h