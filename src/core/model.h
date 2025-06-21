#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>

struct ModelData {
    GLuint VAO = 0, VBO_vertices = 0, VBO_normals = 0, EBO = 0;
    std::vector<glm::vec3> vertices, normals;
    std::vector<unsigned int> faces;
};

bool loadModel(const char* filename, ModelData& model);