#version 430 core
in  vec3 FragPos;
out vec4 FragColor;

uniform sampler3D volumeTex;
uniform vec3 cameraPos;
uniform mat4 invModel;

uniform float stepSize = 0.01;
uniform int maxSteps = 526;

// 0=density, 1=velocity, 2=divergence, 3=pressure
uniform int visualizeMode;

vec2 intersectBox(vec3 ro, vec3 rd, vec3 mn, vec3 mx) {
    vec3 t0s = (mn - ro)/rd;
    vec3 t1s = (mx - ro)/rd;
    vec3 tmin = min(t0s, t1s);
    vec3 tmax = max(t0s, t1s);
    float t0 = max(max(tmin.x, tmin.y), tmin.z);
    float t1 = min(min(tmax.x, tmax.y), tmax.z);
    return vec2(t0, t1);
}

vec4 sampleField(int mode, vec3 uvw) {
    vec4 raw = texture(volumeTex, uvw);

    if(mode == 0) {
        //float d = raw.r;
        //return vec4(d);
        return raw;
    }
    else if(mode == 1) {
        float speed = length(raw.rgb);
        speed = clamp(speed * 2.0, 0.0, 1.0);
        return vec4(speed);
    }
    else if(mode == 2) {
        float div = raw.r;
        float mag = abs(div);
        vec3 col = mix(vec3(0.0,0.0,1.0), vec3(1.0,0.0,0.0), div*0.5+0.5);
        return vec4(vec3(col * mag), mag);
    }
    else{
        float p = raw.r;
        p = clamp(p*0.5 + 0.5, 0.0, 1.0);
        return vec4(p);
    }
}

void main() {
    vec3 ro = (invModel * vec4(cameraPos,1)).xyz;
    vec3 rd = normalize((invModel * vec4(FragPos,1)).xyz - ro);

    vec2 b = intersectBox(ro, rd, vec3(-0.5), vec3(0.5));
    if(b.x > b.y) discard;

    float t    = max(b.x, 0.0);
    float tEnd = b.y;

    vec4 accum = vec4(0.0);
    for(int i = 0; i < maxSteps && t < tEnd && accum.a < 0.80; ++i)
    {
        vec3 pos      = ro + rd * t;
        vec3 texCoord = pos + 0.5;

        vec4 samp = sampleField(visualizeMode, texCoord);

        float a = samp.a * (1.0 - accum.a);
        accum.rgb   += samp.rgb * a;
        accum.a     += a;

        float localStep = mix(stepSize, stepSize*4.0, accum.a);

        t += localStep;
    }

    if(accum.a < 0.01) discard;
    FragColor = accum;
}
