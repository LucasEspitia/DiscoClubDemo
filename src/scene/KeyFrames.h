#ifndef DISCOCLUB_KEYFRAMES_H
#define DISCOCLUB_KEYFRAMES_H

#include <functional>
#include <vector>
#include <utility>
#include <glm/glm.hpp>
#include "KeyFrames.h"


struct Keyframe {
    float time;
    glm::vec3 position;
    glm::vec3 rotation; // x = pitch, y = yaw, z = FOV
};

struct TimedEvent {
    float triggerTime;
    bool triggered = false;
    std::function<void()> callback;
};

class KeyFrames {
public:
    //Store important events
    std::vector<TimedEvent> events;

    //Store frames
    std::vector<Keyframe> frames;
    float animTime = 0.0f;
    bool playing = false;

    KeyFrames() = default;
    std::pair<glm::vec3, glm::vec3> update(float dt);
    void addEvent(float time, std::function<void()> fn);
private:
    static float smooth(float t);
    static glm::vec3 smoothLerp(const glm::vec3 &a, const glm::vec3 &b, float t);
};

#endif // DISCOCLUB_KEYFRAMES_H
