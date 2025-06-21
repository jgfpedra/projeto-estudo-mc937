#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
void setupBuffers(GLuint& VAO, GLuint& VBO_vertices, GLuint& VBO_normals, GLuint& EBO,
                  const std::vector<glm::vec3>& vertices,
                  const std::vector<glm::vec3>& normals,
                  const std::vector<unsigned int>& indices);
void renderScene(GLuint& shaderProgram, GLuint VAO, GLuint EBO, GLuint facesCount, GLFWwindow* window);