#version 330
#define PI 3.141592653589793238462643383279

in vec3 positionVec;
in vec3 normalVec;

uniform vec3 cameraPos;
uniform sampler2D environMap;

out vec4 outputColor;

void main() {
    
    vec3 R = normalize(cameraPos - positionVec);

    float phi = atan(R.z, R.x);
    float theta = asin(R.y);

    float u = (phi + PI) / (2.0 * PI);
    float v = (theta + PI / 2.0) / PI;

    vec2 uv = vec2(u, v);
    outputColor = texture(environMap, uv);

    // idk make a purple sky - kinda works not really
    // vec3 blue = vec3(0.0, 0.0, 1.0);     
    // vec3 purple = vec3(0.5, 0.0, 0.5);   
    // vec3 color = mix(blue, purple, u);

    // outputColor = vec4(color, 1.0);
}