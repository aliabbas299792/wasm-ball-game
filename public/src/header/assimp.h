#ifndef ASSIMP
#define ASSIMP

#include "header/shader.h"

#include <vector>

#include <glm/glm.hpp>
#include <emscripten.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "ecs/ecs.h"

struct vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

struct texture {
    unsigned int id;
    std::string type; //diffuse/specular type
    std::string path; //store the path to compare with other textures and prevent duplicates
};

class mesh{
public:
    //mesh data
    std::vector<vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<texture> textures;

    mesh(std::vector<vertex> vertices, std::vector<unsigned int> indices, std::vector<texture> textures);
    void draw();

    ~mesh(){
      vertices.clear();
      indices.clear();
      textures.clear();
    }

    void setInstancing(const std::array<glm::mat4, MAX_ENTITIES> *transformationMatrices, unsigned int maxIndex);
private:
    unsigned int VAO{}, VBO{}, EBO{}, instancedVBO = -1, instances = 0; //render data

    void setupMesh();
};

class model{
private:
    //model data
    std::vector<texture> textures_loaded;
    std::vector<mesh> meshes;
    std::string directory;

    int vertices = 0;
    int indices = 0;

    void loadModel(const std::string& path);
    void processNode(aiNode *node, const aiScene *scene);
    mesh processMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, const std::string& typeName);
public:
    model(const std::string& path = "");
    void setInstancing(const std::array<glm::mat4, MAX_ENTITIES> *transformationMatrices, unsigned int maxIndex);
    void draw();
    ~model(){
      textures_loaded.clear();
      meshes.clear();
      directory = "";
    }
};

#endif
