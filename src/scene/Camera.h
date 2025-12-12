#ifndef DISCOCLUB_CAMERA_H
#define DISCOCLUB_CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "KeyFrames.h"

class Camera {
public:
    // Position and orientation
    glm::vec3 position{0.0f};
    float rotation = 0.f;
    float tilt = 0.f;

    // View / Projection matrices
    glm::mat4 viewMatrix{1.0f};
    glm::mat4 projectionMatrix{1.0f};

    // Perspective parameters
    float fov;
    float aspect;
    float nearPlane;
    float farPlane;

    float age = 0.0f;
    bool debugEnabled = false;

    //Keyframes for cinematics
    KeyFrames keyframes;
    bool useKeyframes = false;

    // Constructor
    Camera(float fovDeg = 30.0f,
           float ratio = 1.5f,
           float nearP = 0.01f,
           float farP = 500.f);

    // -------------------------------------------------------------
    // Update matrices
    // -------------------------------------------------------------
    void updateView();

    void updateProjectionFOV(float newFov);

    void updateProjection(float newAspect);

    // -------------------------------------------------------------
    // Movement controls
    // -------------------------------------------------------------
    void update(float time);
    void moveX(int dir);
    void moveY(int dir);
    void moveZ(int dir);
    void rotate(int dir);

    void debug();
};

#endif //DISCOCLUB_CAMERA_H
