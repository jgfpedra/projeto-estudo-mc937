#pragma once
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

/**
 * @brief Define as matrizes de view e projection no shader.
 * 
 * @param shaderProgram Programa de shader a ser usado.
 * @param width Largura da janela.
 * @param height Altura da janela.
 * @param cameraPos Posição da câmera.
 * @param cameraFront Vetor de direção da câmera.
 * @param cameraUp Vetor "up" da câmera.
 */
void setViewProjection(
    GLuint shaderProgram,
    float width, float height,
    const glm::vec3& cameraPos,
    const glm::vec3& cameraFront,
    const glm::vec3& cameraUp
);

/**
 * @brief Processa a entrada do teclado para movimentar a câmera.
 * 
 * @param window Ponteiro para a janela GLFW.
 * @param deltaTime Tempo desde o último frame.
 * @param cameraPos Posição da câmera (modificável).
 * @param cameraFront Direção da câmera (modificável).
 * @param cameraUp Vetor "up" da câmera (modificável).
 */
void processInput(GLFWwindow* window, float deltaTime,
                 glm::vec3& cameraPos, glm::vec3& cameraFront, glm::vec3& cameraUp);

/**
 * @brief Processa o movimento do mouse para atualizar a orientação da câmera.
 * 
 * @param window Ponteiro para a janela GLFW.
 * @param xpos Posição X do mouse.
 * @param ypos Posição Y do mouse.
 * @param yaw Ângulo de yaw (modificável).
 * @param pitch Ângulo de pitch (modificável).
 * @param lastX Última posição X do mouse (modificável).
 * @param lastY Última posição Y do mouse (modificável).
 * @param firstMouse Indica se é o primeiro movimento do mouse (modificável).
 * @param cameraFront Direção da câmera (modificável).
 */
void mouse_callback(GLFWwindow* window, double xpos, double ypos,
                    float& yaw, float& pitch, float& lastX, float& lastY, bool& firstMouse,
                    glm::vec3& cameraFront);