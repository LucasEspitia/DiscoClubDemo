#ifndef DISCOCLUB_QUAD_H
#define DISCOCLUB_QUAD_H
#include "../../scene/Object.h"


class LightEmitter : public Object{
public:
    static std::unique_ptr<ppgso::Mesh> meshEmitter;

    glm::vec3 emissiveColor;
    float emissiveStrength;

    LightEmitter(Object *parent, Scene &scene, glm::vec3 color, float strength);

    bool update(Scene &scene, float time, float dt, glm::mat4 parentModelMatrix, glm::vec3 parentRotation) override;
    void render(Scene &scene, ppgso::Shader& shader) override;
};




#endif //DISCOCLUB_QUAD_H