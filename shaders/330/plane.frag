#version 330 

uniform vec3 lightPos;
uniform vec3 cameraPosition;
uniform float myTime;
uniform mat4 myModelMatrix;

uniform sampler2D noiseTexture;

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vWorldPos;
in vec3 vColor;
in vec3 vObjectPos;

out vec4 fragColor;

float genHeight(float x, float z) {
    // Generate a height value based on the x and z position from a noise texture
    // Use the time as a bias to move around the noise texture

    vec2 uv = vec2(x / 16.0 , z / 16.0 );
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
//    vec3 pos = 
    vec3 fragNormal = vNormal;
    float height = genTerrain(vObjectPos.x, vObjectPos.z);

    float epsilon = 0.1;
    vec3 xTangent = vec3(epsilon, genTerrain(vObjectPos.x + epsilon, vObjectPos.z) - height, 0.0);
    vec3 zTangent = vec3(0.0, genTerrain(vObjectPos.x, vObjectPos.z + epsilon) - height, epsilon);
//    fragNormal = normalize(mat3(transpose(inverse(myModelMatrix))) * normalize(cross(zTangent, xTangent)));
    
    
    // calculate normal by finding nearby points' height
    
    // diffuse lighting
    vec3 lightDir = normalize(lightPos - vWorldPos);
    float diff = max(dot(fragNormal, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0);
    fragColor = vec4(diff * vColor, 1.0);
}