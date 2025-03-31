#version 330 core
in vec4 ParticleColor;
out vec4 FragColor;

uniform sampler2D dym;

void main() {
    vec4 texColor = texture(dym, gl_PointCoord);
    FragColor = ParticleColor * texColor;

    if (FragColor.a < 0.6)
        discard;
}
