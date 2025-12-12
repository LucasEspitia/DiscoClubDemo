#define GLM_ENABLE_EXPERIMENTAL
#include "Object.h"
#include <glm/gtx/euler_angles.hpp>

bool Object::update(Scene &scene, float time, float dt,
                    glm::mat4 parentModelMatrix, glm::vec3 parentRotation)
{
    generateModelMatrix(parentModelMatrix);
    return true;
}

bool Object::updateChildren(Scene &scene, float time, float dt,
                            glm::mat4 parentModelMatrix, glm::vec3 parentRotation)
{
    bool keep = update(scene, time, dt, parentModelMatrix, parentRotation);

    globalRotation = parentRotation + rotation;

    auto it = childObjects.begin();
    while (it != childObjects.end()) {
        auto *child = it->get();
        if (!child->updateChildren(scene, time, dt, modelMatrix, globalRotation))
            it = childObjects.erase(it);
        else
            ++it;
    }

    return keep;
}

void Object::renderChildren(Scene &scene, ppgso::Shader& shader) {
    for (auto &child : childObjects)
        child->render(scene, shader),
        child->renderChildren(scene, shader);
}


void Object::renderDepth(Scene &scene, ppgso::Shader &shader) {}

bool Object::renderDepthChildren(Scene &scene, ppgso::Shader &shader) {
    if (castsShadow)
        this->renderDepth(scene, shader);

    for (auto& obj : childObjects) {
        if (obj->castsShadow)
           // std::cout << "Obj: " << obj << " castsShadow = " << obj->castsShadow << "\n";
            obj->renderDepthChildren(scene, shader);
    }

    return true;
}
void Object::applyMaterial(ppgso::Shader& shader)
{
    switch(material) {

        case MaterialType::Matte:
            diffuseStrength = 1.0f;
            specularStrength = 0.05f;
            shininess = 8.0f;
            lightAbsorption = 0.75f;
            break;

        case MaterialType::Glossy:
            diffuseStrength = 0.8f;
            specularStrength = 0.5f;
            shininess = 32.0f;
            lightAbsorption = 0.2f;
            break;

        case MaterialType::Metal:
            diffuseStrength = 0.4f;
            specularStrength = 1.0f;
            shininess = 128.0f;
            lightAbsorption = 0.1f;
            break;

        case MaterialType::Wood:
            diffuseStrength = 0.9f;
            specularStrength = 0.05f;
            shininess = 6.0f;
            lightAbsorption = 0.85f;
            break;

        case MaterialType::Rubber:
            diffuseStrength = 0.9f;
            specularStrength = 0.05f;
            shininess = 4.0f;
            lightAbsorption = 0.7f;
            break;

        case MaterialType::Frosted:
            diffuseStrength = 0.5f;
            specularStrength = 0.3f;
            shininess = 16.0f;
            lightAbsorption = 0.6f;
            break;

        case MaterialType::Emissive:
            diffuseStrength = 0.0f;
            specularStrength = 0.0f;
            shininess = 0.0f;
            lightAbsorption = 0.0f;
            break;
        case MaterialType::Gold:
            diffuseStrength = 0.3f;
            specularStrength = 0.9f;
            shininess = 64.0f;
            lightAbsorption = 0.05f;
        case MaterialType::DiscoBall:
            diffuseStrength = 0.2f;
            specularStrength = 1.2f;
            shininess = 128.0f;
            lightAbsorption = 0.05f;

        case MaterialType::Default:
            diffuseStrength = 1.0f;
            specularStrength = 1.0f;
            shininess = 32.0f;
            lightAbsorption = 1.0f;
    }

     shader.setUniform("DiffuseStrength", diffuseStrength);
     shader.setUniform("SpecularStrength", specularStrength);
     shader.setUniform("shininess", shininess);
     shader.setUniform("LightAbsorption", lightAbsorption);
}

void Object::generateModelMatrix(glm::mat4 parentModelMatrix) {
    modelMatrix =
        parentModelMatrix *
        glm::translate(glm::mat4(1), position) *
        glm::orientate4(rotation) *
        glm::scale(glm::mat4(1), scale);
}


