#version 430 core

layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(binding = 0) uniform writeonly image2D imgOutput;
layout (location = 0) uniform float t;

float random(vec2 p)
{
    vec3 p3  = fract(vec3(p.xyx) * .1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

void main()
{
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv = vec2(pixelCoord);///vec2(gl_NumWorkGroups/16);

    float noise = random(uv+t);

    vec4 color = vec4(
                     noise,//+fract(t),
                     noise,//+fract(t/2),
                     noise,//+fract(t/3),
                     1.0f
                 );

    imageStore(imgOutput, pixelCoord, color);
}
