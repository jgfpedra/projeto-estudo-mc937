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
    // Aplica forças e integra movimento para cada vértice
    float windStrength = 1.0f * sin(glfwGetTime());
    glm::vec3 wind = glm::vec3(0.0f, 0.0f, windStrength);
    float damping = 0.98f;
    float dragCoef = 0.2f;
    for (auto& v : model.vertices) {
        if (v.fixed) continue;
        if (v.notFalling) {
            // Vértice travado: só vento e arrasto
            glm::vec3 force = 5.0f * wind * v.mass;
            glm::vec3 drag = -dragCoef * v.velocity;
            force += drag;

            glm::vec3 acceleration = force / v.mass;
            acceleration[0] = 0;
            acceleration[1] = 0;
            v.velocity += acceleration * dt;
            v.position += v.velocity * dt;  
            continue;
        }
        // Vértice livre: gravidade, vento e arrasto
        glm::vec3 force = glm::vec3(0.0f, -gravity * v.mass, 0.0f);
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

    // Corrige se algum vértice atravessou o chão
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

    // Aplica damping na velocidade dos vértices
    for (auto& v : model.vertices) {
        if (!v.fixed) {
            v.velocity *= damping;
        }
    }

    // Atualiza rotação do modelo
    model.rotation += model.angularVelocity * dt;
    model.angularVelocity *= 0.98f;
    bool onGround = (minY <= groundY + 1e-4);
    
    bool hasFixedVertices = false;
    for (const auto& v : model.vertices) {
        if (v.fixed) {
            hasFixedVertices = true;
            break;
        }
    }
    
    if (onGround && !hasFixedVertices) {
        glm::vec3 center(0.0f);
        for (const auto& v : model.vertices) {
            center += v.position;
        }
        center /= (float)model.vertices.size();
        minY = std::numeric_limits<float>::max();
        glm::vec3 lowestPoint(0.0f);
        for (const auto& v : model.vertices) {
            if (v.position.y < minY) {
                minY = v.position.y;
                lowestPoint = v.position;
            }
        }
        glm::vec3 toCenter = center - lowestPoint;
        float horizontalOffset = glm::length(glm::vec2(toCenter.x, toCenter.z));
        if (horizontalOffset > 0.01f) {
            // Aplica torque para deitar o modelo
            float strength = 0.05f;
            model.angularVelocity.x += strength * toCenter.z;
            model.angularVelocity.z -= strength * toCenter.x;
            float maxY = -std::numeric_limits<float>::max();
            for (const auto& v : model.vertices) {
                if (v.position.y > maxY) maxY = v.position.y;
            }
            float heightVariation = maxY - minY;
            if (heightVariation > 0.3f) {
                glm::vec3 up = glm::normalize(center - lowestPoint);
                float verticalness = glm::abs(glm::dot(up, glm::vec3(0,1,0)));
                if (verticalness > 0.7f) {
                    model.angularVelocity.x += 0.4f; // torque extra se estiver muito em pé
                    if (glfwGetTime() - int(glfwGetTime()) > 0.5)
                        model.angularVelocity.z += 0.1f;
                    else
                        model.angularVelocity.z -= 0.1f;
                }
                else {
                    model.angularVelocity.x += 0.05f;
                }
            }
            if (heightVariation > 0.4f) {
                model.rotationLocked = false;
            }
            model.rotationLocked = false;
        } else {
            // Se estiver quase parado, trava rotação
            model.angularVelocity *= 0.5f;
            if (glm::length(model.angularVelocity) < 0.01f) {
                model.angularVelocity = glm::vec3(0.0f);
                model.rotationLocked = true;
            }
        }
    }

    // Aplica rotação se não estiver travada e não tiver vértices fixos
    if (!model.rotationLocked && !hasFixedVertices) {
        rotateModelAroundCenter(model);
    }

    // Atualiza minY após rotação (para uso externo)
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

    // Calcula médias de velocidade e posição
    glm::vec3 avgVel(0.0f), avgPos(0.0f);
    for (auto& v : model.vertices) {
        avgVel += v.velocity;
        avgPos += v.position;
    }
    avgVel /= (float)model.vertices.size();
    avgPos /= (float)model.vertices.size();

    float windStrength = 1.0f * sin(glfwGetTime());
    glm::vec3 wind = glm::vec3(0.0f, 0.0f, windStrength);

    // Encontra o menor y dos vértices
    float minY = std::numeric_limits<float>::max();
    for (const auto& v : model.vertices) {
        if (v.position.y < minY) minY = v.position.y;
    }

    // Verifica se está no chão
    bool onGround = (minY <= groundY + 1e-4 && avgVel.y <= 0.0f);
    if (!onGround) {
        avgVel += wind * dt;
    }
    avgVel += glm::vec3(0.0f, -gravity, 0.0f) * dt;
    glm::vec3 proposedPos = avgPos + avgVel * dt;

    // Calcula o menor y da posição proposta
    minY = std::numeric_limits<float>::max();
    for (const auto& v : model.vertices) {
        float y = proposedPos.y + (v.position.y - avgPos.y);
        if (y < minY) minY = y;
    }

    // Corrige se atravessar o chão
    if (minY < groundY) {
        float delta = groundY - minY;
        proposedPos.y += delta;
        avgVel.y *= -restitution;
    }

    // Atualiza posições e velocidades dos vértices
    for (auto& v : model.vertices) {
        v.position += (proposedPos - avgPos);
        v.velocity = avgVel;
    }
    
    // Damping na velocidade
    float damping = 0.98f;
    for (auto& v : model.vertices) {
        if (!v.fixed) v.velocity *= damping;
    }

    // Atualiza rotação e aplica damping na rotação
    model.rotation += model.angularVelocity * dt;
    model.angularVelocity *= 0.98f;

    // Aplica torque para buscar equilíbrio se estiver no chão
    if (onGround && model.applyEquilibriumRotation) {
        glm::vec3 center(0.0f);
        for (const auto& v : model.vertices) center += v.position;
        center /= (float)model.vertices.size();

        // Encontra o ponto mais baixo
        minY = std::numeric_limits<float>::max();
        glm::vec3 lowestPoint(0.0f);
        for (const auto& v : model.vertices) {
            if (v.position.y < minY) {
                minY = v.position.y;
                lowestPoint = v.position;
            }
        }

        // Calcula deslocamento horizontal do centro até o ponto mais baixo
        glm::vec3 toCenter = center - lowestPoint;
        float horizontalOffset = glm::length(glm::vec2(toCenter.x, toCenter.z));

        if (horizontalOffset > 0.02f) {
            // Aplica torque para deitar
            float strength = 0.03f; 
            model.angularVelocity.x += strength * toCenter.z;
            model.angularVelocity.z -= strength * toCenter.x;

            // Se estiver muito em pé, aplica torque mais forte
            float maxY = -std::numeric_limits<float>::max();
            for (const auto& v : model.vertices) {
                if (v.position.y > maxY) maxY = v.position.y;
            }
            float heightVariation = maxY - minY;
            if (heightVariation > 0.3f) {
                glm::vec3 up = glm::normalize(center - lowestPoint);
                float verticalness = glm::abs(glm::dot(up, glm::vec3(0,1,0)));
                if (verticalness > 0.7f) {
                    model.angularVelocity.x += 0.2f;
                    if (glfwGetTime() - int(glfwGetTime()) > 0.5)
                        model.angularVelocity.z += 0.1f;
                    else
                        model.angularVelocity.z -= 0.1f;
                }
                else {
                    model.angularVelocity.x += 0.05f;
                }
            }
            model.rotationLocked = false;
        } else {
            // Se estiver quase parado, trava rotação
            model.angularVelocity *= 0.7f;
            if (glm::length(model.angularVelocity) < 0.01f) {
                model.angularVelocity = glm::vec3(0.0f);
                model.rotationLocked = true;
            }
        }
    }

    // Aplica rotação se não estiver travada
    if (!model.rotationLocked) {
        rotateModelAroundCenter(model);
    }

    // Atualiza minY após rotação
    minY = std::numeric_limits<float>::max();
    for (const auto& v : model.vertices) {
        if (v.position.y < minY) minY = v.position.y;
    }
    onGround = (minY <= groundY + 1e-4);

    // Damping extra na rotação se estiver no chão
    if (onGround) {
        model.angularVelocity *= 0.90f;
        if (glm::length(model.angularVelocity) < 0.05f) {
            model.angularVelocity = glm::vec3(0.0f);
        }
    }
}