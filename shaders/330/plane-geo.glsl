#version 330 
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 myModelMatrix;        // Model transformation matrix
uniform mat4 myViewMatrix;         // View transformation matrix
uniform mat4 myPerspectiveMatrix;   // Projection matrix

out vec3 vNormal;
out vec3 vWorldPos;

void main() {
    // Compute two edges of the triangle
    vec3 edge1 = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 edge2 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;

    // Calculate normal for the triangle
    vec3 normal = normalize(cross(edge1, edge2));

    for (int i = 0; i < 3; ++i) {
        vNormal = normalize(mat3(transpose(inverse(myModelMatrix))) * normal);
        vWorldPos = (myModelMatrix * gl_in[i].gl_Position).xyz;
        gl_Position = myPerspectiveMatrix * myViewMatrix * myModelMatrix * gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}