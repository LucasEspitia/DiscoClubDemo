#include "LightEmitter.h"
#include "../../scene/Scene.h"

std::unique_ptr<ppgso::Mesh>  LightEmitter::meshEmitter;

LightEmitter::LightEmitter(Object *parent, Scene &scene, glm::vec3 color, float strength){
    parentObject = parent;

    position = {0, 0, 0};
    rotation = {0, 0, 0};
    scale = {0.1f, 0.1f, 0.1f};

    emissiveColor = color;
    emissiveStrength = strength;

    if (!meshEmitter) {
        meshEmitter = std::make_unique<ppgso::Mesh>("data/objects/cube.obj");
    }
}

bool LightEmitter::update(Scene &scene, float time, float dt, glm::mat4 parentModelMatrix, glm::vec3 parentRotation) {
    generateModelMatrix(parentModelMatrix);
    return true;
}


void LightEmitter::render(Scene &scene, ppgso::Shader& shader) {
    shader.setUniform("ModelMatrix", modelMatrix);
    shader.setUniform("UseTexture", false);
    shader.setUniform("EmissiveColor", emissiveColor);
    shader.setUniform("EmissiveStrength", emissiveStrength);
    meshEmitter->render();

    shader.setUniform("EmissiveColor", glm::vec3(0.0f));
    shader.setUniform("EmissiveStrength", 0.f);
}