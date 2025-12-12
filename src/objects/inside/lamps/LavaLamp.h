//
// Created by lucas on 5/12/2025.
//
#ifndef DISCOCLUB_LAVALAMP_H
#define DISCOCLUB_LAVALAMP_H
#include "Lamp.h"
#include "src/scene/Light.h"
#include "src/scene/Object.h"
#include "src/scene/Scene.h"

enum LavaType {
    LavaBase,
    LavaGlass,
};

class LavaLamp : public Object {
private:
    static std::unique_ptr<ppgso::Mesh> meshLavaBase;
    static std::unique_ptr<ppgso::Mesh> meshLavaGlass;
    glm::vec3 lampColor;
    float emissiveStrength;
    LavaType type;

    Light* light = nullptr;
public:

    LavaLamp(Object* parent,Scene &scene, LavaType lavaType, glm::vec3 color, float emissive);

    bool update(Scene &scene, float time, float dt, glm::mat4 parentModelMatrix, glm::vec3 parentRotation) override;
    void render(Scene &scene, ppgso::Shader& shader) override;
    void renderDepth(Scene &scene, ppgso::Shader &shader) override;

};

#endif //DISCOCLUB_LAVALAMP_H
