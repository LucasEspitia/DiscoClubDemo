#include "TableSet.h"
#include <shaders/phong_vert_glsl.h>
#include <shaders/phong_frag_glsl.h>

#include "../../scene/Scene.h"
#include "lamps/LavaLamp.h"

std::unique_ptr<ppgso::Mesh> TableSet::meshTable;
std::unique_ptr<ppgso::Mesh> TableSet::meshChair;
std::unique_ptr<ppgso::Mesh> TableSet::meshSofa;



TableSet::TableSet(Object *parent,  TypeTableSet typeSet, Scene &scene) {
    parentObject = parent;
    type = typeSet;

    position = {0,0,0};
    rotation = {0,0,0};
    scale = {1.0f,1.0f,1.0f};

    if (!meshTable) {
        meshTable = std::make_unique<ppgso::Mesh>("data/objects/inside/Table.obj");
        meshTable->texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/woodTable.bmp"));
    }
    if (!meshChair) {
        meshChair = std::make_unique<ppgso::Mesh>("data/objects/inside/chair.obj");
        meshChair->texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/denimFabric.bmp"));
        meshChair->normalMap = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/denimFabricNormal.bmp"));
    }

    if (!meshSofa) {
        meshSofa = std::make_unique<ppgso::Mesh>("data/objects/inside/sofa.obj");
        meshSofa->texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/denimFabric.bmp"));
        meshSofa->normalMap = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/denimFabricNormal.bmp"));

    }
    if (type == Table) {
        this->material = MaterialType::Wood;
        // Add chairs
        auto chairLeft = std::make_unique<TableSet>(this, Chair, scene);
        chairLeft->position = {-14.f,0.f,9.5f};
        chairLeft->rotation = glm::vec3(0.f, 0.f, glm::radians(-45.f));
        TableSet* chairLPtr = chairLeft.get();
        scene.phongObjects.push_back(chairLPtr);
        childObjects.push_back(move(chairLeft));

        auto chairRight = std::make_unique<TableSet>(this, Chair, scene);
        chairRight->position = {14.f,0.f,9.5f};
        chairRight->rotation = glm::vec3(0.f, 0.f, glm::radians(45.f));
        TableSet* chairRPtr = chairRight.get();
        scene.phongObjects.push_back(chairRPtr);
        childObjects.push_back(move(chairRight));

        // Add sofa
        auto sofa = std::make_unique<TableSet>(this, Sofa, scene);
        TableSet* sofaPtr = sofa.get();
        scene.phongObjects.push_back(sofaPtr);
        sofa->position = {0.f,0.f,-16.f};
        childObjects.push_back(move(sofa));

        //Add Lava lamps
        std::vector<glm::vec3> lavaColors = {
            {1.4f, 0.4f, 0.25f},
            {0.3f, 1.0f, 0.55f},
            {0.45f, 0.55f, 1.2f}
        };
        int randomIndex = rand() % lavaColors.size();
        glm::vec3 chosenColor = lavaColors[randomIndex];
        float emissive = 2.0f;

        auto lamp = std::make_unique<LavaLamp>(this, scene,LavaBase, chosenColor, emissive);
        LavaLamp* lavaLamp = lamp.get();
        scene.phongObjects.push_back(lavaLamp);
        childObjects.push_back(move(lamp));
    }
    if (type == Chair || type == Sofa) {
        this->material = MaterialType::Gold;
    }

}

bool TableSet::update(Scene &scene, float time, float dt, glm::mat4 parentModelMatrix, glm::vec3 parentRotation) {
    generateModelMatrix(parentModelMatrix);
    return true;
}


void TableSet::render(Scene &scene, ppgso::Shader& shader) {
    shader.setUniform("ModelMatrix", modelMatrix);
    shader.setUniform("UseNormalMap", false);
    applyMaterial(shader);

    switch (this->type) {
        case Chair:
            shader.setUniform("UseTexture", true);
            meshChair->texture->bind(0);
            shader.setUniform("Texture", 0);
            if (meshChair->normalMap) {
                meshChair->normalMap->bind(1);
                shader.setUniform("NormalMap", 1);
                shader.setUniform("UseNormalMap", true);
            }
            meshChair->render();
            break;
        case Sofa:
            shader.setUniform("UseTexture", true);
            meshChair->texture->bind(0);

            shader.setUniform("Texture", 0);
            if (meshSofa->normalMap) {
                meshSofa->normalMap->bind(1);
                shader.setUniform("NormalMap", 1);
                shader.setUniform("UseNormalMap", true);
            }
            meshSofa->render();
            break;
        case Table:
            shader.setUniform("UseTexture", true);
            meshTable->texture->bind(0);
            shader.setUniform("Texture", 0);
            meshTable->render();
            break;
    }
}

void TableSet::renderDepth(Scene &scene, ppgso::Shader &shader) {
    shader.setUniform("ModelMatrix", modelMatrix);
    switch (this->type) {
        case Chair:
            if (meshChair)
                meshChair->render();
            break;
        case Sofa:
            if (meshSofa)
                meshSofa->render();
            break;
        case Table:
            if (meshTable)
                meshTable->render();
            break;
    }
}
