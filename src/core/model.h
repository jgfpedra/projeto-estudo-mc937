#pragma once
#include <vector>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

/**
 * @brief Estrutura que armazena os dados de um modelo 3D.
 */
struct ModelData {
    std::vector<glm::vec3> vertices;   ///< Vetor de vértices do modelo
    std::vector<glm::vec3> normals;    ///< Vetor de normais do modelo
    std::vector<unsigned int> faces;   ///< Vetor de índices das faces
    std::vector<glm::vec2> uvs;        ///< Vetor de coordenadas de textura (UV)
    GLuint VAO, VBO_vertices, VBO_normals, EBO, VBO_uvs; ///< Buffers OpenGL
};

struct ModelPhysics;

/**
 * @brief Carrega um modelo a partir de um arquivo.
 * 
 * @param filename Caminho para o arquivo do modelo.
 * @param model Estrutura onde os dados do modelo serão armazenados.
 * @return true se carregou com sucesso, false caso contrário.
 */
bool loadModel(const char* filename, ModelData& model);

/**
 * @brief Atualiza os modelos gráficos a partir dos dados físicos.
 * 
 * @param models Vetor de modelos gráficos.
 * @param physicsModels Vetor de modelos físicos.
 */
void updateModelsFromPhysics(std::vector<ModelData>& models, const std::vector<ModelPhysics>& physicsModels);

/**
 * @brief Exporta todos os modelos para arquivos OBJ, um por frame.
 * 
 * @param models Vetor de modelos a serem exportados.
 * @param frame Número do frame atual (usado no nome do arquivo).
 */
void exportAllModels(const std::vector<ModelData>& models, int frame);

/**
 * @brief Carrega todos os modelos de uma lista de arquivos.
 * 
 * @param filenames Vetor com os caminhos dos arquivos.
 * @param models Vetor de saída para os modelos carregados.
 * @return true se todos foram carregados com sucesso, false caso contrário.
 */
bool loadAllModels(const std::vector<std::string>& filenames, std::vector<ModelData>& models);
