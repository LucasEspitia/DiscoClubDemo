#include "SecurityCamera.h"
#include "src/scene/Scene.h"

std::unique_ptr<ppgso::Mesh> SecurityCamera::meshCamera;

SecurityCamera::SecurityCamera(Object *parent, Scene &scene) {

    parentObject = parent;
    position = {0,0,0};
    rotation = {0,0,0};
    scale = {1.0f,1.0f,1.0f};
    if (!meshCamera) meshCamera = std::make_unique<ppgso::Mesh>("data/objects/clubDecoration/SecurityCamera.obj");
    meshCamera->texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/securityCamera.bmp"));
    this->material = MaterialType::Metal;

    //---------- For animations -----------------
    // Patrol base rotation
    {
        glm::vec3 blenderEuler = glm::radians(glm::vec3(
            15.294f,
            -5.3932f,
            11.663f
        ));

        glm::quat q =
            glm::angleAxis(blenderEuler.x, glm::vec3(1,0,0)) *
            glm::angleAxis(blenderEuler.y, glm::vec3(0,1,0)) *
            glm::angleAxis(blenderEuler.z, glm::vec3(0,0,1));

        patrolTarget = glm::eulerAngles(q);
    }
    //Cinematic rotation
    {
        glm::vec3 cinematicEulerBlender = glm::radians(glm::vec3(
            21.1458f,
            28.524f,
            41.753f
        ));

        glm::quat qcin =
            glm::angleAxis(cinematicEulerBlender.x, glm::vec3(1,0,0)) *
            glm::angleAxis(cinematicEulerBlender.y, glm::vec3(0,1,0)) *
            glm::angleAxis(cinematicEulerBlender.z, glm::vec3(0,0,1));

        entranceTarget = glm::eulerAngles(qcin);
    }
    cinematicTarget = patrolTarget;
}

bool SecurityCamera::update(Scene &scene, float time, float dt,
                            glm::mat4 parentModelMatrix, glm::vec3 parentRotation)
{
    // --- PHASE A: GO TO TARGET ---
    if (cinematicActive) {
        cinematicTime += dt;
        float t = glm::clamp(cinematicTime / cinematicDuration, 0.0f, 1.0f);
        float k = 1.0f - pow(1.0f - t, 3.0f); // ease-out

        rotation = glm::mix(baseRotation, cinematicTarget, k);

        if (t >= 1.0f) {
            cinematicActive = false;
            cinematicHold = true;
            holdTime = 0.0f;
        }

        generateModelMatrix(parentModelMatrix);
        return true;
    }

    // --- PHASE B: HOLD ---
    if (cinematicHold) {
        holdTime += dt;
        rotation = cinematicTarget;

        if (holdTime >= holdDuration) {
            cinematicHold = false;
            cinematicReturn = true;
            cinematicTime = 0.0f;
        }

        generateModelMatrix(parentModelMatrix);
        return true;
    }

    // --- PHASE C: RETURN ---
    if (cinematicReturn) {
        cinematicTime += dt;
        float t = glm::clamp(cinematicTime / cinematicDuration, 0.0f, 1.0f);
        float k = glm::smoothstep(0.0f, 1.0f, t);

        rotation = glm::mix(cinematicTarget, baseRotation, k);

        if (t >= 1.0f) {
            cinematicReturn = false;
            cinematicActive = true;
            baseRotation = rotation;
            cinematicTime = 0.0f;
        }

        generateModelMatrix(parentModelMatrix);
        return true;
    }

    // --- NO CINEMATIC ACTIVE → do nothing or patrol idle here ---
    generateModelMatrix(parentModelMatrix);
    return true;
}






void SecurityCamera::render(Scene &scene, ppgso::Shader& shader) {
    // Render mesh
    shader .setUniform("ModelMatrix", modelMatrix);

    shader.setUniform("UseTexture", true);
    meshCamera->texture->bind(0);
    shader.setUniform("Texture", 0);
    meshCamera->render();
}

