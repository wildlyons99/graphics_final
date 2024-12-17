#version 330
// Create Procedurally Generated Planets
// Based on the tutorial by Sebastian Lague
// https://www.youtube.com/watch?v=lctXaT9pxA0

layout(location = 0) in vec3 aPosition;  // Vertex position
layout(location = 1) in vec3 aNormal;    // Vertex normal
layout(location = 2) in vec2 aTexCoord;  // Texture coordinates
layout(location = 3) in vec3 aTangent;   // Tangent
layout(location = 4) in vec3 aBitangent; // Bitangent

uniform mat4 myModelMatrix;        // Model transformation matrix
uniform mat4 myViewMatrix;         // View transformation matrix
uniform mat4 myPerspectiveMatrix;   // Projection matrix
uniform float myTime;               // Time

uniform vec3 lightPos;
uniform vec3 cameraPos;

uniform sampler2D noiseTexture;
uniform sampler2D planeNoiseTexture;
uniform sampler2D colorTexture;
uniform sampler2D waterNormals;

uniform float oceanLevel;

out vec3 vWorldPos;
out vec3 vObjectPos;
out vec3 vNormal;
out vec2 vTexCoord;
out vec3 color;
out vec3 vTangent;
out vec3 vBitangent;
//out bool isWater;

float smax(float a, float b, float k)
{
    return log(exp(k*a)+exp(k*b))/k;
}
float genHeight(float x, float z) {
    // Generate a height value based on the x and z position from a noise texture
    // Use the time as a bias to move around the noise texture

    //    vec2 uv = vec2(x / 16.0 , z / 16.0 ); 
    vec2 uv = vec2(x / 16.0 + 0.1 * myTime, z / 16.0 + 0.1 * myTime);
    vec3 tex = texture(planeNoiseTexture, uv).xyz;
    float height = tex.r;

    return  height;
}


float genTerrain(float x, float z) {
    float height = genHeight(x, z);
    if (height < 0.5) {
        return 0.5 * 5 - 5;
    }
    return height * 5 - 5;
}


void main() {
    vTangent = aTangent;
    vBitangent = aBitangent;
    vObjectPos = aPosition;
//    isWater = false;
    vTexCoord = aTexCoord;
    float terrainHeight = texture(noiseTexture, aTexCoord + 0.1 * myTime).r;
//    float height = smax(terrainHeight, 0.01, 0.95);
    float height = max(terrainHeight, 0.5);
    vec3 newPosition = aPosition + aNormal * 1 * (height - 0.5);
    newPosition.y += genTerrain(aPosition.x, 0) + 2.5;
    color = texture(colorTexture, vec2(max(terrainHeight - 0.17, 0.01), 0.5)).xyz;
    vec3 newNormal = normalize(transpose(inverse(mat3(myModelMatrix))) * aNormal);
    if (terrainHeight < 0.45) {
//        isWater = true;
        vec3 waterNormal = texture(waterNormals, aTexCoord * 100 + vec2(0, -1) * 0.05 * myTime).xyz * 2.0 - 1.0;
        newNormal = 0.25 * newNormal + 0.75 * waterNormal;
    }
    vNormal = newNormal;
    vWorldPos = (myModelMatrix * vec4(aPosition, 1)).xyz;
    
    gl_Position = myPerspectiveMatrix * myViewMatrix * myModelMatrix * vec4(newPosition, 1);
    
}