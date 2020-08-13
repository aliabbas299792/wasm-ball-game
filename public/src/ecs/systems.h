#ifndef ECS_SYSTEMS
#define ECS_SYSTEMS

#include "header/shader.h"

#include "ecs/ecs.h"
#include "ecs/components.h"

#include "header/camera.h"

#include "header/externalInteractions.h"

#include <glm/gtx/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>

class physics : public System {
  double prevTime = glfwGetTime();
public:
  void run(ECS* ecs) override {
    double dt = glfwGetTime() - prevTime;

  }
};

class userControlledMovement : public System{
  double prevTime = glfwGetTime();
public:
  void run(ECS* ecs) override {
    double dt = (glfwGetTime() - prevTime) * 60; //this may seem hacky, but it's actually completely valid, I'm just scaling the dt rather than the finely tuned values for motion elsewhere
    prevTime = glfwGetTime();
    for(auto &entity : entitySet){
      auto currentlyBeingControlled = ecs->componentGetData<controlledByUser>(entity);
      if(currentlyBeingControlled){
        glm::vec3 acceleration(0);
        if(GLOBAL::user_data->keysPressed[GLFW_KEY_W])
          acceleration.z = -1;
        if(GLOBAL::user_data->keysPressed[GLFW_KEY_S])
          acceleration.z += 1;
        if(GLOBAL::user_data->keysPressed[GLFW_KEY_D])
          acceleration.x = 1;
        if(GLOBAL::user_data->keysPressed[GLFW_KEY_A])
          acceleration.x += -1;

        acceleration = glm::length(acceleration) > 0 ? 0.07 * glm::normalize(acceleration) : glm::vec3(0);

        glm::vec3 dir = GLOBAL::user_data->mouseDirection;
        dir.y = 0;
        dir = -glm::normalize((float)8 * dir);

        glm::vec3 accelerationTemp = -glm::cross(dir, glm::vec3(0,1,0)) * acceleration.x; //horizontal
        accelerationTemp += dir * acceleration.z; //forward/backwards

        acceleration = accelerationTemp;

        auto currentMovement = ecs->componentGetData<motion>(entity);
        auto transformComponent = ecs->componentGetData<transform>(entity);

        auto orientation = transformComponent->orientation;
        auto velocity = currentMovement->velocity;

        float mag = glm::length(velocity * (float)dt);
        glm::vec3 vDir = mag != 0 ? glm::normalize(velocity * (float)dt) : glm::vec3(0);

        if(mag != 0){
          auto perp = glm::cross(vDir, glm::vec3(0, 1, 0));
          auto length = -mag;
          auto rotQuat = glm::quat(cos(length/2), (float)sin(length/2)*perp);

          transformComponent->orientation = rotQuat * orientation;
        }

        velocity -= (float)dt * ( mag*(float)0.07 * vDir ); //like drag

        //leaving it up to drag rolling stops, theoretically, infinitely far in the future
        if(mag < 0.003) //if the speed is near 0, round it to 0
          velocity = glm::vec3(0);

        velocity += (float)dt * accelerationTemp;

        transformComponent->pos += (float)dt * velocity;
        currentMovement->velocity = velocity;

        camera::updateCameraPos(transformComponent->pos);
      }
    }
  }
};

class drawInstanced : public System{
  std::unordered_map<unsigned int, std::array<glm::mat4, MAX_ENTITIES>> accumulatedTransforms;
  std::unordered_map<unsigned int, unsigned int> entityToCounter;
public:
  void run(ECS* ecs) override {
    unsigned int counter = 0;
    for(auto &entity : entitySet) {
      auto transformComponent = ecs->componentGetData<transform>(entity);
      if (transformComponent->updated) {
        glm::mat4 modelMat =
          glm::translate(glm::mat4(1.0), transformComponent->pos) *
          glm::toMat4(transformComponent->orientation) *
          glm::scale(glm::mat4(1.0), transformComponent->scale);

        auto modelComponent = ecs->componentGetData<instance>(entity)->parentModelID;

        accumulatedTransforms[modelComponent][counter] = modelMat;
        entityToCounter[modelComponent] = counter;
        counter++;
        transformComponent->updated = false;
      }
    }

    for(auto &transformationEntityPair : accumulatedTransforms){
      auto model = ecs->componentGetData<modelComponent>(transformationEntityPair.first);
      model->modelObject.setInstancing(&transformationEntityPair.second, entityToCounter[transformationEntityPair.first]);
      model->modelObject.draw();
    }
  }
};

class draw : public System{
public:
  void run(ECS* ecs) override {
    for(auto &entity : entitySet){
      auto transformComponent = ecs->componentGetData<transform>(entity);
      glm::mat4 modelMat =
        glm::translate(glm::mat4(1.0), transformComponent->pos) *
        glm::toMat4(transformComponent->orientation) *
        glm::scale(glm::mat4(1.0), transformComponent->scale);

      shader::getInstance().setUniform4fv("model", 1, false, glm::value_ptr(modelMat));

      ecs->componentGetData<modelComponent>(entity)->modelObject.draw();
    }
  }
};

#endif