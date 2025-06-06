#version 430
layout(local_size_x=8, local_size_y=8, local_size_z=8) in;

layout(r32f, binding=0) uniform image3D outputPressure;
uniform sampler3D pressure;
uniform sampler3D divergence;
uniform float alpha;
uniform float invBeta;
uniform vec3 gridSize;

void main()
{
    ivec3 g = ivec3(gl_GlobalInvocationID);
    vec3 uv = (vec3(g) + 0.5) / gridSize;

    float pL = texture(pressure, uv - vec3(1.0/gridSize.x,0,0)).x;
    float pR = texture(pressure, uv + vec3(1.0/gridSize.x,0,0)).x;
    float pD = texture(pressure, uv - vec3(0,1.0/gridSize.y,0)).x;
    float pU = texture(pressure, uv + vec3(0,1.0/gridSize.y,0)).x;
    float pB = texture(pressure, uv - vec3(0,0,1.0/gridSize.z)).x;
    float pF = texture(pressure, uv + vec3(0,0,1.0/gridSize.z)).x;
    float div = texture(divergence, uv).x;

    //poisson
    float pNew = (pL + pR + pD + pU + pB + pF + alpha*div) * invBeta;
    imageStore(outputPressure, g, vec4(pNew,0,0,0));
}
