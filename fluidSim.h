#ifndef FLUIDSIM_H_INCLUDED
#define FLUIDSIM_H_INCLUDED

#include "common.h"

class FluidSim
{
public:
    FluidSim(const glm::vec3& worldPos,
             const glm::ivec3& gridSize,
             const glm::vec3& gridSpacing,
             float dissipation      = 0.99f,
             float vorticityStrength = 20.0f,
             int   jacobiIters      = 20,
             float scaleModifier     = 0.01f);

    ~FluidSim();

    void initialize();

    void simulate(const std::array<GLuint,6>& progs, float dt);

    void render(GLuint volumeShader, glm::mat4 view, glm::mat4 proj);

    void addSmokeBox(const glm::ivec3& minGrid,
                     const glm::ivec3& maxGrid,
                     const glm::vec4& color);

    void addSmokeSphere(const glm::vec3& centerGrid,
                        float radius,
                        const glm::vec4& color);

    void addVelocityImpulse(const glm::vec3& centerGrid,
                            float radius,
                            const glm::vec3& impulse);

private:
    glm::ivec3 _gridSize;
    glm::vec3  _gridSpacing;
    float      _dissipation;
    float      _vorticityStrength;
    int        _jacobiIters;
    glm::mat4  _model;
    float   _scaleModifier;

    GLuint _texVelocity,   _texVelocityPrev,   _texVelocityOut;
    GLuint _texDensity,    _texDensityPrev,    _texDensityOut;
    GLuint _texDivergence;
    GLuint _texPressure,   _texPressurePrev;

    static GLuint _cubeVAO, _cubeVBO;

    GLuint createVolumeTexture(GLenum internalFmt, GLenum fmt, GLenum type);
    void   initTexturesCPU();
    void   setupCubeMesh();

    void advect      (GLuint prog, float dt);
    void applyForce  (GLuint prog, float dt);
    void vorticityConfinement(GLuint prog, float dt);
    void computeDivergence   (GLuint prog);
    void solvePressure       (GLuint prog);
    void projectVelocity     (GLuint prog);
};

#endif // FLUIDSIM_H_INCLUDED
