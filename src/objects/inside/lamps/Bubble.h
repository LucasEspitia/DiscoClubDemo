#ifndef DISCOCLUB_BUBLE_H
#define DISCOCLUB_BUBLE_H

#include "ppgso/ppgso.h"
#include "src/scene/Object.h"

struct BubbleConfig {
    glm::vec3 color = {1.0f, 0.4f, 0.3f};
    float emissiveStrength = 1.0f;
};

// Positions of the lamp
constexpr float Z_BOTTOM = 0.66563f;
constexpr float Z_TOP    = 3.21950f;
constexpr float HEIGHT   = Z_TOP - Z_BOTTOM;

constexpr float R_BOTTOM = 0.588f;   // radius
constexpr float R_TOP    = 0.285f;


class Bubble : public Object {
public:
    static std::unique_ptr<ppgso::Mesh> bubbleMesh;

    BubbleConfig config;

    glm::vec3 velocity;
    float bubbleRadius;
    float baseScale;
    float mass;
    int id;
    bool alive = true;

    Bubble(Object* parent, Scene& scene, BubbleConfig cfg);

    bool update(Scene &scene, float time, float dt, glm::mat4 parentModelMatrix, glm::vec3 parentRotation) override;
    void render(Scene &scene, ppgso::Shader &shader) override;

private:
    static float coneRadiusAt(float y) ;
};

#endif //DISCOCLUB_BUBLE_H