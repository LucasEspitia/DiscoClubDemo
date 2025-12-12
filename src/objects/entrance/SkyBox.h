#ifndef DISCOCLUB_SKYBOX_H
#define DISCOCLUB_SKYBOX_H
#include "src/scene/Object.h"

class SkyBox : public Object {
public:
    static std::unique_ptr<ppgso::Mesh> stret;
    GLuint cubeMapTexture;

    SkyBox(Scene &scene);

    bool update(Scene &scene, float time, float dt,
                glm::mat4 parentModelMatrix = glm::mat4(1.0f),
                glm::vec3 parentRotation = glm::vec3(0)) override;

    void render(Scene &scene, ppgso::Shader &shader) override;
    void renderDepth(Scene&, ppgso::Shader&) override {}

private:
    static GLuint loadCubeMap(const std::vector<std::string> &faces);
};

#endif //DISCOCLUB_SKYBOX_H