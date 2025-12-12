#ifndef DISCOCLUB_CONFETTICANON_H
#define DISCOCLUB_CONFETTICANON_H
#include "ConfettiSystem.h"
#include "src/scene/Object.h"

enum class CanonType {
    Left,
    Right
};

class ConfettiCannon : public Object {
public:
    static std::unique_ptr<ppgso::Mesh> meshBaseCanon;
    static std::unique_ptr<ppgso::Mesh> meshCanon;
    static std::unique_ptr<ppgso::Mesh> meshCoverCanon;

    CanonType type;

    //Position of head
    glm::vec3 headPosition = {0, 0.2, 0};
    glm::vec3 headRotation = {0, 0, 0};
    glm::vec3 headScale    = {1, 1, 1};

    glm::vec3 emissionLocal = {0, 0.5f, -1.0};

    glm::vec3 debugDirLeft;
    glm::vec3 debugDirRight;

    //Pointer to partycle System.
    ConfettiSystem* confettiSystem = nullptr;
    bool firing = false;
    float fireRate = 200.0f;
    float fireTimer = 0.0f;



    ConfettiCannon(Object* parent, CanonType typeSet, Scene& scene);

    void animateHead(float time);

    glm::vec3 getEmissionPoint() const;
    glm::vec3 getEmissionDirection() const;

    bool update(Scene &scene, float time, float dt,
                glm::mat4 parentModelMatrix, glm::vec3 parentRotation) override;

    void render(Scene &scene, ppgso::Shader &shader) override;
    void renderDepth(Scene &scene, ppgso::Shader &shader) override;
    void fire();
    void stop();


private:
    glm::mat4 computeHeadMatrix() const;
};

#endif //DISCOCLUB_CONFETTICANON_H
