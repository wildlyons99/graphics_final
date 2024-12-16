#version 330 

layout(location = 0) in vec3 aPosition;  // Vertex position
layout(location = 1) in vec3 aNormal;    // Vertex normal
layout(location = 2) in vec2 aTexCoord;  // Texture coordinates

uniform mat4 myModelMatrix;        // Model transformation matrix
uniform mat4 myViewMatrix;         // View transformation matrix
uniform mat4 myPerspectiveMatrix;   // Projection matrix
uniform float myTime;               // Time

uniform sampler2D noiseTexture;
uniform sampler2D colorTexture;

out vec3 color;
out vec3 myNormal;


float genHeight(float x, float z) {
    // Generate a height value based on the x and z position from a noise texture
    // Use the time as a bias to move around the noise texture
    
    vec2 uv = vec2(aTexCoord.x + 0.1 * myTime, aTexCoord.y + 0.1 * myTime); 
    vec3 tex = texture(noiseTexture, uv).xyz;
    float height = tex.r;
    
    return  height;
}
void main() {
    // randomly move vertex up
    vec3 newPosition = aPosition;
    
    float height = genHeight(aPosition.x, aPosition.z);
    float scale = 5;
    if (height < 0.5) {
        newPosition.y += scale * 0.5;
    }
    else {
        newPosition.y += scale * height;
    }
    
//    if (newPosition.y < 1.25) {
//        newPosition.y = 1.25;
//    }
    newPosition.y -= 4.0;
    color = texture(colorTexture, vec2(height - 0.17, 0.5)).xyz;
    float epsilon = 0.01;
    float heightX1 = genHeight(aPosition.x + epsilon, aPosition.z);
    float heightX2 = genHeight(aPosition.x - epsilon, aPosition.z);
    float heightZ1 = genHeight(aPosition.x, aPosition.z + epsilon);
    float heightZ2 = genHeight(aPosition.x, aPosition.z - epsilon);
    vec3 tangentX = vec3(2 * epsilon, heightX1 - heightX2, 0);
    vec3 tangentZ = vec3(0, heightZ1 - heightZ2, 2 * epsilon);
    myNormal = normalize(mat3(transpose(inverse(myModelMatrix))) * -normalize(cross(tangentX, tangentZ)));
//    myNormal = normalize(mat3(transpose(inverse(myModelMatrix))) * aNormal);
    //    myNormal =  normalize(mat3(transpose(inverse(myModelMatrix))) * normalize(vec3(-dx, 2.0, -dz)));
    gl_Position = vec4(newPosition, 1);
}