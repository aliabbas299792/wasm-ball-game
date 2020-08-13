#include <utility>

#include "header/assimp.h"

mesh::mesh(std::vector<vertex> vertices, std::vector<unsigned int> indices, std::vector<texture> textures) {
  this->vertices = std::move(vertices);
  this->indices = std::move(indices);
  this->textures = std::move(textures);

  setupMesh();
}

void mesh::draw() {
  auto ShaderProgram = &shader::getInstance();

  unsigned int diffuseNumber = 1; //up to 3 diffuse textures
  unsigned int specularNumber = 1; //up to 3 specular textures

  for(unsigned int i = 0; i < textures.size(); i++){
    glActiveTexture(GL_TEXTURE0 + i); //activate the proper texture binding

    std::string number;
    std::string name = textures[i].type;
    if(name == "texture_diffuse")
      number = std::to_string(diffuseNumber++);
    else if(name == "texture_specular")
      number = std::to_string(specularNumber++);

    std::string matName = "material.";
    matName += name;
    matName += number;
    ShaderProgram->setUniform1f(matName, (float)i); //sets the uniform and adds material. to the beginning
    glBindTexture(GL_TEXTURE_2D, textures[i].id);
  }
  glActiveTexture(GL_TEXTURE0); //resets the active texture back to 0

  //draws the mesh
  glBindVertexArray(VAO);

  if(instances == 0){
    ShaderProgram->setUniform1i("usingInstancing", 0);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0/*it is non-zero only when an element buffer object (EBO) is not used*/);
  }else{
    ShaderProgram->setUniform1i("usingInstancing", 1);
    glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0/*it is non-zero only when an element buffer object (EBO) is not used*/, instances);
  }

  glBindVertexArray(0); //reset the bound vertex array
}

void mesh::setupMesh() {
  glGenVertexArrays(1, &VAO); //generate VAO
  //generate buffers for the data
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO); //bind VAO
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), vertices.data(), GL_STATIC_DRAW); //copies the buffer data into memory

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); //ebo object
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned  int), indices.data(), GL_STATIC_DRAW); //copy the indexed ebo data

  //vector.data() basically returns &vector[0] - a pointer to the first element

  int stride = 8 * sizeof(float); //8 floats per element

  //vertex positions
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0); //for basically telling how OpenGL should interpret the VBO data
  glEnableVertexAttribArray(0);
  //vertex normals
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float))); //offset of 3
  glEnableVertexAttribArray(1);
  //vertex texture coords
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float))); //offset of 6
  glEnableVertexAttribArray(2);

  glBindVertexArray(0); //resets the existing vertex array object binding

  //the reason why using vectors and structs works is because they are all sequential in memory, otherwise you can't use them in the buffer data
  //an array of structs of size 10, where each struct has 3 elements is like an array of 30 elements sequential in memory
}

void mesh::setInstancing(const std::array<glm::mat4, MAX_ENTITIES> *transformationMatrices, unsigned int maxIndex) {
  if(instancedVBO == -1)
    glGenBuffers(1, &instancedVBO);

  glBindBuffer(GL_ARRAY_BUFFER, instancedVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * (maxIndex+1), transformationMatrices->data(), GL_STATIC_DRAW); //look at the comment on (or around) line 53
  glBindBuffer(GL_ARRAY_BUFFER, 0); //resets the array buffer binding

  //you can only have up to 4 elements per location, so a mat4 in GLSL uses 4 consecutive locations for its 16 elements, so in our case locations 2, 3, 4 and 5 are used
  glBindBuffer(GL_ARRAY_BUFFER, instancedVBO);

  glBindVertexArray(VAO); //binds the vertex array that we were using earlier

  for(int i = 0; i < 4; i++){
    glEnableVertexAttribArray(3 + i);
    glVertexAttribPointer(3 + i, 4/*4 elements per location*/, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(i * sizeof(glm::vec4)));
    glVertexAttribDivisor(3 + i, 1); //this location will be incremented upon
  }

  instances = maxIndex+1; //one additional instance per transformation matrix

  glBindVertexArray(0); //unbinds the vertex array
}