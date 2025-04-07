#ifndef FLUIDSIM_H_INCLUDED
#define FLUIDSIM_H_INCLUDED

#include "common.h"

const int width = 50;
const int height = 50;
const int depth = 50;

static GLuint velocityTex, densityTex;

static std::vector<float> data(width * height * depth * 4);

void createTextures();

void simulateFluid();



#endif // FLUIDSIM_H_INCLUDED
