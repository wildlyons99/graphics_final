#version 330 

uniform vec3 lightPos;
uniform vec3 cameraPosition;

in vec3 vNormal;
in vec3 vWorldPos;

out vec4 fragColor;

void main() {
    // diffuse lighting
    vec3 lightDir = normalize(lightPos - vWorldPos);
    float diff = max(dot(vNormal, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0);
    fragColor = vec4(diffuse, 1.0);
}
