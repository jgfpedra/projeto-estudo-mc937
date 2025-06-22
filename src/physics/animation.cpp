#include "physics/animation.h"
#include "core/model.h"
#include <cmath>
#include <limits>
#include <GLFW/glfw3.h>

void createPhysicsModels(
    const std::vector<ModelData>& models,
    std::vector<ModelPhysics>& physicsModels,
    const float masses[3],
    const float initialY[3],
    const float initialX[3]) {
    physicsModels.resize(models.size());
    for (size_t i = 0; i < models.size(); ++i) {
        for (const auto& v : models[i].vertices) {
            VertexPhysics vp;
            vp.position = v + glm::vec3(initialX[i], initialY[i], 0.0f);
            vp.velocity = glm::vec3(0.0f);
            vp.fixed = false;
            vp.mass = masses[i];
            physicsModels[i].vertices.push_back(vp);
        }
    }
}

void updateAllPhysics(
    std::vector<ModelPhysics>& physicsModels,
    float dt, float gravity, float groundY,
    const float restitution[3],
    std::function<void(ModelPhysics&, float, float, float, float)> updateRigidBody
) {
    updatePhysics(physicsModels[0], dt, gravity, groundY, restitution[0]);
    updateRigidBody(physicsModels[1], dt, gravity, groundY, restitution[1]);
    updatePhysics(physicsModels[2], dt, gravity, groundY, restitution[2]);
}

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

    float minY = model.vertices[0].position.y;
    for (const auto& v : model.vertices) {
        if (v.position.y < minY) minY = v.position.y;
    }

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

    float dragCoef = 0.2f;
    for (auto& v : model.vertices) {
        if (!v.fixed) {
            glm::vec3 drag = -dragCoef * v.velocity;
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

void handleCollisions(std::vector<ModelPhysics>& physicsModels, const float restitution[]) {
    ModelPhysics& greenRigid = physicsModels[1];
    ModelPhysics& blueRubber = physicsModels[2];

    if (checkAABBCollision(greenRigid, blueRubber)) {
        glm::vec3 blueIncomingVel(0.0f);
        if (!blueRubber.vertices.empty()) {
            for(const auto& v : blueRubber.vertices) blueIncomingVel += v.velocity;
            blueIncomingVel /= (float)blueRubber.vertices.size();
        }

        for (auto& v : blueRubber.vertices) {
            if (!v.fixed) v.velocity.y *= -restitution[2];
        }

        glm::vec3 greenAvgVel(0.0f);
        if (!greenRigid.vertices.empty()) {
            for(const auto& v : greenRigid.vertices) greenAvgVel += v.velocity;
            greenAvgVel /= (float)greenRigid.vertices.size();
        }

        float pushFactor = 0.5f; 

        if (blueIncomingVel.y < 0) {
             greenAvgVel.y += (blueIncomingVel.y * pushFactor);
        }
       
        greenAvgVel.y *= -restitution[1];

        for (auto& v : greenRigid.vertices) {
            if (!v.fixed) v.velocity.y = greenAvgVel.y;
        }
    }
}

void updateRigidBody(ModelPhysics& model, float dt, float gravity, float groundY, float restitution) {
    if (model.vertices.empty()) return;
    glm::vec3 avgVel(0.0f), avgPos(0.0f);
    for (auto& v : model.vertices) {
        avgVel += v.velocity;
        avgPos += v.position;
    }
    avgVel /= (float)model.vertices.size();
    avgPos /= (float)model.vertices.size();

    float windStrength = 0.5f * sin(glfwGetTime());
    glm::vec3 wind = glm::vec3(0.0f, 0.0f, windStrength);

    float minY = std::numeric_limits<float>::max();
    for (const auto& v : model.vertices) {
        if (v.position.y < minY) minY = v.position.y;
    }

    bool onGround = (minY <= groundY + 1e-4 && avgVel.y <= 0.0f);
    if (!onGround) {
        avgVel += wind * dt;
    }
    avgVel += glm::vec3(0.0f, -gravity, 0.0f) * dt;
    glm::vec3 proposedPos = avgPos + avgVel * dt;

    minY = std::numeric_limits<float>::max();
    for (const auto& v : model.vertices) {
        float y = proposedPos.y + (v.position.y - avgPos.y);
        if (y < minY) minY = y;
    }

    if (minY < groundY) {
        float delta = groundY - minY;
        proposedPos.y += delta;
        avgVel.y *= -restitution;
    }

    for (auto& v : model.vertices) {
        v.position += (proposedPos - avgPos);
        v.velocity = avgVel;
    }
    
    float damping = 0.98f;
    for (auto& v : model.vertices) {
        if (!v.fixed) v.velocity *= damping;
    }
}