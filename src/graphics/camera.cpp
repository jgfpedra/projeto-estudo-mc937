#include "graphics/camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

void setViewProjection(GLuint shaderProgram, float width, float height) {
    glm::mat4 view = glm::lookAt(
        glm::vec3(3, 3, 3),   // posição da câmera (diagonal, acima e à frente)
        glm::vec3(0, 0, 0),   // olha para a origem
        glm::vec3(0, 1, 0)    // "cima" da câmera é o eixo Y
    );
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), width/height, 0.1f, 100.0f);
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
}