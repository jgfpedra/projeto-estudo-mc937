#include "physics/animation.h"
#include "core/model.h"
#include <cmath>
#include <limits>
#include <GLFW/glfw3.h>

float getMinY(const std::vector<VertexPhysics>& vertices) {
    float minY = std::numeric_limits<float>::max();
    for (const auto& v : vertices)
        if (v.position.y < minY) minY = v.position.y;
    return minY;
}

void computeCenterAndAvgVel(const std::vector<VertexPhysics>& vertices, glm::vec3& center, glm::vec3& avgVel) {
    center = avgVel = glm::vec3(0.0f);
    for (const auto& v : vertices) {
        center += v.position;
        avgVel += v.velocity;
    }
    center /= (float)vertices.size();
    avgVel /= (float)vertices.size();
}

bool isStableOnGround(const glm::vec3& center, const glm::vec3& avgVel, float groundY) {
    return (fabs(center.y - groundY) < 1e-3) && (glm::length(avgVel) < 0.05f);
}

void createPhysicsModels(
    const std::vector<ModelData>& models,
    std::vector<ModelPhysics>& physicsModels,
    const float masses[3],
    const float initialY[3],
    const float initialX[3],
    const float initialZ[3]) {
    physicsModels.resize(models.size());
    for (size_t i = 0; i < models.size(); ++i) {
        for (const auto& v : models[i].vertices) {
            VertexPhysics vp;
            vp.position = v + glm::vec3(initialX[i], initialY[i], initialZ[i]);
            vp.velocity = glm::vec3(0.0f);
            vp.fixed = false;
            vp.mass = masses[i];
            physicsModels[i].vertices.push_back(vp);
        }
    }
}

glm::vec3 applyRotationXYZ(const glm::vec3& pos, const glm::vec3& rot) {
    float cx = cos(rot.x), sx = sin(rot.x);
    float cy = cos(rot.y), sy = sin(rot.y);
    float cz = cos(rot.z), sz = sin(rot.z);
    glm::vec3 p = pos;
    float y1 = cx * p.y - sx * p.z;
    float z1 = sx * p.y + cx * p.z;
    p.y = y1;
    p.z = z1;
    float x2 = cy * p.x + sy * p.z;
    float z2 = -sy * p.x + cy * p.z;
    p.x = x2;
    p.z = z2;
    float x3 = cz * p.x - sz * p.y;
    float y3 = sz * p.x + cz * p.y;
    p.x = x3;
    p.y = y3;
    return p;
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

void rotateModelAroundCenter(ModelPhysics& model) {
    glm::vec3 center(0.0f);
    for (const auto& v : model.vertices) center += v.position;
    center /= (float)model.vertices.size();
    for (auto& v : model.vertices) {
        glm::vec3 p = v.position - center;
        p = applyRotationXYZ(p, model.rotation);
        v.position = p + center;
    }
}

void updatePhysics(ModelPhysics& model, float dt, float gravity, float groundY, float restitution) {
    float windStrength = 1.0f * sin(glfwGetTime());
    glm::vec3 wind = glm::vec3(0.0f, 0.0f, windStrength);
    float damping = 0.98f;
    float dragCoef = 0.2f;
    for (auto& v : model.vertices) {
        if (v.fixed) continue;
        if (v.notFalling) {
            glm::vec3 force = 5.0f * wind * v.mass;
            glm::vec3 drag = -dragCoef * v.velocity; // Arrasto do vento
            force += drag;

            glm::vec3 acceleration = force / v.mass;
            acceleration[0] = 0;
            acceleration[1] = 0;
            v.velocity += acceleration * dt;
            v.position += v.velocity * dt;  
            continue;
        }

        glm::vec3 force = glm::vec3(0.0f, -gravity * v.mass, 0.0f); // gravidade
        bool onGround = (v.position.y <= groundY + 1e-4 && v.velocity.y <= 0.0f);
        if (!onGround) {
            force += wind * v.mass;
            glm::vec3 drag = -dragCoef * v.velocity;
            force += drag;
        }
        glm::vec3 acceleration = force / v.mass;
        v.velocity += acceleration * dt;
        v.position += v.velocity * dt;
    }
    float minY = std::numeric_limits<float>::max();
    for (const auto& v : model.vertices) {
        if (v.position.y < minY) {
            minY = v.position.y;
        }
    }
    if (minY < groundY) {
        float delta = groundY - minY;
        for (auto& v : model.vertices) {
            if (!v.fixed) {
                v.position.y += delta;
                if(v.velocity.y < 0) {
                   v.velocity.y *= -restitution;
                }
            }
        }
    }
    for (auto& v : model.vertices) {
        if (!v.fixed) {
            v.velocity *= damping;
        }
    }
    model.rotation += model.angularVelocity * dt;
    model.angularVelocity *= 0.98f;
    bool onGround = (minY <= groundY + 1e-4);
    
    // Check if this model has any fixed vertices
    bool hasFixedVertices = false;
    for (const auto& v : model.vertices) {
        if (v.fixed) {
            hasFixedVertices = true;
            break;
        }
    }
    
    // Only apply equilibrium-seeking to models without fixed vertices
    if (onGround && !hasFixedVertices) {
        // Calculate center of mass
        glm::vec3 center(0.0f);
        for (const auto& v : model.vertices) {
            center += v.position;
        }
        center /= (float)model.vertices.size();
        
        // Find lowest point
        minY = std::numeric_limits<float>::max();
        glm::vec3 lowestPoint(0.0f);
        for (const auto& v : model.vertices) {
            if (v.position.y < minY) {
                minY = v.position.y;
                lowestPoint = v.position;
            }
        }
        
        // Vector from lowest point to center of mass 
        glm::vec3 toCenter = center - lowestPoint;
        
        // If center of mass is not directly above the lowest point,
        // apply torque to rotate toward equilibrium
        float horizontalOffset = glm::length(glm::vec2(toCenter.x, toCenter.z));
        if (horizontalOffset > 0.01f) {
            // First apply torque to center the mass
            float strength = 0.05f; // Moderate strength
            
            // Calculate appropriate torques for X and Z axes
            model.angularVelocity.x += strength * toCenter.z;
            model.angularVelocity.z -= strength * toCenter.x;
            
            // IMPORTANT: Also apply torque to make it lie flat (deitado)
            // For a torus, we want to minimize the height variation
            float maxY = -std::numeric_limits<float>::max();
            for (const auto& v : model.vertices) {
                if (v.position.y > maxY) maxY = v.position.y;
            }
            
            // Height variation - higher means less flat
            float heightVariation = maxY - minY;

            // MUCH stronger flattening force when standing up
            if (heightVariation > 0.3f) { // Lower threshold to detect "em pé" state
                // Calculate overall orientation vector to see if it's standing vertically
                glm::vec3 up = glm::normalize(center - lowestPoint);
                float verticalness = glm::abs(glm::dot(up, glm::vec3(0,1,0)));
                
                // If it's very vertical (standing up), apply STRONG corrective force
                if (verticalness > 0.7f) {
                    // Apply MUCH stronger flattening torque in both directions
                    model.angularVelocity.x += 0.2f; // 5-10x stronger!
                    // Randomly choose direction to avoid getting stuck in symmetrical position
                    if (glfwGetTime() - int(glfwGetTime()) > 0.5)
                        model.angularVelocity.z += 0.1f;
                    else
                        model.angularVelocity.z -= 0.1f;
                }
                else {
                    // Normal flattening for non-vertical states
                    model.angularVelocity.x += 0.05f;
                }
            }

            // NEVER lock rotation if the torus is standing up
            if (heightVariation > 0.3f) {
                model.rotationLocked = false;
            }
            
            model.rotationLocked = false;
        } else {
            // Only lock rotation when truly stable
            model.angularVelocity *= 0.5f; // Strong damping
            if (glm::length(model.angularVelocity) < 0.01f) {
                model.angularVelocity = glm::vec3(0.0f);
                model.rotationLocked = true;
            }
        }
    }

    if (!model.rotationLocked && !hasFixedVertices) {
        rotateModelAroundCenter(model);
    }
    minY = std::numeric_limits<float>::max();
    for (const auto& v : model.vertices) {
        if (v.position.y < minY) minY = v.position.y;
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

void handleCollisions(std::vector<ModelPhysics>& physicsModels) {
    for (int i = 1; i < 3; ++i)
        for (int j = i+1; j < 3; ++j)
            if (checkAABBCollision(physicsModels[i], physicsModels[j])) {
                for (auto& v : physicsModels[i].vertices)
                    if (!v.fixed) v.velocity.y *= -0.8f;
                for (auto& v : physicsModels[j].vertices)
                    if (!v.fixed) v.velocity.y *= -0.8f;
                if (i == 1 && j == 2) {
                    glm::vec3 normal(0.0f);
                    for (const auto& v : physicsModels[i].vertices)
                        normal += glm::vec3(0, 1, 0);
                    normal = glm::normalize(normal);

                    // Calcule a média da velocidade do azul
                    glm::vec3 avgVel(0.0f);
                    for (const auto& v : physicsModels[j].vertices)
                        avgVel += v.velocity;
                    avgVel /= (float)physicsModels[j].vertices.size();

                    // Projete a velocidade do azul no plano do chão para obter o impulso lateral
                    glm::vec3 lateral = avgVel - glm::dot(avgVel, normal) * normal;
                    if (glm::length(lateral) < 1e-4) {
                        // Se não houver lateral, gere um impulso artificial baseado na inclinação
                        lateral = glm::vec3(sin(physicsModels[j].rotation.z), 0, sin(physicsModels[j].rotation.x));
                    }
                    lateral = glm::normalize(lateral) * 2.0f; // ajuste o fator para mais/menos impulso

                    // Aplique o impulso lateral ao azul
                    for (auto& v : physicsModels[j].vertices)
                        if (!v.fixed) v.velocity += lateral;

                    // Impulso angular (giro)
                    physicsModels[j].angularVelocity.x += 0.05f * avgVel.y;
                    physicsModels[j].angularVelocity.z += 0.05f * avgVel.x;
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

    float windStrength = 1.0f * sin(glfwGetTime());
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

    // Atualiza a rotação do modelo
    model.rotation += model.angularVelocity * dt;

    // Opcional: aplique damping na rotação
    model.angularVelocity *= 0.98f;

    // If on ground, apply natural settling torque (equilibrium-seeking)
    if (onGround && model.applyEquilibriumRotation) {
        // Calculate center of mass
        glm::vec3 center(0.0f);
        for (const auto& v : model.vertices) {
            center += v.position;
        }
        center /= (float)model.vertices.size();
        
        // Find lowest point
        minY = std::numeric_limits<float>::max();
        glm::vec3 lowestPoint(0.0f);
        for (const auto& v : model.vertices) {
            if (v.position.y < minY) {
                minY = v.position.y;
                lowestPoint = v.position;
            }
        }
        
        // Vector from lowest point to center of mass 
        glm::vec3 toCenter = center - lowestPoint;
        
        // Horizontal distance between center and lowest point (key for stability)
        float horizontalOffset = glm::length(glm::vec2(toCenter.x, toCenter.z));
        
        // Check if we need to rotate to find equilibrium
        if (horizontalOffset > 0.02f) { // Object is unstable
            // Moderate torque
            float strength = 0.03f; 
            
            // Apply impulse to center mass over contact point
            model.angularVelocity.x += strength * toCenter.z;
            model.angularVelocity.z -= strength * toCenter.x;
            
            // Also apply torque to make it lie flat (deitado)
            float maxY = -std::numeric_limits<float>::max();
            for (const auto& v : model.vertices) {
                if (v.position.y > maxY) maxY = v.position.y;
            }
            
            // Height variation - higher means less flat
            float heightVariation = maxY - minY;

            // MUCH stronger flattening force when standing up
            if (heightVariation > 0.3f) { // Lower threshold to detect "em pé" state
                // Calculate overall orientation vector to see if it's standing vertically
                glm::vec3 up = glm::normalize(center - lowestPoint);
                float verticalness = glm::abs(glm::dot(up, glm::vec3(0,1,0)));
                
                // If it's very vertical (standing up), apply STRONG corrective force
                if (verticalness > 0.7f) {
                    // Apply MUCH stronger flattening torque in both directions
                    model.angularVelocity.x += 0.2f; // 5-10x stronger!
                    // Randomly choose direction to avoid getting stuck in symmetrical position
                    if (glfwGetTime() - int(glfwGetTime()) > 0.5)
                        model.angularVelocity.z += 0.1f;
                    else
                        model.angularVelocity.z -= 0.1f;
                }
                else {
                    // Normal flattening for non-vertical states
                    model.angularVelocity.x += 0.05f;
                }
            }

            // NEVER lock rotation if the torus is standing up
            if (heightVariation > 0.3f) {
                model.rotationLocked = false;
            }
            
            model.rotationLocked = false;
        } else {
            // Object is very close to equilibrium, apply strong damping
            model.angularVelocity *= 0.7f;
            
            // When nearly stopped, lock completely
            if (glm::length(model.angularVelocity) < 0.01f) {
                model.angularVelocity = glm::vec3(0.0f);
                model.rotationLocked = true;
            }
        }
    }

    // Make sure rotation is applied only when unlocked
    if (!model.rotationLocked) {
        rotateModelAroundCenter(model);
    }

    // Após rotateModelAroundCenter(model);
    minY = std::numeric_limits<float>::max();
    for (const auto& v : model.vertices) {
        if (v.position.y < minY) minY = v.position.y;
    }
    onGround = (minY <= groundY + 1e-4);

    // Se está no chão, aplique damping extra na rotação
    if (onGround) {
        model.angularVelocity *= 0.90f; // damping mais forte no chão
        // Se a rotação for muito pequena, zere para parar de vez
        if (glm::length(model.angularVelocity) < 0.05f) {
            model.angularVelocity = glm::vec3(0.0f);
        }
    }
}