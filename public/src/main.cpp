#include "header/helper.h"
#include "header/shader.h"
#include "header/assimp.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>

std::function<void()> lambdaDrawLoop;
void drawLoop() { lambdaDrawLoop(); }

//global window, defined as an extern in helper.h
GLFWwindow *GLOBAL::window = NULL;

int main() {
  //initialises the window (it's a global variable), and initialises OpenGL
  initWindow();

  //makes a shader program
  shader ShaderProgram("assets/glsl/vertexshader.glsl", "assets/glsl/fragshader.glsl");

  //loads models
  model ball("models/roll/roll.obj");
  model platform("models/platform/platform.obj");

  std::vector<glm::mat4> transformationMatrices;
  for(int j = -100; j <= 100; j++) {
    for (int i = 0; i < 100; i++) {
      glm::mat4 matrix = glm::mat4(1.0);
      matrix = glm::translate(matrix, glm::vec3(j*8, 0, -8.0 * i - 3));
      transformationMatrices.push_back(matrix);
    }
  }

  platform.setInstancing(&transformationMatrices);

  //the 4 variables are used to minimise the number of times the projection matrix is updated (so only updated when required)
  int width, height, prevWidth, prevHeight;
  double prevAngleRad = 0.5 ;

  glm::vec3 position = glm::vec3(1.0f, 1, -2.0f);
  glm::vec3 velocity = glm::vec3(0);
  glm::vec3 acceleration = glm::vec3(0);

  double prevTime = glfwGetTime();

  glm::quat orientation = glm::quat(1, glm::vec3(0));

  lambdaDrawLoop = [&] {
      //clear the screen
      glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      double dt = (glfwGetTime() - prevTime) * 60; //this may seem hacky, but it's actually completely valid, I'm just scaling the dt rather than the finely tuned values for motion elsewhere

      //use the shader program
      ShaderProgram.useShaderProgram();

      //updates the projection matrix if required
      double angleRad = (100 - GLOBAL::user_data->scrollOffsetScreenIndependent.second)/200 * M_PI_2;
      updateProjection(&width, &height, &prevWidth, &prevHeight, &ShaderProgram, angleRad, prevAngleRad);
      prevAngleRad = angleRad;

      //set the view matrix
      glm::mat4 view = glm::mat4(1.0f);
      glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);

      acceleration = glm::vec3(0);
      if(GLOBAL::user_data->keysPressed[GLFW_KEY_W])
        acceleration.z = -0.05;
      if(GLOBAL::user_data->keysPressed[GLFW_KEY_S])
        acceleration.z = 0.05;
      if(GLOBAL::user_data->keysPressed[GLFW_KEY_D])
        acceleration.x = 0.05;
      if(GLOBAL::user_data->keysPressed[GLFW_KEY_A])
        acceleration.x = -0.05;

      glm::vec3 dir = GLOBAL::user_data->mouseDirection;
      dir.y = 0;
      dir = -glm::normalize((float)8 * dir);

      glm::vec3 accelerationTemp = -glm::cross(dir, glm::vec3(0,1,0)) * acceleration.x; //horizontal
      accelerationTemp += dir * acceleration.z; //forward/backwards

      float mag = glm::length(velocity * (float)dt);
      glm::vec3 vDir = mag != 0 ? glm::normalize(velocity * (float)dt) : glm::vec3(0);

      if(mag != 0){
        auto perp = glm::cross(vDir, glm::vec3(0, 1, 0));
        auto length = -mag;
        auto rotQuat = glm::quat(cos(length/2), (float)sin(length/2)*perp);

        orientation = rotQuat * orientation;
      }

      velocity -= (float)dt * ( mag*(float)0.07 * vDir ); //like drag

      //leaving it up to drag rolling stops, theoretically, infinitely far in the future
      if(mag < 0.003) //if the speed is near 0, round it to 0
        velocity = glm::vec3(0);

      velocity += (float)dt * accelerationTemp;

      position += (float)dt * velocity;

      glm::vec3 eye = position - (float)8 * GLOBAL::user_data->mouseDirection;
      view = glm::lookAt(eye, position, up);
      //set the matrix uniform
      ShaderProgram.setUniform4fv("view", 1, false, glm::value_ptr(view));

      //set the lighting related uniforms (viewPos is used for specular lighting - if the reflection is towards you it disproportionately lights up)
      ShaderProgram.setUniform3f("lightColour", 0.937f, 1.0f, 1.0f);
      ShaderProgram.setUniform3f("lightPos", 0.0f, 10000.0f, 3.0f);
      ShaderProgram.setUniform3f("viewPos", eye.x, eye.y, eye.z);

      //set some general shader uniforms useful for any effects
      ShaderProgram.setUniform2f("screenSize", (float)width, (float)height); //sets the size uniform
      ShaderProgram.setUniform1f("time", (float)glfwGetTime()); //sets the time uniform

      /////
      //draw the models and set appropriate transformation matrices
      ////
      //update the model matrix
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, position) * glm::toMat4(orientation);
      //model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(1.0f, 1.0f, 1.0f));

      ShaderProgram.setUniform4fv("model", 1, false, glm::value_ptr(model));
      //draw the ball
      ball.draw(&ShaderProgram);

      //update the model matrix
      ShaderProgram.setUniform4fv("model", 1, false, glm::value_ptr(model));
      //draw the platform
      platform.draw(&ShaderProgram);

      //swap the buffers, and poll for events
      glfwSwapBuffers(GLOBAL::window);
      glfwPollEvents();

      prevTime = glfwGetTime();
  };

  //sets the emscripten main loop
  emscripten_set_main_loop(drawLoop, -1, true);
}