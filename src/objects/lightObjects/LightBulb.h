#ifndef DISCOCLUB_LIGHTBULB_H
#define DISCOCLUB_LIGHTBULB_H

#include "src/scene/Object.h"
#include "src/scene/Light.h"
#include "LightEmitter.h"



class LightBulb : public Object{
public:
    static std::unique_ptr<ppgso::Mesh> meshBulb;

    glm::vec3 emissiveColor;
    float emissiveStrength;
    Light* bulbLight = nullptr;
    LightEmitter* emitter = nullptr;

    LightBulb(Object *parent, Scene &scene, glm::vec3 color, float strength);

    bool update(Scene &scene, float time, float dt, glm::mat4 parentModelMatrix, glm::vec3 parentRotation) override;
    void render(Scene &scene, ppgso::Shader& shader) override;
};


#endif //DISCOCLUB_LIGHTBULB_H