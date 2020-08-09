#ifndef HELPER
#define HELPER

#include <functional> //for the lambda stuff
#include <unistd.h> //for the emscripten_set_main_loop function

#include <fstream>
#include <iostream>
#include <vector>

#include <emscripten/emscripten.h>
#include <GLFW/glfw3.h>
#include <GLES3/gl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "header/shader.h"

std::string readFile(const std::string &filename); //actually just calls a JS function to get the files
void initWindow(); //initialises the window
unsigned int loadTextureFromFile(std::string directory, std::string filename); //loads a texture from a file


void updateProjection(int *width, int *height, int *prevWidth, int *prevHeight, shader* ShaderProgram, double angleRad, double prevAngleRad);

template <typename T>
bool inArray(T element, std::vector<T> array);

template <typename T, typename U>
class bimap{
private:
    std::unordered_map<T, U> TtoU;
    std::unordered_map<U, T> UtoT;
public:
    U classicGet(T key){ return TtoU[key]; }
    T reverseGet(U key){ return UtoT[key]; }
    void set(T key, U value) { TtoU[key] = value; UtoT[value] = key; }
    unsigned int classicCount(T key) { return TtoU.count(key); }
    unsigned int reverseCount(U key) { return UtoT.count(key); }
    void classicErase(T key) { UtoT.erase(TtoU[key]); TtoU.erase(key); }
    void reverseErase(U key) { TtoU.erase(UtoT[key]); UtoT.erase(key); }
};

namespace GLOBAL{ //store any global data/variables in this namespace
    extern GLFWwindow *window; //this will be a global variable for use elsewhere

    //singleton https://stackoverflow.com/a/1008289
    class userData{
    public:
        std::unordered_map<int, bool> keysPressed;
        std::pair<double, double> cursorPosition;
        glm::vec3 mouseDirection = glm::vec3(0, -0.3, -1); //the direction in polar coordinates (initialised to a length of 1)
        std::pair<double, double> scrollOffsetScreenIndependent;
        std::pair<int, int> screenSize = std::make_pair(800, 450); //initialise the screensize so it's never 0
        bool isCursorInWindow = false;
        bool leftMouseButtonDown = false;
        bool rightMouseButtonDown = false;

        //singleton stuff
        static userData& getInstance(){
          static userData instance;
          return instance;
        }
        userData(userData const&) = delete; //can't construct this anymore
        void operator=(userData const&) = delete; //can't reassign this to anything anymore
    private:
        userData()= default; //default constructor
    };

    extern userData* user_data;
}

#endif