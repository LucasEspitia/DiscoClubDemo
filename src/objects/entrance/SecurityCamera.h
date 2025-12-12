#ifndef DISCOCLUB_SECURITYCAMERA_H
#define DISCOCLUB_SECURITYCAMERA_H

#include <memory>
#include <ppgso/ppgso.h>
#include "src/scene/Object.h"


class SecurityCamera : public Object {
private:
    static std::unique_ptr<ppgso::Mesh> meshCamera;

    glm::vec3 baseRotation;

//    glm::vec3 initialRotation;


    //Cinematic
    bool cinematicActive = true;
    float cinematicTime = 0.0f;
    float cinematicDuration = 4.0f;
    //Hold cinematic
    bool cinematicHold = false;
    float holdDuration = 1.5f;
    float holdTime = 0.0f;
    //Return cinematic
    bool cinematicReturn = false;

public:
    //Target
    glm::vec3 cinematicTarget;
    //Different targets
    glm::vec3 patrolTarget;
    glm::vec3 entranceTarget;
    SecurityCamera(Object* parent, Scene& scene);

    bool update(Scene &scene, float time, float dt, glm::mat4 parentModelMatrix, glm::vec3 parentRotation) override;

    void render(Scene &scene, ppgso::Shader&) override;

};


#endif //DISCOCLUB_SECURITYCAMERA_H