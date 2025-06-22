#pragma once
#include <vector>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

struct ModelData {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> faces;
    GLuint VAO, VBO_vertices, VBO_normals, EBO;
};

struct ModelPhysics;

bool loadModel(const char* filename, ModelData& model);
void updateModelsFromPhysics(std::vector<ModelData>& models, const std::vector<ModelPhysics>& physicsModels);
void exportAllModels(const std::vector<ModelData>& models, int frame);
bool loadAllModels(const std::vector<std::string>& filenames, std::vector<ModelData>& models);
