#pragma once
#include <glm/glm.hpp>
#include <vector>

struct VertexPhysics {
    glm::vec3 position;
    glm::vec3 velocity;
    bool fixed = false;
    float mass;
};

struct ModelPhysics {
    std::vector<VertexPhysics> vertices;
    glm::vec3 aabbMin, aabbMax;
};

void updatePhysics(ModelPhysics& model, float dt, float gravity, float groundY, float restitution = 0.0f);
void updateAABB(ModelPhysics& model);
bool checkAABBCollision(const ModelPhysics& a, const ModelPhysics& b);