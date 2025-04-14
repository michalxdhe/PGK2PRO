#version 430 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 vLocalPos;

uniform sampler3D volumeTex;
uniform float intensity = 3.0;
uniform float stepSize = 0.01;
uniform int maxSteps = 256;

uniform vec3 cameraPos;
uniform mat4 invModel;

vec2 intersectBox(vec3 ro, vec3 rd, vec3 boxMin, vec3 boxMax) {
    vec3 invDir = 1.0 / rd;
    vec3 t0s = (boxMin - ro) * invDir;
    vec3 t1s = (boxMax - ro) * invDir;
    vec3 tmin = min(t0s, t1s);
    vec3 tmax = max(t0s, t1s);
    float t0 = max(max(tmin.x, tmin.y), tmin.z);
    float t1 = min(min(tmax.x, tmax.y), tmax.z);
    return vec2(t0, t1);
}

void main() {
    vec3 rayDir = normalize(FragPos - cameraPos);
    vec3 ro = vec3(invModel * vec4(cameraPos, 1.0));
    vec3 rd = normalize(vec3(invModel * vec4(rayDir, 0.0)));

    vec3 boxMin = vec3(-0.5);
    vec3 boxMax = vec3(0.5);

    vec2 bounds = intersectBox(ro, rd, boxMin, boxMax);
    if (bounds.x > bounds.y || bounds.y < 0.0) discard;

    float t = max(bounds.x, 0.0);
    float tEnd = bounds.y;

    vec4 accum = vec4(0.0);
    float alpha = 0.0;

    for (int i = 0; i < maxSteps && t < tEnd && alpha < 0.95; ++i) {
        vec3 pos = ro + rd * t;
        vec3 texCoord = pos + 0.5;

        vec4 voxel = texture(volumeTex, texCoord);
        vec3 color = voxel.rgb * intensity;
        float localAlpha = voxel.a;

        localAlpha *= (1.0 - accum.a);
        accum.rgb += color * localAlpha;
        accum.a += localAlpha;

        t += stepSize;
    }

    FragColor = accum;
    //FragColor = vec4(1.f);


    if (FragColor.a <= 0.01) discard;
}
