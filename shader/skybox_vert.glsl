#version 330 core

layout (location = 0) in vec3 aPos;

out vec3 TexCoord;

uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

void main()
{
    mat4 viewNoTranslation = mat4(mat3(ViewMatrix));

    TexCoord = aPos;
    gl_Position = ProjectionMatrix * viewNoTranslation * vec4(aPos, 1.0);

    gl_Position = gl_Position.xyww;
}
