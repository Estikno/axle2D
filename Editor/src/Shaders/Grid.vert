#version 460 core

// Fullscreen triangle, generated entirely from gl_VertexID.
// No VBO/attributes required — bind an empty VAO and call:
//     glDrawArrays(GL_TRIANGLES, 0, 3);

layout (std140, binding = 0) uniform Scene {
    // Camera
    mat4 u_ViewMatrix;
    mat4 u_ProjectionMatrix;
    mat4 u_ViewProjectionMatrix;
    vec3 u_CameraPosition;

    vec2 u_ViewportSize;

    // Frame info
    double u_Time;
};

uniform mat4 u_Model;

void main()
{
    vec2 pos = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(pos * 2.0 - 1.0, 0.0, 1.0);
}
