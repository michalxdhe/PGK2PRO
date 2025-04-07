#version 430 core

layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(binding = 0) uniform writeonly image2D imgOutput;
layout (location = 0) uniform float t;

float random(vec2 uv)
{
    return fract(dot(uv,vec2(12.9898,78.233))*43758.5453123);
}

void main() {
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv = vec2(pixelCoord)/vec2(gl_NumWorkGroups/16);

    float noise = random(uv * t);

    vec4 color = vec4(
        fract(t),
        fract(t),
        fract(t),
        1.0f
    );

    imageStore(imgOutput, pixelCoord, color);
}
