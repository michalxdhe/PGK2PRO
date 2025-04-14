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

void initializeQuantTexCPU() {
    int size = width * height * depth * 4;
    std::vector<float> data(size, 0.0f);

    for (int z = depth/4; z < 3*depth/4; ++z) {
        for (int y = height/4; y < 3*height/4; ++y) {
            for (int x = width/4; x < 3*width/4; ++x) {
                int i = 4 * (x + y * width + z * width * height);
                data[i + 0] = 1.0f; // R
                data[i + 1] = 0.2f; // G
                data[i + 2] = 0.0f; // B
                data[i + 3] = 1.0f; // A
            }
        }
    }

    glBindTexture(GL_TEXTURE_3D, quantTex);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, width, height, depth, GL_RGBA, GL_FLOAT, data.data());

    glBindTexture(GL_TEXTURE_3D, quantTexPrev);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, width, height, depth, GL_RGBA, GL_FLOAT, data.data());
}

void initializeVelocityTexCPU() {
    int size = width * height * depth * 4;
    std::vector<float> data(size, 0.0f);

    glm::vec3 center = glm::vec3(width, height, depth) * 0.5f;

    for (int z = 0; z < depth; ++z) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int i = 4 * (x + y * width + z * width * height);

                float dx = x - center.x;
                float dy = y - center.y;
                float dz = z - center.z;

                data[i + 0] = (rand()%20-10) * 0.1f; // X component
                data[i + 1] = (rand()%20-10) * 0.1f;  // Y component
                data[i + 2] = 0.f * 0.1f;  // Z component
                data[i + 3] = 1.0f;
            }
        }
    }

    glBindTexture(GL_TEXTURE_3D, velocityTex);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, width, height, depth, GL_RGBA, GL_FLOAT, data.data());
}



void createTextures(){
    velocityTex = createVolumeTexture();
    quantTex = createVolumeTexture();
    quantTexPrev = createVolumeTexture();
    outputTex = createVolumeTexture();

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
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
    std::swap(quantTex, outputTex);
}

void renderFluid(unsigned int volumeShader){
    glUseProgram(volumeShader);

    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, glm::vec3(0.f,5.f,0.f));

    glUniformMatrix4fv(glGetUniformLocation(volumeShader, "model"), 1, GL_FALSE, glm::value_ptr(model));

    glUniformMatrix4fv(glGetUniformLocation(volumeShader, "invModel"), 1, GL_FALSE, glm::value_ptr(glm::inverse(model)));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, quantTex);
    glUniform1i(glGetUniformLocation(volumeShader, "volumeTex"), 1);

    glBindVertexArray(cubeVAO);
    glDepthMask(GL_FALSE);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);

    glDisable(GL_BLEND);
}
