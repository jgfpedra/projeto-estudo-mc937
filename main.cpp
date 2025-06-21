#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "core/window.h"
#include "core/model.h"
#include "graphics/shaders.h"
#include "graphics/mesh.h"
#include "graphics/camera.h"
#include "graphics/ilumination.h"

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " model1.obj model2.obj model3.obj\n";
        return -1;
    }
    float width = 640.0f, height = 480.0f;
    if (!glfwInit()) return -1;
    GLFWwindow* window = createWindow(width, height);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) {
        std::cerr << "ERROR: GLEW Initialization Failed\n";
        return -1;
    }
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint shaderProgram = glCreateProgram();
    configureShaders(vs, fs, shaderProgram);
    std::vector<ModelData> models(3);
    for (int i = 0; i < 3; ++i) {
        if (!loadModel(argv[i+1], models[i])) {
            std::cerr << "Failed to load OBJ file: " << argv[i+1] << std::endl;
            return -1;
        }
    }
    PhongLight light = { glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f) };
    std::vector<PhongMaterial> materials = {
        { glm::vec3(1.0f, 0.5f, 0.5f), 0.1f, 0.5f, 32 }, // vermelho claro
        { glm::vec3(0.5f, 1.0f, 0.5f), 0.2f, 0.7f, 16 }, // verde claro
        { glm::vec3(0.5f, 0.5f, 1.0f), 0.3f, 1.0f, 64 }  // azul claro
    };
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);
        setViewProjection(shaderProgram, width, height);
        glm::vec3 viewPos = glm::vec3(0, 0, 5); // Camera position

        for (int i = 0; i < 3; ++i) {
            setPhongUniforms(shaderProgram, light, materials[i], viewPos);
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3((i-1)*2.0f, 0.0f, 0.0f));
            drawModel(models[i], shaderProgram, model);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    for (auto& m : models) {
        glDeleteVertexArrays(1, &m.VAO);
        glDeleteBuffers(1, &m.VBO_vertices);
        glDeleteBuffers(1, &m.VBO_normals);
        glDeleteBuffers(1, &m.EBO);
    }
    glDeleteProgram(shaderProgram);
    glDeleteShader(vs);
    glDeleteShader(fs);
    glfwTerminate();
    return 0;
}
