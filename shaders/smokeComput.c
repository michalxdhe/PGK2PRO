#version 430

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

uniform sampler3D velocity;
uniform sampler3D velocityTemp;
uniform sampler3D quantity;
uniform sampler3D quantityPrev;

uniform float timeStep;
uniform float dissipation;
uniform vec3 gridSize;
uniform vec3 gridSpacing;
uniform float vorticityStrength;

layout(rgba32f, binding = 0) uniform image3D outputQuantity;
layout(rgba32f, binding = 1) uniform image3D outputVelocity;

vec3 clampUV(vec3 uvw) {
    vec3 margin = vec3(1.0) / gridSize;
    return clamp(uvw, margin, vec3(1.0) - margin);
}

vec3 getVelocity(vec3 uvw) {
    return texture(velocity, clampUV(uvw)).xyz;
}

vec3 getVelocityTemp(vec3 uvw) {
    return texture(velocityTemp, clampUV(uvw)).xyz;
}

vec4 sampleQuantity(vec3 uvw) {
    return texture(quantity, clampUV(uvw));
}

vec4 sampleQuantityPrev(vec3 uvw) {
    return texture(quantityPrev, clampUV(uvw));
}

vec4 macCormackAdvect(vec3 uvw) {
    vec3 vel = getVelocity(uvw);
    vec3 pos = clampUV(uvw - timeStep * vel * gridSpacing);
    vec4 forward = sampleQuantity(pos);

    vec3 velBack = getVelocity(pos);
    vec3 posBack = clampUV(pos + timeStep * velBack * gridSpacing);
    vec4 backward = sampleQuantityPrev(posBack);

    vec4 orig = sampleQuantity(uvw);
    vec4 corrected = forward + 0.5 * (orig - backward);

    vec4 minVal = min(forward, orig);
    vec4 maxVal = max(forward, orig);
    corrected = clamp(corrected, minVal, maxVal);

    return mix(forward, corrected, 0.9);
}

void main() {
    ivec3 gid = ivec3(gl_GlobalInvocationID.xyz);
    vec3 uvw = (vec3(gid) + 0.5) / gridSize;

    vec4 advected = macCormackAdvect(uvw);
    advected *= dissipation;
    if (length(advected.rgb) < 1e-5) {
        advected.rgb = vec3(0.0);
    }
    advected = max(advected, vec4(0.0));
    imageStore(outputQuantity, gid, advected);


    //i'd put my vorticity here, if i had one
    //imageStore(outputVelocity, gid, vec4(vel, 1.0));
}
