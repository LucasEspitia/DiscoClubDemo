#ifndef DISCOCLUB_TILEGRID_H
#define DISCOCLUB_TILEGRID_H

#include "../../scene/Object.h"
#include <vector>
#include <memory>
#include <ppgso/ppgso.h>

class TileGrid : public Object {
private:
    static std::unique_ptr<ppgso::Mesh> meshTile;
    static std::vector<glm::vec3> tilePalette;

    std::vector<glm::vec3> instancePositions;
    std::vector<glm::vec3> instanceColors;

    GLuint posVBO = 0;
    GLuint colorVBO = 0;

    int width, height;
    std::vector<glm::vec3> baseColors;
public:
    TileGrid(Object* parent, Scene &scene, int width, int height);
    static glm::vec3 randomNeon();

    void animateBreathing(float time);
    void animateWave(float time);
    void animateFlash(float time);

    bool update(Scene &scene, float time, float dt, glm::mat4 parentModelMatrix, glm::vec3 parentRotation) override;
    void render(Scene &scene,  ppgso::Shader&) override;


};

#endif
