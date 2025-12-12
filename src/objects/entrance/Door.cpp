#include "Door.h"
#include <shaders/phong_vert_glsl.h>
#include <shaders/phong_frag_glsl.h>

#include "src/window/ClubbingWindow.h"
#include "../../scene/Scene.h"

std::unique_ptr<ppgso::Mesh> Door::meshLeftDoor;
std::unique_ptr<ppgso::Mesh> Door::meshRightDoor;
std::unique_ptr<ppgso::Mesh> Door::meshLeftBars;
std::unique_ptr<ppgso::Mesh> Door::meshRightBars;

std::unique_ptr<ppgso::Texture> Door::barsTexture;


Door::Door(Object* object, DoorType type, Scene& scene) {
    parentObject = object;
    this->type = type;

    position = {0,0,0};
    rotation = {0,0,0};
    scale = {1.0f,1.0f,1.0f};

    // Load meshes
    if (!meshLeftDoor)  meshLeftDoor  = std::make_unique<ppgso::Mesh>("data/objects/clubDecoration/DoorLeft.obj");
    if (!meshRightDoor) meshRightDoor = std::make_unique<ppgso::Mesh>("data/objects/clubDecoration/DoorRight.obj");
    if (!meshLeftBars)  meshLeftBars  = std::make_unique<ppgso::Mesh>("data/objects/clubDecoration/BarsDoorLeft.obj");
    if (!meshRightBars) meshRightBars = std::make_unique<ppgso::Mesh>("data/objects/clubDecoration/BarsDoorRight.obj");


    if (!barsTexture) {
        barsTexture = std::make_unique<ppgso::Texture>(
            ppgso::image::loadBMP("data/objects/textures/metal.bmp")
        );
    }
    //Transparent Object
    if (type == DoorType::Left || type == DoorType::Right)
        isTransparent = true;
        this->material = MaterialType::Frosted;

    // Attach frame automatically
    if (type == DoorType::Left) {
        auto bars = std::make_unique<Door>(this, DoorType::FrameLeft, scene);
        bars -> scale.x = 1.03f;
        bars -> position.x = -1.f;
        Door* bar = bars.get();
        scene.phongObjects.push_back(bar);
        childObjects.push_back(std::move(bars));
    }

    if (type == DoorType::Right) {
        auto bars = std::make_unique<Door>(this, DoorType::FrameRight, scene);
        bars -> scale.x = 1.03f;
        bars -> position.x = 1.f;
        Door* bar = bars.get();
        scene.phongObjects.push_back(bar);
        childObjects.push_back(std::move(bars));
    }

}
bool Door::update(Scene &scene, float time, float dt,
                  glm::mat4 parentModelMatrix, glm::vec3 parentRotation)
{
    if (type == DoorType::Left || type == DoorType::Right) {

        float baseZ = 54.7f;
        float baseX =
            (type == DoorType::Left)
            ? -134.6f
            : -117.3f;

        float baseY = position.y;

        glm::vec3 finalPos(baseX, baseY, baseZ);

        if (state == DoorState::Opening || state == DoorState::Closing) {

            animTime += dt / animDuration;
            animTime = glm::clamp(animTime, 0.0f, 1.0f);

            float t = glm::smoothstep(0.0f, 1.0f, animTime);

            float dirX = (type == DoorType::Left) ? -1.0f : 1.0f;

            float slideT = (state == DoorState::Opening) ? t : (1.0f - t);

            float slideX = dirX * maxSlideX * slideT;
            float slideZ = maxSlideZ * slideT;

            finalPos.x = baseX + slideX;
            finalPos.z = baseZ - slideZ;

            if (animTime >= 1.0f) {
                state = (state == DoorState::Opening)
                        ? DoorState::Open
                        : DoorState::Closed;
                animTime = 0.0f;
            }

            modelMatrix =
                parentModelMatrix *
                glm::translate(glm::mat4(1.0f), finalPos) *
                glm::orientate4(rotation) *
                glm::scale(glm::mat4(1.0f), scale);

            position = finalPos;
            return true;
        }

        if (state == DoorState::Open) {
            float dirX = (type == DoorType::Left) ? -1.0f : 1.0f;
            float slideX = dirX * maxSlideX;
            float slideZ = maxSlideZ;

            finalPos.x = baseX + slideX;
            finalPos.z = baseZ - slideZ;
        }
        else {
            finalPos.x = baseX;
            finalPos.z = baseZ;
        }

        modelMatrix =
            parentModelMatrix *
            glm::translate(glm::mat4(1.0f), finalPos) *
            glm::orientate4(rotation) *
            glm::scale(glm::mat4(1.0f), scale);

        position = finalPos;
        return true;
    }

    modelMatrix =
        parentModelMatrix *
        glm::translate(glm::mat4(1.0f), position) *
        glm::orientate4(rotation) *
        glm::scale(glm::mat4(1.0f), scale);

    return true;
}




void Door::open() {
    state = DoorState::Opening;
}

void Door::close() {
    state = DoorState::Closing;
}



void Door::render(Scene &scene, ppgso::Shader& shader) {
    shader.setUniform("ModelMatrix", modelMatrix);
    switch (type) {
        case DoorType::Left:
            shader.setUniform("UseTexture", false);
            shader.setUniform("ObjectColor", glm::vec3(0.85f, 0.85f, 0.9f));
            shader.setUniform("Transparency", 0.75f);
            meshLeftDoor->render();
            break;

        case DoorType::Right:
            shader.setUniform("UseTexture", false);
            shader.setUniform("ObjectColor", glm::vec3(0.85f, 0.85f, 0.9f));
            shader.setUniform("Transparency", 0.95f);

            meshRightDoor->render();
            break;

        case DoorType::FrameLeft:
            shader.setUniform("UseTexture", true);
            barsTexture->bind(0);
            shader.setUniform("Texture", 0);
            meshLeftBars->render();
            break;

        case DoorType::FrameRight:
            shader.setUniform("UseTexture", true);
            barsTexture->bind(0);
            shader.setUniform("Texture", 0);
            meshRightBars->render();
            break;
    }
}
