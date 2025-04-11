#include "fluidSim.h"

GLuint createVolumeTexture() {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_3D, tex);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, width, height, depth, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    return tex;
}

void createTextures(){
    velocityTex = createVolumeTexture();
    quantTex = createVolumeTexture();
    quantTexPrev = createVolumeTexture();
    outputTex = createVolumeTexture();
}

void simulateFluid(unsigned int shaderProgram, float dt){
    glUseProgram(shaderProgram);

    glm::ivec3 gridSize = glm::ivec3(width,height,depth);

    glUniform1f(glGetUniformLocation(shaderProgram, "timeStep"), dt);
    glUniform1f(glGetUniformLocation(shaderProgram, "dissipation"), 0.99f);
    glUniform3f(glGetUniformLocation(shaderProgram, "gridSize"),
                gridSize.x, gridSize.y, gridSize.z);
    glUniform3f(glGetUniformLocation(shaderProgram, "gridSpacing"),
                1.0f / gridSize.x, 1.0f / gridSize.y, 1.0f / gridSize.z);

    glBindImageTexture(0, outputTex, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, velocityTex);
    glUniform1i(glGetUniformLocation(shaderProgram, "velocity"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, quantTex);
    glUniform1i(glGetUniformLocation(shaderProgram, "quantity"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_3D, quantTexPrev);
    glUniform1i(glGetUniformLocation(shaderProgram, "quantityPrev"), 3);

    glDispatchCompute((gridSize.x + 7) / 8, (gridSize.y + 7) / 8, (gridSize.z + 7) / 8);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}
