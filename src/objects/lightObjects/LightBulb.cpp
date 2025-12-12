#include "LightBulb.h"
#include "LightEmitter.h"
#include "../../scene/Scene.h"

std::unique_ptr<ppgso::Mesh>  LightBulb::meshBulb;

LightBulb::LightBulb(Object *parent, Scene &scene, glm::vec3 color, float strength){
    parentObject = parent;

    position = {0, 0, 0};
    rotation = {0, 0, 0};
    scale = {1.f, 1.f, 1.f};

    emissiveColor = color;
    emissiveStrength = strength;

    if (!meshBulb) {
        meshBulb = std::make_unique<ppgso::Mesh>("data/objects/inside/lamp/lightBulb.obj");
    }
    isTransparent = true;

    //Add emitter
    auto light = std::make_unique<Light>(LightType::Point);

    light->ambient = color * 0.1f;
    light->diffuse = color;
    light->specular = color;

    light->constant = 1.0f;
    light->linear = 0.0014f;
    light->quadratic = 0.007f;

    scene.lights.push_back(std::move(light));
    bulbLight = scene.lights.back().get();

    // add emitter
    emitter = scene.addEmitterForSpot(*bulbLight);
}

bool LightBulb::update(Scene &scene, float time, float dt, glm::mat4 parentModelMatrix, glm::vec3 parentRotation)
{
    generateModelMatrix(parentModelMatrix);

    if (bulbLight) {
        glm::vec3 bulbPos = glm::vec3(modelMatrix * glm::vec4(-0.206f, 11.6f,-1.2048f,1));
        bulbLight->position = bulbPos;
    }

    if (emitter) {
        emitter->position = bulbLight->position;
        emitter->scale = glm::vec3(0.07f);
    }

    return true;
}


void LightBulb::render(Scene &scene, ppgso::Shader& shader) {
    shader.setUniform("ModelMatrix", modelMatrix);
    shader.setUniform("UseTexture", false);
    shader.setUniform("EmissiveColor", emissiveColor * 0.2f);
    shader.setUniform("EmissiveStrength", emissiveStrength);
    shader.setUniform("Transparency", 0.2f);
    meshBulb->render();
    shader.setUniform("EmissiveColor", glm::vec3(0.0f));
    shader.setUniform("EmissiveStrength", 0.0f);

}