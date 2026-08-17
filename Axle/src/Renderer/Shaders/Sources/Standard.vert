#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_UV;
layout(location = 3) in vec4 a_Tangent;

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
uniform mat4 u_NormalMatrix; // transpose(inverse(mat3(uModel))), computed CPU-side

out VS_OUT {
    vec3 worldPos;
    vec2 uv;
    mat3 TBN;
} vs_out;

void main(){
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    vs_out.worldPos = worldPos.xyz;
    vs_out.uv = a_UV;

    vec3 N = normalize(u_NormalMatrix * a_Normal);
    vec3 T = normalize(u_NormalMatrix * a_Tangent.xyz);
    T = normalize(T - N * dot(N, T)); // re-orthogonalize (Gram-Schmidt)
    vec3 B = cross(N, T) * a_Tangent.w;

    vs_out.TBN = mat3(T, B, N);

    gl_Position = u_ViewProjectionMatrix * worldPos;
}
