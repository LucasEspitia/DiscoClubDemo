#version 330 core

// ---------- INPUTS ----------
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

// ---------- OUTPUTS to Fragment ----------
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec4 FragPosLightSpace;

// Para normal mapping
out mat3 TBN;

// ---------- UNIFORMS ----------
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 lightSpaceMatrix;

void main() {

    vec4 worldPos = ModelMatrix * vec4(aPos, 1.0);
    gl_Position = ProjectionMatrix * ViewMatrix * worldPos;

    FragPos = worldPos.xyz;

    Normal = normalize(mat3(transpose(inverse(ModelMatrix))) * aNormal);

    TexCoord = aTexCoord;

    FragPosLightSpace = lightSpaceMatrix * worldPos;

    vec3 T = normalize(mat3(ModelMatrix) * aTangent);
    vec3 B = normalize(mat3(ModelMatrix) * aBitangent);
    vec3 N = normalize(mat3(ModelMatrix) * aNormal);

    T = normalize(T - dot(T, N) * N);
    B = normalize(cross(N, T));

    TBN = mat3(T, B, N);
}
