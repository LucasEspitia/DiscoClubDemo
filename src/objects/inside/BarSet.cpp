#include "BarSet.h"
#include "src/scene/Scene.h"
#include "lamps/Lamp.h"


std::unique_ptr<ppgso::Mesh>  BarSet::meshBar;
std::unique_ptr<ppgso::Mesh> BarSet::meshShelf;
std::unique_ptr<ppgso::Mesh> BarSet::meshChair;
//std::unique_ptr<ppgso::Mesh> Bar::meshCan;
//std::unique_ptr<ppgso::Mesh> Bar::meshBottle;



BarSet::BarSet(Object *parent,  BarType typeSet, Scene &scene) {
    parentObject = parent;
    type = typeSet;

    position = {0,0,0};
    rotation = {0,0,0};
    scale = {1.0f,1.0f,1.0f};

    if (!meshBar) {
        meshBar = std::make_unique<ppgso::Mesh>("data/objects/inside/Bar.obj");
        meshBar->texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/barWood.bmp"));
        meshBar->normalMap = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/barWoodNormal.bmp"));
    }
    if (!meshShelf) {
        meshShelf = std::make_unique<ppgso::Mesh>("data/objects/inside/Shelf.obj");
        meshShelf->texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/shelfWood.bmp"));
        meshShelf->normalMap = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/shelfWoodNormal.bmp"));
    }
    if (!meshChair) {
         //meshChair = std::make_unique<ppgso::Mesh>("data/objects/inside/chair.obj");
         //meshChair->texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/blueCotton.bmp"));
    }

    // If this is a Bar, create child CHAIRS
    switch (type) {
        case Bar:
            generateShelves(scene);
            generateLamps(scene);
            this->material = MaterialType::Wood;
            break;
        case Shelf:
            this->material = MaterialType::Wood;
            break;
        case ChairBar:
            this->material = MaterialType::Wood;
            break;
    }



}
void BarSet::generateLamps(Scene& scene) {

    //Warm Lamp
    LampConfig warmLamp {
         glm::vec3(1.0f, 0.85f, 0.6f),
        2.5f,
        0.025f,
        0.03f
    };

    auto lamp1 = std::make_unique<Lamp>(this, scene, LampType::Body, warmLamp);
    lamp1->position = glm::vec3(3.f, 0.f, -30.f);
    Lamp* lamp1Ptr = lamp1.get();
    scene.phongObjects.push_back(lamp1Ptr);
    childObjects.push_back(std::move(lamp1));

    LampConfig coolLamp {
        glm::vec3(1.2f, 1.6f, 3.2f),
       4.5f,
        0.025f,
        0.03f
    };

    auto lamp2 = std::make_unique<Lamp>(this, scene, LampType::Body, coolLamp);
    lamp2->position = glm::vec3(2.f, 0.f, 10.f);
    Lamp* lamp2Ptr = lamp2.get();
    scene.phongObjects.push_back(lamp2Ptr);
    childObjects.push_back(std::move(lamp2));

}
void BarSet::generateShelves(Scene &scene) {
    const float spacing = 7.0f;
    const float baseHeight = 8.f;

    for (int i = 0; i < 3; i++) {
        auto shelf = std::make_unique<BarSet>(this, BarType::Shelf, scene);
        shelf->position = {25.0f, baseHeight + i * spacing, -14.f};
        shelf->rotation = glm::vec3(glm::radians(0.f), 0.f, glm::radians(-45.f));
        shelf->scale = {1.1f, 1.0f, 1.0f};
        BarSet* shelvePtr = shelf.get();
        scene.phongObjects.push_back(shelvePtr);
        childObjects.push_back(std::move(shelf));
    }
}

bool BarSet::update(Scene &scene, float time, float dt, glm::mat4 parentModelMatrix, glm::vec3 parentRotation) {
    generateModelMatrix(parentModelMatrix);
    return true;
}


void BarSet::render(Scene &scene, ppgso::Shader& shader) {
    shader.setUniform("ModelMatrix", modelMatrix);
    applyMaterial(shader);
    switch (this->type) {
        case Bar:
            shader.setUniform("UseTexture", true);
            meshBar->texture->bind(0);
            shader.setUniform("Texture", 0);
            if (meshBar->normalMap) {
                meshBar->normalMap->bind(1);
                shader.setUniform("NormalMap", 1);
                shader.setUniform("UseNormalMap", true);
            }
            meshBar->render();
            break;
        case Shelf:
            shader.setUniform("UseTexture", true);
            meshShelf->texture->bind(0);
            shader.setUniform("Texture", 0);
            if (meshShelf->normalMap) {
                meshShelf->normalMap->bind(1);
                shader.setUniform("NormalMap", 1);
                shader.setUniform("UseNormalMap", true);
            }
            meshShelf->render();
            break;
        case ChairBar:
            shader.setUniform("UseTexture", true);
            meshChair->texture->bind(0);
            shader.setUniform("Texture", 0);
            meshChair->render();
            break;
        case Bottle:
            break;;
        case Can:
            break;
    }
}

void BarSet::renderDepth(Scene &scene, ppgso::Shader &shader) {
    shader.setUniform("ModelMatrix", modelMatrix);
    switch (this->type) {
        case Bar:
            if (meshBar)
                meshBar->render();
            break;
        case Shelf:
            if (meshShelf)
                meshShelf->render();
            break;
        case ChairBar:
            if (meshChair)
                 meshChair->render();
            break;
        case Bottle:
            break;;
        case Can:
            break;
    }
}
