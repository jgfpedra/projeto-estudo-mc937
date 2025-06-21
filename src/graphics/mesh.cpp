#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "core/model.h"

void drawModel(const ModelData& model, GLuint shaderProgram, const glm::mat4& modelMatrix) {
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glBindVertexArray(model.VAO);
    glDrawElements(GL_TRIANGLES, model.faces.size(), GL_UNSIGNED_INT, 0);
}

void setupBuffers(GLuint& VAO, GLuint& VBO_vertices, GLuint& VBO_normals, GLuint& EBO, const std::vector<glm::vec3>& vertices, const std::vector<glm::vec3>& normals, const std::vector<unsigned int>& indices) {
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO_vertices);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_vertices);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &VBO_normals);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_normals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
}

void renderScene(GLuint& shaderProgram, GLuint VAO, GLuint EBO, GLuint facesCount, GLFWwindow* window) {
    glClearColor(0.2f, 0.2f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // Get matrix uniform locations
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");

    // Set up matrices
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(2.0f)); // Scale 2x bigger

    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 3.0f), // Camera position
        glm::vec3(0.0f, 0.0f, 0.0f),  // Look at center
        glm::vec3(0.0f, 1.0f, 0.0f)); // Up vector

    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),          // FOV
        640.0f/480.0f,                // Aspect ratio
        0.1f, 100.0f);                // Near/far planes

    // Send matrices to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Draw
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glDrawElements(GL_TRIANGLES, facesCount, GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
}