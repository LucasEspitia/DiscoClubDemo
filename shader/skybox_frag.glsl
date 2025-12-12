#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out float BloomMask;
layout (location = 2) out float ToneMapMask;

in vec3 TexCoord;
uniform samplerCube skybox;

void main()
{
    vec3 color = texture(skybox, normalize(TexCoord)).rgb;

    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);

    BloomMask = 0.0;
    ToneMapMask = 0.0;
}
