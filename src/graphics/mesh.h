#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "core/model.h"
#include "graphics/light.h"
#include "graphics/camera.h"
#include "physics/animation.h"

/**
 * @brief Desenha um modelo usando o shader e a matriz de transformação fornecidos.
 * 
 * @param model Dados do modelo a ser desenhado.
 * @param shaderProgram Programa de shader a ser usado.
 * @param modelMatrix Matriz de transformação do modelo.
 */
void drawModel(const ModelData& model, GLuint shaderProgram, const glm::mat4& modelMatrix);

/**
 * @brief Desenha todos os modelos da cena.
 * 
 * @param models Vetor com todos os modelos.
 * @param shaderProgram Programa de shader a ser usado.
 * @param materials Materiais para cada modelo.
 * @param light Luz da cena.
 * @param viewPos Posição da câmera/observador.
 */
void drawAllModels(const std::vector<ModelData>& models, GLuint shaderProgram, const std::vector<PhongMaterial>& materials, const PhongLight& light, const glm::vec3& viewPos);

/**
 * @brief Desenha o chão da cena.
 * 
 * @param groundModel Dados do modelo do chão.
 * @param shaderProgram Programa de shader a ser usado.
 * @param light Luz da cena.
 * @param viewPos Posição da câmera/observador.
 * @param groundY Posição Y do chão.
 */
void drawGround(const ModelData& groundModel, GLuint shaderProgram, const PhongLight& light, const glm::vec3& viewPos, float groundY);

/**
 * @brief Configura os buffers OpenGL (VAO, VBO, EBO) para um mesh.
 * 
 * @param VAO Referência para o Vertex Array Object.
 * @param VBO_vertices Referência para o Vertex Buffer Object dos vértices.
 * @param VBO_normals Referência para o Vertex Buffer Object das normais.
 * @param EBO Referência para o Element Buffer Object (índices).
 * @param vertices Vetor de vértices.
 * @param normals Vetor de normais.
 * @param indices Vetor de índices das faces.
 */
void setupBuffers(GLuint& VAO, GLuint& VBO_vertices, GLuint& VBO_normals, GLuint& EBO,
                  const std::vector<glm::vec3>& vertices,
                  const std::vector<glm::vec3>& normals,
                  const std::vector<unsigned int>& indices);

/**
 * @brief Renderiza a cena usando o shader e buffers fornecidos.
 * 
 * @param shaderProgram Programa de shader a ser usado.
 * @param VAO Vertex Array Object.
 * @param EBO Element Buffer Object.
 * @param facesCount Quantidade de faces a desenhar.
 * @param window Ponteiro para a janela GLFW.
 */
void renderScene(GLuint& shaderProgram, GLuint VAO, GLuint EBO, GLuint facesCount, GLFWwindow* window);

/**
 * @brief Calcula as normais de model
 * 
 * @param model Modelo que ira recalcular as normais
 */
void recalculateNormals(ModelData& model);