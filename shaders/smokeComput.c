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

layout(rgba32f, binding = 0) uniform image3D outputQuantity;
layout(rgba32f, binding = 1) uniform image3D outputVelocity;

vec3 clampUV(vec3 uvw)
{
    vec3 m = vec3(1.0) / gridSize;
    return clamp(uvw, m, vec3(1.0) - m);
}

vec3 getVel(sampler3D tex, vec3 uvw)
{
    return texture(tex, clampUV(uvw)).xyz;
}

vec4 sampleQ(sampler3D tex, vec3 uvw)
{
    return texture(tex, clampUV(uvw));
}

vec4 advectScalar(sampler3D srcTex, sampler3D prevTex, vec3 uvw)
{
    vec3 vel = getVel(velocity, uvw);
    vec3 p = clampUV(uvw - timeStep * vel * gridSpacing);
    vec4 fwd = sampleQ(srcTex, p);
    vec3 velBack = getVel(velocity, p);
    vec3 pBack = clampUV(p + timeStep * velBack * gridSpacing);
    vec4 bwd = sampleQ(prevTex, pBack);
    vec4 orig = sampleQ(srcTex, uvw);
    vec4 corr = clamp(fwd + 0.5 * (orig - bwd), min(fwd, orig), max(fwd, orig));
    return mix(fwd, corr, 0.9);
}

vec3 advectVel(vec3 uvw)
{
    vec3 vel = getVel(velocity, uvw);
    vec3 p = clampUV(uvw - timeStep * vel * gridSpacing);
    vec3 fwd = getVel(velocity, p);
    vec3 velBack = getVel(velocity, p);
    vec3 pBack = clampUV(p + timeStep * velBack * gridSpacing);
    vec3 bwd = getVel(velocityTemp, pBack);
    vec3 corr = clamp(fwd + 0.5 * (vel - bwd), min(fwd, vel), max(fwd, vel));
    return mix(fwd, corr, 0.9);
}

void main()
{
    ivec3 gid = ivec3(gl_GlobalInvocationID);
    vec3 uvw = (vec3(gid) + 0.5) / gridSize;

    vec4 q = advectScalar(quantity, quantityPrev, uvw);
    q *= dissipation;
    imageStore(outputQuantity, gid, max(q, vec4(0.0)));

    vec3 v = advectVel(uvw);
    v *= 0.99;
    imageStore(outputVelocity, gid, vec4(v, 1.0));
}
