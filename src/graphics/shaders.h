#pragma once
#include <GL/glew.h>
#include <string>
std::string loadShaderSource(const char* filepath);
void configureShaders(GLuint& vs, GLuint& fs, GLuint& shaderProgram);