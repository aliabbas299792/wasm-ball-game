#version 300 es
precision mediump float; //needed for OpenGL ES

#define MAX_DIRECTIONAL_LIGHTS 10;
#define MAX_POINT_LIGHTS 100;
#define MAX_SPOT_LIGHTS 100;

struct Material {
  sampler2D diffuse;
  sampler2D specular;
  float shininess;
};

uniform vec2 screenSize;
uniform float time;

uniform vec3 lightColour;
uniform vec3 lightPos;
uniform vec3 viewPos;

in vec3 fragPos;
in vec2 texCoords;
in vec3 normal;

//textures defined via assimp
uniform Material material;

out vec4 FragColour;

void main() {
  float ambientStrength = 0.05f;
  vec3 ambient = ambientStrength * vec3(0.5, 0.6, 0.7);

  vec3 norm = normalize(normal);
  vec3 lightDir = normalize(lightPos - fragPos);

  float diff = max(dot(norm, lightDir), 0.1f) * 0.5f;
  vec3 diffuse = diff * ( texture(material.diffuse, texCoords).rgb + 0.1);

  float specularStrength = 1.0f;
  vec3 viewDir = normalize(viewPos - fragPos);
  vec3 reflectDir = reflect(-lightDir, norm);

  float spec = pow(max(dot(reflectDir, viewDir), 0.0), 32.0);
  vec3 specular = specularStrength * spec * lightColour * texture(material.specular, texCoords).rgb;

  FragColour = vec4((ambient + diffuse + specular), 1.0f);
}