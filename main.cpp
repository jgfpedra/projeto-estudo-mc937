#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "core/window.h"
#include "core/objloader.h"
#include "graphics/shaders.h"
#include "graphics/mesh.h"

int main(int argc, char* argv[]) {
    GLFWwindow* window;
    GLuint VAO, VBO_vertices, VBO_normals, EBO, vs, fs, shaderProgram;
    std::vector<glm::vec3> vertices, normals;
    std::vector<unsigned int> faces;
    float width = 640.0f, height = 480.0f;
    if (!glfwInit()) {
        return -1;
    }
    window = createWindow(width, height);
    if (!window) {
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) {
        std::cerr << "ERROR: GLEW Initialization Failed\n";
        return -1;
    }

    if (!carregarObj(argv[1], vertices, normals, faces)) {
        std::cerr << "Failed to load OBJ file: " << argv[1] << std::endl;
        return -1;
    }
    setupBuffers(VAO, VBO_vertices, VBO_normals, EBO, vertices, normals, faces);
    vs = glCreateShader(GL_VERTEX_SHADER);
    fs = glCreateShader(GL_FRAGMENT_SHADER);
    shaderProgram = glCreateProgram();
    configureShaders(vs, fs, shaderProgram);
    while (!glfwWindowShouldClose(window)) {
        renderScene(shaderProgram, VAO, EBO, static_cast<GLuint>(faces.size()), window);
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO_vertices);
    glDeleteBuffers(1, &VBO_normals);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glDeleteShader(vs);
    glDeleteShader(fs);
    glfwTerminate();
    return 0;
}
