#version 330 

in vec3 myPosition;
in vec3 myNormal;

uniform mat4 myModelMatrix;        // Model transformation matrix
uniform mat4 myViewMatrix;         // View transformation matrix
uniform mat4 myPerspectiveMatrix;   // Projection matrix
uniform float myTime;               // Time

out vec3 vNormal;
out vec3 vWorldPos;

uniform sampler2D noiseTexture;
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
    float height = genTerrain(myPosition.x, 0);
    vWorldPos = (myModelMatrix * vec4(myPosition, 1.0)).xyz;
    vNormal = mat3(transpose(inverse(myModelMatrix))) * myNormal;
    mat4 scaleDouble = mat4(2.0);
    scaleDouble[3][3] = 1.0;
    
    
    gl_Position = myPerspectiveMatrix * myViewMatrix * myModelMatrix * scaleDouble * vec4(myPosition + vec3(0,height + 1.5,0), 1.0);
}