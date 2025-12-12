#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

layout (location = 3) in vec3 InstancePos;
layout (location = 4) in vec3 InstanceColor;

out vec2 TexCoord;
out vec3 vColor;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

void main()
{
    mat4 T = mat4(1.0);
    T[3] = vec4(InstancePos, 1.0);

    mat4 M = ModelMatrix * T;

    gl_Position = ProjectionMatrix * ViewMatrix * M * vec4(aPos, 1.0);

    TexCoord = aTexCoord;
    vColor   = InstanceColor;
}
