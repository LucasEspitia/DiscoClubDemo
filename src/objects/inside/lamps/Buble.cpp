#include "Bubble.h"

std::unique_ptr<ppgso::Mesh> Bubble::bubbleMesh;

Bubble::Bubble(Object* parent, Scene &scene, BubbleConfig cfg) {
    static int nextID = 0;
    id = nextID++;
    this->material = MaterialType::Frosted;

    //Add params in bubble
    parentObject = parent;
    config = cfg;
    position = glm::vec3{0.0f};
    scale = glm::vec3{0.4f,0.4f,0.4} ;

    //Load mesh
    if (!bubbleMesh) bubbleMesh = std::make_unique<ppgso::Mesh>("data/objects/sphere.obj");
    //Base scale of buble.
    baseScale = 0.15f + ((rand() % 100) / 180.0f);
    bubbleRadius = baseScale * 0.5f;

    //Properties of materials.
    mass = pow(baseScale, 3.0f);

    velocity = glm::vec3(
        ((rand() % 100)/100.0f - 0.5f) * 0.7f,
        0.6f + ((rand() % 100)/100.0f) * 0.8f,
        ((rand() % 100)/100.0f - 0.5f) * 0.6f
    );

    position.y = Z_BOTTOM;
}


bool Bubble::update(Scene &scene, float time, float dt,
                    glm::mat4 parentModelMatrix, glm::vec3 parentRotation)
{
    // 1) Real movement
    position += velocity * dt;

    // 2) Bound in top and bottom
    if (position.y + bubbleRadius > Z_TOP) {
        position.y = Z_TOP - bubbleRadius;
        velocity.y *= -0.5f;
    }

    if (position.y - bubbleRadius < Z_BOTTOM) {
        position.y = Z_BOTTOM + bubbleRadius;
        velocity.y = fabs(velocity.y) * 0.8f;
    }

    // 3) Coalition with conic cristal
    float allowedRadius = coneRadiusAt(position.y);
    float distXZ = sqrt(position.x*position.x + position.z*position.z);

    if (distXZ + bubbleRadius > allowedRadius)
    {
        float overlap = (distXZ + bubbleRadius) - allowedRadius;

        glm::vec3 normal = glm::normalize(glm::vec3(position.x, 0, position.z));

        position -= normal * overlap;

        float bounce = 0.6f;
        velocity -= normal * (glm::dot(velocity, normal) * (1.0f + bounce));
    }
    float speed = glm::length(velocity);
    float squash = glm::clamp(speed * 0.1f, 0.0f, 0.25f);


    scale = glm::vec3(
        baseScale * (1.0f - squash),
        baseScale * (1.0f + squash),
        baseScale * (1.0f - squash)
    );

    generateModelMatrix(parentModelMatrix);
    return true;
}

void Bubble::render(Scene &scene, ppgso::Shader &shader) {
    shader.setUniform("ModelMatrix", modelMatrix);
    shader.setUniform("UseTexture", false);

    shader.setUniform("EmissiveColor", config.color);
    shader.setUniform("EmissiveStrength", config.emissiveStrength);
    applyMaterial(shader);
    bubbleMesh->render();

    // restore
    shader.setUniform("EmissiveStrength", 0.0f);
}

float Bubble::coneRadiusAt(float y) {
    //Normalization between height from bottom and top.
    float h = (y - Z_BOTTOM) / HEIGHT;
    h = glm::clamp(h, 0.0f, 1.0f);

    // Lineal interpolation between bottom and top radius
    return R_BOTTOM * (1.0f - h) + R_TOP * h;
}

