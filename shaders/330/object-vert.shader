#version 330

#define PI 3.1415926

in vec3 myPosition;
in vec3 myNormal;

uniform mat4 myModelMatrix;
uniform mat4 myViewMatrix;
uniform mat4 myPerspectiveMatrix;

out vec3 positionVec;
out vec3 normalVec;

out vec2 sharedTexCoord;
out vec3 pos;

vec2 isectSphere(vec3 point) {
    vec2 coord;

    vec3 normalizedPoint = normalize(point);

    coord.x = atan(normalizedPoint.z, normalizedPoint.x) / (2.0 * PI) + 0.5;

    coord.y = acos(normalizedPoint.y) / PI;


    return coord;
}


void main() {
    vec4 worldPos = myModelMatrix * vec4(myPosition, 1.0);
    positionVec = worldPos.xyz;
    normalVec = mat3(transpose(inverse(myModelMatrix))) * myNormal;

    gl_Position = myPerspectiveMatrix * myViewMatrix * worldPos;
    sharedTexCoord = isectSphere(myPosition);
    pos = myPosition;
}