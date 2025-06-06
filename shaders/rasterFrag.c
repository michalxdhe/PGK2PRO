#version 430 core
in vec3 vWorldPos;

uniform vec3  uMinBounds;
uniform float uHalfSlice;
uniform float uSliceZ;

layout(location=0) out vec4 outDensity;

void main() {
    if (abs(vWorldPos.z - uSliceZ) <= uHalfSlice) {
        outDensity = vec4(1.0);
    } else {
        discard;
    }
}
