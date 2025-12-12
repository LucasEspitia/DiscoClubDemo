#include "ClubbingWindow.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <windows.h>
#include <mmsystem.h>

#include <random>

#include "src/objects/Club.h"
#include "src/objects/djSet/DiscoBall.h"
#include "src/objects/djSet/DjSet.h"
#include "src/objects/entrance/SecurityCamera.h"
#include "src/objects/entrance/Door.h"
#include "src/objects/inside/BarSet.h"
#include "src/objects/inside/TableSet.h"
#include "src/objects/inside/TileGrid.h"
#include "src/objects/entrance/SkyBox.h"


ClubbingWindow::ClubbingWindow()
    : Window{"Club Disco", SIZEWIDTH, SIZEHEIGHT}
{

    glfwSetWindowUserPointer(window, this);
    // *******************************************************
    // ***********      Input Callback        ****************
    // *******************************************************
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetCursorPosCallback(window, [](GLFWwindow* win, double xpos, double ypos) {
        auto self = static_cast<ClubbingWindow*>(glfwGetWindowUserPointer(win));
        if (self) {
            self->onMouseMove(xpos, ypos);
        }
    });
    // *******************************************************
    // ***********      Resize Callback       ****************
    // *******************************************************
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int w, int h) {
        auto self = static_cast<ClubbingWindow*>(glfwGetWindowUserPointer(win));
        if (self) self->onResize(w, h);
    });

    startClub();

    // *******************************************************
    // ***********  SHADOW MAP INITIALIZATION  ****************
    // *******************************************************

    glGenFramebuffers(1, &scene.depthFBO);

    glGenTextures(1, &scene.depthTexture);
    glBindTexture(GL_TEXTURE_2D, scene.depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        Scene::SHADOW_WIDTH, Scene::SHADOW_HEIGHT, 0,
        GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, scene.depthFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D, scene.depthTexture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // *******************************************************
    // ***********  POSTPROCESSING FBO INITIALIZATION  ********
    // *******************************************************

    glGenFramebuffers(1, &scene.sceneFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, scene.sceneFBO);

    // ----- Color texture (HDR style: RGBA16F) -----
    glGenTextures(1, &scene.sceneColor);
    glBindTexture(GL_TEXTURE_2D, scene.sceneColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
                 SIZEWIDTH, SIZEHEIGHT, 0,
                 GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, scene.sceneColor, 0);

    // ----- Bloom mask texture (0 = no bloom, 1 = bloom)  -----
    glGenTextures(1, &scene.sceneBloomMask);
    glBindTexture(GL_TEXTURE_2D, scene.sceneBloomMask);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8,
                 SIZEWIDTH, SIZEHEIGHT, 0,
                 GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
                           GL_TEXTURE_2D, scene.sceneBloomMask, 0);

    // -------- Tone Mapping (0 = no Tone Mapping, 1 = Tone Mapping) -----
    glGenTextures(1, &scene.toneMapMask);
    glBindTexture(GL_TEXTURE_2D, scene.toneMapMask);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, SIZEWIDTH, SIZEHEIGHT, 0,
                 GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2,
                           GL_TEXTURE_2D, scene.toneMapMask, 0);

    // -------------------------------------------------------
    // SET THE DRAW BUFFERS
    // -------------------------------------------------------
    GLenum attachments[3] = {
        GL_COLOR_ATTACHMENT0, // sceneColor
        GL_COLOR_ATTACHMENT1, // bloomMask
        GL_COLOR_ATTACHMENT2  // toneMapMask
    };
    glDrawBuffers(3, attachments);

    // ----- Depth buffer -----
    glGenTextures(1, &scene.sceneDepth);
    glBindTexture(GL_TEXTURE_2D, scene.sceneDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 SIZEWIDTH, SIZEHEIGHT, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D, scene.sceneDepth, 0);

    // Validate FBO
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR: Scene FBO is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // *******************************************************
    // *********** BLOOM PING-PONG FBO INITIALIZATION ********
    // *******************************************************

    glGenFramebuffers(2, scene.bloomFBO);
    glGenTextures(2, scene.bloomColor);

    for (int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, scene.bloomFBO[i]);

        glBindTexture(GL_TEXTURE_2D, scene.bloomColor[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
                     SIZEWIDTH, SIZEHEIGHT, 0,
                     GL_RGBA, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, scene.bloomColor[i], 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR: Bloom FBO " << i << " is not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


}

void ClubbingWindow::onIdle() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Current absolute time
    float currentTime = (float)glfwGetTime();

    // Compute delta time
    float dt = currentTime - lastTime;
    if (dt > 0.1f) dt = 0.1f;

    lastTime = currentTime;

    scene.update(currentTime, dt);
    scene.render();

}

void ClubbingWindow::startClub() {
    scene.close();
    scene.currentScene = GameScene::Entrance;

    // -------------------------
    //      CAMERA SETTINGS
    // -------------------------
    float aspect = static_cast<float>(SIZEWIDTH) / static_cast<float>(SIZEHEIGHT);
    auto camera = std::make_unique<Camera>(80.0f, aspect, 0.1f, 500.0f);
    //start position -> No Key frames
    camera->position = glm::vec3(-127.5, 10.f, 122.5);
    camera->tilt     = -0.45f;
    camera->rotation = -231.749f;
    camera->keyframes.playing = true;
    camera->useKeyframes = true;
    camera->keyframes.animTime = 0.0f;
    scene.camera = std::move(camera);

    // -------------------------
    //        ADD LIGHTS
    // -------------------------
    generateShadowLight();
    generateLightsEntrance();
    // -------------------------
    //       STATIC OBJECTS
    // -------------------------
    {
        auto club = std::make_unique<Club>(nullptr, ClubWalls, scene);
        Club* clubPtr = club.get();
        scene.phongObjects.push_back(clubPtr);
        scene.rootObjects.push_back(std::move(club));

        generateTableSets(scene);

        // Cube 'SkyBox'
        auto street = std::make_unique<SkyBox>(scene);
        SkyBox* streetPtr = street.get();
        scene.skyBoxObjects.push_back(streetPtr);
        scene.rootObjects.push_back(std::move(street));

        auto bar = std::make_unique<BarSet>(nullptr, Bar, scene);
        bar->position = {123.0f, 0.0f, -75.0f};
        bar->rotation = glm::vec3(glm::radians(0.f), 0.f, glm::radians(45.f));
        BarSet* barPtr = bar.get();
        scene.phongObjects.push_back(barPtr);
        scene.rootObjects.push_back(std::move(bar));

        auto djSet = std::make_unique<DjSet>(nullptr, Set, scene);
        djSet->position = {0.f,0.f,100.f};
        djsetPtr = djSet.get();
        scene.phongObjects.push_back(djsetPtr);
        scene.rootObjects.push_back(std::move(djSet));
    }
    // -------------------------
    //       DYNAMIC OBJECTS
    // -------------------------
    {
        auto securityCamera = std::make_unique<SecurityCamera>(nullptr, scene);
        securityCamera->position = glm::vec3(-140.83f, 19.557f, 58.1f);
        securityCameraPtr = securityCamera.get();
        scene.phongObjects.push_back(securityCameraPtr);
        scene.rootObjects.push_back(std::move(securityCamera));

        auto doorLeft = std::make_unique<Door>(nullptr, DoorType::Left, scene);
        doorLeft->position = { -134.6f, 0.0f, 54.7f };
        doorLeft->scale.x = 1.03f;
        doorLPtr = doorLeft.get();
        scene.phongObjects.push_back(doorLPtr);
        scene.rootObjects.push_back(std::move(doorLeft));

        auto doorRight = std::make_unique<Door>(nullptr, DoorType::Right, scene);
        doorRight->position = { -117.3f, 0.0f, 54.7f };
        doorRight->scale.x = 1.03f;
        doorRPtr = doorRight.get();
        scene.phongObjects.push_back(doorRPtr);
        scene.rootObjects.push_back(std::move(doorRight));

        auto tiles = std::make_unique<TileGrid>(nullptr, scene, 25,16);
        TileGrid* tileGridPtr = tiles.get();
        scene.instancedObjects.push_back(tileGridPtr);
        scene.phongObjects.push_back(std::move(tileGridPtr));
        scene.rootObjects.push_back(std::move(tiles));

        auto discoBallHolder = std::make_unique<DiscoBall>(nullptr, Base, scene);
        discoBallHolder->position = {0.f, 42.f, 35.f};
        discoBallHolder->scale = {0.3f,8.0f, 0.3f};
        DiscoBall* discoBallHolderPtr = discoBallHolder.get();
        scene.phongObjects.push_back(discoBallHolderPtr);
        scene.rootObjects.push_back(std::move(discoBallHolder));
    }
    //----------------------------
    //        START CINEMATICS
    //----------------------------
    cinematicIntro();
}

void ClubbingWindow::cinematicIntro() {
    scene.camera->keyframes.frames = {
        // KF 1 → Start Looking the Street (SkyBox)
        { 0.0f,
          glm::vec3(-126.5f, 10.f, 122.5f),
          glm::vec3(4.2, -235.95, 80.0f) },

        // KF 2 → Rotating to see all SkyBox, till it is in the entrance
        { 7.0f,
        glm::vec3(-126.5f, 10.f, 122.5f),
        glm::vec3(3.0f, -49.35f, 80.0f)
        },
        //KF 3 -> Watching the Wall
        {9.0f,
            glm::vec3(-126.5f, 10.f, 122.5f),
            glm::vec3(3.0f, -49.35f, 80.0f)
        },
        //KF 4 -> Rotating to the Lady Neon
        {
            11.0f,
            glm::vec3(-126.5f, 10.f, 112.214f),
            glm::vec3(-8.1f, 59.40f, 80.0f)

        },
        //KF 5 -> Watching Lady Light
        {
            13.0f,
            glm::vec3(-126.5f, 10.f, 112.214f),
            glm::vec3(-8.1f, 59.40f, 80.0f)
        },
        //KF 6 -> See Camera
        {
            15.0f,
            glm::vec3(-126.5f, 10.f, 112.214f),
            glm::vec3(-10.2f, -16.349f, 80.0f)
        },
        //KF 7 -> Zoom in in Camera
        {
            18.0f,
            glm::vec3(-126.5f, 10.f, 112.214f),
            glm::vec3(-10.2f, -16.349f, 30.0f)
        },
        //KF 8 -> Coming Back Zoom
        {
            20.0f,
        glm::vec3(-126.5f, 10.f, 112.214f),
        glm::vec3(-10.2f, -16.349f, 80.0f)
        },
        //KF 9 -> Rotating the door
        {
            22.0f,
            glm::vec3(-126.5f, 10.f, 112.214f),
                glm::vec3(0.0f, 0.0f, 80.0f)
        },
        // KF 10 -> Walking to the door
        {
            26.f,
            glm::vec3(-126.5f, 10.f, 69.714),
            glm::vec3(0.0f, 0.0f, 80.0f)
        },
        // KF 11 -> Looking the camera
        {
            28.f,
            glm::vec3(-126.5f, 10.f, 69.714),
            glm::vec3(-23.63f, -48.86f, 80.0f)
        },
        //KF -> Keeps looking at the Camera -> cinematic
        {
            34.f,
          glm::vec3(-126.5f, 10.f, 69.714),
            glm::vec3(-23.63f, -48.86f, 80.0f)
        },
        //KF 12 -> Looking back to the door
        {
    36.f,
         glm::vec3(-126.5f, 10.f, 69.714),
         glm::vec3(0.0f, 0.0f, 80.0f)
        },
        //KF 13 -> Watching Door Opening
        {
    38.f,
         glm::vec3(-126.5f, 10.f, 69.714),
         glm::vec3(0.0f, 0.0f, 80.0f)
        },
        //KF 14 -> Getting inside the club
        {
            42.f,
            glm::vec3(-126.5f, 10.f, 37.5),
            glm::vec3(0.0f, 0.0f, 80.0f)
        }
        ,
        //KF 15 -> Rotating to the Door
        {
            44.f,
            glm::vec3(-126.5f, 10.f, 37.5),
            glm::vec3(0.0f, 180.0f, 80.0f)
        },
        //KF 16 -> Closing Door
        {
            46.f,
            glm::vec3(-126.5f, 10.f, 37.5),
            glm::vec3(0.0f, 180.0f, 80.0f)
        },
        //KF 17 -> Rotating Camera and deactivating MoonLight
        {
            48.f,
            glm::vec3(-126.5f, 10.f, 37.5),
            glm::vec3(0.0f, 0.0f, 80.0f)
        }
    };
    //Security Camera Movement -> Target us
    scene.camera->keyframes.addEvent(28.0f, [this]() {
        if (this->securityCameraPtr)
            this->securityCameraPtr->cinematicTarget =
                this->securityCameraPtr->entranceTarget;
    });
    //Security Camera Movement -> Target patrol
    scene.camera->keyframes.addEvent(44.0f, [this]() {
    if (this->securityCameraPtr)
        this->securityCameraPtr->cinematicTarget =
            this->securityCameraPtr->patrolTarget;
    });
    //Door Opening Event
    scene.camera->keyframes.addEvent(38.0f, [this]() {

          if (this->doorLPtr) this->doorLPtr->open();
          if (this->doorRPtr) this->doorRPtr->open();
    });
    //Door Closing Event
    scene.camera->keyframes.addEvent(44.0f, [this]() {

        if (this->doorLPtr) this->doorLPtr->close();
        if (this->doorRPtr) this->doorRPtr->close();
  });
    //Disabling moon Light inside
    scene.camera->keyframes.addEvent(46.0f, [this]() {
        if (this->moonPtr) this->moonPtr->enabled = false;
            moonPtr->ambient  = glm::vec3(0.00001f, 0.00001f, 0.00001f);
    });
    //Changing Scene
    scene.camera->keyframes.addEvent(48.0f, [this]() {
        this->initInteriorScene();
    });

}

void ClubbingWindow::initInteriorScene() {
    scene.currentScene = GameScene::Interior;

    //Fixed Position Camera
    scene.camera->position = glm::vec3(-126.5f, 10.f, 37.5);
    scene.camera->rotation = 0.0f;
    scene.camera->keyframes.animTime = 48.f;
    scene.camera->useKeyframes = true;
    cinematicInterior();
}

void ClubbingWindow::cinematicInterior() {
    scene.camera->keyframes.frames ={
        //KF 1 -> Starting KF
        {
            49.f,
        glm::vec3(-126.5f, 10.f, 37.5),
         glm::vec3(0.0f, 0.0f, 80.0f)
        },
        //KF 2 -> Observing tables
        {
            52.f,
        glm::vec3(-144.f, 37.5f, 0.0f),
         glm::vec3(26.85f, 43.5f, 80.0f)
        },
        //KF 3 -> Zoom Tables
        {
            54.f,
       glm::vec3(-144.f, 37.5f, 0.0f),
        glm::vec3(26.85f, 43.5f, 40.0f)
        },
        // KF 4 -> Zoom Bar
        {
            56.f,
        glm::vec3(-144.f, 37.5f, 0.0f),
        glm::vec3(7.65f, 67.65f, 40.0f)
        },
        // KF 5-> Zoom Tales
        {
            58.f,
        glm::vec3(-144.f, 37.5f, 0.0f),
        glm::vec3(10.95f, 100.5f, 40.0f)
        },
        // KF 6 -> Zoom Dj
        {
            60.f,
        glm::vec3(-144.f, 37.5f, 0.0f),
         glm::vec3(9.f, 126.f, 40.0f)
        },
        //KF 7 -> Zoom Out
        {
            62.f,
        glm::vec3(-144.f, 37.5f, 0.0f),
            glm::vec3(9.f, 126.f, 80.0f)
        },
        //KF 8 -> Tables again
        {
        64.f,
        glm::vec3(-144.f, 37.5f, 0.0f),
            glm::vec3(26.85f, 43.5f, 80.0f)
        },
        // KF 9 -> Goes down with the tables
        {
            66.f,
            glm::vec3(-119, 12.5, -12.5),
            glm::vec3(16.2f, 4.8f, 80.0f)
        },
        //KF 10 -> Move to middle
        {
            70.f,
            glm::vec3(-4.5, 12.5, -12.5),
            glm::vec3(14.25f, 16.8f, 80.0f)
        },
        //KF 11 -> Move to the end of tables
        {
            74.f,
            glm::vec3(63, 12.5, -12.5),
            glm::vec3(6.6f, -39.3f, 80.0f)
        },
        //KF 12 -> Move to the next row on table
        {
            76.f,
        glm::vec3(63, 12.5, -65.f),
        glm::vec3(15.15f, -46.05f, 80.0f)
        },
        //KF 13 -> Move to lava Table
       {
           78.f,
       glm::vec3(63, 12.5, -65.f),
       glm::vec3(19.95f, -47.25f, 80.0f)
       },
        //KF 14 -> Watch Lava Movement
        {
            80.f,
            glm::vec3(43, 2.5, -82.5f),
            glm::vec3(11.85f, -33.6, 80.0f)
        },
        //KF 15 -> Orbit Lava Table
        {
            82.f,
            glm::vec3(43, 2.5, -90.f),
            glm::vec3(12.45f, -135.9, 80.0f)
        },
        {
            84.f,
            glm::vec3(38, 2.5, -90.f),
            glm::vec3(7.35f, -215.7, 80.0f)
        },
        {
            86.f,
            glm::vec3(38, 2.5, -85.f),
            glm::vec3(7.35f, -315, 80.0f)
        },
        // TK 16 -> Observe Lava Lamp
        {
            92.f,
            glm::vec3(38, 2.5, -85.f),
            glm::vec3(7.35f, -315, 80.0f)
        },
        //TK 17 -> Go to the Bar
        {
            94.f,
            glm::vec3(90.5, 12.5, -102.5f),
            glm::vec3(3.75f, -258.9, 80.0f)
        },
        //TK 18 -> Observe the lamp
        {
            95.f,
            glm::vec3(98, 12.5, -97.5f),
            glm::vec3(10.8f, -280.9, 80.0f)
        },
        {
            98.f,
            glm::vec3(98, 12.5, -97.5f),
            glm::vec3(10.8f, -280.9, 80.0f)
        },
        //TK 19 -> Go to other lamp
        {
            102.f,
            glm::vec3(128, 12.5, -72.5f),
            glm::vec3(13.5f, -221.55, 80.0f)
        },
        {
            104.f,
            glm::vec3(128, 12.5, -72.5f),
            glm::vec3(13.5f, -221.55, 80.0f)
        },
        //TK 20 -> See the bar in general view
        {
            106.f,
            glm::vec3(95.5, 20, -60.f),
            glm::vec3(4.8f, -311.851, 80.0f)
        },
        {
            108.f,
            glm::vec3(95.5, 20, -60.f),
            glm::vec3(4.8f, -311.851, 80.0f)
        },
        // TK 21 -> Watch the dance floor
        {
            110.f,
            glm::vec3(95.5, 20, -60.f),
            glm::vec3(5.85f, -143.101, 80.0f)
        },
        {
            111.f,
            glm::vec3(95.5, 20, -60.f),
            glm::vec3(5.85f, -143.101, 80.0f)
        },
        // TK 22 -> Go to the Dance Floor
        {
            114.f,
            glm::vec3(88.f, 20, 40.f),
            glm::vec3(13.8f, -85.85, 80.0f)
        },
        //TK 23 -> Last Observe Tiles before change
        {
            120.f,
            glm::vec3(88.f, 20, 40.f),
            glm::vec3(13.8f, -85.85, 80.0f)
        }
    };
    //Changing Scene
    scene.camera->keyframes.addEvent(120.0f, [this]() {
        this->initDanceFloorScene();
    });
}

void ClubbingWindow::initDanceFloorScene() {
    scene.currentScene = GameScene::DanceFloor;
    //Fixed Position Camera
    scene.camera->position = glm::vec3(88.f, 20, 40.f);
    scene.camera-> rotation = -85.85f;
    scene.camera->tilt = 13.8;
    scene.camera->keyframes.animTime = 120.f;
    scene.camera->useKeyframes = true;

    cinematicDanceFloor();
}

void ClubbingWindow::cinematicDanceFloor() {
    scene.camera->keyframes.frames = {
        // KF 0 — starting point of new scene
        {
            120.f,
            glm::vec3(88.f, 20, 40.f),
            glm::vec3(13.8f, -85.85f, 80.0f)
        },
        {
            126.f,
            glm::vec3(88.f, 20, 40.f),
            glm::vec3(13.8f, -85.85, 80.0f)
        },
        // KF 2 -> Go up and watch Ball
        {
            130.f,
            glm::vec3(0.f, 35, -2.5f),
            glm::vec3(24.8f, -180.8, 80.0f)
        },
        // KF 3 -> Watch Ball
        {
            134.f,
        glm::vec3(0.f, 35, -2.5f),
        glm::vec3(24.8f, -180.8, 80.0f)
        },
        // KF 4 -> Look Tales
        {
            136.f,
            glm::vec3(0.f, 35, -5.f),
            glm::vec3(50.15f, -180.68, 80.0f)
        },
        {
            140.f,
            glm::vec3(0.f, 35, -5.f),
            glm::vec3(50.15f, -180.68, 80.0f)
        },
        // KF 5 -> Go to Dj
        {
            142.f,
            glm::vec3(0.f, 20, 67.5f),
            glm::vec3(33.95f, -180.0, 80.0f)
        },
        {
            144.f,
            glm::vec3(0.f, 20, 67.5f),
            glm::vec3(33.95f, -180.0, 80.0f)
        },
        // KF 6 -> Watch canon
        {
            146.f,
            glm::vec3(-34.5f, 10.0, 90.0f),
            glm::vec3(44.3f, -218.3, 90.0f)
        },
        {
            149.f,
            glm::vec3(-34.5f, 10.0, 90.0f),
            glm::vec3(44.3f, -218.3, 90.0f)
        },
        //KF 7 -> Watch From view DJ
        {
            151.f,
            glm::vec3(0.f, 12.5, 130.0f),
           glm::vec3(0.0f, -360.0, 90.0f)
        },
        {
            155.f,
            glm::vec3(0.f, 12.5, 130.0f),
           glm::vec3(0.0f, -360.0, 90.0f)
        },
        // KF 8 -> Go near DJ
        {
            157.f,
            glm::vec3(0.f, 12.5, 105.0f),
           glm::vec3(0.0f, -360.0, 90.0f)
        },
        {
            160.f,
            glm::vec3(0.f, 12.5, 105.0f),
           glm::vec3(0.0f, -360.0, 90.0f)
        },
        // KF 9 -> Quick scan DJ console
        {
            162.f,
            glm::vec3(0.f, 12.5, 105.0f),
           glm::vec3(9.0f, -317.4, 90.0f)
        },
        {
            166.f,
            glm::vec3(0.f, 12.5, 105.0f),
           glm::vec3(15.75f, -409.1, 90.0f)
        },
        //KF 10 -> Go back again. end Scene
        {
            168.f,
            glm::vec3(0.f, 12.5, 130.0f),
           glm::vec3(0.0f, -360.0, 90.0f)
        },
    };
    //Changing Scene
    scene.camera->keyframes.addEvent(168.0f, [this]() {
        this->initEndingScene();
    });
}

void ClubbingWindow::initEndingScene() {
    scene.currentScene = GameScene::Ending;
    //Fixed Position Camera
    scene.camera->position = glm::vec3(0.f, 12.5, 130.0f);
    scene.camera-> rotation = -360.0;
    scene.camera->tilt = 0.0f;
    //scene.camera->keyframes.animTime = 168.f;
    scene.camera->useKeyframes = true;
    scene.camera->keyframes.animTime = 168.f;

    cinematicEnding();
}

void ClubbingWindow::cinematicEnding() {
    scene.camera->keyframes.frames = {
        // KF 0 — starting point of new scene
        {
            168.f,
              glm::vec3(0.f, 12.5, 130.0f),
             glm::vec3(0.0f, -360.0, 90.0f)
        },
        //Stays and watch the confetti
        {
            178.f,
              glm::vec3(0.f, 12.5, 130.0f),
             glm::vec3(0.0f, -360.0, 90.0f)
        },
        //Sees other canon
        {
            180.f,
              glm::vec3(30.f, 12.5, 82.5f),
             glm::vec3(28.65f, -542.55, 90.0f)
        },
        {
            182.f,
              glm::vec3(30.f, 12.5, 82.5f),
             glm::vec3(28.65f, -542.55, 90.0f)
        },
        //Watch top
        {
            185.f,
        glm::vec3(2.5f, 32.5, 40.f),
        glm::vec3(10.35f, -538.64, 90.0f)
        },
        {
        188.f,
    glm::vec3(2.5f, 32.5, 40.f),
    glm::vec3(10.35f, -538.64, 90.0f)
        },
        //Watch in tiles
        {
            192.f,
        glm::vec3(0.f, 7.5, 20.f),
        glm::vec3(-1.8f, -536.24, 90.0f)
            }
    };
    //Shoot only left
    scene.camera->keyframes.addEvent(170.0f, [this]() {
        this->djsetPtr->cannonLPtr->fire();
    });
    //Shoot both
    scene.camera->keyframes.addEvent(175.0f, [this]() {
        this->djsetPtr->cannonRPtr->fire();
    });
}




/**
 * Generate Tables in the disco
 * @param scene
 */
void ClubbingWindow::generateTableSets(Scene& scene) {
    const int rows = 2;
    const int cols = 4;
    const float spacingX = 50.0f;
    const float spacingZ = 50.0f;

    const glm::vec3 startPos = {-110.0f, 0.0f, -87.0f};


    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {

            auto table = std::make_unique<TableSet>(nullptr, TypeTableSet::Table, scene);

            float x = startPos.x + c * spacingX;
            float y = 0.0f;
            float z = startPos.z + r * spacingZ;

            table->position = {x, y, z};
            TableSet* tablePtr = table.get();
            scene.phongObjects.push_back(tablePtr);
            scene.rootObjects.push_back(std::move(table));
        }
    }
}

/**
 * Generate Lights
 * Outside and Inside
 */

void ClubbingWindow::generateLightsEntrance() {

    //Directional (Moon) -> Only from outside.
    auto moon = std::make_unique<Light>(LightType::Directional);
    moon->ambient  = glm::vec3(0.05f, 0.03f, 0.01f);
    moon->diffuse  = glm::vec3(0.25f, 0.18f, 0.10f);
    moon->specular = glm::vec3(0.30f, 0.22f, 0.12f);
    moon->direction = glm::vec3(-0.4f, -1.0f, 0.1f);

    moonPtr = moon.get();
    scene.lights.push_back(std::move(moon));

    auto createBlueSpot = [&](glm::vec3 pos){
        auto L = std::make_unique<Light>(LightType::Spot);
        L->position = pos;
        L->spotDirection = glm::normalize(glm::vec3(0.f, 1.0f, 0.0f));

        L->cutOff = glm::cos(glm::radians(30.0f));
        L->outerCutOff = glm::cos(glm::radians(35.0f));

        glm::vec3 noirBlueHDR = glm::vec3(0.15f, 0.30f, 2.2f);
        L->ambient  = noirBlueHDR * 0.07f;
        L->diffuse  = noirBlueHDR * 0.4f;
        L->specular = noirBlueHDR * 0.5f;


        L->constant  = 1.0f;
        L->linear    = 0.006f;
        L->quadratic = 0.001f;

        scene.addEmitterForSpot(*L);
        scene.lights.push_back(std::move(L));
    };
    //Right Wall
    createBlueSpot({-107.2f, -6.6f, 112.5f});
    createBlueSpot({-107.2f, -6.6f, 92.5f});
    createBlueSpot({-107.2f, -6.6f, 72.5f});

    //Left Wall
    createBlueSpot({-145.3f, -6.6f, 112.5f});
    createBlueSpot({-145.3f, -6.6f, 92.5f});
    createBlueSpot({-145.3f, -6.6f, 72.5f});

    //Next to Doors
    createBlueSpot({-112.95f, -6.6f, 55.2f});
    createBlueSpot({-139.5f, -6.6f, 55.2f});

    generateShadowLight();

}

/**
 * Generate main shadow Light
 */
void ClubbingWindow::generateShadowLight() {
    auto shadowLight = std::make_unique<Light>(LightType::Spot);
    shadowLight->position = glm::vec3(0, 46.0, 0.f);

    shadowLight->spotDirection = glm::normalize(glm::vec3(0.f, -1.f, 0.0f));

    shadowLight->cutOff = glm::cos(glm::radians(160.0f));
    shadowLight->outerCutOff = glm::cos(glm::radians(160.0f));

    shadowLight->ambient  = glm::vec3(0.25f, 0.05f, 0.35f);
    shadowLight->diffuse  = glm::vec3(1.0f, 0.1f, 0.3f);
    shadowLight->specular = glm::vec3(1.0f, 0.2f, 0.4f);

    shadowLight->constant  = 1.0f;
    shadowLight->linear    = 0.0017f;
    shadowLight->quadratic = 0.00017;
    scene.shadowLight = shadowLight.get();
    scene.lights.push_back(std::move(shadowLight));
}

/**
 * @param key
 * @param scanCode
 * @param action *
 * @param mods
 */
void ClubbingWindow::onKey(int key, int scanCode, int action, int mods) {
    keys[key] = action;

    if (!scene.camera) return;

    // Camera movement
    // X
    if (keys[GLFW_KEY_A]) {
        scene.camera->moveX(-1);
    }
    if (keys[GLFW_KEY_D]) {
        scene.camera->moveX(1);
    }
    // Y
    if (keys[GLFW_KEY_W]) {
        scene.camera->moveY(1);
    }
    if (keys[GLFW_KEY_S]) {
        scene.camera->moveY(-1);
    }
    // Z
    if (keys[GLFW_KEY_Z]) {
        scene.camera->moveZ(1);
    }
    if (keys[GLFW_KEY_X]) {
        scene.camera->moveZ(-1);
    }
    if (keys[GLFW_KEY_R]) {
        if (musicPlaying) {
            PlaySound(NULL, 0, 0);
            musicPlaying = false;
        } else {
            PlaySound(TEXT("data/audio/audio.wav"), NULL, SND_ASYNC);
            musicPlaying = true;
        }
    }


    // Camera rotation
    if (keys[GLFW_KEY_Q]) {
        scene.camera->rotate(-1);
    }
    if (keys[GLFW_KEY_E]) {
        scene.camera->rotate(1);
    }
    // Change cinematic mode
    if (keys[GLFW_KEY_F]) {
        scene.camera->keyframes.playing = !scene.camera->keyframes.playing;
        scene.camera->useKeyframes = !scene.camera->useKeyframes;
        switch (scene.currentScene) {
            case GameScene::Entrance:
                scene.camera->keyframes.animTime = 0.0f;
                break;
            case GameScene::Interior:
                scene.camera->keyframes.animTime = 48.0f;
                break;
            case GameScene::DanceFloor:
                scene.camera->keyframes.animTime = 120.f;
                break;
            case GameScene::Ending:
                scene.camera->keyframes.animTime = 168.0f;
                break;
        }
    }
    if (keys[GLFW_KEY_L]) {
        if (djsetPtr->cannonLPtr)
            djsetPtr->cannonLPtr->firing = !djsetPtr->cannonLPtr->firing;
    }
    if (keys[GLFW_KEY_K]) {
        if (djsetPtr->cannonRPtr)
            djsetPtr->cannonRPtr->firing = !djsetPtr->cannonRPtr->firing;
    }

    // Debug
    if (keys[GLFW_KEY_C]) {
        if (scene.camera->debugEnabled) {
            scene.camera->debugEnabled = false;
            std::cout << "Camera Disabled" << std::endl;
        } else {
            scene.camera->debugEnabled = true;
            std::cout << "Camera Enabled" << std::endl;
            scene.camera->debug();
        }
    }
    //Debug change scene
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        scene.camera->keyframes.animTime = 0.0f;
        startClub();
    }

    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        scene.camera->keyframes.animTime = 48.0f;
        initInteriorScene();
    }

    if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
        scene.camera->keyframes.animTime = 120.f;
        initDanceFloorScene();
    }

    /* BUG
    if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
        scene.camera->keyframes.animTime = 166.0f;
        initEndingScene();
    }
    */

    //Enable Camera
    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        mouseLookEnabled = !mouseLookEnabled;

        if (mouseLookEnabled) {
            std::cout << "Mouse look ENABLED\n";
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;
        } else {
            std::cout << "Mouse look DISABLED\n";
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

void ClubbingWindow::onMouseMove(double xpos, double ypos) {
    if (!mouseLookEnabled) return;
    if (!scene.camera) return;

    if (firstMouse) {
        lastMouseX = xpos;
        lastMouseY = ypos;
        firstMouse = false;
        return;
    }

    float xoffset = static_cast<float>(xpos - lastMouseX);
    float yoffset = static_cast<float>(ypos - lastMouseY);

    lastMouseX = xpos;
    lastMouseY = ypos;

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    scene.camera->rotation += xoffset; // yaw
    scene.camera->tilt     += yoffset; // pitch

    // Limitar pitch
    if (scene.camera->tilt > 89.0f)
        scene.camera->tilt = 89.0f;
    if (scene.camera->tilt < -89.0f)
        scene.camera->tilt = -89.0f;
}

void ClubbingWindow::onResize(int width, int height)
{
    this->width = width;
    this->height = height;

    glViewport(0, 0, width, height);

    if (scene.camera)
        scene.camera->updateProjection((float)width / (float)height);

    scene.resize(width, height);
}
