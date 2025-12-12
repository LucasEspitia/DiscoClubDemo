#ifndef DISCOCLUB_LIGHT_H
#define DISCOCLUB_LIGHT_H

#include <glm/glm.hpp>
#include <ppgso/ppgso.h>


enum class LightType {
    Directional,
    Point,
    Spot
};

class Light {
public:
    LightType type;
    // Common
    glm::vec3 ambient {0.1f};
    glm::vec3 diffuse {1.0f};
    glm::vec3 specular{1.0f};

    glm::vec3 baseAmbient;
    glm::vec3 baseDiffuse;
    glm::vec3 baseSpecular;

    // Directional
    glm::vec3 direction { -0.2f, -1.0f, -0.3f };

    // Point & Spot
    glm::vec3 position {0.0f};

    float constant  = 1.0f;
    float linear    = 0.09f;
    float quadratic = 0.032f;

    // Spot only
    glm::vec3 spotDirection {0, -1, 0};
    float cutOff       = glm::cos(glm::radians(12.5f));
    float outerCutOff  = glm::cos(glm::radians(17.5f));

    //Active
    bool enabled = true;

    // Constructor
    Light(LightType t) : type(t),
            baseAmbient(0.0f),
            baseDiffuse(0.0f),
            baseSpecular(0.0f) {}

    void upload(ppgso::Shader &shader, int index);
};

#endif //DISCOCLUB_LIGHT_H