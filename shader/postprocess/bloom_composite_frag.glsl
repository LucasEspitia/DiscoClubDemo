#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D scene;
uniform sampler2D bloom;
uniform sampler2D toneMask;

uniform float bloomStrength;

// ACES tone mapping
vec3 ACESFilm(vec3 x)
{
    return clamp((x * (1.8 * x + 0.1)) /
    (x * (1.5 * x + 0.5) + 0.2),
    0.0, 1.0);
}

void main()
{
    vec3 hdrColor = texture(scene, TexCoord).rgb;
    vec3 bloomColor = texture(bloom, TexCoord).rgb * bloomStrength;
    float mask = texture(toneMask, TexCoord).r;

    vec3 color = hdrColor + bloomColor;

    vec3 mapped;

    if(mask > 0.5) {
        // Apply tone mapping
        mapped = ACESFilm(color);
        mapped = pow(mapped, vec3(1.0/2.2)); // gamma
    }
    else {
        mapped = clamp(color, 0.0, 1.0);
    }

    FragColor = vec4(mapped, 1.0);
}
