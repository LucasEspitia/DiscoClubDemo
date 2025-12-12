#include "SkyBox.h"

#include "src/scene/Scene.h"

std::unique_ptr<ppgso::Mesh> SkyBox::stret;

SkyBox::SkyBox(Scene &scene) {
    position = glm::vec3(0,0,0);
    scale = glm::vec3(1.0f);

    if (!stret)
        stret = std::make_unique<ppgso::Mesh>("data/objects/sky/cubeInverted.obj");

    std::vector<std::string> faces = {
        "data/objects/sky/right.bmp",
        "data/objects/sky/left.bmp",
        "data/objects/sky/top.bmp",
        "data/objects/sky/bottom.bmp",
        "data/objects/sky/front.bmp",
        "data/objects/sky/back.bmp"
    };

    cubeMapTexture = loadCubeMap(faces);
}

bool SkyBox::update(Scene &scene, float time, float dt,
                    glm::mat4 parentModelMatrix, glm::vec3 parentRotation) {

    generateModelMatrix(glm::mat4(1.0f));
    return true;
}

void SkyBox::render(Scene &scene, ppgso::Shader &shader) {

    shader.setUniform("ModelMatrix", glm::mat4(1.0f));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
    shader.setUniform("skybox", 0);

    stret->render();
}

GLuint SkyBox::loadCubeMap(const std::vector<std::string>& faces) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    for (unsigned int i = 0; i < faces.size(); i++) {
/*
        if (faces[i] == "") {
            // sube un color negro 1x1 para esa cara
            unsigned char blank[3] = {0,0,0};
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, 1, 1, 0,
                GL_RGB, GL_UNSIGNED_BYTE,
                blank
            );
            continue;
        }
*/
        ppgso::Image img = ppgso::image::loadBMP(faces[i]);

        int width = img.width;
        int height = img.height;

        // La imagen está en framebuffer como vector<Pixel>
        std::vector<unsigned char> data;
        data.resize(width * height * 3);

        const auto& fb = img.getFramebuffer();
        for (int p = 0; p < width * height; p++) {
            data[p * 3 + 0] = fb[p].r;
            data[p * 3 + 1] = fb[p].g;
            data[p * 3 + 2] = fb[p].b;
        }

        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0,
            GL_RGB,
            width,
            height,
            0,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            data.data()
        );
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
