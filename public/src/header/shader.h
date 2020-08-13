#ifndef SHADER
#define SHADER

#include <iostream>
#include <unordered_map>
#include <GLFW/glfw3.h>
#include <GLES3/gl3.h>
#include <emscripten.h>

class shader{
public:
  unsigned int useShaderProgram();
  void setUniform1f(std::string uniformName, float value);
  void setUniform1i(std::string uniformName, int value);
  void setUniform2f(std::string uniformName, float value1, float value2);
  void setUniform3f(std::string uniformName, float value1, float value2, float value3);
  void setUniform4fv(std::string uniformName, int count, bool transpose, const float* value);
  static void initSources(const std::string& vertexShaderSource, const std::string& fragmentShaderSource){
    vertSource = vertexShaderSource;
    fragSource = fragmentShaderSource;
  }

  //singleton stuff
  static shader& getInstance(){
    static shader instance;

    if(instance.shaderProgramID == -1) {
      if (!shader::vertSource.empty() && !shader::fragSource.empty()) {
        instance.init(shader::vertSource, shader::fragSource); //initialise the instance
      } else {
        std::cout << "Initialise first by using the initSource(...) function (with non empty source strings)" << "\n";
        emscripten_force_exit(-1);
      }
    }

    return instance;
  }
  shader(shader const&) = delete; //can't construct this anymore
  void operator=(shader const&) = delete; //can't reassign this to anything anymore
private:
  void init(const std::string& vertexShaderSource, const std::string& fragmentShaderSource);
  static unsigned int compileShader(const std::string& fileName, GLenum shader_type);
  unsigned int compileVertexShader(const std::string& fileName);
  unsigned int compileFragmentShader(const std::string& fileName);
  unsigned int shaderProgramID = -1;

  static std::string vertSource;
  static std::string fragSource;

  shader() = default; //default constructor
};

#endif
