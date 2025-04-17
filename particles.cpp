#include "particles.h"

Particle::Particle() : position(0.0f), velocity(0.0f), color(glm::vec4(1.f,1.f,1.f,rand()%1000/1000.f)), life(0.0f), gravity(0.5f) { }

float smoothstep(float edge0, float edge1, float x) {
    x = glm::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);
}

void createParticleTexture() {
    const int width = 64;
    const int height = 64;
    unsigned char data[width * height * 4];

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = (y * width + x) * 4;
            float dx = (x - width / 2) / (float)(width / 2);
            float dy = (y - height / 2) / (float)(height / 2);
            float distance = sqrt(dx * dx + dy * dy);

            float alpha = 1.0f - smoothstep(0.4f, 1.0f, distance);

            data[index] = 255;        // Red
            data[index + 1] = 255;    // Green
            data[index + 2] = 255;    // Blue
            data[index + 3] = (unsigned char)(alpha * 255); // Alpha
        }
    }

    glGenTextures(1, &teksturaDymu);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, teksturaDymu);

    //glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindImageTexture(1, teksturaDymu, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void initParticles(){
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, nr_particles * sizeof(Particle), nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, color));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, life));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    createParticleTexture();
}

void DrawParticles(unsigned int shaderProgram, std::vector<Particle>& particles){
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, particles.size() * sizeof(Particle), particles.data());

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, teksturaDymu);
    glUniform1i(glGetUniformLocation(shaderProgram, "dym"), 1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDrawArrays(GL_POINTS, 0, particles.size());
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_BLEND);
}

