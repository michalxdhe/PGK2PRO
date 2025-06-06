#version 430 core
layout(local_size_x = 128) in;

// SSBOs
layout(std430, binding = 0) readonly buffer VertSSBO { vec4 positions[]; };
layout(std430, binding = 1) readonly buffer IndexSSBO { uvec3 indices[]; };

// Output volume
layout(rgba32f, binding = 0) uniform image3D densityVolume;

// Uniforms
uniform uint  uNumTris;
uniform uvec3 uGridSize;

// Akenine‐Möller triangle–box overlap test
bool triBoxOverlap(vec3 bc, vec3 bh, vec3 v0, vec3 v1, vec3 v2) {
    v0 -= bc; v1 -= bc; v2 -= bc;
    vec3 e0 = v1 - v0, e1 = v2 - v1, e2 = v0 - v2;
    float p0,p1,p2,r;
    #define TEST(a,b,fa,fb) \
        p0 = a*v0.y - b*v0.z; \
        p1 = a*v1.y - b*v1.z; \
        p2 = a*v2.y - b*v2.z; \
        { float mn = min(min(p0,p1),p2); float mx = max(max(p0,p1),p2); \
          if(max(-mx,mn) > fa*bh.y + fb*bh.z) return false; }
    TEST(e0.z,e0.y,abs(e0.z),abs(e0.y));
    TEST(e1.z,e1.y,abs(e1.z),abs(e1.y));
    TEST(e2.z,e2.y,abs(e2.z),abs(e2.y));
    TEST(e0.z,e0.x,abs(e0.z),abs(e0.x));
    TEST(e1.z,e1.x,abs(e1.z),abs(e1.x));
    TEST(e2.z,e2.x,abs(e2.z),abs(e2.x));
    TEST(e0.y,e0.x,abs(e0.y),abs(e0.x));
    TEST(e1.y,e1.x,abs(e1.y),abs(e1.x));
    TEST(e2.y,e2.x,abs(e2.y),abs(e2.x));
    #undef TEST

    vec3 mn = min(min(v0,v1),v2), mx = max(max(v0,v1),v2);
    if(mn.x >  bh.x || mx.x < -bh.x) return false;
    if(mn.y >  bh.y || mx.y < -bh.y) return false;
    if(mn.z >  bh.z || mx.z < -bh.z) return false;

    vec3 nrm = cross(e0,e1);
    float d = -dot(nrm,v0);
    r = bh.x*abs(nrm.x) + bh.y*abs(nrm.y) + bh.z*abs(nrm.z);
    if(abs(d) > r) return false;

    return true;
}

void main() {
    uint tid = gl_GlobalInvocationID.x;
    if(tid >= uNumTris) return;

    uvec3 tri = indices[tid];
    vec3 p0 = positions[tri.x].xyz;
    vec3 p1 = positions[tri.y].xyz;
    vec3 p2 = positions[tri.z].xyz;

    ivec3 mn = ivec3(floor(min(min(p0,p1),p2)));
    ivec3 mx = ivec3(ceil (max(max(p0,p1),p2)));
    mn = clamp(mn, ivec3(0), ivec3(uGridSize)-1);
    mx = clamp(mx, ivec3(0), ivec3(uGridSize)-1);

    vec3 bh = vec3(0.5);
    for(int z=mn.z; z<=mx.z; ++z)
    for(int y=mn.y; y<=mx.y; ++y)
    for(int x=mn.x; x<=mx.x; ++x){
        vec3 bc = vec3(x,y,z) + bh;
        if(triBoxOverlap(bc,bh,p0,p1,p2))
            imageStore(densityVolume, ivec3(x,y,z), vec4(1.0));
    }
}
