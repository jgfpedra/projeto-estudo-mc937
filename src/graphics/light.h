#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>

/**
 * @brief Estrutura que representa uma luz do tipo Phong.
 */
struct PhongLight {
    glm::vec3 position; ///< Posição da luz
    glm::vec3 color;    ///< Cor da luz
};

/**
 * @brief Estrutura que representa um material para iluminação Phong.
 */
struct PhongMaterial {
    glm::vec3 color;        ///< Cor base do material
    float ambientStrength;  ///< Intensidade do componente ambiente
    float specularStrength; ///< Intensidade do componente especular
    int shininess;          ///< Brilho (expoente especular)
};

/**
 * @brief Define os uniforms de iluminação Phong no shader.
 * 
 * @param shaderProgram Programa de shader a ser usado.
 * @param light Estrutura com os dados da luz.
 * @param material Estrutura com os dados do material.
 * @param viewPos Posição da câmera/observador.
 */
void setPhongUniforms(GLuint shaderProgram, const PhongLight& light, const PhongMaterial& material, const glm::vec3& viewPos);