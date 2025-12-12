#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"
#include "Object.h"

Camera::Camera(float fovDeg, float ratio, float nearP, float farP)
    : fov(fovDeg),
      aspect(ratio),
      nearPlane(nearP),
      farPlane(farP)
{
    updateProjection(aspect);
    updateView();
}

void Camera::update(float dt) {

    if (useKeyframes && keyframes.playing) {

        auto [pos, rot] = keyframes.update(dt);

        position = pos;
        rotation = rot.y;
        tilt     = rot.x;

        //Zoom
        float fovTarget = rot.z;
        updateProjectionFOV(fovTarget);
    }

    glm::mat4 V = glm::mat4(1.0f);

    V = glm::rotate(V, glm::radians(tilt),   glm::vec3(1,0,0));
    V = glm::rotate(V, glm::radians(rotation), glm::vec3(0,1,0));
    V = glm::translate(V, -position);

    viewMatrix = V;
}

/**
 * Function util, when Zoom is done.
 * @param newFov
 */
void Camera::updateProjectionFOV(float newFov) {
    fov = newFov;
    projectionMatrix = glm::perspective(
        glm::radians(fov),
        aspect,
        nearPlane,
        farPlane
    );
}


/**
 *  Function on change size window.
 */
void Camera::updateView() {
    glm::vec3 forward{
        cos(glm::radians(tilt)) * sin(glm::radians(rotation)),
        sin(glm::radians(tilt)),
        cos(glm::radians(tilt)) * cos(glm::radians(rotation))
    };

    viewMatrix = glm::lookAt(position, position + forward, glm::vec3(0,1,0));
}

void Camera::updateProjection(float newAspect) {
    aspect = newAspect;
    projectionMatrix = glm::perspective(
            glm::radians(fov),
            aspect,
            nearPlane,
            farPlane
    );
}

void Camera::moveX(int dir) {
    position.x += dir * 2.5f;
    debug();
}

void Camera::moveY(int dir) {
    position.y += dir * 2.5f;
    debug();
}

void Camera::moveZ(int dir) {
    position.z += dir * 2.5f;
    debug();
}

void Camera::rotate(int dir) {
    rotation += dir * 2.5f;
}

void Camera::debug() {
    if (debugEnabled) {
        std::cout << "Camera position: "
                  << position.x << " "
                  << position.y << " "
                  << position.z << " "
                  << " yaw=" << rotation
                  << " pitch=" << tilt << std::endl;
    }
}
