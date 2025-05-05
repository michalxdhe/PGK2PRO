#version 430

layout(local_size_x=8, local_size_y=8, local_size_z=8) in;

layout(rgba32f, binding=0) uniform image3D outputVelocity;

uniform sampler3D velocity;
uniform sampler3D quantity;
uniform float timeStep;
uniform vec3 gridSize;
uniform float alpha;
uniform float ambientDensity;

void main()
{
    ivec3 g = ivec3(gl_GlobalInvocationID);
    vec3 uv = (vec3(g) + 0.5) / gridSize;

    vec3 v = texture(velocity, uv).xyz;
    float d = texture(quantity, uv).x - ambientDensity;

    vec3 buoy = vec3(0.0, alpha * d, 0.0);
    v += timeStep * buoy;

    imageStore(outputVelocity, g, vec4(v, 1.0));
}
