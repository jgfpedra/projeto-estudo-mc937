#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "window/window.h"
#include "core/model.h"
#include "physics/animation.h"
#include "core/objexporter.h"
#include "graphics/shaders.h"
#include "graphics/light.h"
#include "graphics/camera.h"
#include "graphics/mesh.h"
#include <functional>

void updatePhysicsAll(std::vector<ModelPhysics>& physicsModels, float dt, float gravity, float groundY) {
    for (auto& pm : physicsModels)
        updatePhysics(pm, dt, gravity, groundY);
}

void updateModelsFromPhysics(std::vector<ModelData>& models, const std::vector<ModelPhysics>& physicsModels) {
    for (size_t i = 0; i < models.size(); ++i) {
        for (size_t j = 0; j < models[i].vertices.size(); ++j)
            models[i].vertices[j] = physicsModels[i].vertices[j].position;
        glBindBuffer(GL_ARRAY_BUFFER, models[i].VBO_vertices);
        glBufferSubData(GL_ARRAY_BUFFER, 0, models[i].vertices.size() * sizeof(glm::vec3), models[i].vertices.data());
    }
}

void exportAllModels(const std::vector<ModelData>& models, int frame) {
    for (size_t i = 0; i < models.size(); ++i) {
        std::string filename = "anim_model" + std::to_string(i+1) + "_frame_" + std::to_string(frame) + ".obj";
        exportObjFrame(filename, models[i].vertices, models[i].normals, models[i].faces);
    }
}

void handleCollisions(std::vector<ModelPhysics>& physicsModels) {
    for (int i = 1; i < 3; ++i)
        for (int j = i+1; j < 3; ++j)
            if (checkAABBCollision(physicsModels[i], physicsModels[j])) {
                for (auto& v : physicsModels[i].vertices)
                    if (!v.fixed) v.velocity.y *= -0.8f;
                for (auto& v : physicsModels[j].vertices)
                    if (!v.fixed) v.velocity.y *= -0.8f;
            }
}

void drawAllModels(
    const std::vector<ModelData>& models,
    GLuint shaderProgram,
    const std::vector<PhongMaterial>& materials,
    const PhongLight& light,
    const glm::vec3& viewPos)
{
    for (int i = 0; i < 3; ++i) {
        setPhongUniforms(shaderProgram, light, materials[i], viewPos);
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3((i-1)*2.0f, 0.0f, 0.0f));
        drawModel(models[i], shaderProgram, model);
    }
}

void drawGround(
    const ModelData& groundModelData,
    GLuint shaderProgram,
    const PhongLight& light,
    const glm::vec3& viewPos,
    float groundY)
{
    setPhongUniforms(shaderProgram, light, {glm::vec3(0.8f, 0.8f, 0.7f), 0.1f, 0.0f, 1}, viewPos);
    glm::mat4 groundModel = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, groundY, 0.0f)), glm::vec3(8.0f, 1.0f, 8.0f));
    drawModel(groundModelData, shaderProgram, groundModel);
}


void updateAllPhysics(
    std::vector<ModelPhysics>& physicsModels,
    float dt, float gravity, float groundY,
    const float restitution[3],
    std::function<void(ModelPhysics&, float, float, float, float)> updateRigidBody)
{
    updatePhysics(physicsModels[0], dt, gravity, groundY, restitution[0]); // tecido
    updateRigidBody(physicsModels[1], dt, gravity, groundY, restitution[1]); // rígido
    updatePhysics(physicsModels[2], dt, gravity, groundY, restitution[2]); // borracha
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " model1.obj model2.obj model3.obj\n";
        return -1;
    }
    float width = 640.0f, height = 480.0f;
    if (!glfwInit()) return -1;
    GLFWwindow* window = createWindow(width, height);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) {
        std::cerr << "ERROR: GLEW Initialization Failed\n";
        return -1;
    }
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint shaderProgram = glCreateProgram();
    configureShaders(vs, fs, shaderProgram);
    std::vector<ModelData> models(3);
    for (int i = 0; i < 3; ++i) {
        if (!loadModel(argv[i+1], models[i])) {
            std::cerr << "Failed to load OBJ file: " << argv[i+1] << std::endl;
            return -1;
        }
    }
    PhongLight light = { glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f) };
    std::vector<PhongMaterial> materials = {
        { glm::vec3(1.0f, 0.5f, 0.5f), 0.1f, 0.5f, 32 }, // vermelho claro
        { glm::vec3(0.5f, 1.0f, 0.5f), 0.2f, 0.7f, 16 }, // verde claro
        { glm::vec3(0.5f, 0.5f, 1.0f), 0.3f, 1.0f, 64 }  // azul claro
    };
    // Inicialização da física para cada modelo
    std::vector<ModelPhysics> physicsModels(3);
    for (int i = 0; i < 3; ++i) {
        for (const auto& v : models[i].vertices) {
            VertexPhysics vp;
            vp.position = v;
            vp.velocity = glm::vec3(0.0f);
            vp.fixed = false;
            physicsModels[i].vertices.push_back(vp);
        }
    }
    // Exemplo: pendure o primeiro vértice do primeiro modelo
    if (!physicsModels[0].vertices.empty())
        physicsModels[0].vertices[0].fixed = true;

    float gravity = 9.8f;
    float groundY = -2.0f;
    static float lastTime = glfwGetTime();
    float currentTime = glfwGetTime();
    float dt = currentTime - lastTime;
    lastTime = currentTime;

    int frame = 0;

    float restitution[3] = {0.0f, 0.5f, 0.95f}; // tecido, rígido, borracha

    // Função lambda para updateRigidBody (pode ir fora do main também)
    auto updateRigidBody = [](ModelPhysics& model, float dt, float gravity, float groundY, float restitution) {
        if (model.vertices.empty()) return;
        glm::vec3 avgVel(0.0f), avgPos(0.0f);
        for (auto& v : model.vertices) {
            avgVel += v.velocity;
            avgPos += v.position;
        }
        avgVel /= (float)model.vertices.size();
        avgPos /= (float)model.vertices.size();

        avgVel += glm::vec3(0.0f, -gravity * dt, 0.0f);
        avgPos += avgVel * dt;

        if (avgPos.y < groundY) {
            avgPos.y = groundY;
            avgVel.y *= -restitution;
        }
        for (auto& v : model.vertices) {
            v.position = avgPos;
            v.velocity = avgVel;
        }
    };

    // Criação do chão (groundModelData)
    ModelData groundModelData;

    // Vértices do quad no plano XZ, centrado na origem, y = 0
    groundModelData.vertices = {
        glm::vec3(-0.5f, 0.0f, -0.5f),
        glm::vec3( 0.5f, 0.0f, -0.5f),
        glm::vec3( 0.5f, 0.0f,  0.5f),
        glm::vec3(-0.5f, 0.0f,  0.5f)
    };

    // Normais para cima
    groundModelData.normals = {
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    };

    // Índices para dois triângulos
    groundModelData.faces = {
        0, 1, 2,
        2, 3, 0
    };

    // Crie os buffers OpenGL para o chão
    setupBuffers(
        groundModelData.VAO,
        groundModelData.VBO_vertices,
        groundModelData.VBO_normals,
        groundModelData.EBO,
        groundModelData.vertices,
        groundModelData.normals,
        groundModelData.faces
    );

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.9f, 0.9f, 0.95f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);
        setViewProjection(shaderProgram, width, height);
        glm::vec3 viewPos = glm::vec3(0, 0, 5);

        // Atualiza física
        updateAllPhysics(physicsModels, dt, gravity, groundY, restitution, updateRigidBody);

        // Colisões
        handleCollisions(physicsModels);

        // Atualiza vértices dos modelos
        updateModelsFromPhysics(models, physicsModels);

        // Desenha modelos
        drawAllModels(models, shaderProgram, materials, light, viewPos);

        // Desenha chão
        drawGround(groundModelData, shaderProgram, light, viewPos, groundY);

        // Exporta animação
        exportAllModels(models, frame);
        frame++;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    for (auto& m : models) {
        glDeleteVertexArrays(1, &m.VAO);
        glDeleteBuffers(1, &m.VBO_vertices);
        glDeleteBuffers(1, &m.VBO_normals);
        glDeleteBuffers(1, &m.EBO);
    }
    glDeleteProgram(shaderProgram);
    glDeleteShader(vs);
    glDeleteShader(fs);
    glfwTerminate();
    return 0;
}