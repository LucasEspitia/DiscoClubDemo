#include "Light.h"
void Light::upload(ppgso::Shader &shader, int index)
{
    switch (type)
    {
        case LightType::Directional:
            shader.setUniform("dirLight.direction", direction);
            shader.setUniform("dirLight.ambient",  ambient);
            shader.setUniform("dirLight.diffuse",  diffuse);
            shader.setUniform("dirLight.specular", specular);
            break;

        case LightType::Point:
        {
            const std::string base = "pointLights[" + std::to_string(index) + "]";

            shader.setUniform(base + ".position",  position);
            shader.setUniform(base + ".ambient",   ambient);
            shader.setUniform(base + ".diffuse",   diffuse);
            shader.setUniform(base + ".specular",  specular);

            shader.setUniform(base + ".constant",  constant);
            shader.setUniform(base +  ".linear",    linear);
            shader.setUniform(base + ".quadratic", quadratic);
            break;
        }

        case LightType::Spot:
        {
            const std::string base = "spotLights[" + std::to_string(index) + "]";

            shader.setUniform(base + ".position",  position);
            shader.setUniform(base + ".direction", spotDirection);

            shader.setUniform(base + ".cutOff",      cutOff);
            shader.setUniform(base + ".outerCutOff", outerCutOff);

            shader.setUniform(base + ".ambient",  ambient);
            shader.setUniform(base + ".diffuse",  diffuse);
            shader.setUniform(base + ".specular", specular);

            shader.setUniform(base + ".constant",  constant);
            shader.setUniform(base + ".linear",    linear);
            shader.setUniform(base + ".quadratic", quadratic);
            break;
        }
    }
}