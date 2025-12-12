#include "ConfettiSystem.h"
#include "src/scene/Scene.h"

const int ConfettiSystem::MAX_PARTICLES = 6000;

//Util function
float randf(float a, float b) {
    return a + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (b - a);
}

ConfettiSystem::ConfettiSystem(Object *parent, Scene &scene) {
    parentObject = parent;

    // Usa un quad simple como partícula
    if (!meshConfetti)
        meshConfetti = std::make_unique<ppgso::Mesh_Assimp>("data/objects/particle.obj");

    instancePositions.reserve(MAX_PARTICLES);
    instanceColors.reserve(MAX_PARTICLES);
    particles.reserve(MAX_PARTICLES);

    // ---- VBO POSITION ----
    glGenBuffers(1, &posVBO);
    glBindBuffer(GL_ARRAY_BUFFER, posVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 MAX_PARTICLES * sizeof(glm::vec3),
                 nullptr,
                 GL_DYNAMIC_DRAW);
    meshConfetti->addInstanceBuffer(posVBO, 3, 3);

    // ---- VBO COLORS ----
    glGenBuffers(1, &colorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 MAX_PARTICLES * sizeof(glm::vec3),
                 nullptr,
                 GL_DYNAMIC_DRAW);
    meshConfetti->addInstanceBuffer(colorVBO, 4, 3);

    // ---- VBO ANGLES ----
    glGenBuffers(1, &angleVBO);
    glBindBuffer(GL_ARRAY_BUFFER, angleVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 MAX_PARTICLES * sizeof(float),
                 nullptr,
                 GL_DYNAMIC_DRAW);

    meshConfetti->addInstanceBuffer(angleVBO, 5, 1);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ConfettiSystem::emit(int count, const glm::vec3 &emissionPoint, const glm::vec3 &emissionDir) {
    if (particles.size() >= MAX_PARTICLES)
        return;

    glm::vec3 dir = glm::normalize(emissionDir);


    for (int i = 0; i < count && particles.size() < MAX_PARTICLES; i++) {
        Particle p;
        p.pos  = emissionPoint;
        p.life = 10.f;
        // Initial Speed
        p.vel = dir * v0;

        // Speed with spread
        p.vel.x += randf(-spread * 1.3, spread * 1.3);
        p.vel.y += randf(-spread * 0.3f, spread * 0.3f);
        p.vel.z += randf(-spread, spread);

        //Color
        p.color = glm::vec4(randf(0,1), randf(0,1), randf(0,1), 1.0f);

        //Angular Speed
        p.angularVel = randf(-5.0f, 5.0f);

        particles.push_back(p);
    }
}

bool ConfettiSystem::update(Scene &scene, float time, float dt,
                            glm::mat4 parentModelMatrix, glm::vec3 parentRotation) {
    instancePositions.clear();
    instanceColors.clear();
    instanceAngles.clear();

    for (auto &p : particles) {
        p.life -= dt;
        if (p.life <= 0.0f) continue;

        p.vel.y += gravity * dt;

        // APPLY FORCES
        glm::vec3 wind = glm::vec3(sin(time * 2.0f) * 1.5f, 0.0f, 0.0f);

        // Semi implicit Euler
        p.vel.y += gravity * dt;
        p.vel += wind * dt;

        p.pos += p.vel * dt;

        p.angle += p.angularVel * dt;


        if (p.pos.y < groundY) {
            p.pos.y = groundY;

            //Bound with Normal y.
            glm::vec3 normal = glm::vec3(0, 1, 0);

            p.vel = glm::reflect(p.vel, normal);

            p.vel *= 0.3f;

            p.vel.x *= 0.6f;
            p.vel.z *= 0.6f;

            if (fabs(p.vel.y) < 0.1f) {
                p.vel.y = 0.0f;
            }
        }

        instancePositions.push_back(p.pos);
        instanceColors.push_back(glm::vec3(p.color));
        instanceAngles.push_back(p.angle);

    }


    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
                       [](const Particle& p){ return p.life <= 0.0f; }),
        particles.end()
    );
    glBindBuffer(GL_ARRAY_BUFFER, posVBO);
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    instancePositions.size() * sizeof(glm::vec3),
                    instancePositions.data());

    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    instanceColors.size() * sizeof(glm::vec3),
                    instanceColors.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    generateModelMatrix(parentModelMatrix);
    return true;
}

void ConfettiSystem::render(Scene &scene, ppgso::Shader &shader) {
    if (instancePositions.empty())
        return;

    shader.setUniform("ModelMatrix", glm::mat4(1.0f));

    meshConfetti->renderInstanced(instancePositions.size());
}


