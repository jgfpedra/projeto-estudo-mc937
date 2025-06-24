#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <string>

/**
 * @brief Exporta um frame para um arquivo OBJ.
 * 
 * @param filename Nome do arquivo de saída.
 * @param vertices Vetor de vértices.
 * @param normals Vetor de normais.
 * @param faces Vetor de índices das faces.
 */
void exportObjFrame(const std::string& filename,
                    const std::vector<glm::vec3>& vertices,
                    const std::vector<glm::vec3>& normals,
                    const std::vector<unsigned int>& faces);