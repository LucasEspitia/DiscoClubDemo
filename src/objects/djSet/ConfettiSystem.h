#ifndef CONFETTI_SYSTEM_H
#define CONFETTI_SYSTEM_H

#include "src/scene/Object.h"
#include "ppgso/ppgso.h"

class ConfettiSystem : public Object {
public:
    struct Particle {
        glm::vec3 pos;
        glm::vec3 vel;
        glm::vec4 color;
        float life;

        float angle = 0.0f;
        float angularVel = 0.0f;
    };

    std::unique_ptr<ppgso::Mesh_Assimp> meshConfetti;
    static const int MAX_PARTICLES;

    std::vector<Particle> particles;

    std::vector<glm::vec3> instancePositions;
    std::vector<glm::vec3> instanceColors;
    std::vector<float> instanceAngles;

    GLuint angleVBO;
    GLuint posVBO = 0;
    GLuint colorVBO = 0;


    glm::vec3 emissionPoint{0.0f};
    glm::vec3 emissionDir{0, 1, 0};

    float particleScale = 1.3f;

    float gravity      = -9.8f;
    float windStrength = 1.5f;

    float v0 = 44.0f;
    float spread = 5.0f;

    float groundY = -5.4f;


    ConfettiSystem(Object* parent, Scene& scene);

    void emit(int count, const glm::vec3 &emissionPoint, const glm::vec3 &emissionDir);

    bool update(Scene &scene, float time, float dt,
                glm::mat4 parentModelMatrix, glm::vec3 parentRotation) override;

    void render(Scene &scene, ppgso::Shader &shader) override;
};

#endif
