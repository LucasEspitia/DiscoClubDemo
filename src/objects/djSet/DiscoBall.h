#ifndef DISCOCLUB_DISCOBALL_H
#define DISCOCLUB_DISCOBALL_H
#include "src/scene/Object.h"

enum DiscoBallType {
    Base,
    Ball
};

class DiscoBall : public Object{
private:
    static std::unique_ptr<ppgso::Mesh> meshDiscoBall;
    static std::unique_ptr<ppgso::Mesh> meshBaseDisco;
    DiscoBallType type;

    float emissiveStrength;
    glm::vec3 emissiveColor;

public:
    DiscoBall(Object* parent, DiscoBallType type, Scene &scene);
    bool update(Scene&, float, float, glm::mat4, glm::vec3) override;
    void render(Scene &scene, ppgso::Shader&) override;
    void renderDepth(Scene &scene, ppgso::Shader &shader) override;

};

#endif //DISCOCLUB_DISCOBALL_H