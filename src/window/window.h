#pragma once
#include <GLFW/glfw3.h>

/**
 * @brief Cria uma janela GLFW com configurações básicas de OpenGL.
 * 
 * @param width Largura da janela.
 * @param height Altura da janela.
 * @return Ponteiro para GLFWwindow se sucesso, nullptr se falhar.
 */
GLFWwindow* createWindow(float width, float height);