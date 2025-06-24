#include "graphics/light.h"

void setPhongUniforms(GLuint shaderProgram, const PhongLight& light, const PhongMaterial& material, const glm::vec3& viewPos) {
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, &light.position[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor"), 1, &light.color[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, &material.color[0]);
    glUniform1f(glGetUniformLocation(shaderProgram, "ambientStrength"), material.ambientStrength);
    glUniform1f(glGetUniformLocation(shaderProgram, "specularStrength"), material.specularStrength);
    glUniform1f(glGetUniformLocation(shaderProgram, "shininess"), material.shininess);
    glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, &viewPos[0]);
}
