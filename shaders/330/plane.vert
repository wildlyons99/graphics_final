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
    // Use the time as a bias to move around the noise texture
    
    vec2 uv = vec2((x + 1.5) / 3.0f + 0.1 * myTime, (z + 1.5) / 3.0f + 0.1 * myTime); 
    vec3 tex = texture(noiseTexture, uv).xyz;
    float height = tex.r;
    return  height;
}
void main() {
    // randomly move vertex up
    vec3 newPosition = aPosition;
    newPosition.y -= 1;
    newPosition.y += genHeight(aPosition.x, aPosition.z);
     gl_Position = vec4(newPosition, 1);
}