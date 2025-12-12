#include "DjSet.h"

#include "ConfettiCannon.h"
#include "../../scene/Scene.h"
#include <glm/gtx/string_cast.hpp>

std::unique_ptr<ppgso::Mesh>  DjSet::meshSet;
std::unique_ptr<ppgso::Mesh> DjSet::meshTable;



DjSet::DjSet(Object *parent,  DjType typeSet, Scene &scene) {
    parentObject = parent;
    type = typeSet;

    position = {0,0,0};
    rotation = {0,0,0};
    scale = {1.0f,1.0f,1.0f};

    if (!meshSet) {
        meshSet = std::make_unique<ppgso::Mesh>("data/objects/djSet/DjSet.obj");
        meshSet->texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/dj.bmp"));
    }
    if (!meshTable) {
        //meshTable = std::make_unique<ppgso::Mesh>("data/objects/inside/Shelf.obj");
        //meshTable->texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/shelfWood.bmp"));
    }
    if (type == Set) {
        this->material = MaterialType::Glossy;
        auto cannonR = std::make_unique<ConfettiCannon>(this, CanonType::Right, scene);
        cannonR->position = glm::vec3(30.f, 0.0f, -2.f);
        cannonR->rotation.z = glm::radians(25.f);
        cannonRPtr = cannonR.get();
        scene.phongObjects.push_back(cannonRPtr);
        childObjects.push_back(std::move(cannonR));

        auto cannonL = std::make_unique<ConfettiCannon>(this, CanonType::Left, scene);
        cannonL->position = glm::vec3(-30.f, 0.0f, -2.f);
        cannonL->rotation.z = glm::radians(-25.f);
        cannonLPtr = cannonL.get();
        scene.phongObjects.push_back(cannonLPtr);
        childObjects.push_back(std::move(cannonL));


    }

    if (type == DjType::TableDj) {
        //ToDo -> Generate child -> Computer
    }


}

bool DjSet::update(Scene &scene, float time, float dt, glm::mat4 parentModelMatrix, glm::vec3 parentRotation) {
    generateModelMatrix(parentModelMatrix);
    return true;
}


void DjSet::render(Scene &scene, ppgso::Shader& shader) {
    shader.setUniform("ModelMatrix", modelMatrix);
    switch (this->type) {
        case DjType::Set:
            shader.setUniform("UseTexture", true);
            meshSet->texture->bind(0);
            shader.setUniform("Texture", 0);
            meshSet->render();
            break;
        case TableDj:
            break;
            shader.setUniform("UseTexture", true);
            meshTable->texture->bind(0);
            shader.setUniform("Texture", 0);
            meshTable->render();
    }
}
void DjSet::renderDepth(Scene &scene, ppgso::Shader &shader) {
    shader.setUniform("ModelMatrix", modelMatrix);
    switch (type) {
        case Set:
            if (meshSet)
                meshSet->render();
            break;

        case TableDj:
            if (meshTable)
                meshTable->render();
            break;
    }
}
