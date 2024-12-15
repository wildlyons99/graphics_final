#version 330 

layout(location = 0) in vec3 aPosition;  // Vertex position
layout(location = 1) in vec3 aNormal;    // Vertex normal

uniform mat4 myModelMatrix;        // Model transformation matrix
uniform mat4 myViewMatrix;         // View transformation matrix
uniform mat4 myPerspectiveMatrix;   // Projection matrix
uniform float myTime;               // Time

uniform sampler2D noiseTexture;


float genHeight(float x, float z) {
    // Generate a height value based on the x and z position from a noise texture
    vec2 uv = vec2(x , z);
    vec3 tex = texture(noiseTexture, uv).xyz;
    float height = tex.r;
    return 0.1 * height;
}
void main() {
    // randomly move vertex up
    vec3 newPosition = aPosition;
    newPosition.y -= 0.5;
    newPosition.y += genHeight(aPosition.x, aPosition.z);
    // calculate the new normal by generating close points and taking the cross product
    float epsilon = 0.01;
    vec3 dx = vec3(epsilon, 0, 0);
    vec3 dz = vec3(0, 0, epsilon);
    float heightX1 = genHeight(aPosition.x + epsilon, aPosition.z);
    float heightX2 = genHeight(aPosition.x - epsilon, aPosition.z);
    float heightZ1 = genHeight(aPosition.x, aPosition.z + epsilon);
    float heightZ2 = genHeight(aPosition.x, aPosition.z - epsilon);
    vec3 tangentX = vec3(2 * epsilon, heightX1 - heightX2, 0);
    vec3 tangentZ = vec3(0, heightZ1 - heightZ2, 2 * epsilon);
    vec3 newNormal = -normalize(cross(tangentX, tangentZ));
    
    // Transform the vertex position to world space
    vec4 worldPos = myModelMatrix * vec4(newPosition, 1.0);
//    vWorldPos = worldPos.xyz;

    // Transform the normal (ignore translation)
//    vNormal = normalize(mat3(transpose(inverse(myModelMatrix))) * newNormal);

    // Transform to clip space
//    gl_Position = myPerspectiveMatrix * myViewMatrix * worldPos;
     gl_Position = vec4(newPosition, 1);
}