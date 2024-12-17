#version 330

#define PI 3.1415926
#define NUM_OCTAVES 8
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


uniform float oceanLevel;

out vec3 vWorldPos;
out vec3 vObjectPos;
out vec3 vNormal;
out vec2 vTexCoord;
out vec3 color;
out vec3 vTangent;
out vec3 vBitangent;

float hashFunction(vec2 p) {
    return fract(sin(dot(p, vec2(127.1,311.7))) * 43758.5453123);
}

// We'll create a 3D hash by extending the logic:
float hashFunction3D(vec3 cell) {
    // Fold the cell coordinates into a 2D coordinate for hashing
    // By choosing a large prime offset, we ensure a good distribution
    float zFactor = 157.0;
    return hashFunction(vec2(cell.x, cell.y + cell.z*zFactor));
}

float basicNoise3D(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);

    // Smooth interpolation curve
    f = f*f*(3.0 - 2.0*f);

    // 8 corners of the unit cube
    float n000 = hashFunction3D(i + vec3(0,0,0));
    float n100 = hashFunction3D(i + vec3(1,0,0));
    float n010 = hashFunction3D(i + vec3(0,1,0));
    float n110 = hashFunction3D(i + vec3(1,1,0));
    float n001 = hashFunction3D(i + vec3(0,0,1));
    float n101 = hashFunction3D(i + vec3(1,0,1));
    float n011 = hashFunction3D(i + vec3(0,1,1));
    float n111 = hashFunction3D(i + vec3(1,1,1));

    float nx00 = mix(n000, n100, f.x);
    float nx10 = mix(n010, n110, f.x);
    float nx01 = mix(n001, n101, f.x);
    float nx11 = mix(n011, n111, f.x);

    float nxy0 = mix(nx00, nx10, f.y);
    float nxy1 = mix(nx01, nx11, f.y);

    return mix(nxy0, nxy1, f.z);
}

float fbm3D(vec3 p, float H) {
    float G = exp2(-H);
    float frequency = 1.0;
    float amplitude = 1.0;
    float total = 0.0;
    for (int i = 0; i < NUM_OCTAVES; i++) {
        total += amplitude * basicNoise3D(p * frequency);
        frequency *= 2.0;
        amplitude *= G;
    }
    return total;
}

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

float genTerrain2(vec3 pos) {
    float height = fbm3D(pos, 0.5) / NUM_OCTAVES;
    if (height < 0.5) {
        return 0.5 * 5 - 5;
    }
    return height * 5 - 5;
}
void main() {
    // use the vertex position to generate a height value, normalize the noise between 0 and 1
    vTangent = aTangent;
    vBitangent = aBitangent;
    vObjectPos = aPosition;
    //    isWater = false;
    vTexCoord = aTexCoord;
    float terrainHeight = fbm3D(aPosition + vec3(1, 1, 1) * myTime, 0.5) / NUM_OCTAVES;
    float height = genTerrain2(aPosition);
    //    float height = smax(terrainHeight, 0.01, 0.95);
    vec3 newPosition = aPosition + aNormal * 1 * (height - 0.5);
    color = texture(colorTexture, vec2(max(terrainHeight - 0.17, 0.01), 0.5)).xyz;
    vec3 newNormal = normalize(transpose(inverse(mat3(myModelMatrix))) * aNormal);
//    if (terrainHeight < 0.45) {
//        //        isWater = true;
//        vec3 waterNormal = texture(waterNormals, aTexCoord * 100 + vec2(0, -1) * 0.05 * myTime).xyz * 2.0 - 1.0;
//        newNormal = 0.25 * newNormal + 0.75 * waterNormal;
//    }
    vNormal = newNormal;
    vWorldPos = (myModelMatrix * vec4(aPosition, 1)).xyz;

    gl_Position = myPerspectiveMatrix * myViewMatrix * myModelMatrix * vec4(newPosition, 1);
}