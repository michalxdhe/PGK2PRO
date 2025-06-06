#version 430 core
layout(location=0) in vec3 aPos;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

out vec3 vWorldPos;

void main() {
    vec4 wp = uModel * vec4(aPos, 1.0);
    vWorldPos   = wp.xyz;
    gl_Position = uProj * uView * wp;
}
