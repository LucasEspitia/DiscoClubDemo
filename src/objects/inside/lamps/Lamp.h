#ifndef DISCOCLUB_LAMP_H
#define DISCOCLUB_LAMP_H
#include "src/objects/lightObjects/LightEmitter.h"
#include "src/scene/Light.h"
#include "src/scene/Object.h"


struct LampConfig {
    glm::vec3 color;
    float strength;
    float linearAtt = 0.14f;
    float quadraticAtt = 0.07f;
};

enum class LampType {
    Body,
    Outer,
    Shade,
    Bulb
};

class Lamp : public Object{
private:
    static std::unique_ptr<ppgso::Mesh> meshBody;
    static std::unique_ptr<ppgso::Mesh> meshOuter;
    static std::unique_ptr<ppgso::Mesh> meshShade;

    LampType type;
    LampConfig config;
public:
    Lamp(Object* parent, Scene& scene, LampType type, LampConfig config = {});

    bool update(Scene &scene, float time, float dt, glm::mat4 parentModelMatrix, glm::vec3 parentRotation) override;
    void render(Scene &scene, ppgso::Shader& shader) override;
    void renderDepth(Scene &scene, ppgso::Shader &shader) override;

};





#endif //DISCOCLUB_LAMP_H