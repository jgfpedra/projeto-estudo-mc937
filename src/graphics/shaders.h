#pragma once
#include <GL/glew.h>
#include <string>

/**
 * @brief Carrega o código-fonte de um shader a partir de um arquivo.
 * 
 * @param filepath Caminho para o arquivo do shader.
 * @return O código-fonte do shader como uma string.
 */
std::string loadShaderSource(const char* filepath);

/**
 * @brief Compila e configura os shaders e o programa de shader.
 * 
 * @param vs Referência para o identificador do vertex shader.
 * @param fs Referência para o identificador do fragment shader.
 * @param shaderProgram Referência para o identificador do programa de shader.
 */
void configureShaders(GLuint& vs, GLuint& fs, GLuint& shaderProgram);