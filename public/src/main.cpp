#include "header/helper.h"
#include "header/shader.h"
#include "header/camera.h"

#include "ecs/components.h"
#include "ecs/systems.h"
#include "ecs/ecs.h"

std::function<void()> lambdaDrawLoop;
void drawLoop() { lambdaDrawLoop(); }

//global window, defined as an extern in helper.h
GLFWwindow *GLOBAL::window = NULL;

int main() {
  //initialises the window (it's a global variable), and initialises OpenGL
  initWindow();

  ECS ecs;
  ecs.componentRegister<transform>();
  ecs.componentRegister<motion>();
  ecs.componentRegister<rotation>();
  ecs.componentRegister<controlledByUser>();
  ecs.componentRegister<modelComponent>();
  ecs.componentRegister<instance>();

  ecs.systemRegister<physics>();
  ecs.systemAddComponentToSignature<physics, transform>();
  ecs.systemAddComponentToSignature<physics, motion>();
  ecs.systemAddComponentToSignature<physics, rotation>();

  ecs.systemRegister<userControlledMovement>();
  ecs.systemAddComponentToSignature<userControlledMovement, transform>();
  ecs.systemAddComponentToSignature<userControlledMovement, motion>();
  ecs.systemAddComponentToSignature<userControlledMovement, rotation>();
  ecs.systemAddComponentToSignature<userControlledMovement, controlledByUser>();

  ecs.systemRegister<draw>();
  ecs.systemAddComponentToSignature<draw, transform>();
  ecs.systemAddComponentToSignature<draw, modelComponent>();

  ecs.systemRegister<drawInstanced>();
  ecs.systemAddComponentToSignature<drawInstanced, transform>();
  ecs.systemAddComponentToSignature<drawInstanced, instance>();

  //makes a shader program
  shader::initSources("assets/glsl/vertexshader.glsl", "assets/glsl/fragshader.glsl");
  auto ShaderProgram = &shader::getInstance();

  //user controlled ball
  Entity user = ecs.entityCreate();
  ecs.componentAddEntity(user, transform(glm::vec3(1, 1, -2)));
  ecs.componentAddEntity(user, motion());
  ecs.componentAddEntity(user, rotation());
  ecs.componentAddEntity(user, controlledByUser(true)); //mark this one as being controlled by the user
  ecs.componentAddEntity(user, modelComponent("models/roll/roll.obj"));

  //instanced platforms
  auto platformModel = ecs.entityCreate();
  ecs.componentAddEntity(platformModel, modelComponent("models/platform/platform.obj"));

  std::vector<glm::mat4> transformationMatrices;
  for(int j = -50; j < 50; j++) {
    for (int i = 0; i < 100; i++) {
      auto platform = ecs.entityCreate();
      ecs.componentAddEntity(platform, instance(platformModel));
      ecs.componentAddEntity(platform, transform(glm::vec3(j*8, -0.4, -8.0 * i - 3)));
    }
  }

  lambdaDrawLoop = [&] {
      //clear the screen
      glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      //updates the projection matrix if required
      camera::getInstance().updateProjection();

      //set the lighting related uniforms (viewPos is used for specular lighting - if the reflection is towards you it disproportionately lights up)
      ShaderProgram->setUniform3f("lightColour", 0.937f, 1.0f, 1.0f);
      ShaderProgram->setUniform3f("lightPos", 0.0f, 10000.0f, 3.0f);

      //set some general shader uniforms useful for any effects
      ShaderProgram->setUniform1f("time", (float)glfwGetTime()); //sets the time uniform

      //user movement
      ecs.systemRun<userControlledMovement>(&ecs);
      //draw
      ecs.systemRun<draw>(&ecs);
      //draw instanced
      ecs.systemRun<drawInstanced>(&ecs);

      //swap the buffers, and poll for events
      glfwSwapBuffers(GLOBAL::window);
      glfwPollEvents();
  };

  //sets the emscripten main loop
  emscripten_set_main_loop(drawLoop, -1, true);
}