#version 460 core

layout(location = 0) out vec4 FragColor;

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

layout (std140, binding = 1) uniform Material {
    vec3  u_BaseColorFactor; 

    float u_MetallicFactor; 
    float u_RoughnessFactor; 
    float u_Reflectance; 

    vec3  u_EmissiveFactor; 
};

in VS_OUT {
    vec3 worldPos;
    vec2 uv;
    mat3 TBN;
} fs_in;

uniform mat4 u_NormalMatrix; // transpose(inverse(mat3(uModel))), computed CPU-side

// Temporal light data
uniform vec3  u_LightPos;
uniform vec3  u_LightColor;
uniform float u_LightIntensity;

layout(binding = 0) uniform sampler2D t_BaseColor;
layout(binding = 1) uniform sampler2D t_Normal;
layout(binding = 2) uniform sampler2D t_MetallicRoughness
layout(binding = 3) uniform sampler2D t_AO;
layout(binding = 4) uniform sampler2D t_Emissive;
layout(binding = 5) uniform sampler2D t_Height;
layout(binding = 6) uniform sampler2D t_Opacity;

const float PI = 3.14159265359;

float D_GGX(float NoH, float a) {
    float a2 = a * a;
    float f = (NoH * a2 - NoH) * NoH + 1.0;
    return a2 / (PI * f * f);
}

vec3 F_Schlick(float u, vec3 f0) {
    return f0 + (vec3(1.0) - f0) * pow(1.0 - u, 5.0);
}

float V_SmithGGXCorrelated(float NoV, float NoL, float a) {
    float a2 = a * a;
    float GGXL = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
    float GGXV = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
    return 0.5 / max(GGXV + GGXL, 1e-5);
}

float Fd_Lambert() {
    return 1.0 / PI;
}

vec3 BRDF(vec3 n, vec3 v, vec3 l, vec3 diffuseColor, vec3 f0, float roughness) {
    vec3 h = normalize(v + l);

    float NoV = abs(dot(n, v)) + 1e-5;
    float NoL = clamp(dot(n, l), 0.0, 1.0);
    float NoH = clamp(dot(n, h), 0.0, 1.0);
    float LoH = clamp(dot(l, h), 0.0, 1.0);

    float D = D_GGX(NoH, roughness);
    vec3  F = F_Schlick(LoH, f0);
    float V = V_SmithGGXCorrelated(NoV, NoL, roughness);

    vec3 Fr = (D * V) * F;              // specular
    vec3 Fd = diffuseColor * Fd_Lambert(); // diffuse

    return (Fd + Fr) * NoL;
}

void main() {
    // --- Sample textures ---
    vec4 baseColorSample = texture(t_BaseColor, fs_in.uv);
    vec3 baseColor = baseColorSample.rgb * uBaseColorFactor;

    vec2 mr = texture(t_MetallicRoughness, fs_in.uv).gb; // g=rough, b=metal
    float perceptualRoughness = clamp(mr.x * u_RoughnessFactor, 0.0, 1.0);
    float metallic = clamp(mr.y * u_MetallicFactor, 0.0, 1.0);

    float ao = texture(t_AO, fs_in.uv).r;
    vec3 emissive = texture(t_Emissive, fs_in.uv).rgb * u_EmissiveFactor;

    // --- Normal mapping ---
    vec3 tangentNormal = texture(t_Normal, fs_in.uv).xyz * 2.0 - 1.0;
    vec3 n = normalize(fs_in.TBN * tangentNormal);

    vec3 v = normalize(u_CameraPosition - fs_in.worldPos);
    vec3 l = normalize(u_LightPos - fs_in.worldPos);

    // --- Remapping ---
    float roughness = perceptualRoughness * perceptualRoughness;
    roughness = max(roughness, 0.045 * 0.045);

    vec3 diffuseColor = (1.0 - metallic) * baseColor;
    vec3 f0 = 0.16 * u_Reflectance * u_Reflectance * (1.0 - metallic) + baseColor * metallic;

    float dist = length(u_LightPos - fs_in.worldPos);
    float attenuation = 1.0 / max(dist * dist, 1e-4);
    vec3 radiance = u_LightColor * u_LightIntensity * attenuation;

    vec3 color = BRDF(n, v, l, diffuseColor, f0, roughness) * radiance;

    vec3 ambient = diffuseColor * 0.03 * ao;
    color += ambient;
    color += emissive;

    FragColor = vec4(color, 1.0);
}
