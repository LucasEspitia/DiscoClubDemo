#ifndef DISCOCLUB_TABLESET_H
#define DISCOCLUB_TABLESET_H

#include "../../scene/Object.h"

enum TypeTableSet {
    Table,
    Chair,
    Sofa
};

class TableSet : public Object{
private:
    static std::unique_ptr<ppgso::Mesh> meshTable;
    static std::unique_ptr<ppgso::Mesh> meshChair;
    static std::unique_ptr<ppgso::Mesh> meshSofa;



    TypeTableSet type;
public:
    TableSet(Object* parent,  TypeTableSet type, Scene &scene);
    bool update(Scene&, float, float, glm::mat4, glm::vec3) override;
    void render(Scene&, ppgso::Shader&) override;
    void renderDepth(Scene &scene, ppgso::Shader &shader) override;
};
#endif //DISCOCLUB_TABLESET_H