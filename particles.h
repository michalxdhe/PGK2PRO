#ifndef PARTICLES_H_INCLUDED
#define PARTICLES_H_INCLUDED

#include "common.h"

static unsigned int nr_particles = 500;

struct Particle {
    glm::vec3 position, velocity;
    glm::vec4 color;
    float life;
    float gravity;

    Particle();
};

static unsigned int VAO, VBO;

void initParticles();

void DrawParticles(unsigned int shaderProgram, std::vector<Particle>& particles);

#endif // UNITS_H_INCLUDED
