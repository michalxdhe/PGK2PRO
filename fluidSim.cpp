#include "FluidSim.h"

GLuint FluidSim::_cubeVAO = 0;
GLuint FluidSim::_cubeVBO = 0;

FluidSim::FluidSim(const glm::vec3& worldPos,
                   const glm::ivec3& gridSize,
                   const glm::vec3& gridSpacing,
                   float dissipation,
                   float vorticityStrength,
                   int jacobiIters,
                   float scaleModifier)
    : _gridSize(gridSize)
    , _gridSpacing(gridSpacing)
    , _dissipation(dissipation)
    , _vorticityStrength(vorticityStrength)
    , _jacobiIters(jacobiIters)
    , _scaleModifier(scaleModifier)
{

    glm::vec3 renderSize = glm::vec3(gridSize) * scaleModifier;

    _model = glm::translate(glm::mat4(1.0f), worldPos);
    _model = glm::scale(_model, renderSize);

}

FluidSim::~FluidSim()
{
    GLuint toDel[9] =
    {
        _texVelocity, _texVelocityPrev, _texVelocityOut,
        _texDensity, _texDensityPrev, _texDensityOut,
        _texDivergence,
        _texPressure, _texPressurePrev
    };
    glDeleteTextures(9, toDel);
}

void FluidSim::initialize()
{
    initTexturesCPU();
    if (_cubeVAO == 0) setupCubeMesh();
}

GLuint FluidSim::createVolumeTexture(GLenum internalFmt, GLenum fmt, GLenum type)
{
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_3D, tex);
    glTexImage3D(GL_TEXTURE_3D, 0, internalFmt,
                 _gridSize.x, _gridSize.y, _gridSize.z,
                 0, fmt, type, nullptr);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //float borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    //glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
    return tex;
}

void FluidSim::initTexturesCPU()
{
    _texDensity = createVolumeTexture(GL_RGBA32F, GL_RGBA, GL_FLOAT);
    _texDensityPrev = createVolumeTexture(GL_RGBA32F, GL_RGBA, GL_FLOAT);
    _texDensityOut  = createVolumeTexture(GL_RGBA32F, GL_RGBA, GL_FLOAT);

    _texVelocity = createVolumeTexture(GL_RGBA32F, GL_RGBA, GL_FLOAT);
    _texVelocityPrev = createVolumeTexture(GL_RGBA32F, GL_RGBA, GL_FLOAT);
    _texVelocityOut = createVolumeTexture(GL_RGBA32F, GL_RGBA, GL_FLOAT);

    _texDivergence = createVolumeTexture(GL_R32F, GL_RED, GL_FLOAT);
    _texPressure = createVolumeTexture(GL_R32F, GL_RED, GL_FLOAT);
    _texPressurePrev = createVolumeTexture(GL_R32F, GL_RED, GL_FLOAT);

    size_t voxels = size_t(_gridSize.x)*_gridSize.y*_gridSize.z;
    std::vector<float> zeroRGBA(voxels*4, 0.0f);
    std::vector<float> zeroR  (voxels,   0.0f);

    auto clearTex = [&](GLuint tex, GLenum fmt, void* data)
    {
        glBindTexture(GL_TEXTURE_3D, tex);
        glTexSubImage3D(GL_TEXTURE_3D,0,0,0,0,
                        _gridSize.x,_gridSize.y,_gridSize.z,
                        fmt, GL_FLOAT, data);
    };

    clearTex(_texDensity, GL_RGBA, zeroRGBA.data());
    clearTex(_texDensityPrev,GL_RGBA, zeroRGBA.data());
    clearTex(_texVelocity, GL_RGBA, zeroRGBA.data());
    clearTex(_texVelocityPrev,GL_RGBA, zeroRGBA.data());
    clearTex(_texDivergence, GL_RED,  zeroR.data());
    clearTex(_texPressure, GL_RED,  zeroR.data());
    clearTex(_texPressurePrev,GL_RED,  zeroR.data());
    clearTex(_texDensityOut, GL_RGBA, zeroRGBA.data());
    clearTex(_texVelocityOut, GL_RGBA, zeroRGBA.data());
}

void FluidSim::setupCubeMesh()
{
    float verts[] =
    {
            -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f, 0.5f,-0.5f,
            0.5f, 0.5f,-0.5f, -0.5f, 0.5f,-0.5f, -0.5f,-0.5f,-0.5f,
            -0.5f,-0.5f, 0.5f,  0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f,
            0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f,-0.5f, 0.5f,
            -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f, -0.5f,-0.5f,-0.5f,
            -0.5f,-0.5f,-0.5f, -0.5f,-0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
            0.5f, 0.5f, 0.5f,  0.5f, 0.5f,-0.5f,  0.5f,-0.5f,-0.5f,
            0.5f,-0.5f,-0.5f,  0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f,
            -0.5f,-0.5f, 0.5f,  0.5f,-0.5f, 0.5f,  0.5f,-0.5f,-0.5f,
            0.5f,-0.5f,-0.5f, -0.5f,-0.5f,-0.5f, -0.5f,-0.5f, 0.5f,
            -0.5f, 0.5f,-0.5f,  0.5f, 0.5f,-0.5f,  0.5f, 0.5f, 0.5f,
            0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f
        };
    glGenVertexArrays(1, &_cubeVAO);
    glGenBuffers(1, &_cubeVBO);
    glBindVertexArray(_cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, _cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void FluidSim::simulate(const std::array<GLuint,7>& P, float dt)
{
    glCopyImageSubData(
        _texDensity, GL_TEXTURE_3D, 0, 0,0,0,
        _texDensityPrev, GL_TEXTURE_3D, 0, 0,0,0,
        _gridSize.x, _gridSize.y, _gridSize.z
    );

    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glCopyImageSubData(
        _texVelocity, GL_TEXTURE_3D, 0, 0,0,0,
        _texVelocityPrev, GL_TEXTURE_3D, 0, 0,0,0,
        _gridSize.x, _gridSize.y, _gridSize.z
    );

    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    advect(P[0], dt);
    std::swap(_texVelocity, _texVelocityOut);
    applyForce(P[1], dt);
    std::swap(_texVelocity, _texVelocityOut);
    vorticityConfinement(P[2], dt);
    std::swap(_texVelocity, _texVelocityOut);
    computeDivergence(P[3]);
    solvePressure(P[4]);
    projectVelocity(P[5]);
    //fixBoundries(P[6]);
    std::swap(_texDensity,  _texDensityOut);
    std::swap(_texVelocity, _texVelocityOut);
}

void FluidSim::advect(GLuint prog, float dt)
{
    glUseProgram(prog);

    glUniform1f(glGetUniformLocation(prog, "timeStep"),     dt);
    glUniform1f(glGetUniformLocation(prog, "dissipation"),  _dissipation);
    glUniform3fv(glGetUniformLocation(prog,"gridSize"),1, glm::value_ptr(glm::vec3(_gridSize)));
    glUniform3fv(glGetUniformLocation(prog,"gridSpacing"),1, glm::value_ptr(_gridSpacing));

    glBindImageTexture(0, _texDensityOut, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(1, _texVelocityOut,0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, _texVelocity);
    glUniform1i(glGetUniformLocation(prog, "velocity"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, _texVelocityPrev);
    glUniform1i(glGetUniformLocation(prog, "velocityTemp"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_3D, _texDensity);
    glUniform1i(glGetUniformLocation(prog, "quantity"), 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_3D, _texDensityPrev);
    glUniform1i(glGetUniformLocation(prog, "quantityPrev"), 4);

    GLuint wx = (_gridSize.x +7)/8,
           wy = (_gridSize.y +7)/8,
           wz = (_gridSize.z +7)/8;
    glDispatchCompute(wx, wy, wz);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void FluidSim::applyForce(GLuint prog, float dt)
{
    glUseProgram(prog);

    GLint locVel = glGetUniformLocation(prog, "velocity");
    assert(locVel >= 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, _texVelocity);
    glUniform1i(locVel, 1);

    GLint locDen = glGetUniformLocation(prog, "quantity");
    //assert(locDen >= 0);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, _texDensity);
    glUniform1i(locDen, 2);

    GLint locTs = glGetUniformLocation(prog, "timeStep");
    GLint locGs = glGetUniformLocation(prog, "gridSize");
    GLint locAl = glGetUniformLocation(prog, "alpha");
    GLint locAd = glGetUniformLocation(prog, "ambientDensity");

    glUniform1f(locTs, dt);
    glUniform3fv(locGs, 1, glm::value_ptr(glm::vec3(_gridSize)));
    glUniform1f(locAl, 0.1f);
    glUniform1f(locAd, 0.1f);

    glBindImageTexture(0, _texVelocityOut,
                       0, GL_TRUE, 0,
                       GL_WRITE_ONLY, GL_RGBA32F);

    GLuint wx = (_gridSize.x +7)/8,
           wy = (_gridSize.y +7)/8,
           wz = (_gridSize.z +7)/8;
    glDispatchCompute(wx, wy, wz);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}


void FluidSim::vorticityConfinement(GLuint prog, float dt)
{
    glUseProgram(prog);

    glUniform1f(glGetUniformLocation(prog, "timeStep"), dt);
    glUniform3fv(glGetUniformLocation(prog,"gridSize"),1, glm::value_ptr(glm::vec3(_gridSize)));
    glUniform3fv(glGetUniformLocation(prog,"gridSpacing"),1, glm::value_ptr(_gridSpacing));
    glUniform1f(glGetUniformLocation(prog, "vorticityStrength"), _vorticityStrength);

    glBindImageTexture(0, _texVelocityOut, 0, GL_TRUE, 0,
                       GL_WRITE_ONLY, GL_RGBA32F);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, _texVelocity);
    glUniform1i(glGetUniformLocation(prog, "velocity"), 1);

    GLuint wx = (_gridSize.x +7)/8,
           wy = (_gridSize.y +7)/8,
           wz = (_gridSize.z +7)/8;
    glDispatchCompute(wx, wy, wz);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void FluidSim::computeDivergence(GLuint prog)
{
    glUseProgram(prog);

    GLint locVel = glGetUniformLocation(prog, "velocity");
    assert(locVel >= 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, _texVelocity);
    glUniform1i(locVel, 1);

    GLint locGS = glGetUniformLocation(prog, "gridSize");
    GLint locGSp = glGetUniformLocation(prog, "gridSpacing");

    glUniform3fv(locGS, 1, glm::value_ptr(glm::vec3(_gridSize)));
    glUniform3fv(locGSp, 1, glm::value_ptr(_gridSpacing));

    glBindImageTexture(0, _texDivergence,
                       0, GL_TRUE, 0,
                       GL_WRITE_ONLY, GL_R32F);

    GLuint wx = (_gridSize.x +7)/8,
           wy = (_gridSize.y +7)/8,
           wz = (_gridSize.z +7)/8;
    glDispatchCompute(wx, wy, wz);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}


void FluidSim::solvePressure(GLuint prog)
{
    glUseProgram(prog);

    float alpha = -_gridSpacing.x * _gridSpacing.x;
    float invBeta = 1.0f / 6.0f;
    glUniform1f(glGetUniformLocation(prog, "alpha"),   alpha);
    glUniform1f(glGetUniformLocation(prog, "invBeta"), invBeta);
    glUniform3fv(glGetUniformLocation(prog,"gridSize"),1, glm::value_ptr(glm::vec3(_gridSize)));

    GLuint wx = (_gridSize.x +7)/8,
           wy = (_gridSize.y +7)/8,
           wz = (_gridSize.z +7)/8;

    for(int i = 0; i < _jacobiIters; ++i)
    {
        glBindImageTexture(0, _texPressurePrev,  0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_3D, _texPressure);
        glUniform1i(glGetUniformLocation(prog, "pressure"), 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_3D, _texDivergence);
        glUniform1i(glGetUniformLocation(prog, "divergence"), 2);

        glDispatchCompute(wx, wy, wz);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        std::swap(_texPressure, _texPressurePrev);
    }
}

void FluidSim::projectVelocity(GLuint prog)
{
    glUseProgram(prog);

    glUniform3fv(glGetUniformLocation(prog,"gridSize"),1, glm::value_ptr(glm::vec3(_gridSize)));
    glUniform3fv(glGetUniformLocation(prog,"gridSpacing"),1, glm::value_ptr(_gridSpacing));

    glBindImageTexture(0, _texVelocityOut, 0, GL_TRUE, 0,
                       GL_WRITE_ONLY, GL_RGBA32F);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, _texVelocity);
    glUniform1i(glGetUniformLocation(prog, "velocity"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, _texPressure);
    glUniform1i(glGetUniformLocation(prog, "pressure"), 2);

    GLuint wx = (_gridSize.x +7)/8,
           wy = (_gridSize.y +7)/8,
           wz = (_gridSize.z +7)/8;
    glDispatchCompute(wx, wy, wz);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void FluidSim::fixBoundries(GLuint prog)
{
    glUseProgram(prog);
    glUniform3i(glGetUniformLocation(prog, "gridSize"),
                _gridSize.x, _gridSize.y, _gridSize.z);
    glBindImageTexture(0, _texVelocityOut, 0, GL_TRUE, 0,
                       GL_WRITE_ONLY, GL_RGBA32F);

    GLuint wx = (_gridSize.x +7)/8;
    GLuint wy = (_gridSize.y +7)/8;
    GLuint wz = (_gridSize.z +7)/8;

    glDispatchCompute(1, wy, wz);
    glDispatchCompute(wx, 1, wz);
    glDispatchCompute(wx, wy, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void FluidSim::render(GLuint volumeShader, glm::mat4 view, glm::mat4 proj)
{
 std::array<GLuint,4> textures = {
        _texDensity,
        _texVelocity,
        _texDivergence,
        _texPressure
    };

    glUseProgram(volumeShader);

    GLint locModel    = glGetUniformLocation(volumeShader, "model");
    GLint locInvModel = glGetUniformLocation(volumeShader, "invModel");
    GLint locView     = glGetUniformLocation(volumeShader, "view");
    GLint locProj     = glGetUniformLocation(volumeShader, "projection");
    GLint locVolume   = glGetUniformLocation(volumeShader, "volumeTex");

    glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(locProj, 1, GL_FALSE, glm::value_ptr(proj));

    float sx = _gridSize.x * _gridSpacing.x;
    float sy = _gridSize.y * _gridSpacing.y;
    float sz = _gridSize.z * _gridSpacing.z;

    float cubeSize = std::max(std::max(sx, sy), sz);

    float spacing = cubeSize * 1.2f;

    glBindVertexArray(_cubeVAO);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    for (int i = 0; i < 4; ++i) {
        float offset = (i - 1.5f) * spacing;

        glm::mat4 M = glm::translate(glm::mat4(1.0f), glm::vec3(offset, 0.0f, 0.0f))* _model;

        glUniformMatrix4fv(locModel,    1, GL_FALSE, glm::value_ptr(M));
        glUniformMatrix4fv(locInvModel, 1, GL_FALSE,
             glm::value_ptr(glm::inverse(M)));

        glUniform1i(glGetUniformLocation(volumeShader, "visualizeMode"),i);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, textures[i]);
        glUniform1i(locVolume, 0);

        glDrawArrays(GL_TRIANGLES, 0, 36);
        //break;
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glBindVertexArray(0);
}


void FluidSim::addSmokeBox(const glm::ivec3& minG,
                           const glm::ivec3& maxG,
                           const glm::vec4& color)
{
    glm::ivec3 lo = glm::clamp(minG, glm::ivec3(0), _gridSize - 1);
    glm::ivec3 hi = glm::clamp(maxG, glm::ivec3(0), _gridSize - 1);

    int sx = hi.x - lo.x + 1;
    int sy = hi.y - lo.y + 1;
    int sz = hi.z - lo.z + 1;

    std::vector<float> block(sx * sy * sz * 4);

    for (int z = 0; z < sz; ++z)
        for (int y = 0; y < sy; ++y)
            for (int x = 0; x < sx; ++x)
            {
                int i = 4 * (x + y*sx + z*sx*sy);
                block[i+0] = color.r;
                block[i+1] = color.g;
                block[i+2] = color.b;
                block[i+3] = color.a;
            }

    for (GLuint tex :
            {
                _texDensity, _texDensityPrev
            })
    {
        glBindTexture(GL_TEXTURE_3D, tex);
        glTexSubImage3D(GL_TEXTURE_3D, 0,
                        lo.x, lo.y, lo.z,
                        sx, sy, sz,
                        GL_RGBA, GL_FLOAT,
                        block.data());
    }

    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void FluidSim::addSmokeSphere(const glm::vec3& centerG,
                              float radius,
                              const glm::vec4& color)
{
    int r = int(std::ceil(radius));
    glm::ivec3 c = glm::ivec3(centerG + glm::vec3(0.5f));

    glm::ivec3 lo = c - glm::ivec3(r);
    glm::ivec3 hi = c + glm::ivec3(r);

    lo = glm::clamp(lo, glm::ivec3(0), _gridSize - 1);
    hi = glm::clamp(hi, glm::ivec3(0), _gridSize - 1);

    int sx = hi.x - lo.x + 1;
    int sy = hi.y - lo.y + 1;
    int sz = hi.z - lo.z + 1;

    std::vector<float> block(sx * sy * sz * 4, 0.0f);
    for (int z = 0; z < sz; ++z)
    {
        for (int y = 0; y < sy; ++y)
        {
            for (int x = 0; x < sx; ++x)
            {
                glm::vec3 p = glm::vec3(lo + glm::ivec3(x,y,z)) - centerG;
                if (glm::dot(p,p) <= radius*radius)
                {
                    int i = 4 * (x + y*sx + z*sx*sy);
                    block[i+0] = color.r;
                    block[i+1] = color.g;
                    block[i+2] = color.b;
                    block[i+3] = color.a;
                }
            }
        }
    }

    for (GLuint tex :
            {
                _texDensity, _texDensityPrev
            })
    {
        glBindTexture(GL_TEXTURE_3D, tex);
        glTexSubImage3D(GL_TEXTURE_3D, 0,
                        lo.x, lo.y, lo.z,
                        sx, sy, sz,
                        GL_RGBA, GL_FLOAT,
                        block.data());
    }
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void FluidSim::addVelocityImpulse(const glm::vec3& cGrid,
                                  float radius,
                                  const glm::vec3& impulse)
{
    int nx = _gridSize.x, ny = _gridSize.y, nz = _gridSize.z;
    size_t totalVoxels = size_t(nx) * ny * nz;
    size_t totalFloats = totalVoxels * 4;

    std::vector<float> velData(totalFloats);
    glBindTexture(GL_TEXTURE_3D, _texVelocity);
    glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, velData.data());

    int r = int(std::ceil(radius));
    int cx = int(std::round(cGrid.x));
    int cy = int(std::round(cGrid.y));
    int cz = int(std::round(cGrid.z));

    int xmin = glm::clamp(cx - r, 0, nx - 1);
    int xmax = glm::clamp(cx + r, 0, nx - 1);
    int ymin = glm::clamp(cy - r, 0, ny - 1);
    int ymax = glm::clamp(cy + r, 0, ny - 1);
    int zmin = glm::clamp(cz - r, 0, nz - 1);
    int zmax = glm::clamp(cz + r, 0, nz - 1);

    float r2 = radius * radius;

    for (int z = zmin; z <= zmax; ++z)
    {
        for (int y = ymin; y <= ymax; ++y)
        {
            for (int x = xmin; x <= xmax; ++x)
            {
                float dx = float(x) - cGrid.x;
                float dy = float(y) - cGrid.y;
                float dz = float(z) - cGrid.z;
                if (dx*dx + dy*dy + dz*dz <= r2)
                {
                    size_t idx = size_t(z)*ny*nx + size_t(y)*nx + size_t(x);
                    size_t f = idx * 4;
                    velData[f+0] += impulse.x;
                    velData[f+1] += impulse.y;
                    velData[f+2] += impulse.z;
                    velData[f+3] = 1.f;
                }
            }
        }
    }

    for (GLuint tex :
            {
                _texVelocity
            })
    {
        glBindTexture(GL_TEXTURE_3D, tex);
        glTexSubImage3D(GL_TEXTURE_3D, 0,
                        0, 0, 0,
                        nx, ny, nz,
                        GL_RGBA, GL_FLOAT,
                        velData.data());
    }

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

void FluidSim::rasterizeVoxelizeModel(Model& model, GLuint rasterProg, const glm::mat4& modelMatrix)
{
    glm::vec3 minB = model.GetMinBounds();
    glm::vec3 maxB = model.GetMaxBounds();
    glm::vec3 size = maxB - minB;
    glm::vec3 center = (minB + maxB) * 0.5f;

    glm::vec3 grid((float)_gridSize.x, (float)_gridSize.y, (float)_gridSize.z);

    glm::vec3 worldPerVoxelVec = size / grid;
    float worldPerVoxel = std::max(std::max(worldPerVoxelVec.x, worldPerVoxelVec.y), worldPerVoxelVec.z);

    glm::vec3 totalWorldSize = worldPerVoxel * grid;
    glm::vec3 adjMin = center - (totalWorldSize * 0.5f);
    glm::vec3 adjMax = center + (totalWorldSize * 0.5f);

    float dz = worldPerVoxel;

    std::cout << "Og Min bounds: " << glm::to_string(minB) << "\n";
    std::cout << "Og Max bounds: " << glm::to_string(maxB) << "\n";
    std::cout << "Adj Min bounds: " << glm::to_string(adjMin) << "\n";
    std::cout << "Adj Max bounds: " << glm::to_string(adjMax) << "\n";

    GLint oldFBO, oldDrawBuf, oldVP[4];
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFBO);
    glGetIntegerv(GL_DRAW_BUFFER, &oldDrawBuf);
    glGetIntegerv(GL_VIEWPORT, oldVP);
    GLboolean oldDepth = glIsEnabled(GL_DEPTH_TEST);
    GLboolean oldCull  = glIsEnabled(GL_CULL_FACE);

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    std::vector<float> zeros(_gridSize.x * _gridSize.y * _gridSize.z * 4, 0.0f);
    glBindTexture(GL_TEXTURE_3D, _texDensity);
    glTexSubImage3D(GL_TEXTURE_3D,
                    0,
                    0, 0, 0,
                    _gridSize.x, _gridSize.y, _gridSize.z,
                    GL_RGBA, GL_FLOAT,
                    zeros.data());

    glUseProgram(rasterProg);

    glViewport(0, 0, _gridSize.x, _gridSize.y);

    GLint locM = glGetUniformLocation(rasterProg, "uModel");
    GLint locV = glGetUniformLocation(rasterProg, "uView");
    GLint locP = glGetUniformLocation(rasterProg, "uProj");
    GLint locMinB = glGetUniformLocation(rasterProg, "uMinBounds");
    GLint locHalfZ = glGetUniformLocation(rasterProg, "uHalfSlice");
    GLint locSlice = glGetUniformLocation(rasterProg, "uSliceZ");

    glUniformMatrix4fv(locM, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    glm::mat4 view = glm::mat4(1.0f);
    glUniformMatrix4fv(locV, 1, GL_FALSE, glm::value_ptr(view));

    glm::mat4 proj = glm::ortho(
        adjMin.x, adjMax.x,
        adjMin.y, adjMax.y,
        -1.0f, 1.0f
    );

    glUniformMatrix4fv(locP, 1, GL_FALSE, glm::value_ptr(proj));

    glUniform3fv(locMinB, 1, glm::value_ptr(adjMin));

    glUniform1f(locHalfZ, dz * 0.5f);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    for (int z = 0; z < _gridSize.z; ++z) {
        float sliceZ = adjMin.z + float(z) * dz;
        glUniform1f(locSlice, sliceZ);

        glFramebufferTextureLayer(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            _texDensity,
            0,
            z
        );
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        model.Draw(rasterProg, false);
    }

    if(oldDepth) glEnable(GL_DEPTH_TEST);
    else glDisable(GL_DEPTH_TEST);

    if(oldCull) glEnable(GL_CULL_FACE);
    else glDisable(GL_CULL_FACE);

    glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
    glDrawBuffer(oldDrawBuf);
    glViewport(oldVP[0], oldVP[1], oldVP[2], oldVP[3]);

    glDeleteFramebuffers(1, &fbo);

    glCopyImageSubData(
        _texDensity,      GL_TEXTURE_3D, 0, 0, 0, 0,
        _texDensityPrev,  GL_TEXTURE_3D, 0, 0, 0, 0,
        _gridSize.x, _gridSize.y, _gridSize.z
    );
}


