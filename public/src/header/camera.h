#ifndef CAMERA
#define CAMERA

#include "header/externalInteractions.h"

#include <GLFW/glfw3.h>

class camera{
public:
  void updateProjection(){ //updates the projection matrix as necessary
    int localWidth, localHeight;
    float localAngleRad = (100 - GLOBAL::user_data->scrollOffsetScreenIndependent.second)/200 * M_PI_2;
    glfwGetWindowSize(GLOBAL::window, &localWidth, &localHeight); //sets the width and height variables

    if(localWidth != width || localHeight != height || localAngleRad != angleRad){ //so only runs when the screen size changes
      width = localWidth;
      height = localHeight;
      angleRad = localAngleRad;

      GLOBAL::user_data->screenSize.first = width;
      GLOBAL::user_data->screenSize.second = height;

      glm::mat4 projection = glm::mat4(1.0f);
      projection = glm::perspective((float)angleRad, (float)width/(float)height, 0.1f, 2000.0f);
      shader::getInstance().setUniform4fv("projection", 1, false, glm::value_ptr(projection));

      shader::getInstance().setUniform2f("screenSize", (float)width, (float)height); //sets the size uniform
    }
  }

  static void updateCameraPos(glm::vec3 position, glm::vec3 up = glm::vec3(0, 1, 0)){
    glm::vec3 eye = position - (float)8 * GLOBAL::user_data->mouseDirection;
    glm::mat4 view = glm::lookAt(eye, position, up);
    //set the matrix uniform
    shader::getInstance().setUniform4fv("view", 1, false, glm::value_ptr(view));

    shader::getInstance().setUniform3f("viewPos", eye.x, eye.y, eye.z);
  }

  //singleton stuff
  static camera& getInstance(){
    static camera instance;
    return instance;
  }
  camera(camera const&) = delete; //can't construct this anymore
  void operator=(camera const&) = delete; //can't reassign this to anything anymore
private:
  int width = 0, height = 0;
  float angleRad = 0.5;
  camera() = default;
};

#endif