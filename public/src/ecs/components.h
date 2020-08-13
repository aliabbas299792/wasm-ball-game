#ifndef ECS_COMPONENTS
#define ECS_COMPONENTS

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "header/assimp.h"

struct transform{
  explicit transform(glm::vec3 pos = glm::vec3(0)) {
    this->pos = pos;
  }
  glm::vec3 scale = glm::vec3(1); //so that the models are scaled by 1 by default
  glm::quat orientation = glm::quat(0, 1, 0, 0); //initialise the vector bit of the quaternion to have a magnitude of 1 (otherwise rotations won't work properly)
  glm::vec3 pos{};
  bool updated = true; //when created must draw at least once, then can use cached stuff (for instanced drawing)
};

struct controlledByUser{
  explicit controlledByUser(bool currentlyBeingControlled = false) { this->currentlyBeingControlled = currentlyBeingControlled; }
  bool currentlyBeingControlled = false;
};

struct motion {
  explicit motion(glm::vec3 velocity = glm::vec3(0), glm::vec3 acceleration = glm::vec3(0), bool onGround = true) {
    this->velocity = velocity;
    this->acceleration = acceleration;
    this->onGround = onGround;
  }
  //usual motion stuff
  glm::vec3 velocity{};
  glm::vec3 acceleration{};
  bool onGround = true;
};

struct rotation{
  //rotational motion
  glm::quat rotationalSpeed;
  glm::quat rotationalAcceleration;
};

struct boundingBox{
  //the bounding box
  glm::vec3 _000, _001, _100, _011, _110, _111, _010, _101; //the variable names are the (x, y, z) coordinates of a unit cube
  glm::vec3 origin;
};

struct boundingSphere{
  float radius;
  glm::vec3 origin;
};

struct shaderProperties{
  float opacity = 1;
  bool varyingColours = false;
};

struct lightDirectional{
  float brightness = 1.0;
  glm::vec3 colour{};
};

struct lightPoint{
  float brightness = 1.0;
  glm::vec3 colour{};
  float constant{}, linear{}, quadratic{}; //attenuation
};

struct lightSpot{
  float brightness = 1.0;
  glm::vec3 colour{};
  float constant{}, linear{}, quadratic{}; //attenuation
  float cutoffAngleInDegrees = 30; //the cutoff angle for a spotlight
};

struct modelComponent{
  explicit modelComponent(const std::string& path = "") { this->modelObject = model(path); }
  model modelObject;
};

struct instance{
  explicit instance(int parentModelID = -1) { this->parentModelID = parentModelID; }
  int parentModelID;
};

#endif