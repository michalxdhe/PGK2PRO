#ifndef FLUIDSIM_H_INCLUDED
#define FLUIDSIM_H_INCLUDED

#include "common.h"

const static int width = 50;
const static int height = 50;
const static int depth = 50;

static GLuint velocityTex, quantTex, quantTexPrev, outputTex;

static std::vector<float> data(width * height * depth * 4);

GLuint createVolumeTexture();

void createTextures();

void simulateFluid(unsigned int shaderProgram, float dt);



#endif // FLUIDSIM_H_INCLUDED
