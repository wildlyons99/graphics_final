#version 330

#define PI 3.1415926
#define NUM_OCTAVES 8
layout(location = 0) in vec3 aPosition; // Vertex position
layout(location = 1) in vec3 aNormal;// Vertex normal

uniform mat4 myModelMatrix;
uniform mat4 myViewMatrix;
uniform mat4 myPerspectiveMatrix;

uniform float myTime;

out vec3 positionVec;
out vec3 normalVec;

out vec3 pos;

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


void main() {
    // use the vertex position to generate a height value, normalize the noise between 0 and 1
    float height = fbm3D(aPosition + vec3(1, 1, 1) * myTime, 0.5) / NUM_OCTAVES;
//    // move the vertex out based on the height value
    vec3 newPosition = aPosition + aNormal * 5 * height;
    vec4 worldPos = myModelMatrix * vec4(newPosition, 1.0);
    positionVec = worldPos.xyz;
    normalVec = mat3(transpose(inverse(myModelMatrix))) * aNormal;

    gl_Position = myPerspectiveMatrix * myViewMatrix * worldPos;
    
    pos = aPosition;
}