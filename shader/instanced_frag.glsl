#version 330 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out float BloomMask;
layout (location = 2) out float ToneMapMask;


in vec2 TexCoord;
in vec3 vColor;

uniform sampler2D Texture;
uniform bool UseTexture;
uniform vec3 ObjectColor;

void main()
{
    vec3 baseColor;

    if (UseTexture)
    baseColor = texture(Texture, TexCoord).rgb;
    else if (length(vColor) > 0.001)
    baseColor = vColor;
    else
    baseColor = ObjectColor;

    FragColor = vec4(baseColor, 1.0);

    BloomMask = 1.0;
    ToneMapMask = 1.0f;
}
