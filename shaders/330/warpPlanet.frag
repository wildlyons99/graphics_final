#version 330

#define PI 3.141592653589793238462643383279
#define NUM_OCTAVES 8

in vec3 positionVec;  // Incoming position from vertex shader
in vec3 normalVec;    // Incoming normal from vertex shader

uniform float myTime; 
uniform vec3 lightPos;

uniform float scaler; 

out vec4 outputColor;
in vec3 pos; 

////////////////////////////////////
// Hash and 3D Noise Functions
////////////////////////////////////

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

////////////////////////////////////
// 3D Domain Warping Functions
////////////////////////////////////

// A simple rotation matrix in 3D can be more complex, but let's do a few manipulations:
mat3 rotationMatrix3D(float angle) {
    float c = cos(angle);
    float s = sin(angle);
    // Rotate around Y axis as an example
    return mat3(c, 0, s,
                0, 1, 0,
               -s, 0, c);
}

vec3 domainWarp3D(vec3 p, float H) {
    // Example: apply one FBM call to get a vector to warp with
    // We'll create a warp vector from 3 separate fbm calls offsetting p
    float w1 = fbm3D(p * 2.0, H);
    float w2 = fbm3D(p * 2.0 + vec3(5.2,1.3,2.7), H);
    float w3 = fbm3D(p * 4.0 + vec3(1.7,9.2,3.5), H);

    vec3 warpVec = vec3(w1, w2, w3) * 0.5; // Scale the warp strength

    // Apply a rotation
    mat3 rot = rotationMatrix3D(myTime*0.1);
    warpVec = rot * warpVec;

    // Add the warp vector to the original position
    return p + warpVec;
}

////////////////////////////////////
// Main Pattern Computation in 3D
////////////////////////////////////

float compute3DPatternFields(vec3 p, out vec2 octaveField, out vec2 noiseVector) {
    // Apply a slight time-based distortion
    p += 0.05 * sin(vec3(0.11,0.13,0.07)*myTime + vec3(length(p.xy)*4.0, length(p.yz)*4.0, length(p.zx)*4.0));

    // Another subtle scale modulation
    p *= 0.7 + 0.2*cos(0.05*myTime);

    // Let's derive a 2D field from some slices of 3D fbm for octaveField:
    float ofx = fbm3D(p + scaler*vec3(1.0,0.8,0.0), 0.6);
    float ofy = fbm3D(p + scaler*vec3(6.2,0.0,9.0), 0.6);
    octaveField = 0.5 + 0.5*vec2(ofx, ofy);

    octaveField += 0.02*sin(vec2(0.13,0.11)*myTime * length(octaveField));

    // noiseVector can also come from fbm of a transformed coordinate:
    float nx = fbm3D(p*4.0 + vec3(9.2,0.0,0.0), 0.6);
    float ny = fbm3D(p*4.0 + vec3(5.7,0.0,0.0), 0.6);
    noiseVector = vec2(nx, ny);

    vec3 pp = p + vec3(2.0*noiseVector.x + 1.0, 2.0*noiseVector.y + 1.0, 1.0);
    float patternValue = 0.5 + 0.5*fbm3D(pp*2.0, 0.6);

    // Enhance and shape the pattern as before
    patternValue = mix(patternValue, patternValue*patternValue*patternValue*3.5, patternValue*abs(noiseVector.x));
    patternValue *= 1.0 - 0.5*pow(0.5+0.5*sin(8.0*pp.x)*sin(8.0*pp.y), 8.0);

    return patternValue;
}

void main() {
    vec3 p = normalize(pos);

    // Warp the domain 
    vec3 warpedPos = domainWarp3D(p / 15, 0.6);

    vec2 octaveField, noiseVector;
    float patternValue = compute3DPatternFields(warpedPos, octaveField, noiseVector);

    // Create complex color from multiple fbm-related values
    vec3 color = scaler * vec3(0.7, 0.2, 0.4);
    color = mix(color, vec3(0.7,0.2,0.2), patternValue);
    color = 0.5*mix(color, vec3(scaler*0.9,0.9,0.9), dot(noiseVector, noiseVector));
    color = mix(color, vec3(scaler*0.5,0.2,0.2), 0.5*octaveField.y*octaveField.y);
    color = mix(color, vec3(0.4,0.2,0.4), 0.5*smoothstep(1.2,1.3,abs(noiseVector.y)+abs(noiseVector.x)));
    color *= patternValue*0.6;

    // Lighting
    vec3 lightDir = normalize(lightPos - positionVec);
    vec3 nNormal = normalize(normalVec);
    float diffuseTerm = clamp(0.3 + 0.7*dot(nNormal, lightDir), 0.0, 1.0);
    vec3 lightingContribution = vec3(0.85,0.90,0.95)*(nNormal.y*0.5+0.5);
    lightingContribution += vec3(0.15,0.10,0.05)*diffuseTerm;
    color *= lightingContribution;

    // Post-processing
    color = vec3(1.0) - (color*scaler);
    color = color*color;

    outputColor = vec4(color, 1.0);
}

