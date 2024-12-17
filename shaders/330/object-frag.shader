#version 330

#define PI 3.141592653589793238462643383279

in vec3 positionVec;
in vec3 normalVec;
uniform vec3 lightPos;
uniform float textureBlend;

uniform vec3 cameraPos;
uniform sampler2D environMap;
uniform sampler2D objectTexture;
uniform int useDiffuse;

out vec4 outputColor;
in vec3 pos;


vec2 isectSphere(vec3 point) {
    vec2 coord;

    vec3 normalizedPoint = normalize(point);
    coord.x = atan(normalizedPoint.z, normalizedPoint.x) / (2.0 * PI) + 0.5;

    coord.y = acos(normalizedPoint.y) / PI;

    coord.x = clamp(coord.x, 0.0, 1.0);
    coord.y = clamp(coord.y, 0.0, 1.0);

    return coord;
}

vec4 textureColorAt(vec3 point, sampler2D textureMap) {
    float phi = atan(point.x, point.z);
    float theta = acos(point.y);

    float u = ((phi + PI) / (2.0 * PI));
    float v = 0.5 - (theta - PI / 2.0) / PI;

    vec2 uv = isectSphere(point);

    return texture(textureMap, uv);
}



void main() {

    vec3 I = normalize(positionVec- cameraPos );
    vec3 R = normalize(reflect(I, normalize(normalVec)));
    R.x *= -1;
    R.z *= -1;

    vec4 reflectionColor = textureColorAt(R, environMap);
    
    vec4 textureColor = textureColorAt(pos, objectTexture);

   float dotP = 1;
   if (useDiffuse == 1) {
        vec3 normalizedLight = normalize(lightPos - positionVec);
	    vec3 normalizedNormal = normalize(normalVec);

        dotP = max(dot(normalizedLight, normalizedNormal), 0);
   }
   outputColor = dotP * (((1 - textureBlend) * reflectionColor) + (textureBlend * textureColor));
}