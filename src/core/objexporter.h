#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <string>

void exportObjFrame(const std::string& filename,
                    const std::vector<glm::vec3>& vertices,
                    const std::vector<glm::vec3>& normals,
                    const std::vector<unsigned int>& faces);