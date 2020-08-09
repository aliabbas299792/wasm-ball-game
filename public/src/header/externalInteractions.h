#ifndef EXTERNALINTERACTIONS
#define EXTERNALINTERACTIONS

#include "header/helper.h"

extern "C" {

void EMSCRIPTEN_KEEPALIVE setWindowSize(int width, int height);

}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods); //the key_callback used by glfw
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos); //cursor position callback for glfw
void cursor_enter_callback(GLFWwindow* window, int entered); //callback for when a mouse enters/leaves the window
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods); //callback for when mouse buttons are pressed
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset); //callback for when the user scrolls

#endif
