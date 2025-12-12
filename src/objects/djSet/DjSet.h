#ifndef DISCOCLUB_DJSET_H
#define DISCOCLUB_DJSET_H

#include "ConfettiCannon.h"
#include "../../scene/Object.h"

enum DjType {
    Set,
    TableDj,
};


class DjSet : public Object{
private:
    static std::unique_ptr<ppgso::Mesh> meshSet;
    static std::unique_ptr<ppgso::Mesh> meshTable;

    DjType type;
public:
    ConfettiCannon* cannonLPtr;
    ConfettiCannon* cannonRPtr;

    DjSet(Object* parent,  DjType type, Scene &scene);
    bool update(Scene&, float, float, glm::mat4, glm::vec3) override;
    void render(Scene &scene, ppgso::Shader&) override;
    void renderDepth(Scene &scene, ppgso::Shader &shader) override;
};

#endif //DISCOCLUB_DJSET_H