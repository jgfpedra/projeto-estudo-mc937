#include "physics/animation.h"

void updatePhysics(ModelPhysics& model, float dt, float gravity, float groundY, float restitution) {
    // Atualiza velocidades e posições normalmente
    for (auto& v : model.vertices) {
        if (v.fixed) continue;
        v.velocity += glm::vec3(0.0f, -gravity * dt, 0.0f);
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