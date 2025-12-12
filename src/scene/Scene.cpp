#include "Scene.h"
#include <shaders/phong_vert_glsl.h>
#include <shaders/phong_frag_glsl.h>
#include <shaders/instanced_vert_glsl.h>
#include <shaders/instanced_frag_glsl.h>
#include <shaders/particle_frag_glsl.h>
#include <shaders/particle_vert_glsl.h>
#include <shaders/shadow_depth_frag_glsl.h>
#include <shaders/shadow_depth_vert_glsl.h>
#include <shaders/fullscreen_vert_glsl.h>
#include <shaders/bright_extract_frag_glsl.h>
#include <shaders/blur_frag_glsl.h>
#include <shaders/bloom_composite_frag_glsl.h>

#include "src/objects/lightObjects/LightEmitter.h"
#include <map>
#include <shaders/skybox_frag_glsl.h>
#include <shaders/skybox_vert_glsl.h>

Scene::Scene() = default;

void Scene::update(float time, float dt) {
    camera->update(dt);

    // ---------------------------------------
    // SPOTLIGHT SHADOW MATRIX UPDATE
    // ---------------------------------------
    if (shadowLight) {
        glm::vec3 eye = shadowLight->position;
        glm::vec3 center = shadowLight->position + shadowLight->spotDirection;
        glm::vec3 up(0, 0, 1);

        // 1. View Matrix
        glm::mat4 lightView = glm::lookAt(eye, center, up);
        // 2. Projection Matrix
        float fov = glm::degrees(acos(shadowLight->cutOff));
        glm::mat4 lightProjection = glm::perspective(
            glm::radians(fov),
            1.0f,
            1.0f,
            100.0f
        );
        // 3. Combined matrix

        lightSpaceMatrix = lightProjection * lightView;

    }
    // ----------------------------------
    // Animation for lights
    // ----------------------------------
    animationLady(dt);

    // ---------------------------------------
    // UPDATE OBJECTS AS USUAL
    // ---------------------------------------
    auto i = std::begin(rootObjects);

    while (i != std::end(rootObjects)) {
        auto object = i->get();
        if (!object->updateChildren(*this, time, dt, glm::mat4{1.0f},
{0,0,0})) {
            i = rootObjects.erase(i);
        } else {
            ++i;
        }
    }
}

// ------- Animation lights ---------
float neonFlicker(float time)
{
    float n = glm::fract(sin(time * 12.456f + 4.123f) * 43758.5453f);

    if (n > 0.90f)
        return 0.0f;

    return 1.0f;
}

void Scene::animationLady(float dt) {
    if (neonLight) {
        neonFlickValue = neonFlicker(dt);

        neonLight->ambient  = neonLight->baseAmbient  * (0.2f + 0.8f * neonFlickValue);
        neonLight->diffuse  = neonLight->baseDiffuse  * neonFlickValue;
        neonLight->specular = neonLight->baseSpecular * neonFlickValue;
    }
}
// ----------- Renders ---------------
void Scene::render()
{
    //---------------------------
    // 1. SHADER: Shadow Pass
    //---------------------------

    renderShadowMap();

    // ---------------------------
    // 2. Render scene into HDR FBO
    // ---------------------------

    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // --------------------------------
    // 2. CUBE MAP SKYBOX
    // --------------------------------
    if (!skyBoxObjects.empty() && skyBoxShader) {
        glDepthFunc(GL_LEQUAL);

        skyBoxShader->use();
        skyBoxShader->setUniform("ProjectionMatrix", camera->projectionMatrix);

        glm::mat4 viewNoPos = glm::mat4(glm::mat3(camera->viewMatrix));
        skyBoxShader->setUniform("ViewMatrix", viewNoPos);

        for (auto* obj : skyBoxObjects)
            obj->render(*this, *skyBoxShader);

        glDepthFunc(GL_LESS);
    }


    // ---------------------------
    //  3. SHADER: Phong Bing Pass
    // ---------------------------

    phongShader->use();

    phongShader->setUniform("lightSpaceMatrix", lightSpaceMatrix);
    phongShader->setUniform("shadowLightPos", shadowLight->position);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    phongShader->setUniform("shadowMap", 5);

    phongShader->setUniform("ViewMatrix", camera->viewMatrix);
    phongShader->setUniform("ProjectionMatrix", camera->projectionMatrix);
    phongShader->setUniform("ViewPosition", camera->position);

    // --------------------------
    // Set up Lights
    // --------------------------
    uploadLightsToShader(*phongShader);

    // ---------------------------
    // Set default materials
    // ---------------------------
    phongShader->setUniform("EmissiveColor", glm::vec3(0.0f));
    phongShader->setUniform("EmissiveStrength", 1.0f);

    phongShader->setUniform("DiffuseStrength", 1.0f);
    phongShader->setUniform("SpecularStrength", 1.0f);
    phongShader->setUniform("LightAbsorption", 1.0f);

    // ---------------------------
    // Opaque Pass
    // ---------------------------
    glDisable(GL_BLEND);

    for (auto* obj : phongObjects)
        if (!obj->isTransparent)
            obj->render(*this, *phongShader);

    // ---------------------------
    // Transparent Sorting
    // ---------------------------
    std::vector<Object*> transparentList;
    transparentList.reserve(phongObjects.size());

    for (auto* obj : phongObjects)
        if (obj->isTransparent)
            transparentList.push_back(obj);

    std::sort(transparentList.begin(), transparentList.end(),
        [&](Object* a, Object* b) {
            float da = glm::distance(camera->position, a->position);
            float db = glm::distance(camera->position, b->position);
            return da > db; // far -> near
        }
    );

    // ---------------------------
    // Transparent Pass
    // ---------------------------
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (auto* obj : transparentList)
        obj->render(*this, *phongShader);

    glDisable(GL_BLEND);

    // ---------------------------------
    //  4. SHADER: NORMAL MAP
    // ---------------------------------
    /*
    if (!normalMapObjects.empty() && normalMapShader)
    {
        normalMapShader->use();

        normalMapShader->setUniform("ViewMatrix", camera->viewMatrix);
        normalMapShader->setUniform("ProjectionMatrix", camera->projectionMatrix);
        normalMapShader->setUniform("ViewPosition", camera->position);

        // lights uniforms

        for (auto* obj : normalMapObjects) {
            obj->render(*this, *normalMapShader);
            uploadLightsToShader(*phongShader);
        }
    }
    */

    // ---------------------------------
    //  5. SHADER: INSTANCED (tiles)
    // ---------------------------------
    if (!instancedObjects.empty() && instancedShader)
    {
        instancedShader->use();

        instancedShader->setUniform("ViewMatrix", camera->viewMatrix);
        instancedShader->setUniform("ProjectionMatrix", camera->projectionMatrix);

        for (auto* obj : instancedObjects)
            obj->render(*this, *instancedShader);}

    // ---------------------------------
    //  6. SHADER: PARTICLES (confeti)
    // ---------------------------------
    if (!particleObjects.empty() && particleShader)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glDisable(GL_CULL_FACE);

        particleShader->use();

        particleShader->setUniform("ViewMatrix", camera->viewMatrix);
        particleShader->setUniform("ProjectionMatrix", camera->projectionMatrix);

        for (auto* p : particleObjects)
            p->render(*this, *particleShader);

        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);
    }

    // -----------------------------
    //  7. End scene Render
    // -----------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //-------------------------------
    // 8. SHADER:  Bloom Bright Pass
    //-------------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO[0]);
    glClear(GL_COLOR_BUFFER_BIT);

    //Color Texture
    bloomBrightShader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneColor);
    bloomBrightShader->setUniform("scene", 0);

    //Bloom Mask Texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, sceneBloomMask);
    bloomBrightShader->setUniform("bloomMask", 1);

    renderQuad();

    //---------------------------------
    // 9. SHADER: Gaussian Blur Ping-pong
    //---------------------------------
    bool horizontal = true;
    int passes = 12;

    for (int i = 0; i < passes; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO[horizontal ? 1 : 0]);
        glClear(GL_COLOR_BUFFER_BIT);

        bloomBlurShader->use();
        bloomBlurShader->setUniform("horizontal", horizontal);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bloomColor[horizontal ? 0 : 1]);
        bloomBlurShader->setUniform("image", 0);

        renderQuad();

        horizontal = !horizontal;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // ---------------------------------
    //  10. SHADER: FINAL COMPOSITE PASS → SCREEN
    // ---------------------------------
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    bloomCompositeShader->use();
    bloomCompositeShader->setUniform("bloomStrength", bloomStrength);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneColor);
    bloomCompositeShader->setUniform("scene", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bloomColor[horizontal ? 0 : 1]);
    bloomCompositeShader->setUniform("bloom", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, toneMapMask);
    bloomCompositeShader->setUniform("toneMask", 2);

    renderQuad();
}

void Scene::renderShadowMap() {

    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glClear(GL_DEPTH_BUFFER_BIT);


    glEnable(GL_DEPTH_TEST);

    glCullFace(GL_FRONT);

    shadowShader->use();
    shadowShader->setUniform("lightSpaceMatrix", lightSpaceMatrix);

    for (auto* obj : phongObjects) {
        if (!obj->castsShadow) continue;
        obj->renderChildren(*this, *shadowShader);
    }


    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screenWidth, screenHeight);
}

void Scene::renderQuad() {
    if (quadVAO == 0) {
        float quadVertices[] = {
            // positions   // texCoords
            -1.0f,  1.0f,   0.0f, 1.0f,
            -1.0f, -1.0f,   0.0f, 0.0f,
             1.0f, -1.0f,   1.0f, 0.0f,

            -1.0f,  1.0f,   0.0f, 1.0f,
             1.0f, -1.0f,   1.0f, 0.0f,
             1.0f,  1.0f,   1.0f, 1.0f
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);

        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        // position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                4 * sizeof(float), (void*)0);

        // texCoord
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                4 * sizeof(float), (void*)(2 * sizeof(float)));
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Scene::uploadLightsToShader(ppgso::Shader& shader) const {
    bool hasDir  = false;
    bool hasPoint = false;
    bool hasSpot = false;

    int spotIndex  = 0;
    int pointIndex = 0;

    for (auto &L : lights) {
        if (!L->enabled) continue;
        switch (L->type) {
            case LightType::Directional:
                hasDir = true;
                break;

            case LightType::Point:
                hasPoint = true;
                pointIndex++;
                break;

            case LightType::Spot:
                hasSpot = true;
                spotIndex++;
                break;
        }
    }
    //Only if there is at least one light in each type
    shader.setUniform("hasDirLight",   hasDir);
    shader.setUniform("hasPointLight", hasPoint);
    shader.setUniform("hasSpotLight",  hasSpot);

    //Add number of lights
    shader.setUniform("numSpotLights", spotIndex);
    shader.setUniform("numPointLights", pointIndex);

    //Reset to add lights
    spotIndex  = 0;
    pointIndex = 0;

    for (auto &L : lights) {
        if (!L->enabled) continue;
        switch (L->type) {
            case LightType::Directional:
                L->upload(shader, 0);
                break;

            case LightType::Point:
                L->upload(shader, pointIndex);
                pointIndex++;
                break;

            case LightType::Spot:
                L->upload(shader, spotIndex);
                spotIndex++;
                break;
        }
    }

}

// ------------- Utils ----------------
LightEmitter *Scene::addEmitterForSpot(const Light &spot) {
    auto emitter = std::make_unique<LightEmitter>(
        nullptr,
        *this,
        spot.diffuse,
        4.0f
    );
    emitter->position = spot.position + glm::vec3(0, 0.05f, 0);
    emitter->scale = glm::vec3(0.2f);
    LightEmitter* emitterPtr = emitter.get();
    phongObjects.push_back(emitterPtr);
    rootObjects.push_back(std::move(emitter));
    return emitterPtr;
}

void Scene::resize(int width, int height) {
    this->screenWidth = width;
    this->screenHeight = height;

    // -------------------------
    // Resize HDR color buffer
    // -------------------------
    glBindTexture(GL_TEXTURE_2D, sceneColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
                 width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

    // -------------------------
    // Resize bloom mask buffer
    // -------------------------
    glBindTexture(GL_TEXTURE_2D, sceneBloomMask);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8,
                 width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    // -------------------------
    // Resize tone-map mask
    // -------------------------
    glBindTexture(GL_TEXTURE_2D, toneMapMask);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8,
                 width, height, 0,
                 GL_RED, GL_UNSIGNED_BYTE, nullptr);

    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

    //Update draw buffers
    GLenum attachments[3] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2
    };
    glDrawBuffers(3, attachments);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // -------------------------
    // Resize depth buffer
    // -------------------------
    glBindTexture(GL_TEXTURE_2D, sceneDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    // -------------------------
    // Resize bloom ping-pong buffers
    // -------------------------
    for (int i = 0; i < 2; i++) {
        glBindTexture(GL_TEXTURE_2D, bloomColor[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
                     width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    }


    glBindTexture(GL_TEXTURE_2D, 0);
}

//------------ Clean and start --------
void Scene::close() {
    //Clean List Objects
    rootObjects.clear();

    //Clean pointers for shaders
    phongObjects.clear();
    normalMapObjects.clear();
    instancedObjects.clear();
    particleObjects.clear();
    skyBoxObjects.clear();

    //Clean Lights
    lights.clear();
    shadowLight = nullptr;

    //Loading shaders
    if (!phongShader) phongShader  = std::make_unique<ppgso::Shader>(phong_vert_glsl, phong_frag_glsl);
    if (!instancedShader) instancedShader = std::make_unique<ppgso::Shader>(instanced_vert_glsl, instanced_frag_glsl);
    if (!particleShader) particleShader  = std::make_unique<ppgso::Shader>(particle_vert_glsl, particle_frag_glsl);
    if (!shadowShader) shadowShader = std::make_unique<ppgso::Shader>(shadow_depth_vert_glsl, shadow_depth_frag_glsl);
    if (!skyBoxShader) skyBoxShader = std::make_unique<ppgso::Shader>(skybox_vert_glsl, skybox_frag_glsl);

    if (!bloomBrightShader) bloomBrightShader = std::make_unique<ppgso::Shader>(fullscreen_vert_glsl, bright_extract_frag_glsl);
    if (!bloomBlurShader) bloomBlurShader = std::make_unique<ppgso::Shader>(fullscreen_vert_glsl, blur_frag_glsl);
    if (!bloomCompositeShader) bloomCompositeShader = std::make_unique<ppgso::Shader>(fullscreen_vert_glsl, bloom_composite_frag_glsl);

    //normalMapShader = std::make_unique<ppgso::Shader>(normalmap_vert_glsl, normalmap_frag_glsl);

}


