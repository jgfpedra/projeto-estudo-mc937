#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <functional>
#include <GLFW/glfw3.h>

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

void updateAllPhysics(
    std::vector<ModelPhysics>& physicsModels,
    float dt, float gravity, float groundY,
    const float restitution[3],
    std::function<void(ModelPhysics&, float, float, float, float)> updateRigidBody
);
void updatePhysics(ModelPhysics& model, float dt, float gravity, float groundY, float restitution);
void updateAABB(ModelPhysics& model);
bool checkAABBCollision(const ModelPhysics& a, const ModelPhysics& b);
void handleCollisions(std::vector<ModelPhysics>& physicsModels);
void updateRigidBody(ModelPhysics& model, float dt, float gravity, float groundY, float restitution);