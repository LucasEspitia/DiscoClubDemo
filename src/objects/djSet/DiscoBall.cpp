#include "DiscoBall.h"

#include "src/scene/Scene.h"


std::unique_ptr<ppgso::Mesh>  DiscoBall::meshBaseDisco;
std::unique_ptr<ppgso::Mesh> DiscoBall::meshDiscoBall;

//Util
glm::mat4 removeScale(const glm::mat4& m)
{
    glm::vec3 t = glm::vec3(m[3]);

    glm::vec3 x = glm::normalize(glm::vec3(m[0]));
    glm::vec3 y = glm::normalize(glm::vec3(m[1]));
    glm::vec3 z = glm::normalize(glm::vec3(m[2]));

    glm::mat4 r(1.0f);
    r[0] = glm::vec4(x, 0.0f);
    r[1] = glm::vec4(y, 0.0f);
    r[2] = glm::vec4(z, 0.0f);
    r[3] = glm::vec4(t, 1.0f);

    return r;
}


DiscoBall::DiscoBall(Object *parent, DiscoBallType typeDisco, Scene &scene) {
    parentObject = parent;
    type = typeDisco;

    position = {0,0,0};
    rotation = {0,0,0};
    scale = {1.0f,1.0f,1.0f};

    if (!meshBaseDisco) {
        meshBaseDisco = std::make_unique<ppgso::Mesh>("data/objects/cube.obj");
        meshBaseDisco->texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/blueMetal.bmp"));
        meshBaseDisco->normalMap = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/blueMetalNormal.bmp"));
    }
    if (!meshDiscoBall) {
        meshDiscoBall = std::make_unique<ppgso::Mesh>("data/objects/djSet/discoBall.obj");
        // meshDiscoBall->texture = getColorTexture({0.45f, 0.35f, 0.55f})
        meshDiscoBall->texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/discoBall.bmp"));
        meshDiscoBall->normalMap = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("data/objects/textures/discoBallNormal.bmp"));
    }

    if (type == Base) {
        this->material = MaterialType::DiscoBall;
        auto ballDisco = std::make_unique<DiscoBall>(this, Ball, scene);
        ballDisco->position.y = -3.8f;
        ballDisco->scale = {22.5f, -22.5f, 22.5f};
        DiscoBall* discoBallPtr = ballDisco.get();
        scene.phongObjects.push_back(discoBallPtr);
        childObjects.push_back(std::move(ballDisco));
    }
    if (type == Ball) {
        this->material = MaterialType::Gold;
        emissiveColor = glm::vec3(0.6f, 0.3f, 1.0f);
        emissiveStrength = 0.35f;
    }
}

bool DiscoBall::update(Scene &scene, float time, float dt,
                       glm::mat4 parentModelMatrix, glm::vec3 parentRotation)
{
    glm::mat4 parentForThis = parentModelMatrix;

    if (type == Ball) {
        parentForThis = removeScale(parentModelMatrix);

        rotation.z = sin(time * 0.6f) * 5.0f;

    }

    generateModelMatrix(parentForThis);
    return true;
}


void DiscoBall::render(Scene &scene, ppgso::Shader& shader) {
    shader.setUniform("ModelMatrix", modelMatrix);
    shader.setUniform("UseTexture", true);

    applyMaterial(shader);
    switch (this->type) {
        case Base:
            meshBaseDisco->texture->bind(0);
            shader.setUniform("Texture", 0);
            meshDiscoBall->normalMap->bind(1);
            shader.setUniform("NormalMap", 1);
            meshBaseDisco->render();
            break;
        case Ball:
            shader.setUniform("UseTexture", true);
            shader.setUniform("EmissiveColor", emissiveColor);
            shader.setUniform("EmissiveStrength", emissiveStrength);
            meshDiscoBall->texture->bind(0);
            shader.setUniform("Texture", 0);
            meshBaseDisco->normalMap->bind(1);
            shader.setUniform("NormalMap", 1);
            meshDiscoBall->render();
            shader.setUniform("EmissiveColor", glm::vec3(0.0f));
            shader.setUniform("EmissiveStrength", 0.0f);
            break;
    }
}
void DiscoBall::renderDepth(Scene &scene, ppgso::Shader &shader) {
    shader.setUniform("ModelMatrix", modelMatrix);
    switch (type) {
        case Base:
            if (meshBaseDisco)
                meshBaseDisco->render();
            break;

        case Ball:
            if (meshDiscoBall)
                meshDiscoBall->render();
            break;
    }
}
