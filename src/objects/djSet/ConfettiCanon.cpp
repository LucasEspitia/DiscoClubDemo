#include "ConfettiCannon.h"
#include "ConfettiSystem.h"
#include "src/scene/Scene.h"

std::unique_ptr<ppgso::Mesh> ConfettiCannon::meshBaseCanon;
std::unique_ptr<ppgso::Mesh> ConfettiCannon::meshCanon;
std::unique_ptr<ppgso::Mesh> ConfettiCannon::meshCoverCanon;

ConfettiCannon::ConfettiCannon(Object *parent, CanonType typeSet, Scene &scene) {
    parentObject = parent;
    type = typeSet;

    position = {0,0,0};
    rotation = {0,0,0};
    scale = {1,1,1};

    headRotation.x = glm::radians(65.0f);


    if (!meshBaseCanon) {
        meshBaseCanon = std::make_unique<ppgso::Mesh>("data/objects/djSet/BaseCannon.obj");
        meshBaseCanon->texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/shelfWood.bmp"));
    }

    if (!meshCanon) {
        meshCanon = std::make_unique<ppgso::Mesh>("data/objects/djSet/Cannon.obj");
            std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/shelfWood.bmp"));
        meshCanon->texture =  std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/blueMetal.bmp"));
        meshCanon->normalMap =  std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/blueMetalNormal.bmp"));

    }

    if (!meshCoverCanon) {
        meshCoverCanon = std::make_unique<ppgso::Mesh>("data/objects/djSet/CoverCannon.obj");
        meshCoverCanon->texture =
            std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/coverCanon.bmp"));
    }
    if (type == CanonType::Left) {
        //Particles
        auto confetti = std::make_unique<ConfettiSystem>(this, scene);
        confettiSystem = confetti.get();

        scene.particleObjects.push_back(confettiSystem);
        childObjects.push_back(std::move(confetti));
    }
    else {
        auto confetti = std::make_unique<ConfettiSystem>(this, scene);
        confettiSystem = confetti.get();

        scene.particleObjects.push_back(confettiSystem);
        childObjects.push_back(std::move(confetti));
    }


}


bool ConfettiCannon::update(Scene &scene, float time, float dt,
                            glm::mat4 parentModelMatrix, glm::vec3 parentRotation)
{
    generateModelMatrix(parentModelMatrix);

    animateHead(time);

    if (confettiSystem) {
        glm::vec3 point = getEmissionPoint();
        glm::vec3 dir   = getEmissionDirection();
        if (firing) {
            int emitCount = static_cast<int>(300 * dt);
            if (emitCount > 0)
                if (type == CanonType::Left) confettiSystem->emit(emitCount, point, dir);
                if (type == CanonType::Right) confettiSystem->emit(emitCount, point, dir);
        }
    }

    return true;
}


void ConfettiCannon::animateHead(float time) {
    float speed = 0.8f;
    float maxTiltZ = glm::radians(20.0f);

    if (type == CanonType::Left) {
        headRotation.z = sin(time * speed * 0.6f) * maxTiltZ;
    }
    if (type == CanonType::Right) {
        headRotation.z = -sin(time * speed * 0.6f) * maxTiltZ;
    }
}

glm::vec3 ConfettiCannon::getEmissionPoint() const {
    glm::mat4 head = computeHeadMatrix();
    return glm::vec3(head * glm::vec4(emissionLocal, 1.0f));
}

glm::vec3 ConfettiCannon::getEmissionDirection() const {
    glm::mat4 headMatrix = computeHeadMatrix();

    glm::vec3 forward = headMatrix * glm::vec4(0, 0, -1, 0);
    return glm::normalize(forward);
}


glm::mat4 ConfettiCannon::computeHeadMatrix() const {
    glm::mat4 head =
        modelMatrix *
        glm::translate(glm::mat4(1.0f), headPosition) *
        glm::orientate4(headRotation) *
        glm::scale(glm::mat4(1.0f), headScale);

    return head;
}


void ConfettiCannon::render(Scene &scene, ppgso::Shader &shader) {
    // BASE
    this->material = MaterialType::Wood;
    applyMaterial(shader);
    shader.setUniform("ModelMatrix", modelMatrix);
    shader.setUniform("UseTexture", true);
    meshBaseCanon->texture->bind(0);
    shader.setUniform("Texture", 0);
    meshBaseCanon->render();

    // HEAD (canon tube) — uses its own transform!
    this->material = MaterialType::Gold;
    applyMaterial(shader);
    glm::mat4 headMatrix = computeHeadMatrix();
    shader.setUniform("ModelMatrix", headMatrix);
    meshCanon->texture->bind(0);
    shader.setUniform("Texture", 0);
    if (meshCanon->normalMap) {
        meshCanon->normalMap->bind(1);
        shader.setUniform("NormalMap", 1);
        shader.setUniform("UseNormalMap", true);
    }
    meshCanon->render();
    shader.setUniform("UseNormalMap", true);

    // COVER (static, attached to base)
    this->material = MaterialType::Default;
    applyMaterial(shader);
    shader.setUniform("ModelMatrix", headMatrix);
    meshCoverCanon->texture->bind(0);
    shader.setUniform("Texture", 0);
    meshCoverCanon->render();
}


void ConfettiCannon::renderDepth(Scene &scene, ppgso::Shader &shader) {
    shader.setUniform("ModelMatrix", computeHeadMatrix());
    if (meshBaseCanon)
        meshBaseCanon ->render();

    glm::mat4 headMatrix = computeHeadMatrix();
    shader.setUniform("ModelMatrix", headMatrix);
    if (meshCanon)
        meshCanon ->render();
}

void ConfettiCannon::fire() {
   firing = true;
}
void ConfettiCannon::stop() {
    firing = false;
}