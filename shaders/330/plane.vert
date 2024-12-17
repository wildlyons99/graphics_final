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
out vec2 myTexCoord;
out vec3 myObjectPos;


float genHeight(float x, float z) {
    // Generate a height value based on the x and z position from a noise texture
    // Use the time as a bias to move around the noise texture
    
//    vec2 uv = vec2(x / 16.0 , z / 16.0 ); 
    vec2 uv = vec2(x / 16.0 + 0.1 * myTime, z / 16.0 + 0.1 * myTime); 
    vec3 tex = texture(noiseTexture, uv).xyz;
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
    myTexCoord = aTexCoord;
    myObjectPos = aPosition;
    // randomly move vertex up
    vec3 newPosition = aPosition;
    
    float height = genHeight(aPosition.x, aPosition.z);
    newPosition.y = genTerrain(aPosition.x, aPosition.z);
    
    color = texture(colorTexture, vec2(height - 0.17, 0.5)).xyz;
    float epsilon = 0.1;
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