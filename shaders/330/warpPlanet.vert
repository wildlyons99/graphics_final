#version 330

#define PI 3.1415926

layout(location = 0) in vec3 aPosition; // Vertex position
layout(location = 1) in vec3 aNormal;// Vertex normal

uniform mat4 myModelMatrix;
uniform mat4 myViewMatrix;
uniform mat4 myPerspectiveMatrix;

out vec3 positionVec;
out vec3 normalVec;

out vec3 pos;

void main() {
    vec4 worldPos = myModelMatrix * vec4(aPosition, 1.0);
    positionVec = worldPos.xyz;
    normalVec = mat3(transpose(inverse(myModelMatrix))) * aNormal;

    gl_Position = myPerspectiveMatrix * myViewMatrix * worldPos;
    
    pos = aPosition;
}