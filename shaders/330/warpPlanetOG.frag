#version 330

#define PI 3.141592653589793238462643383279
#define NUM_OCTAVES 8

in vec3 positionVec;
in vec3 normalVec;

uniform vec3 cameraPos;

uniform float myTime; 

uniform vec3 lightPos;

// uniform float textureBlend; // think can delete
// uniform sampler2D environMap;
// uniform sampler2D objectTexture;
// uniform int useDiffuse;

out vec4 outputColor;
in vec3 pos;


float hashFunction(vec2 p) {
    return fract(sin(dot(p, vec2(127.1,311.7))) * 43758.5453123);
}

float basicNoise(vec2 uv) {
    vec2 cell = floor(uv);
    vec2 fractPart = fract(uv);
    fractPart = fractPart*fractPart*(3.0 - 2.0*fractPart);

    float a = hashFunction(cell + vec2(0,0));
    float b = hashFunction(cell + vec2(1,0));
    float c = hashFunction(cell + vec2(0,1));
    float d = hashFunction(cell + vec2(1,1));

    return mix(mix(a, b, fractPart.x), mix(c, d, fractPart.x), fractPart.y);
}

// float hashFunctionPeriodic(vec2 p, float period) {
//     // Adjust constants to enforce periodicity
//     vec2 k = (2.0 * 3.14159265 / period) * vec2(127.1, 311.7);
//     float value = sin(dot(p, k)) * 43758.5453123;
//     return fract(value);
// }

// float basicNoise(vec2 uv) {
//     float period = 10.0f; 
//     vec2 cell = floor(uv);
//     vec2 fractPart = fract(uv);
//     fractPart = fractPart * fractPart * (3.0 - 2.0 * fractPart); // Smooth interpolation

//     // Wrap cell coordinates to enforce periodicity
//     cell = mod(cell, period);

//     // Use the periodic hash function
//     float a = hashFunctionPeriodic(cell + vec2(0.0, 0.0), period);
//     float b = hashFunctionPeriodic(cell + vec2(1.0, 0.0), period);
//     float c = hashFunctionPeriodic(cell + vec2(0.0, 1.0), period);
//     float d = hashFunctionPeriodic(cell + vec2(1.0, 1.0), period);

//     // Bilinear interpolation
//     return mix(mix(a, b, fractPart.x), mix(c, d, fractPart.x), fractPart.y);
// }


////////////////////////////////////
// Fractional Brownian Motion (FBM)
////////////////////////////////////
float fbm(vec2 uv, float H) {
    float G = exp2(-H);
    float frequency = 1.0;
    float amplitude = 1.0;
    float total = 0.0;
    for (int i = 0; i < NUM_OCTAVES; i++) {
        total += amplitude * basicNoise(frequency * uv);
        frequency *= 2.0;
        amplitude *= G;
    }
    return total;
}

////////////////////////////////////
// Domain Warping Matrices & Functions
// Found in someone else project but they work great
////////////////////////////////////
const mat2 warpMatrix = mat2(0.80, 0.60,
                            -0.60, 0.80);

float fbm4(vec2 p) {
    float f = 0.0;
    f += 0.5 * (-1.0 + 2.0*basicNoise(p)); p = warpMatrix*p*2.02;
    f += 0.25 * (-1.0 + 2.0*basicNoise(p)); p = warpMatrix*p*2.03;
    f += 0.125* (-1.0 + 2.0*basicNoise(p)); p = warpMatrix*p*2.01;
    f += 0.0625*(-1.0 + 2.0*basicNoise(p));
    return f / 0.9375;
}

float fbm6(vec2 p) {
    float f = 0.0;
    f += 0.5    * basicNoise(p); p = warpMatrix*p*2.02;
    f += 0.25   * basicNoise(p); p = warpMatrix*p*2.03;
    f += 0.125  * basicNoise(p); p = warpMatrix*p*2.01;
    f += 0.0625 * basicNoise(p); p = warpMatrix*p*2.04;
    f += 0.03125* basicNoise(p); p = warpMatrix*p*2.01;
    f += 0.015625*basicNoise(p);
    return f / 0.96875;
}

vec2 fbm4_2(vec2 p) {
    return vec2(fbm4(p + vec2(1.0)), fbm4(p + vec2(6.2)));
}

vec2 fbm6_2(vec2 p) {
    return vec2(fbm6(p + vec2(9.2)), fbm6(p + vec2(5.7)));
}


////////////////////////////////////
// Main Pattern Computation
////////////////////////////////////
// Computes the main scalar pattern and associated vector fields
// Input: uvCoords - 2D UV coordinates
// Output:
//    patternValue   - main scalar pattern intensity
//    octaveField    - intermediate field from fbm4_2
//    noiseVector    - vector from fbm6_2 used for coloring and shaping
float computePatternFields(vec2 uvCoords, out vec2 octaveField, out vec2 noiseVector) {
    // Apply time and distance-based distortion
    uvCoords += 0.05 * sin(vec2(0.11,0.13)*myTime + length(uvCoords)*4.0);

    uvCoords *= 0.7 + 0.2*cos(0.05*myTime);

    octaveField = 0.5 + 0.5*fbm4_2(uvCoords);
    octaveField += 0.02*sin(vec2(0.13,0.11)*myTime * length(octaveField));

    noiseVector = fbm6_2(4.0*octaveField + 0.1);

    vec2 p = uvCoords + 2.0*noiseVector + 1.0;
    float patternValue = 0.5 + 0.5*fbm4(2.0*p);

    // Enhance and shape the pattern
    patternValue = mix(patternValue, patternValue*patternValue*patternValue*3.5, patternValue*abs(noiseVector.x));
    patternValue *= 1.0 - 0.5*pow(0.5+0.5*sin(8.0*p.x)*sin(8.0*p.y), 8.0);

    return patternValue;
}

// A simpler approach with domain warping
vec2 applyWarp(vec2 uv, float H) {
    // First warp: subtle
    vec2 warpUV = uv;
    warpUV += fbm(warpUV * 2.0, H) * 0.5; 
    
    float angle = 32; 
    mat2 rot = mat2(cos(angle), -sin(angle), sin(angle),  cos(angle));
    warpUV *= fbm(rot * uv * 5, H);

    warpUV += fbm(warpUV * 4.0, H) * 0.25;
    
    mat2 rotation = mat2(0.8, -0.6, 0.6, 0.8);
    warpUV += fbm(rotation * uv * 4.0, H) * 0.2;
    
    return warpUV; 
}


////////////////////////////////////
// Convert 3D position on sphere to UV
////////////////////////////////////
vec2 getUV(vec3 point) {
    vec2 uv;

    vec3 normalizedPoint = normalize(point);
    uv.x = atan(normalizedPoint.z, normalizedPoint.x) / (2.0 * PI) + 0.5;

    uv.y = acos(normalizedPoint.y) / PI;

    uv.x = clamp(uv.x, 0.0, 1.0);
    uv.y = clamp(uv.y, 0.0, 1.0);

    return uv;
}

void main() {
    // Convert sphere position to UV
    vec2 uv = getUV(pos);

    vec2 warpedUV = applyWarp(uv, 0.6);
    float patternValue = fbm(warpedUV, 0.6);

    // Get the domain-warped values 
    // Compute main pattern and vector fields
    // vec2 octaveField = 0.5 + 0.5*fbm4_2(warpedUV);
    // vec2 octaveField = 0.5 + 0.5*fbm(warpedUV, 0.6);
    // vec2 octaveField = vec2(fbm(warpedUV.x + vec2(9.2), 0.5), fbm(warpedUV + vec2(5.7), 0.5)); 
    // octaveField += 0.02*sin(vec2(0.13,0.11) * myTime * length(octaveField)); 
    
    
    vec2 noiseVector, octaveField;
    computePatternFields(warpedUV, octaveField, noiseVector);

    //////////////////////////////////////
    // Creates complex color from multiple fbm 
    //////////////////////////////////////
    // set initial reddish purple base color
    vec3 color = vec3(0.7, 0.2, 0.4);
    // mixes with redish color for higher pattern values
    color = mix(color, vec3(0.3,0.05,0.05), patternValue);
    // for stronger noise introduce more white
    color = mix(color, vec3(0.9,0.9,0.9), dot(noiseVector, noiseVector));
    // another random noise driven value w quadratic rel to color
    color = mix(color, vec3(0.5,0.2,0.2), 0.5*octaveField.y*octaveField.y); // muted reddish-brown
    // add some of aditional red smoothly in some random noise areas
    color = mix(color, vec3(0.7,0.2,0.4), 0.5*smoothstep(1.2,1.3,abs(noiseVector.y)+abs(noiseVector.x)));
    // scales patternValue
    color *= patternValue*2.0;

    //////////////////////////////////////
    // Applies lighting effect
    //////////////////////////////////////
    vec3 lightDir = normalize(lightPos - positionVec);
	vec3 nNormal = normalize(normalVec);

    float diffuseTerm = clamp(0.3 + 0.7*dot(nNormal, lightDir), 0.0, 1.0);
    vec3 lightingContribution = vec3(0.85,0.90,0.95)*(nNormal.y*0.5+0.5);
    lightingContribution += vec3(0.15,0.10,0.05)*diffuseTerm;
    color *= lightingContribution;

    //////////////////////////////////////
    // More post processing for effect
    //////////////////////////////////////
    color = vec3(1.0) - color;     // invert to make it look cool
    color = color*color;           // get more contrast
    color *= vec3(1.25,1.25,1.25); // make slightly brighter

    outputColor = vec4(color, 1.0);
}