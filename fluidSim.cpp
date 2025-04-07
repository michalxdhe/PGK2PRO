#include "fluidSim.h"

void createTextures(){
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, width, height, depth, GL_RGBA, GL_FLOAT, data.data());
}
