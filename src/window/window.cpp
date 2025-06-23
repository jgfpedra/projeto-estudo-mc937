#include <iostream>
#include <GLFW/glfw3.h>

GLFWwindow* createWindow(float width, float height) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, "Projeto-MC937", nullptr, nullptr);
    if (!window) {
        std::cerr << "Nao carregou a janela GLFW" << std::endl;
        return nullptr;
    }
    return window;
}
