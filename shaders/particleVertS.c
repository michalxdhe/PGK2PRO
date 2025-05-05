#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in float aLife;

out vec4 ParticleColor;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPos;
uniform float baseSize = 15.f;

void main()
{
    if (aLife > 0.0)
    {
        vec4 worldPos = vec4(aPos, 1.0);
        float testingos = length(cameraPos - aPos);

        float size = baseSize / (0.1 + testingos * 0.2);

        gl_Position = projection * view * worldPos;
        gl_PointSize = size;
        ParticleColor = aColor;
    }
}
