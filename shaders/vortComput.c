#version 430
layout(local_size_x=8, local_size_y=8, local_size_z=8) in;

uniform sampler3D velocity;
uniform float timeStep;
uniform vec3 gridSize;
uniform vec3 gridSpacing;
uniform float vorticityStrength;

layout(rgba32f, binding=0) uniform image3D outputVelocity;

vec3 curlAt(ivec3 g)
{
    float dz = gridSpacing.z, dy=gridSpacing.y, dx=gridSpacing.x;
    vec3 c;
    c.x = (texture(velocity, (g+ivec3(0,1,0))/gridSize).z - texture(velocity, (g+ivec3(0,-1,0))/gridSize).z)/(2*dy)
          - (texture(velocity, (g+ivec3(0,0,1))/gridSize).y - texture(velocity, (g+ivec3(0,0,-1))/gridSize).y)/(2*dz);
    c.y = (texture(velocity, (g+ivec3(0,0,1))/gridSize).x - texture(velocity, (g+ivec3(0,0,-1))/gridSize).x)/(2*dz)
          - (texture(velocity, (g+ivec3(1,0,0))/gridSize).z - texture(velocity, (g+ivec3(-1,0,0))/gridSize).z)/(2*dx);
    c.z = (texture(velocity, (g+ivec3(1,0,0))/gridSize).y - texture(velocity, (g+ivec3(-1,0,0))/gridSize).y)/(2*dx)
          - (texture(velocity, (g+ivec3(0,1,0))/gridSize).x - texture(velocity, (g+ivec3(0,-1,0))/gridSize).x)/(2*dy);
    return c;
}
void main()
{
    ivec3 g = ivec3(gl_GlobalInvocationID);

    if (g.x == 0 || g.x == int(gridSize.x)-1 ||
        g.y == 0 || g.y == int(gridSize.y)-1 ||
        g.z == 0 || g.z == int(gridSize.z)-1)
    {
        imageStore(outputVelocity, g, vec4(0.0,0.0,0.0,1.0));
        return;
    }

    vec3 uvw = (vec3(g) + 0.5) / gridSize;
    vec3 v   = texture(velocity, uvw).xyz;
    vec3 w   = curlAt(g);

    float magL = length(curlAt(g + ivec3(-1,0,0)));
    float magR = length(curlAt(g + ivec3( 1,0,0)));
    float magD = length(curlAt(g + ivec3(0,-1,0)));
    float magU = length(curlAt(g + ivec3(0, 1,0)));
    float magB = length(curlAt(g + ivec3(0,0,-1)));
    float magF = length(curlAt(g + ivec3(0,0, 1)));

    vec3 grad = vec3(magR - magL, magU - magD, magF - magB);

    float lenG = length(grad);
    vec3  N    = (lenG > 1e-6 ? grad / lenG : vec3(0.0));

    vec3 f = vorticityStrength * cross(N, w);

    imageStore(outputVelocity, g, vec4(v, 1.0));
}
