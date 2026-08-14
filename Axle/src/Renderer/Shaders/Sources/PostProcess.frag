#version 460 core

out vec4 FragColor;

in vec2 TexCoords;
layout (binding = 0) uniform sampler2D tex;

void main(){
    float gamma = 2.2;
    vec4 linearColor = texture(tex, TexCoords);
    vec3 gammaCorrected = pow(linearColor.rgb, vec3(1.0 / gamma));
    FragColor = vec4(gammaCorrected, linearColor.a);
}
