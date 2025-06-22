#include "physics/animation.h"
#include <GLFW/glfw3.h>

void updatePhysics(ModelPhysics& model, float dt, float gravity, float groundY, float restitution) {
    float windStrength = 0.5f * sin(glfwGetTime());
    glm::vec3 wind = glm::vec3(0.0f, 0.0f, windStrength);

    for (auto& v : model.vertices) {
        if (v.fixed) continue;
        glm::vec3 force = glm::vec3(0.0f, -gravity * v.mass, 0.0f); // gravidade
        bool onGround = (v.position.y <= groundY + 1e-4 && v.velocity.y <= 0.0f);
        if (!onGround) {
            force += wind * v.mass; // vento
        }

        glm::vec3 acceleration = force / v.mass;
        v.velocity += acceleration * dt;
        v.position += v.velocity * dt;
    }

    // Encontra o menor Y após o movimento
    float minY = model.vertices[0].position.y;
    for (const auto& v : model.vertices) {
        if (v.position.y < minY) minY = v.position.y;
    }

    // Se algum vértice passou do chão, corrija todos juntos
    if (minY < groundY) {
        float delta = groundY - minY;
        for (auto& v : model.vertices) {
            if (!v.fixed) {
                v.position.y += delta;
                v.velocity.y *= -restitution;
            }
        }
    }

    float damping = 0.98f;
    for (auto& v : model.vertices) {
        if (!v.fixed) v.velocity *= damping;
    }

    float dragCoef = 0.2f; // ajuste para mais/menos efeito
    for (auto& v : model.vertices) {
        if (!v.fixed) {
            glm::vec3 drag = -dragCoef * v.velocity; // força de arrasto
            glm::vec3 force = glm::vec3(0.0f, -gravity * v.mass, 0.0f) + wind * v.mass + drag;
            glm::vec3 acceleration = force / v.mass;
            v.velocity += acceleration * dt;
            v.position += v.velocity * dt;
        }
    }
}

void updateAABB(ModelPhysics& model) {
    if (model.vertices.empty()) return;
    glm::vec3 min = model.vertices[0].position;
    glm::vec3 max = model.vertices[0].position;
    for (const auto& v : model.vertices) {
        min = glm::min(min, v.position);
        max = glm::max(max, v.position);
    }
    model.aabbMin = min;
    model.aabbMax = max;
}

bool checkAABBCollision(const ModelPhysics& a, const ModelPhysics& b) {
    return (a.aabbMin.x <= b.aabbMax.x && a.aabbMax.x >= b.aabbMin.x) &&
           (a.aabbMin.y <= b.aabbMax.y && a.aabbMax.y >= b.aabbMin.y) &&
           (a.aabbMin.z <= b.aabbMax.z && a.aabbMax.z >= b.aabbMin.z);
}