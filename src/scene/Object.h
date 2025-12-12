#ifndef DISCOCLUB_OBJECT_H
#define DISCOCLUB_OBJECT_H

#include <memory>
#include <list>

#include <ppgso/shader.h>
#include "ppgso/ppgso.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

// Forward declare a scene
class Scene;

enum class MaterialType {
    Matte,
    Glossy,
    Metal,
    Wood,
    Rubber,
    Frosted,
    Emissive,
    Gold,
    DiscoBall,
    Default
};



/*!
 *  Abstract object interface
 *  All objects must be able to update, render and start the first transformation
 *  to generate its content
 */
class Object {
public:
    //Object properties
    glm::vec3 globalRotation{0,0,0};
    glm::vec3 position{0,0,0};
    glm::vec3 rotation{0,0,0};
    glm::vec3 scale{1,1,1};
    glm::mat4 modelMatrix{1};

    // Creates a List for children
    std::list< std::unique_ptr<Object> > childObjects;
    // Null pointer to father object
    Object* parentObject = nullptr;
    MaterialType material = MaterialType::Default;

    // Default constructors of Object abstract clss
    Object() = default;
    Object(const Object&) = default;
    Object(Object&&) = default;
    virtual ~Object()= default;

    //Properties
    bool castsShadow = true;
    bool isTransparent = false;
    bool isUnlit = false;
    float diffuseStrength = 1.0f;
    float specularStrength = 1.0f;
    float lightAbsorption = 1.0f;
    float shininess = 0.0f;


    //Methods to update and render
    virtual bool update(Scene &scene, float time, float dt, glm::mat4 parentModelMatrix, glm::vec3 parentRotation) = 0;
    bool updateChildren(Scene &scene, float time, float dt, glm::mat4 parentModelMatrix, glm::vec3 parentRotation);

    virtual void render(Scene &scene, ppgso::Shader& shader) = 0;
    void renderChildren(Scene &scene, ppgso::Shader& shader);

    virtual void renderDepth(Scene &scene, ppgso::Shader &shader);
    bool renderDepthChildren(Scene &scene, ppgso::Shader &shader);

    void applyMaterial(ppgso::Shader&);


protected:
    /*!
    * Generate modelMatrix from position, rotation and scale
    */
    void generateModelMatrix(glm::mat4 parentModelMatrix);

    glm::vec3 getWorldPosition() const {
        return glm::vec3(modelMatrix[3]);
    }
};


#endif //DISCOCLUB_OBJECT_H