#version 430 core

in vec3 FragPos;
in vec3 Normal;
in vec2 uv;

out vec4 FragColor;

uniform float t;
uniform sampler2D densityTex;


void main()
{
    FragColor = vec4(uv , 1.0);
}
