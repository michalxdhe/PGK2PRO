#version 430
layout(local_size_x=8, local_size_y=8, local_size_z=1) in;

layout(rgba32f, binding=0) uniform image3D velOut;
uniform ivec3 gridSize;

void main() {
    int x = int(gl_GlobalInvocationID.x);
    int y = int(gl_GlobalInvocationID.y);
    int z = int(gl_GlobalInvocationID.z);

    imageStore(velOut, ivec3(x,y,z), vec4(0.0));
}
