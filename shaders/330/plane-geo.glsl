#version 330 
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 myModelMatrix;        // Model transformation matrix
uniform mat4 myViewMatrix;         // View transformation matrix
uniform mat4 myPerspectiveMatrix;   // Projection matrix
in vec3 color[];
in vec3 myNormal[];
in vec2 myTexCoord[];
in vec3 myObjectPos[];

out vec3 vNormal;
out vec3 vWorldPos;
out vec3 vColor;
out vec2 vTexCoord;
out vec3 vObjectPos;
void main() {
    // Compute two edges of the triangle
    vec3 edge1 = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 edge2 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;

    // Calculate normal for the triangle
    vec3 normal = normalize(cross(edge1, edge2));

    for (int i = 0; i < 3; ++i) {
        vNormal = normalize(mat3(transpose(inverse(myModelMatrix))) * normal);
//        vNormal = myNormal[i];
        vWorldPos = (myModelMatrix * gl_in[i].gl_Position).xyz;
        gl_Position = myPerspectiveMatrix * myViewMatrix * myModelMatrix * gl_in[i].gl_Position;
        vColor = color[i];
        vTexCoord = myTexCoord[i];
        vObjectPos = myObjectPos[i];
        EmitVertex();
    }
    EndPrimitive();
}