#pragma once
#include <glm/glm.hpp>
#include <GL/glew.h>

void setViewProjection(
    GLuint shaderProgram,
    float width, float height,
    const glm::vec3& cameraPos,
    const glm::vec3& cameraFront,
    const glm::vec3& cameraUp
);