#version 330 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out float BloomMask;
layout (location = 2) out float ToneMapMask;


in vec3 vColor;

void main()
{
    FragColor = vec4(vColor, 1.0);
    BloomMask = 1.0;
    ToneMapMask = 1.0f;
}
