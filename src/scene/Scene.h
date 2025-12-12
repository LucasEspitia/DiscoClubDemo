#ifndef DISCOCLUB_SCENE_H
#define DISCOCLUB_SCENE_H

#include <memory>
#include <list>

#include "Object.h"
#include "Camera.h"
#include "Light.h"
#include "src/objects/inside/lamps/Bubble.h"
#include "src/objects/lightObjects/LightEmitter.h"


enum class GameScene {
    Entrance,
    Interior,
    DanceFloor,
    Ending
};

class Scene {
public:
    //Scene ID
    GameScene currentScene;
    //Camera object
    std::unique_ptr<Camera> camera;
    //Size by default
    int screenWidth = 1420;
    int screenHeight = 920;


    //All objects root
    std::list<std::unique_ptr<Object>> rootObjects;

    //List objects for different shaders
    std::vector<Object*> phongObjects;
    std::vector<Object*> normalMapObjects;
    std::vector<Object*> instancedObjects;
    std::vector<Object*> particleObjects;
    std::vector<Object*> skyBoxObjects;


    //List of lights in the scene
    std::vector<std::unique_ptr<Light>> lights;

    //Shadow Maps
    GLuint depthFBO;
    GLuint depthTexture;
    static const unsigned int SHADOW_WIDTH = 2048;
    static const unsigned int SHADOW_HEIGHT = 2048;
    glm::mat4 lightSpaceMatrix;
    Light* shadowLight = nullptr;

    //Postprocessing
    GLuint sceneFBO;
    GLuint sceneColor;
    GLuint sceneDepth;
    GLuint sceneBloomMask;
    GLuint toneMapMask;

    // BLOOM ping-pong buffers
    GLuint bloomFBO[2];
    GLuint bloomColor[2];
    // BLOOM ping-pong buffers
    GLuint quadVAO = 0;
    GLuint quadVBO = 0;

    // BLOOM settings
    float bloomStrength = 0.25f;

    //Lady animation light
    Light* neonLight = nullptr;
    float neonFlickValue = 1.0f;

    // -------------- Shaders ----------------------
    std::unique_ptr<ppgso::Shader> phongShader;
    std::unique_ptr<ppgso::Shader> instancedShader;
    std::unique_ptr<ppgso::Shader> particleShader;
    std::unique_ptr<ppgso::Shader> skyBoxShader;
    std::unique_ptr<ppgso::Shader> shadowShader;


    //Bloom shaders
    std::unique_ptr<ppgso::Shader> bloomBrightShader;
    std::unique_ptr<ppgso::Shader> bloomBlurShader;
    std::unique_ptr<ppgso::Shader> bloomCompositeShader;



    //std::unique_ptr<ppgso::Shader> normalMapShader;

    //Constructor of Scene
    Scene();

    /*!
     *Update all objects in the Scene.
     *@param time
     *@param dt
     */
    void update(float time, float dt);

    /*!
     *Render all objects in the Scene
     */
    void render();

    /*!
     * Render the shadow map
     */
    void renderShadowMap();
    /*!
     * Upload Lights in different Shaders
     */
    void uploadLightsToShader(ppgso::Shader& shader) const;
    /*!
     * Render quads for bloom
     */
    void renderQuad();
    /*!
     * Resize All buffers when window change
     *
     * @param width
     * @param height
     */
    void resize(int width, int height);

    /*!
     *
     * Util for adding object emitter for spotlight
     * @param spot
     * @return
     */
    LightEmitter *addEmitterForSpot(const Light &spot);

    /*!
    * Animation lady
    */
    void animationLady(float);

    /*!
     * Destroy all objects in the Scene
     */
    void close();






};


#endif //DISCOCLUB_SCENE_H