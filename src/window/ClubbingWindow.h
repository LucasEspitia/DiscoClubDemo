// ClubbingWindow.h
#pragma once
#include <ppgso/window.h>
#include "../scene/Scene.h"
#include "src/objects/djSet/DjSet.h"
#include "src/objects/entrance/Door.h"
#include "src/objects/entrance/SecurityCamera.h"
#include "src/objects/inside/BarSet.h"
#include <windows.h>



class ClubbingWindow : public ppgso::Window {
    // Windows size
    static constexpr int SIZEWIDTH  = 1420;
    static constexpr int SIZEHEIGHT = 920;

protected:
    // Scene
    Scene scene;

    //Music playback
    bool musicPlaying = false;

    //Pointers to important objects
    Door* doorLPtr = nullptr;
    Door* doorRPtr = nullptr;
    SecurityCamera* securityCameraPtr = nullptr;
    DjSet* djsetPtr = nullptr;

    Light* moonPtr = nullptr;
    //Light*

    // Store keyboard inputs
    std::map<int, int> keys;

    // Mouse events
    double lastMouseX = SIZEWIDTH / 2.0;
    double lastMouseY = SIZEHEIGHT / 2.0;
    bool firstMouse = true;
    float mouseSensitivity = 0.15f;
    bool mouseLookEnabled = false;

    //delta time
    float lastTime;



    // Separate Scenes
    void startClub();
    void initInteriorScene();
    void initDanceFloorScene();
    void initEndingScene();

    //Generation Lights
    void generateLightsEntrance();
    void generateShadowLight();

    //Cinematic
    void cinematicIntro();
    void cinematicInterior();
    void cinematicDanceFloor();
    void cinematicEnding();

    //Creation objects
    static void generateTableSets(Scene& scene);
public:
    // Constructor
    ClubbingWindow();

    // Main Loop
    void onIdle() override;

    // Resize callback
    void onResize(int width, int height);

    // Keyboard callback
    void onKey(int key, int scanCode, int action, int mods) override;

    // Mouse move callback
    void onMouseMove(double xpos, double ypos);
};
