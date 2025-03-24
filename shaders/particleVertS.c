#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in float aLife;

out vec4 ParticleColor;

uniform mat4 view;
uniform mat4 projection;

void main() {
    if(aLife > 0.0){
    gl_Position = projection * view * vec4(aPos, 1.0);
    gl_PointSize = 1.0;
    ParticleColor = aColor;
    }
}
