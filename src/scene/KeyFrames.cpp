#include "KeyFrames.h"

// Util for interpolation -> curve interpolation
float KeyFrames::smooth(float t) {
    return t * t * (3 - 2 * t);
}

// Soft interpolation
glm::vec3 KeyFrames::smoothLerp(const glm::vec3 &a, const glm::vec3 &b, float t) {
    float s = smooth(t);
    return a + (b - a) * s;
}

// Update Keyframes
std::pair<glm::vec3, glm::vec3> KeyFrames::update(float dt) {
    // ----------------------------------------------------
    // Safety checks
    // - If animation is not playing
    // - If there are not enough keyframes to interpolate
    // ----------------------------------------------------
    if (!playing || frames.size() < 2) {

        // No keyframes → return default
        if (frames.empty()) {
            return std::make_pair(glm::vec3(0), glm::vec3(0));
        }
        // Only one keyframe → stay in that position/rotation
        else {
            return std::make_pair(frames[0].position, frames[0].rotation);
        }
    }

    // ----------------------------------------------------
    // Advance animation time
    // ----------------------------------------------------
    animTime += dt;

    // ----------------------------------------------------
    // Execute scheduled events when passing their trigger time
    // ----------------------------------------------------
    for (auto &ev : events) {
        if (!ev.triggered && animTime >= ev.triggerTime) {
            ev.triggered = true;      // Mark event as executed
            ev.callback();            // Run the event (open door, trigger lights, etc.)
        }
    }

    // ----------------------------------------------------
    // Stop animation when reaching the last keyframe
    // ----------------------------------------------------
    if (animTime >= frames.back().time) {
        animTime = frames.back().time;
        playing = false;
    }

    // ----------------------------------------------------
    // Find the two keyframes to interpolate between (k1 → k2)
    // ----------------------------------------------------
    Keyframe k1 = frames[0];
    Keyframe k2 = frames.back();

    for (int i = 0; i < frames.size() - 1; i++) {
        if (animTime >= frames[i].time && animTime <= frames[i+1].time) {
            k1 = frames[i];
            k2 = frames[i+1];
            break;
        }
    }

    // ----------------------------------------------------
    // Normalized interpolation factor between k1 and k2
    // ----------------------------------------------------
    float t = (animTime - k1.time) / (k2.time - k1.time);

    // ----------------------------------------------------
    // Smooth interpolation for both position and rotation
    // ----------------------------------------------------
    glm::vec3 pos = smoothLerp(k1.position, k2.position, t);
    glm::vec3 rot = smoothLerp(k1.rotation,  k2.rotation,  t);

    // ----------------------------------------------------
    // Return interpolated transform to the caller (camera)
    // ----------------------------------------------------
    return std::make_pair(pos, rot);
}



// Add events
void KeyFrames::addEvent(float time, std::function<void()> fn)
{
    events.push_back({ time, false, fn });
}
