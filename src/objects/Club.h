#ifndef DISCOCLUB_CLUB_H
#define DISCOCLUB_CLUB_H

#include <memory>
#include <ppgso/ppgso.h>

#include "../scene/Object.h"
#include "src/scene/Light.h"

enum ClubType {
    ClubWalls,
    ClubFloor,
    ClubRedCarpet,
    ClubCeiling,
    ClubWoodDecoration,
    ClubLadyDecoration,
    ClubWallGoldDecoration
};

class Club : public Object {
private:
    //Meshes
    static std::unique_ptr<ppgso::Mesh> meshWalls;
    static std::unique_ptr<ppgso::Mesh> meshFloor;
    static std::unique_ptr<ppgso::Mesh> meshRedCarpet;
    static std::unique_ptr<ppgso::Mesh> meshCeiling;
    static std::unique_ptr<ppgso::Mesh> meshToilets;
    static std::unique_ptr<ppgso::Mesh> meshWoodDecoration;
    static std::unique_ptr<ppgso::Mesh> meshLadyDecoration;
    static std::unique_ptr<ppgso::Mesh> meshGoldDecoration;

    //Type
    ClubType clubType;

    //Animation Lady Neon
    float emissiveStrength = 2.0f;

public:
    Club(Object* parent, ClubType type, Scene& scene);

    bool update(Scene &scene, float time, float dt, glm::mat4 parentModelMatrix, glm::vec3 parentRotation) override;

    void render(Scene &scene, ppgso::Shader&) override;
    void renderDepth(Scene&, ppgso::Shader &shader) override;
private:
    //Help to render mesh with modularity
    void renderMesh(ppgso::Mesh* mesh, ppgso::Shader&);
};


#endif //DISCOCLUB_CLUB_H