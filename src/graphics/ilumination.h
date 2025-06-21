#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>

struct PhongLight {
    glm::vec3 position;
    glm::vec3 color;
};

struct PhongMaterial {
    glm::vec3 color;
    float ambientStrength;
    float specularStrength;
    int shininess;
};

void setPhongUniforms(GLuint shaderProgram, const PhongLight& light, const PhongMaterial& material, const glm::vec3& viewPos);