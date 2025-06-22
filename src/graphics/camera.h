#pragma once
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

void setViewProjection(
    GLuint shaderProgram,
    float width, float height,
    const glm::vec3& cameraPos,
    const glm::vec3& cameraFront,
    const glm::vec3& cameraUp
);

void processInput(GLFWwindow* window, float deltaTime,
                 glm::vec3& cameraPos, glm::vec3& cameraFront, glm::vec3& cameraUp);
void mouse_callback(GLFWwindow* window, double xpos, double ypos,
                    float& yaw, float& pitch, float& lastX, float& lastY, bool& firstMouse,
                    glm::vec3& cameraFront);