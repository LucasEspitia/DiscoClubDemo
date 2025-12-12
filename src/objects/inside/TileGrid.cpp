#include "TileGrid.h"
#include <shaders/phong_vert_glsl.h>
#include <shaders/phong_frag_glsl.h>
#include "../../scene/Scene.h"
#include <random>

std::unique_ptr<ppgso::Mesh_Assimp> TileGrid::meshTile;

static std::mt19937 rng{ std::random_device{}() };
static float randf(float a, float b) {
    std::uniform_real_distribution<float> dist(a, b);
    return dist(rng);
}

TileGrid::TileGrid(Object *parent, Scene &scene, int width, int height){
    // Initial transform
    position ={-60.f, 0.2f, -2.f};
    rotation = {0, 0, 0};
    scale = {0.5f, 1.f, 0.5f};

    // Load tile mesh once
    if (!meshTile) meshTile = std::make_unique<ppgso::Mesh_Assimp>("data/objects/inside/Tile.obj");

    // ------------------------------------------------------
    // 1) Procedural instance positions + colors
    // ------------------------------------------------------
    const float spacing = 10.4f;

    for (int x = 0; x < width; x++) {
        for (int z = 0; z < height; z++) {

            instancePositions.emplace_back(x * spacing, 0.0f, z * spacing);
            glm::vec3 color = randomNeon();
            baseColors.push_back(color);
            instanceColors.push_back(color);
        }
    }

    // ------------------------------------------------------
    // 2) Create VBO for INSTANCE POSITION
    // ------------------------------------------------------
    glGenBuffers(1, &posVBO);
    glBindBuffer(GL_ARRAY_BUFFER, posVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 instancePositions.size() * sizeof(glm::vec3),
                 instancePositions.data(),
                 GL_STATIC_DRAW);

    // Attach to mesh VAOs (attrib location 3)
    meshTile->addInstanceBuffer(posVBO, 3, 3);

    // ------------------------------------------------------
    // 3) Create VBO for INSTANCE COLOR
    // ------------------------------------------------------
    glGenBuffers(1, &colorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 instanceColors.size() * sizeof(glm::vec3),
                 instanceColors.data(),
                 GL_STATIC_DRAW);

    // Attach to mesh VAOs (attrib location 4)
    meshTile->addInstanceBuffer(colorVBO, 4, 3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

glm::vec3 TileGrid::randomNeon() {
    static std::vector<glm::vec3> neon = {
        {0.95f, 0.10f, 0.10f},
        {1.00f, 0.40f, 0.00f},
        {1.00f, 0.80f, 0.05f},
        {0.10f, 0.95f, 0.10f},
        {0.05f, 0.80f, 1.00f},
        {0.30f, 0.10f, 1.00f},
        {1.00f, 0.05f, 0.80f},
        {0.80f, 0.05f, 1.00f}
    };

    return neon[rand() % neon.size()];
}

/**
* ANIMATION 1 – Breathing
*  Soft pulses (entrance)
*/
void TileGrid::animateBreathing(float time)
{

    float pulse = 0.5f + 0.5f * sin(time * 1.5f);  // suave

    for (int i = 0; i < instanceColors.size(); i++) {
        instanceColors[i] = baseColors[i] * pulse;
    }
}

/**
* ANIMATION 2 – Wave Pulse
* Wave that travels across the track (central part)
*/
void TileGrid::animateWave(float time)
{
    glm::vec2 waveOrigin = glm::vec2(0, 0);
    float speed = 4.0f;

    for (int i = 0; i < instanceColors.size(); i++) {
        glm::vec3 pos = instancePositions[i];
        float dist = glm::length(glm::vec2(pos.x, pos.z) - waveOrigin);

        float wave = sin(dist * 0.8f - time * speed);
        float pulse = 0.4f + 0.6f * ((wave + 1.0f) * 0.5f);

        instanceColors[i] = baseColors[i] * pulse;
    }
}
/**
* ANIMATION 3 – Flash Party
* Random blinking (party mode / drop)
*/
void TileGrid::animateFlash(float time)
{
    float flash = (sin(time * 20.0f) + 1.0f) * 0.5f;

    for (int i = 0; i < instanceColors.size(); i++) {
        float r = randf(0.5f, 1.0f);
        instanceColors[i] = baseColors[i] * (0.3f + 0.7f * flash * r);
    }
}



bool TileGrid::update(Scene &scene, float time, float dt, glm::mat4 parentModelMatrix, glm::vec3 parentRotation)
{
    switch (scene.currentScene) {
        case GameScene::Interior:
            animateBreathing(time);
            break;
        case GameScene::DanceFloor:
            animateWave(time);
            break;
        case GameScene::Ending:
            animateFlash(time);
            break;
        default:
            break;
    }

    // Updating GPU Buffer
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    instanceColors.size() * sizeof(glm::vec3),
                    instanceColors.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    generateModelMatrix(parentModelMatrix);
    return true;
}

void TileGrid::render(Scene &scene, ppgso::Shader& shader)
{
    shader.setUniform("ModelMatrix", modelMatrix);
    shader.setUniform("UseTexture", false);
    meshTile->renderInstanced(instancePositions.size());
}

