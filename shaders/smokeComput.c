#version 430

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

uniform sampler3D velocity;
uniform sampler3D quantity;
uniform sampler3D quantityPrev;

uniform float timeStep;
uniform float dissipation;
uniform vec3 gridSize;
uniform vec3 gridSpacing;

layout(rgba32f, binding = 0) uniform image3D outputQuantity;

vec3 getVelocity(vec3 pos) {
    return texture(velocity, pos).xyz;
}

vec4 sampleQuantity(vec3 pos) {
    return texture(quantity, pos);
}

vec4 macCormackAdvect(vec3 uvw) {
    vec3 vel = getVelocity(uvw);
    vec3 pos = uvw - timeStep * vel * gridSpacing;
    vec4 forward = sampleQuantity(pos);

    vec3 posBack = pos + timeStep * getVelocity(pos) * gridSpacing;
    vec4 backward = texture(quantityPrev, posBack);

    vec4 corrected = forward + 0.5 * (sampleQuantity(uvw) - backward);
    return mix(forward, corrected, 0.9);
}

void main() {
    ivec3 gid = ivec3(gl_GlobalInvocationID.xyz);
    vec3 uvw = (vec3(gid) + 0.5) / gridSize;

    vec4 advected = macCormackAdvect(uvw);
    advected *= dissipation;

    imageStore(outputQuantity, gid, advected);
}
