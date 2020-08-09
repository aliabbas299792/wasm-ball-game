#include "header/helper.h"
#include "header/externalInteractions.h"

#define STB_IMAGE_IMPLEMENTATION //since we're going to make functions which use it
#include <stb/stb_image.h>

extern "C"{
  extern const char* getFile(const char* filename, int length);
}

template <typename T>
bool inArray(T element, std::vector<T> array){
  bool presentInArray = false;
  for(const auto &item : array){
    if(item == element){ //maybe look into SFINAE stuff to check if the type T actually has the == operator
      presentInArray = true;
      break;
    }
  }
  return presentInArray;
}

std::string readFile(const std::string &filename){ //calls a JavaScript function to get the data (it should be pre-fetched however)
  const char* data = getFile(filename.c_str(), filename.size()); //get the file via JavaScript
  const std::string returnData = data; //make a copy of the string
  free((void*)data); //free the allocated memory

  return returnData;
}

unsigned int loadTextureFromFile(std::string directory, std::string filename) {
  //stbi_set_flip_vertically_on_load(true);
  //temporarily disabled, since it appears blender models are actually oriented the correct way so no need to flip - in fact flipping it screws up the texture
  //OpenGL expects the 0.0 coordinate on the y-axis to be on the bottom side of the image, but images usually have 0.0 at the top of the y-axis
  //This vertically flips it
  //directly quoted from learnopengl.com

  std::string filepath = directory + '/' + filename;

  int width, height, numChannels;
  unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &numChannels, 0);
  //set last param to 0 to allow however many channels there are normally (usually 8, i.e usually 8 bits per pixel)

  unsigned int texture;
  glGenTextures(1, &texture); //generates a texture
  glBindTexture(GL_TEXTURE_2D, texture);

  //s, t and r are the texture equivalents for x, y and z
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  //GL_LINEAR basically blurs it by taking the average of pixels if the texture is scaled up (so kinda smoother), and it does the same for mipmaps too
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  if(data){
    GLenum colourComponents;
    switch (numChannels) {
      case 1:
        colourComponents = GL_RED; //according to stb_image.h this is the 'grey' component, GL_RED is also a single component colour
        //there isn't a colour+alpha representation in OpenGL as far as I can tell from the reference pages, so I'm missing out the case of when numChannels is 2
        break;
      case 3:
        colourComponents = GL_RGB;
        break;
      case 4:
        colourComponents = GL_RGBA;
        break;
    }

    glTexImage2D( //set properties of the texture
      GL_TEXTURE_2D,
      0/*mip map level*/,
      colourComponents/*colour components in the image*/,
      width,
      height,
      0,
      colourComponents/*format of the pixel data*/,
      GL_UNSIGNED_BYTE,
      data
    );
    glGenerateMipmap(GL_TEXTURE_2D); //currently bound texture object now has the texture image attached to it

    stbi_image_free(data); //frees the memory for this image
  }else{
    std::cout << "There was an error with loading the image from: " << filepath << "\n";
    stbi_image_free(data); //frees the memory for this image
    emscripten_force_exit(-1); //there was an error
  }

  return texture;
}

void initWindow(){ //will create the window using the global window
  if(!glfwInit()){
    std::cout << "Failed to initialise GLFW: " << stderr << "\n";
    emscripten_force_exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
  glfwWindowHint(GLFW_DEPTH_BITS, 24);

  GLOBAL::window  = glfwCreateWindow(800, 450, "Window", NULL, NULL); //initial size of 800x450

  if(!GLOBAL::window){
    std::cout << "Failed to create GLFW window" << "\n";
    glfwTerminate();
    emscripten_force_exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(GLOBAL::window);

  glViewport(0, 0, 800, 450);
  glEnable(GL_DEPTH_TEST);

  glfwSetInputMode(GLOBAL::window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //captures the mousew

  //setting the callbacks
  glfwSetScrollCallback(GLOBAL::window, scroll_callback);
  glfwSetKeyCallback(GLOBAL::window, key_callback);
  glfwSetCursorPosCallback(GLOBAL::window, cursor_position_callback);
  glfwSetCursorEnterCallback(GLOBAL::window, cursor_enter_callback);
  glfwSetMouseButtonCallback(GLOBAL::window, mouse_button_callback);
}

void updateProjection(int *width, int *height, int *prevWidth, int *prevHeight, shader* ShaderProgram, double angleRad, double prevAngleRad){
  *prevWidth = *width;
  *prevHeight = *height;
  glfwGetWindowSize(GLOBAL::window, width, height); //sets the width and height variables

  if(*width != *prevWidth || *height != *prevHeight || angleRad != prevAngleRad){ //so only runs when the screen size changes
    GLOBAL::user_data->screenSize.first = *width;
    GLOBAL::user_data->screenSize.second = *height;

    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective((float)angleRad, (float)*width/(float)*height, 0.1f, 2000.0f);
    ShaderProgram->setUniform4fv("projection", 1, false, glm::value_ptr(projection));
  }
}

