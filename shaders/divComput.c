#version 430

layout(local_size_x=8, local_size_y=8, local_size_z=8) in;

layout(r32f, binding = 0) uniform image3D outputDivergence;
uniform sampler3D velocity;
uniform vec3 gridSize;
uniform vec3 gridSpacing;

void main()
{
    ivec3 g = ivec3(gl_GlobalInvocationID);

    vec3 uv = (vec3(g) + 0.5) / gridSize;

    float vl = texture(velocity, uv - vec3(gridSpacing.x,0,0)).x;
    float vr = texture(velocity, uv + vec3(gridSpacing.x,0,0)).x;
    float vd = texture(velocity, uv - vec3(0,gridSpacing.y,0)).y;
    float vu = texture(velocity, uv + vec3(0,gridSpacing.y,0)).y;
    float vb = texture(velocity, uv - vec3(0,0,gridSpacing.z)).z;
    float vf = texture(velocity, uv + vec3(0,0,gridSpacing.z)).z;

    float div = (vr - vl)/(2.0*gridSpacing.x)
                + (vu - vd)/(2.0*gridSpacing.y)
                + (vf - vb)/(2.0*gridSpacing.z);

    imageStore(outputDivergence, g, vec4(div, 0,0,0));
}
