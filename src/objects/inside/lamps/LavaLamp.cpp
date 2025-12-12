#include "LavaLamp.h"

#include "Bubble.h"

std::unique_ptr<ppgso::Mesh> LavaLamp::meshLavaBase;
std::unique_ptr<ppgso::Mesh> LavaLamp::meshLavaGlass;

LavaLamp::LavaLamp(Object* parent,Scene &scene, LavaType lavaType, glm::vec3 color, float emissive) {

    parentObject = parent;
    lampColor = color;
    emissiveStrength = emissive;

    position = {0,0,0};
    scale = {1,1,1};

    this->type = lavaType;

    if (!meshLavaBase)
        meshLavaBase = std::make_unique<ppgso::Mesh>("data/objects/inside/lamp/LavaBase.obj");

    if (!meshLavaGlass)
        meshLavaGlass = std::make_unique<ppgso::Mesh>("data/objects/inside/lamp/LavaGlass.obj");

    if (type == LavaBase) {
        this->material = MaterialType::Gold;
        // Add glass
        auto lavaGlass = std::make_unique<LavaLamp>(this, scene, LavaGlass, lampColor, emissive);
        LavaLamp* lavaGlassPtr = lavaGlass.get();
        scene.phongObjects.push_back(lavaGlassPtr);
        childObjects.push_back(std::move(lavaGlass));
        // Add bubbles
        for (int i = 0; i < 4; i++) {
            BubbleConfig cfg;
            cfg.color = color;
            cfg.emissiveStrength = emissive;
            auto b = std::make_unique<Bubble>(this, scene, cfg);
            Bubble* ptr = b.get();
            scene.phongObjects.push_back(ptr);
            childObjects.push_back(std::move(b));
        }

        //Add point light
        auto lightP = std::make_unique<Light>(LightType::Point);
        glm::vec3 colorHDR = color * 2.0f;

        lightP->ambient  = colorHDR * 0.03f;
        lightP->diffuse  = colorHDR * 0.8f;
        lightP->specular = colorHDR * 0.15f;

        lightP->constant  = 1.0f;
        lightP->linear    = 0.08f;
        lightP->quadratic = 0.0012f;

        scene.lights.push_back(std::move(lightP));
        light = scene.lights.back().get();

    }

    if (type == LavaGlass) {
        isTransparent = true;
        this->material = MaterialType::Glossy;
    }
}

bool LavaLamp::update(Scene &scene, float time, float dt, glm::mat4 parentModelMatrix, glm::vec3 parentRotation) {
    if (light) {
        glm::vec3 lightPos = glm::vec3(modelMatrix * glm::vec4(0.0f,6.f,0.0f,1));
        light->position = lightPos;

    }
    generateModelMatrix(parentModelMatrix);
    return true;
}

void LavaLamp::render(Scene &scene, ppgso::Shader &shader) {

    shader.setUniform("ModelMatrix", modelMatrix);
    shader.setUniform("UseFog", true);

    switch (type)
    {
        case LavaGlass:
            shader.setUniform("UseTexture", false);
            shader.setUniform("ObjectColor", glm::vec3(0.5, 0.6, 0.7));
            shader.setUniform("Transparency", 0.15f);
            shader.setUniform("EmissiveColor", lampColor * 0.1f);
            shader.setUniform("EmissiveStrength", emissiveStrength * 0.3f);
            meshLavaGlass->render();
            shader.setUniform("EmissiveStrength", 0.0f);

            break;
        case LavaBase:
            shader.setUniform("ObjectColor", glm::vec3(0.50f, 0.50f, 0.52f));
            shader.setUniform("UseFog", true);
            meshLavaBase->render();
            break;
         }
        shader.setUniform("UseFog", false);

}

void LavaLamp::renderDepth(Scene &scene, ppgso::Shader &shader) {
    shader.setUniform("ModelMatrix", modelMatrix);
    switch (type)
    {
        case LavaGlass:
            if (meshLavaGlass)
                meshLavaGlass->render();
            break;
        case LavaBase:
            if (meshLavaBase)
                meshLavaBase->render();
            break;
    }

}
