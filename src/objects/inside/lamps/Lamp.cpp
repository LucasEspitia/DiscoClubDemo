#include "Lamp.h"
#include "src/objects/lightObjects/LightBulb.h"
#include "src/objects/lightObjects/LightEmitter.h"
#include "src/scene/Scene.h"

std::unique_ptr<ppgso::Mesh>  Lamp::meshBody;
std::unique_ptr<ppgso::Mesh>  Lamp::meshOuter;
std::unique_ptr<ppgso::Mesh>  Lamp::meshShade;

Lamp::Lamp(Object *parent, Scene &scene, LampType typeLamp, LampConfig cfg)
{
    parentObject = parent;
    type = typeLamp;
    config = cfg;

    position = {0,0,0};
    rotation = {0,0,0};
    scale = {1.0f, 1.0f, 1.0f};

    if (!meshBody) meshBody = std::make_unique<ppgso::Mesh>("data/objects/inside/lamp/body.obj");
    if (!meshOuter) meshOuter = std::make_unique<ppgso::Mesh>("data/objects/inside/lamp/outer.obj");
    if (!meshShade) meshShade = std::make_unique<ppgso::Mesh>("data/objects/inside/lamp/shade.obj");

    //Only bodies creates children and Lights
    if (type == LampType::Body) {
        this->material = MaterialType::Glossy;
        //Outer
        auto outer = std::make_unique<Lamp>(this, scene, LampType::Outer, config);
        Lamp* outerPtr = outer.get();
        scene.phongObjects.push_back(outerPtr);
        childObjects.push_back(std::move(outer));

        //Shade
        auto shade = std::make_unique<Lamp>(this, scene, LampType::Shade, config);
        Lamp* shaderPtr = shade.get();
        scene.phongObjects.push_back(shaderPtr);
        childObjects.push_back(std::move(shade));

        //Bulb mesh
        auto lightBulb = std::make_unique<LightBulb>(this, scene, config.color, config.strength);
        LightBulb* bulbPtr = lightBulb.get();
        scene.phongObjects.push_back(bulbPtr);
        childObjects.push_back(std::move(lightBulb));
    }
    if (type == LampType::Outer) {
        this->material = MaterialType::Metal;
    }
    if (type == LampType::Shade) {
        this->isTransparent = true;
        this->material = MaterialType::Default;
    }
}

bool Lamp::update(Scene &scene, float time, float dt, glm::mat4 parentModelMatrix, glm::vec3 parentRotation)
{
    generateModelMatrix(parentModelMatrix);
    return true;
}

void Lamp::render(Scene &scene, ppgso::Shader &shader)
{
    shader.setUniform("ModelMatrix", modelMatrix);
    shader.setUniform("UseTexture", false);
    applyMaterial(shader);

    switch (type)
    {
        case LampType::Body:
            shader.setUniform("ObjectColor", glm::vec3(0.1f, 0.1f, 0.1f));
            meshBody->render();
            break;

        case LampType::Outer:
            shader.setUniform("ObjectColor", glm::vec3(0.05f, 0.05f, 0.05f));
            meshOuter->render();
            break;

        case LampType::Shade:
            shader.setUniform("ObjectColor", glm::vec3(0.2f, 0.18f, 0.15f));

            shader.setUniform("EmissiveColor", config.color * 0.4f);
            shader.setUniform("EmissiveStrength", 0.5f);
            shader.setUniform("Transparency", 0.7f);
            meshShade->render();

            shader.setUniform("EmissiveColor", glm::vec3(0));
            shader.setUniform("EmissiveStrength", 0);
            shader.setUniform("Transparency", 1);

            break;

        default:
            break;
    }
}

void Lamp::renderDepth(Scene &scene, ppgso::Shader &shader) {
    shader.setUniform("ModelMatrix", modelMatrix);
    switch (type)
    {
        case LampType::Body:
            meshBody->render();
            break;

        case LampType::Outer:
            meshOuter->render();
            break;

        case LampType::Shade:
            meshShade->render();
            break;
        default:
            break;
    }
}
