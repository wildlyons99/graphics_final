#version 330
// Create Procedurally Generated Planets
// Based on the tutorial by Sebastian Lague
// https://www.youtube.com/watch?v=lctXaT9pxA0

layout(location = 0) in vec3 aPosition;  // Vertex position
layout(location = 1) in vec3 aNormal;    // Vertex normal
layout(location = 2) in vec2 aTexCoord;  // Texture coordinates

uniform mat4 myModelMatrix;        // Model transformation matrix
uniform mat4 myViewMatrix;         // View transformation matrix
uniform mat4 myPerspectiveMatrix;   // Projection matrix
uniform float myTime;               // Time

uniform sampler2D noiseTexture;
uniform sampler2D colorTexture;

uniform float oceanLevel;

out vec3 vWorldPos;
out vec3 vNormal;

float smoothMax(float a, float b, float k) {
    float h = clamp(0.5 + 0.5 * (b - a) / k, 0.0, 1.0);
    return mix(b, a, h) - k * h * (1.0 - h);
}
void main() {
    float terrainHeight = texture(noiseTexture, aTexCoord).r;
    float height = smoothMax(terrainHeight, 0.5, 0.1);
//    float height = terrainHeight;
    vec3 newPosition = aPosition + aNormal * 1 * height;
    
    vNormal = normalize(transpose(inverse(mat3(myModelMatrix))) * aNormal);
    vWorldPos = (myModelMatrix * vec4(aPosition, 1)).xyz;
    
    gl_Position = myPerspectiveMatrix * myViewMatrix * myModelMatrix * vec4(newPosition, 1);
    
}