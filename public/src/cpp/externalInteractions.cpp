#include "header/externalInteractions.h"

////
//Emscripten functions
////

extern "C" {

void EMSCRIPTEN_KEEPALIVE setWindowSize(int width, int height){ //since this is called from JS it needs to be able to access the global variable window from main.cpp
  glfwSetWindowSize(GLOBAL::window, width, height);
  glViewport(0, 0, width, height);
}

}

////
//GLFW Callbacks
////

auto* GLOBAL::user_data = &GLOBAL::userData::getInstance();

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
  if(action == GLFW_PRESS)
    GLOBAL::user_data->keysPressed[key] = true; //this key has been pressed
  if(action == GLFW_RELEASE)
    GLOBAL::user_data->keysPressed[key] = false; //this key has been released
}

//double lastX = 0, lastY = 0, yaw = M_PI_2, pitch = M_PI * 0.333; //needed for the camera movement system
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
  GLOBAL::user_data->cursorPosition.first = xpos;
  GLOBAL::user_data->cursorPosition.second = ypos;

  double yaw = (GLOBAL::user_data->cursorPosition.first/GLOBAL::user_data->screenSize.first - 1.75) * M_PI * 2; //allow infinitely going round in XZ plane
  double pitch = std::max(std::min(-(GLOBAL::user_data->cursorPosition.second/GLOBAL::user_data->screenSize.second - 0.3) * 0.4*M_PI, 0.0), -0.75); //ensures always negative or 0, and more than 0.75

  /* I prefer the mouse movement method above to this stuff
   * yaw += (lastX - xpos)/GLOBAL::user_data->screenSize.first * M_PI * 2;
  pitch += (lastY - ypos)/GLOBAL::user_data->screenSize.second * M_PI;
  pitch = std::max(std::min(pitch, 0.0), -0.75);
  lastX = xpos;
  lastY = ypos;*/

  glm::vec3 direction(0);
  direction.x = cos(yaw) * cos(pitch);
  direction.y = sin(pitch);
  direction.z = sin(yaw) * cos(pitch);

  GLOBAL::user_data->mouseDirection = glm::normalize(direction);
}

void cursor_enter_callback(GLFWwindow* window, int entered){
  GLOBAL::user_data->isCursorInWindow = (bool)entered;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
  if(button == 1)
    GLOBAL::user_data->leftMouseButtonDown = (bool)action;
  if(button == 2)
    GLOBAL::user_data->rightMouseButtonDown = (bool)action;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
  //inverts the directions of increase for scroll, so scroll up is positive, scroll down is negative etc
  GLOBAL::user_data->scrollOffsetScreenIndependent.first += -xoffset/std::max(abs(xoffset), 1.0);
  GLOBAL::user_data->scrollOffsetScreenIndependent.second += -yoffset/std::max(abs(yoffset), 1.0);

  if(GLOBAL::user_data->scrollOffsetScreenIndependent.first > 100) GLOBAL::user_data->scrollOffsetScreenIndependent.first = 100;
  if(GLOBAL::user_data->scrollOffsetScreenIndependent.first < -100) GLOBAL::user_data->scrollOffsetScreenIndependent.first = -100;

  if(GLOBAL::user_data->scrollOffsetScreenIndependent.second > 100) GLOBAL::user_data->scrollOffsetScreenIndependent.second = 100;
  if(GLOBAL::user_data->scrollOffsetScreenIndependent.second < -100) GLOBAL::user_data->scrollOffsetScreenIndependent.second = -100;
}