#version 330 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out float BloomMask;
layout (location = 2) out float ToneMapMask;


in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLightSpace;
in mat3 TBN;

//Fog
uniform bool UseFog;


//Texture and color
uniform sampler2D Texture;
uniform bool UseTexture;
uniform vec3 ObjectColor;

//Normal Map
uniform bool UseNormalMap;
uniform sampler2D NormalMap;

// Shadow map
uniform sampler2D shadowMap;
uniform vec3 shadowLightPos;


// --------- Materials ---------
uniform bool IsUnlit;
uniform float Transparency;
uniform vec3 EmissiveColor;
uniform float EmissiveStrength;

uniform float DiffuseStrength;
uniform float SpecularStrength;
uniform float LightAbsorption;
uniform float shininess;

// ------- Lights conditions ------
uniform bool hasDirLight;
uniform bool hasPointLight;
uniform bool hasSpotLight;

uniform vec3 ViewPosition;

// ----------- LIGHT STRUCTS HERE ----------
struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirLight dirLight;

#define MAX_POINT_LIGHTS 16
struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int numPointLights;

#define MAX_SPOT_LIGHTS 16
struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];
uniform int numSpotLights;


// ------------------- SHADOW ------------------------
float calculateShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
    projCoords.y < 0.0 || projCoords.y > 1.0 ||
    projCoords.z > 1.0)
    return 0.0;

    float ndotl = max(dot(normalize(normal), normalize(lightDir)), 0.0);
    float bias = max(0.002 * (1.0 - ndotl), 0.0015);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    for (int x = -1; x <= 1; ++x)
    for (int y = -1; y <= 1; ++y)
    {
        vec2 offset = vec2(x, y) * texelSize;

        vec2 sampleUV = clamp(projCoords.xy + offset, 0.0, 1.0);

        float closestDepth = texture(shadowMap, sampleUV).r;

        shadow += (projCoords.z - bias > closestDepth) ? 1.0 : 0.0;
    }

    shadow /= 9.0;

    return clamp(shadow, 0.0, 1.0);
}

// ----------------- DIRECTIONAL LIGHT --------------------

vec3 computeDirectional(vec3 baseColor, vec3 N, vec3 V)
{
    vec3 lightDir = normalize(-dirLight.direction);

    float diff = max(dot(N, lightDir), 0.0);

    vec3 H = normalize(V + lightDir);
    float spec = pow(max(dot(N, H), 0.0), shininess);

    vec3 ambientD  = dirLight.ambient  * baseColor;
    vec3 diffuseD  = dirLight.diffuse  * diff * baseColor * DiffuseStrength;
    vec3 specularD = dirLight.specular * spec * SpecularStrength;

    vec3 result = (ambientD + diffuseD + specularD) * LightAbsorption;

    return result;
}


// ----------------- PUNCTUAL LIGHT --------------------

vec3 computePoint(PointLight L, vec3 baseColor, vec3 N, vec3 V)
{
    vec3 lightDir = normalize(L.position - FragPos);

    float diff = max(dot(N, lightDir), 0.0);

    vec3 H = normalize(V + lightDir);
    float spec = pow(max(dot(N, H), 0.0), shininess);

    //Correction attenuation
    float dist = length(L.position - FragPos);
    float attenuation =
    1.0 / (L.constant + L.linear * dist + L.quadratic * dist * dist);
    //compute light
    vec3 ambientP  = L.ambient  * baseColor * attenuation;
    vec3 diffuseP  = L.diffuse  * diff * baseColor * attenuation * (DiffuseStrength * 0.6);
    vec3 specularP = L.specular * spec      * attenuation * (SpecularStrength * 0.2);

    vec3 lighting = (ambientP + diffuseP + specularP) * LightAbsorption;

    // Glow volumetric
    float rawIntensity = length(diffuseP + specularP);

    float glowFactor = smoothstep(0.05, 0.25, rawIntensity);

    vec3 glowColor = L.diffuse * glowFactor * 0.4;  // 0.4 = fuerza del glow

    lighting += glowColor;

    return lighting;

}


// ----------------- SPOT LIGHT --------------------

vec3 computeSpot(SpotLight L, vec3 baseColor, vec3 N, vec3 V)
{
    vec3 lightDir = normalize(L.position - FragPos);

    float diff = max(dot(N, lightDir), 0.0);

    //Calculate Half-Vector -> Bling
    vec3 H = normalize(V + lightDir);
    float spec = pow(max(dot(N, H), 0.0), shininess);

    float theta   = dot(lightDir, normalize(-L.direction));

    float epsilon = max(L.cutOff - L.outerCutOff, 0.0001);
    float intensity = clamp((theta - L.outerCutOff) / epsilon, 0.0, 1.0);
    //Correct depth-based light attenuation
    float dist = length(L.position - FragPos);
    float attenuation =
    1.0 / (L.constant + L.linear * dist + L.quadratic * dist * dist);

    vec3 ambientS  = L.ambient  * baseColor * attenuation * intensity;

    vec3 diffuseS  = L.diffuse * diff * baseColor
    * attenuation * intensity * DiffuseStrength;

    vec3 specularS = L.specular * spec
    * attenuation * intensity * SpecularStrength;

    vec3 lighting = (ambientS + diffuseS + specularS) * LightAbsorption;

    // Glow volumetric
    float rawIntensity = length(diffuseS + specularS);

    float glowFactor = smoothstep(0.03, 0.2, rawIntensity);

    vec3 glowColor = L.diffuse * glowFactor * 0.6;

    lighting += glowColor;

    return lighting;
}

void main()
{
    vec3 baseColor = UseTexture ?
        pow(texture(Texture, TexCoord).rgb, vec3(2.2))
        : ObjectColor;
    //Ignore light
    if (IsUnlit) {
        vec3 color = UseTexture ?
        pow(texture(Texture, TexCoord).rgb, vec3(2.2)) :
        ObjectColor;

        FragColor = vec4(pow(color, vec3(1.0/2.2)), Transparency);
        return;
    }
    //Normal mapping
    vec3 N;
    if (UseNormalMap) {
        vec3 normalTex = texture(NormalMap, TexCoord).rgb;
        normalTex = normalTex * 2.0 - 1.0;
        N = normalize(TBN * normalTex);
    }
    else {
        N = normalize(Normal);
    }

    vec3 V = normalize(ViewPosition - FragPos);

    vec3 lighting = vec3(0.0);

    if (hasDirLight)
    lighting += computeDirectional(baseColor, N, V);

    if (hasPointLight)
    for (int i = 0; i < numPointLights; i++)
    lighting += computePoint(pointLights[i], baseColor, N, V);


    vec3 spotTotal = vec3(0.0);

    if (hasSpotLight)
    for(int i = 0; i < numSpotLights; i++)
    spotTotal += computeSpot(spotLights[i], baseColor, N, V);

    vec3 lightDir = shadowLightPos - FragPos;
    float shadow = calculateShadow(FragPosLightSpace, N, lightDir);

    spotTotal *= (1.0 - shadow);

    lighting += spotTotal;
    lighting += EmissiveColor * EmissiveStrength;



    if (UseFog) {
        float dist = length(ViewPosition - FragPos);

        float fogStart = 8.0;
        float fogEnd   = 25.0;
        float fogAmount = smoothstep(fogStart, fogEnd, dist);

        vec3 fogColor = vec3(0.05, 0.02, 0.03);
        lighting = mix(lighting, fogColor, fogAmount);
    }

    FragColor = vec4(lighting, Transparency);

    float brightness = dot(lighting, vec3(0.2126, 0.7152, 0.0722));

    BloomMask = brightness > 1.0 ? 1.0 : 0.0;
    ToneMapMask = 1.0f;
}

