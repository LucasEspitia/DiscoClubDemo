#include "Club.h"
#include "../scene/Scene.h"

std::unique_ptr<ppgso::Mesh> Club::meshWalls;
std::unique_ptr<ppgso::Mesh> Club::meshFloor;
std::unique_ptr<ppgso::Mesh> Club::meshRedCarpet;
std::unique_ptr<ppgso::Mesh> Club::meshCeiling;
std::unique_ptr<ppgso::Mesh> Club::meshWoodDecoration;
std::unique_ptr<ppgso::Mesh> Club::meshLadyDecoration;
std::unique_ptr<ppgso::Mesh> Club::meshGoldDecoration;

Club::Club(Object* object, ClubType type, Scene& scene) {
    parentObject = object;
    this->clubType = type;

    position = {0,0,0};
    rotation = {0,0,0};
    scale = {1.0f,1.0f,1.0f};

    if (!meshWalls) meshWalls = std::make_unique<ppgso::Mesh>("data/objects/clubDecoration/walls.obj");
    if (!meshFloor) meshFloor = std::make_unique<ppgso::Mesh>("data/objects/clubDecoration/floor.obj");
    if (!meshRedCarpet) meshRedCarpet = std::make_unique<ppgso::Mesh>("data/objects/clubDecoration/redCarpet.obj");
    if (!meshWoodDecoration) meshWoodDecoration = std::make_unique<ppgso::Mesh>("data/objects/clubDecoration/woodDecoration.obj");
    if (!meshLadyDecoration) meshLadyDecoration = std::make_unique<ppgso::Mesh>("data/objects/clubDecoration/DecorationLady.obj");
    if (!meshGoldDecoration) meshGoldDecoration = std::make_unique<ppgso::Mesh>("data/objects/clubDecoration/DecorationWallGold.obj");
    if (!meshCeiling) meshCeiling = std::make_unique<ppgso::Mesh>("data/objects/clubDecoration/Ceiling.obj");

    //Set Textres
    meshWalls->texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/metal.bmp"));
    meshRedCarpet->texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/redCarpet.bmp"));
    meshWoodDecoration->texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/woodB.bmp"));
    meshFloor->texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/floor.bmp"));
    meshCeiling->texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/ceiling.bmp"));

    //Set Normal maps
    meshFloor->normalMap = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/floorNormal.bmp"));
    meshRedCarpet->normalMap = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/redCarpetNormal.bmp"));
    meshWoodDecoration->normalMap = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/woodBNormal.bmp"));
    meshWalls->normalMap = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/metalNormal.bmp"));


    if (clubType == ClubWalls) {
        auto floor = std::make_unique<Club>(this, ClubFloor, scene);
        Club* floorPtr = floor.get();
        scene.phongObjects.push_back(floorPtr);
        childObjects.push_back(move(floor));

        auto ceiling = std::make_unique<Club>(this, ClubCeiling, scene);
        Club* ceilingPtr = ceiling.get();
        ceilingPtr->castsShadow = false;
        scene.phongObjects.push_back(ceilingPtr);
        childObjects.push_back(move(ceiling));

        auto redCarpet = std::make_unique<Club>(this, ClubRedCarpet, scene);
        Club* redCarpetPtr = redCarpet.get();
        scene.phongObjects.push_back(redCarpetPtr);
        childObjects.push_back(move(redCarpet));

        auto woodDecoration = std::make_unique<Club>(this, ClubWoodDecoration, scene);
        Club* woodDecorationPtr = woodDecoration.get();
        scene.phongObjects.push_back(woodDecorationPtr);
        childObjects.push_back(move(woodDecoration));

        auto lady = std::make_unique<Club>(this, ClubLadyDecoration, scene);
        Club* ladyPtr = lady.get();
        scene.phongObjects.push_back(ladyPtr);
        childObjects.push_back(move(lady));

        // Spotlight -> Red Neon Lady
        auto neon = std::make_unique<Light>(LightType::Spot);
        neon->position     = glm::vec3(-107.4, 17.6, 102.5f);
        neon->spotDirection = glm::normalize(glm::vec3(-1.f, 0.2f, 0.f));

        neon->cutOff = glm::cos(glm::radians(20.0f));
        neon->outerCutOff = glm::cos(glm::radians(40.0f));

        neon->ambient  = glm::vec3(0.04f, 0.01f, 0.01f);
        neon->diffuse  = glm::vec3(1.0f, 0.1f, 0.3f);
        neon->specular = glm::vec3(1.0f, 0.2f, 0.4f);

        neon->constant  = 1.0f;
        neon->linear    = 0.07f;
        neon->quadratic = 0.002;

        neon->baseAmbient = neon->ambient;
        neon->baseDiffuse = neon->diffuse;
        neon->baseSpecular = neon->specular;
        scene.neonLight = neon.get();
        scene.lights.push_back(std::move(neon));

        auto gold = std::make_unique<Club>(this, ClubWallGoldDecoration, scene);
        Club* goldPtr = gold.get();
        scene.phongObjects.push_back(goldPtr);
        childObjects.push_back(move(gold));


    }
    switch (clubType) {
        case ClubCeiling:
            this->material = MaterialType::Matte;
            break;
        case ClubWalls:
            this->material = MaterialType::Matte;
            break;
        case ClubRedCarpet:
            this->material = MaterialType::Glossy;
            break;
        case ClubWoodDecoration:
            this->material = MaterialType::Wood;
            break;
        case ClubLadyDecoration:
            break;
        case ClubWallGoldDecoration:
            this->material = MaterialType::Gold;
            break;
        case ClubFloor:
            this->material = MaterialType::Frosted;
            break;
    }
}

bool Club::update(Scene &scene, float time, float dt, glm::mat4 parentModelMatrix, glm::vec3 parentRotation) {
    if (clubType == ClubLadyDecoration) {
        emissiveStrength = scene.neonFlickValue * 2.0f;
    }
    generateModelMatrix(parentModelMatrix);
    return true;
}

void Club::render(Scene &scene, ppgso::Shader& shader) {
    shader.setUniform("ModelMatrix", modelMatrix);
    shader.setUniform("EmissiveColor", glm::vec3(0.0f));
    shader.setUniform("UseNormalMap", false);
    applyMaterial(shader);
    switch (this->clubType) {
        case ClubWalls:
            if (meshWalls && meshWalls->texture) {
                shader.setUniform("UseTexture", true);
                meshWalls->texture->bind(0);
                shader.setUniform("Texture", 0);
                if (meshWalls->normalMap) {
                     //meshWalls->normalMap->bind(1);
                     //shader.setUniform("NormalMap", 1);
                     //shader.setUniform("UseNormalMap", true);
                }
            }
            meshWalls->render();
            break;

        case ClubFloor:
            if (meshFloor && meshFloor->texture) {
                 shader.setUniform("UseTexture", true);
                 meshFloor->texture->bind(0);
                 shader.setUniform("Texture", 0);
                if (meshFloor->normalMap) {
                    meshFloor->normalMap->bind(1);
                    shader.setUniform("NormalMap", 1);
                    shader.setUniform("UseNormalMap", true);
                }
            }
            meshFloor->render();
            break;
        case ClubCeiling:
            if (meshCeiling && meshCeiling->texture) {
                renderMesh(meshCeiling.get(), shader);
            }
            break;
        case ClubRedCarpet:
            if (meshRedCarpet && meshRedCarpet->texture) {
                shader.setUniform("UseTexture", true);
                meshRedCarpet->texture->bind(0);
                shader.setUniform("Texture", 0);
                if (meshRedCarpet->normalMap) {
                    meshRedCarpet->normalMap->bind(1);
                    shader.setUniform("NormalMap", 1);
                    shader.setUniform("UseNormalMap", true);
                }
                meshRedCarpet->render();
            }
            break;

        case ClubWoodDecoration:
            if (meshWoodDecoration && meshWoodDecoration->texture) {
                shader.setUniform("UseTexture", true);
                meshWoodDecoration->texture->bind(0);
                shader.setUniform("Texture", 0);
                if (meshWoodDecoration->normalMap) {
                    meshWoodDecoration->normalMap->bind(1);
                    shader.setUniform("NormalMap", 1);
                    shader.setUniform("UseNormalMap", true);
                }
                meshWoodDecoration->render();
            }
            break;
        case ClubLadyDecoration:
            if (meshLadyDecoration) {
                shader.setUniform("UseTexture", false);
                shader.setUniform("ObjectColor", glm::vec3(0.05f, 0.0f, 0.0f));
                shader.setUniform("EmissiveColor", glm::vec3(4.0f, 0.0f, 1.0f));
                shader.setUniform("EmissiveStrength", this->emissiveStrength);
                meshLadyDecoration->render();
            }
            break;

        case ClubWallGoldDecoration:
            if (meshGoldDecoration) {
                glDisable(GL_CULL_FACE);
                shader.setUniform("UseTexture", false);
                shader.setUniform("ObjectColor", glm::vec3(0.83f, 0.63f, 0.22f));
                shader.setUniform("EmissiveColor", glm::vec3(0.83f, 0.63f, 0.22f));
                shader.setUniform("EmissiveStrength", 0.05f);

                meshGoldDecoration->render();
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
            }
            break;
        default:
            break;
    }
}

void Club::renderMesh(ppgso::Mesh* mesh, ppgso::Shader& shader) {
    if (!mesh || !mesh->texture) return;
    shader.setUniform("UseTexture", true);
    mesh->texture->bind(0);
    shader.setUniform("Texture", 0);
    mesh->render();
}

void Club::renderDepth(Scene&, ppgso::Shader &shader) {
    switch (clubType) {
        case ClubWalls:
            if (meshWalls)
               // meshWalls->render();
            break;

        case ClubFloor:
            if (meshFloor)
                //meshFloor->render();
            break;

        case ClubCeiling:
            if (meshCeiling)
               // meshCeiling->render();
            break;

        case ClubRedCarpet:
            if (meshRedCarpet)
               // meshRedCarpet->render();
            break;

        case ClubWoodDecoration:
            if (meshWoodDecoration)
                 //meshWoodDecoration->render();
            break;

        case ClubLadyDecoration:
            if (meshLadyDecoration)
              //  meshLadyDecoration->render();
            break;

        default:
            break;

    }
}
