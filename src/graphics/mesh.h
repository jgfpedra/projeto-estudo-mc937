#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "core/objloader.h"
#include "core/model.h"
#include "core/objexporter.h"
#include "graphics/shaders.h"
#include "graphics/light.h"
#include "graphics/camera.h"
#include "physics/animation.h"

void drawModel(const ModelData& model, GLuint shaderProgram, const glm::mat4& modelMatrix);
void drawAllModels(const std::vector<ModelData>&, GLuint, const std::vector<PhongMaterial>&, const PhongLight&, const glm::vec3&);
void drawGround(const ModelData&, GLuint, const PhongLight&, const glm::vec3&, float);
void setupBuffers(GLuint& VAO, GLuint& VBO_vertices, GLuint& VBO_normals, GLuint& EBO,
                  const std::vector<glm::vec3>& vertices,
                  const std::vector<glm::vec3>& normals,
                  const std::vector<unsigned int>& indices);
void renderScene(GLuint& shaderProgram, GLuint VAO, GLuint EBO, GLuint facesCount, GLFWwindow* window);