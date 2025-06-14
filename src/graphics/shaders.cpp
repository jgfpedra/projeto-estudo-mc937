#pragma once
#include <GL/glew.h>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

std::string loadShaderSource(const char* filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir shader: " << filepath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void configureShaders(GLuint& vs, GLuint& fs, GLuint& shaderProgram) {
    std::string vertexCode = loadShaderSource("shaders/vertex_shader.glsl");
    std::string fragmentCode = loadShaderSource("shaders/fragment_shader.glsl");
    const char* vertex_shader = vertexCode.c_str();
    const char* fragment_shader = fragmentCode.c_str();

    glShaderSource(vs, 1, &vertex_shader, nullptr);
    glCompileShader(vs);
    glShaderSource(fs, 1, &fragment_shader, nullptr);
    glCompileShader(fs);
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);
}