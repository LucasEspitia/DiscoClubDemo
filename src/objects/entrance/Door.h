#ifndef DISCOCLUB_DOOR_H
#define DISCOCLUB_DOOR_H
#include <memory>
#include <ppgso/ppgso.h>

#include "../../scene/Object.h"

enum class DoorType {
    Left,
    Right,
    FrameLeft,
    FrameRight
};

enum class DoorState {
    Closed,
    Opening,
    Open,
    Closing
};


class Door: public Object {
private:
    DoorType type;
    static std::unique_ptr<ppgso::Mesh> meshLeftDoor;
    static std::unique_ptr<ppgso::Mesh> meshRightDoor;
    static std::unique_ptr<ppgso::Mesh> meshLeftBars;
    static std::unique_ptr<ppgso::Mesh> meshRightBars;

    //Texture for bars
    static std::unique_ptr<ppgso::Texture> barsTexture;
    //Animation
    float animTime = 0.0f;
    float animDuration = 1.2f;
    float maxSlideX = 8.0f;
    float maxSlideZ = 1.0f;
    glm::vec3 openedPosition;
    glm::vec3 closedPosition;


public:
    DoorState state = DoorState::Closed;

    Door(Object* parent, DoorType type, Scene& scene);
    bool update(Scene &scene, float time, float dt, glm::mat4 parentModelMatrix, glm::vec3 parentRotation) override;
    void render(Scene &scene, ppgso::Shader&) override;
    void open();
    void close();

};


#endif //DISCOCLUB_DOOR_H