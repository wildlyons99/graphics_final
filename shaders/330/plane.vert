#version 330 

layout(location = 0) in vec3 aPosition;  // Vertex position
layout(location = 1) in vec3 aNormal;    // Vertex normal

uniform mat4 myModelMatrix;        // Model transformation matrix
uniform mat4 myViewMatrix;         // View transformation matrix
uniform mat4 myPerspectiveMatrix;   // Projection matrix

out vec3 vNormal;           // Pass normal to fragment shader
out vec3 vWorldPos;         // Pass world position to fragment shader

void main() {
    // randomly move vertex up
    vec3 newPosition = aPosition;
    newPosition.y -= 1;
    newPosition.y += 0.1 * sin(aPosition.x * 10.0) * sin(aPosition.z * 10.0);
    // Transform the vertex position to world space
    vec4 worldPos = myModelMatrix * vec4(newPosition, 1.0);
    vWorldPos = worldPos.xyz;

    // Transform the normal (ignore translation)
    vNormal = mat3(transpose(inverse(myModelMatrix))) * aNormal;

    // Transform to clip space
    gl_Position = myPerspectiveMatrix * myViewMatrix * worldPos;
}