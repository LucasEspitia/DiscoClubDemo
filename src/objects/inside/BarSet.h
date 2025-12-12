#ifndef DISCOCLUB_BAR_H
#define DISCOCLUB_BAR_H

#include "../../scene/Object.h"

enum BarType {
    Bar,
    Shelf,
    ChairBar,
    Bottle,
    Can
};

class BarSet : public Object{
private:
    static std::unique_ptr<ppgso::Mesh> meshBar;
    static std::unique_ptr<ppgso::Mesh> meshChair;
    static std::unique_ptr<ppgso::Mesh> meshShelf;

    //static std::unique_ptr<ppgso::Mesh> meshCan;
    //static std::unique_ptr<ppgso::Mesh> meshBottle;

    BarType type;
    void generateShelves(Scene &scene);
    void generateLamps(Scene& scene);
public:
    BarSet(Object* parent,  BarType type, Scene &scene);
    bool update(Scene&, float, float, glm::mat4, glm::vec3) override;
    void render(Scene&, ppgso::Shader&) override;
    void renderDepth(Scene &scene, ppgso::Shader &shader) override;
};


#endif //DISCOCLUB_BAR_H