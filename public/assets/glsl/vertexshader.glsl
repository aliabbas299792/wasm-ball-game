#version 300 es
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 tex;
layout (location = 3) in mat4 instanceMatrix;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform int usingInstancing;

out vec3 fragPos;
out vec2 texCoords;
out vec3 normal;

out vec3 colour;

void main() {
  mat4 modelMatrix = usingInstancing == 1 ? instanceMatrix : model;

  fragPos = vec3(modelMatrix * vec4(pos, 1.0f));
  texCoords = tex;
  normal = mat3(transpose(inverse(modelMatrix))) * norm;

  gl_Position = projection * view * vec4(fragPos, 1.0);
}