#include "particles.h"

Particle::Particle() : position(0.0f), velocity(0.0f), color(1.0f), life(0.0f), gravity(0.5f) { }

void initParticles(){
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, nr_particles * sizeof(Particle), nullptr, GL_DYNAMIC_DRAW);

    //pozycja
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)0);
    glEnableVertexAttribArray(0);

    //kolor
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, color));
    glEnableVertexAttribArray(1);

    //zycko
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, life));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void DrawParticles(unsigned int shaderProgram, std::vector<Particle>& particles){
    glUseProgram(shaderProgram);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, particles.size() * sizeof(Particle), particles.data());

    glDrawArrays(GL_POINTS, 0, particles.size());

    glBindVertexArray(0);
}
