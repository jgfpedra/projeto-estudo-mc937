#pragma once
#include <vector>
#include <glm/glm.hpp>

/**
 * @brief Carrega um arquivo OBJ simples (apenas vértices, normais e faces triangulares).
 * 
 * @param filename Caminho para o arquivo OBJ.
 * @param vertices Vetor de saída para os vértices.
 * @param normals Vetor de saída para as normais.
 * @param faces Vetor de saída para os índices das faces.
 * @return true se carregou com sucesso, false caso contrário.
 */
bool loadObj(const char* filename, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<unsigned int>& faces);