#version 330 core

layout(location = 0) in vec3 vertexPos;
layout(location = 3) in vec3 instancePos;
layout(location = 4) in vec3 instanceColor;
layout(location = 5) in float instanceAngle;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

out vec3 vColor;


void main() {
    float a = instanceAngle;
    mat3 rotZ = mat3(
        cos(a), -sin(a), 0.0,
        sin(a),  cos(a), 0.0,
        0.0,     0.0,    1.0
    );
    vec3 rotatedVertex = rotZ * vertexPos;
    vec3 worldPos = instancePos + vertexPos;

    gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(worldPos, 1.0);
    vColor = instanceColor;
}
