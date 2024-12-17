#version 330 

uniform vec3 lightPos;
uniform vec3 cameraPos;

uniform sampler2D noiseTexture;

uniform float myTime;


in vec3 vNormal;
in vec3 vWorldPos;
in vec3 vObjectPos;
in vec3 color;
in vec2 vTexCoord;
in vec3 vTangent;
in vec3 vBitangent;
out vec4 fragColor;


void main() {
    vec3 fragNormal = vNormal;
//    if (!isWater) {
//        // calculate normal by finding nearby points' height
//        vec2 texCoord = vTexCoord.xy;
//    }
    // calculate normal by finding nearby points' height
    float height = texture(noiseTexture, vTexCoord.xy + 0.1 * myTime).r;
    vec3 dx = vTangent * 0.01 + vObjectPos;
    vec2 uvdx = vec2(0.5 + atan(dx.z, dx.x) / (2 * 3.14159265359), 0.5 - asin(dx.y) / 3.14159265359);
    vec3 dy = vBitangent * 0.01 + vObjectPos;
    vec2 uvdy = vec2(0.5 + atan(dy.z, dy.x) / (2 * 3.14159265359), 0.5 - asin(dy.y) / 3.14159265359);
    float h_dx = texture(noiseTexture, uvdx + 0.1 * myTime).r;
    float h_dy = texture(noiseTexture, uvdy + 0.1 * myTime).r;
    vec3 pos = vObjectPos * (1 + height);
    vec3 pos_dx = (dx) * (1 + h_dx);
    vec3 pos_dy = (dy) * (1 + h_dy);
//    fragNormal = normalize(cross(pos_dx - pos, pos_dy - pos));
    
    // diffuse lighting
    vec3 lightDir = normalize(lightPos - vWorldPos);
    float diff = max(dot(fragNormal, lightDir), 0.0);
    // specular lighting
    vec3 V = normalize(cameraPos - vWorldPos);
    vec3 R = normalize(reflect(-lightDir, fragNormal));
    float spec = pow(max(dot(V, R), 0.0), 32);
    
    fragColor = vec4((0.1 + diff) * color + 0.5 * spec, 1.0);
}