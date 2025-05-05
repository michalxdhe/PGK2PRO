#version 430
layout(local_size_x=8, local_size_y=8, local_size_z=8) in;

uniform sampler3D velocity;
uniform sampler3D pressure;
uniform vec3 gridSize;
uniform vec3 gridSpacing;

layout(rgba32f, binding=0) uniform image3D outputVelocity;

void main()
{
    ivec3 g = ivec3(gl_GlobalInvocationID);
    vec3 uvw = (vec3(g)+0.5)/gridSize;
    float dx=gridSpacing.x, dy=gridSpacing.y, dz=gridSpacing.z;

    float pL = texture(pressure, (uvw - vec3(1,0,0)/gridSize)).r;
    float pR = texture(pressure, (uvw + vec3(1,0,0)/gridSize)).r;
    float pD = texture(pressure, (uvw - vec3(0,1,0)/gridSize)).r;
    float pU = texture(pressure, (uvw + vec3(0,1,0)/gridSize)).r;
    float pB = texture(pressure, (uvw - vec3(0,0,1)/gridSize)).r;
    float pF = texture(pressure, (uvw + vec3(0,0,1)/gridSize)).r;

    vec3 v = texture(velocity, uvw).xyz;
    v.x -= (pR - pL) / (2*dx);
    v.y -= (pU - pD) / (2*dy);
    v.z -= (pF - pB) / (2*dz);

    imageStore(outputVelocity, g, vec4(v,1.0));
}
