#version 330

uniform vec3 lightPos;
uniform vec3 cameraPos;

in vec3 vNormal;
in vec3 vWorldPos;

out vec4 fragColor;

float mod289(float x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 mod289(vec4 x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 perm(vec4 x){return mod289(((x * 34.0) + 1.0) * x);}

float noise(vec3 p){
    vec3 a = floor(p);
    vec3 d = p - a;
    d = d * d * (3.0 - 2.0 * d);

    vec4 b = a.xxyy + vec4(0.0, 1.0, 0.0, 1.0);
    vec4 k1 = perm(b.xyxy);
    vec4 k2 = perm(k1.xyxy + b.zzww);

    vec4 c = k2 + a.zzzz;
    vec4 k3 = perm(c);
    vec4 k4 = perm(c + 1.0);

    vec4 o1 = fract(k3 * (1.0 / 41.0));
    vec4 o2 = fract(k4 * (1.0 / 41.0));

    vec4 o3 = o2 * d.z + o1 * (1.0 - d.z);
    vec2 o4 = o3.yw * d.x + o3.xz * (1.0 - d.x);

    return o4.y * d.y + o4.x * (1.0 - d.y);
}


float fbm(vec3 x) {
    float v = 0.0;
    float a = 0.5;
    vec3 shift = vec3(100);
    for (int i = 0; i < 5; ++i) {
        v += a * noise(x);
        x = x * 2.0 + shift;
        a *= 0.5;
    }
    return v;
}

void main() {
    // diffuse and specular lighting
    vec3 fragNormal = vNormal;
    vec3 lightDir = normalize(lightPos - vWorldPos);
    float diff = max(dot(fragNormal, lightDir), 0.0);
    vec3 V = normalize(cameraPos - vWorldPos);
    vec3 R = normalize(reflect(-lightDir, fragNormal));
    float spec = pow(max(dot(V, R), 0.0), 32);
    fragColor = vec4((0.1 + diff) * vec3(fbm(fbm(vWorldPos) + vWorldPos)) + 0.5 * spec, 1.0);
}