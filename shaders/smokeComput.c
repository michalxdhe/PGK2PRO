#version 430

layout (local_size_x = 8, local_size_y = 8) in;

layout (binding = 0, rgba32f) readonly uniform image2D velocity;
layout (binding = 1, r32f) readonly uniform image2D densityIn;
layout (binding = 2, r32f) writeonly uniform image2D densityOut;


uniform float dt = 0.1;
uniform float gridScale = 1.0;
uniform ivec2 simResolution;

vec2 getVelocity(ivec2 pos) {
    return imageLoad(velocity, pos).xy;
}

float sampleDensity(ivec2 pos) {
    return imageLoad(densityIn, pos).r;
}

void main() {
    ivec2 id = ivec2(gl_GlobalInvocationID.xy);
    vec2 vel = getVelocity(id);

    vec2 prevPos = vec2(id) - vel * dt / gridScale;

    ivec2 samplePos = ivec2(clamp(prevPos, vec2(0.0), vec2(simResolution - 1)));
    float newDensity = sampleDensity(samplePos);

    imageStore(densityOut, id, vec4(newDensity, 0.0, 0.0, 0.0));
}
