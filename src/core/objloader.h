#pragma once
#include <vector>
#include <glm/glm.hpp>
bool carregarObj(const char* filename, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<unsigned int>& faces);