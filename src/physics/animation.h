#pragma once
#include "core/model.h"
#include <glm/glm.hpp>
#include <vector>
#include <functional>
#include <GLFW/glfw3.h>

/**
 * @brief Estrutura que representa a física de um vértice.
 */
struct VertexPhysics {
    glm::vec3 position;   ///< Posição do vértice
    glm::vec3 velocity;   ///< Velocidade do vértice
    bool notFalling = false; ///< Indica se o vértice está "preso" (não cai)
    bool fixed = false;      ///< Indica se o vértice está fixo
    float mass;              ///< Massa do vértice
};

/**
 * @brief Estrutura que representa a física de um modelo.
 */
struct ModelPhysics {
    std::vector<VertexPhysics> vertices; ///< Lista de vértices físicos
    glm::vec3 aabbMin, aabbMax;          ///< Bounding box do modelo
    glm::vec3 angularVelocity = glm::vec3(0.0f); ///< Velocidade angular
    glm::vec3 rotation = glm::vec3(0.0f);        ///< Rotação atual
    bool rotationLocked = false;                 ///< Indica se a rotação está travada
    bool applyEquilibriumRotation = true;        ///< Indica se deve buscar equilíbrio rotacional
};

/**
 * @brief Cria modelos físicos a partir dos modelos gráficos.
 * 
 * @param models Modelos gráficos de entrada.
 * @param physicsModels Vetor de saída para os modelos físicos.
 * @param masses Massas de cada modelo.
 * @param initialY Posições iniciais em Y.
 * @param initialX Posições iniciais em X.
 * @param initialZ Posições iniciais em Z.
 */
void createPhysicsModels(
    const std::vector<ModelData>& models,
    std::vector<ModelPhysics>& physicsModels,
    const float masses[3],
    const float initialY[3],
    const float initialX[3],
    const float initialZ[3]);

/**
 * @brief Atualiza a física de todos os modelos.
 * 
 * @param physicsModels Vetor de modelos físicos.
 * @param dt Delta de tempo.
 * @param gravity Gravidade.
 * @param groundY Posição do chão em Y.
 * @param restitution Coeficiente de restituição para cada modelo.
 * @param updateRigidBody Função para atualizar corpos rígidos.
 */
void updateAllPhysics(std::vector<ModelPhysics>& physicsModels, float dt, float gravity, float groundY, const float restitution[3], std::function<void(ModelPhysics&, float, float, float, float)> updateRigidBody);

/**
 * @brief Atualiza a física de um modelo.
 */
void updatePhysics(ModelPhysics& model, float dt, float gravity, float groundY, float restitution);

/**
 * @brief Atualiza a bounding box (AABB) de um modelo.
 */
void updateAABB(ModelPhysics& model);

/**
 * @brief Verifica colisão entre duas AABBs.
 */
bool checkAABBCollision(const ModelPhysics& a, const ModelPhysics& b);

/**
 * @brief Lida com colisões entre todos os modelos físicos.
 */
void handleCollisions(std::vector<ModelPhysics>& physicsModels);

/**
 * @brief Atualiza um corpo rígido.
 */
void updateRigidBody(ModelPhysics& model, float dt, float gravity, float groundY, float restitution);