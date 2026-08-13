#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 u_Model;

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

void main()
{
    TexCoords = aTexCoords;    
    gl_Position = u_ViewProjectionMatrix * u_Model * vec4(aPos, 1.0);
}
