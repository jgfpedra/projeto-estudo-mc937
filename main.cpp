#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <functional>
#include <sys/stat.h>
#include <cstdlib> 
#include "window/window.h"
#include "core/model.h"
#include "physics/animation.h"
#include "core/objexporter.h"
#include "graphics/shaders.h"
#include "graphics/light.h"
#include "graphics/camera.h"
#include "graphics/mesh.h"

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
        std::string filename = "model_animations/anim_model" + std::to_string(i+1) + "_frame_" + std::to_string(frame) + ".obj";
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
        float z = 0.0f;
        // Experimente z positivo para o verde:
        if (i == 1) z = 0.5f;
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3((i-1)*0.68f, 0.0f, z));
        model = glm::scale(model, glm::vec3(1.0f));
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

void drawAABB(const ModelPhysics& model, GLuint shaderProgram, const glm::mat4& view, const glm::mat4& projection) {
    // Define os 8 vértices da caixa
    glm::vec3 min = model.aabbMin;
    glm::vec3 max = model.aabbMax;
    glm::vec3 verts[8] = {
        {min.x, min.y, min.z},
        {max.x, min.y, min.z},
        {max.x, max.y, min.z},
        {min.x, max.y, min.z},
        {min.x, min.y, max.z},
        {max.x, min.y, max.z},
        {max.x, max.y, max.z},
        {min.x, max.y, max.z}
    };
    // Linhas da caixa (12 arestas)
    GLuint indices[24] = {
        0,1, 1,2, 2,3, 3,0, // base
        4,5, 5,6, 6,7, 7,4, // topo
        0,4, 1,5, 2,6, 3,7  // laterais
    };

    GLuint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    // Use um shader simples só com cor (ou seu shader atual, mas setando cor fixa)
    glUseProgram(shaderProgram);
    // Sete uniforms de view/projection se necessário

    // Desenhe em modo wireframe
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
}

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
    glm::vec3 proposedPos = avgPos + avgVel * dt;

    // Calcula o menor Y dos vértices se mover para proposedPos
    float minY = std::numeric_limits<float>::max();
    for (const auto& v : model.vertices) {
        float y = proposedPos.y + (v.position.y - avgPos.y);
        if (y < minY) minY = y;
    }

    // Se algum vértice ficaria abaixo do chão, ajusta o centro para que o menor Y fique em groundY
    if (minY < groundY) {
        float delta = groundY - minY;
        proposedPos.y += delta;
        avgVel.y *= -restitution;
    }

    // Aplica o movimento corrigido
    for (auto& v : model.vertices) {
        v.position += (proposedPos - avgPos);
        v.velocity = avgVel;
    }
};

int main(int argc, char* argv[]) {
    struct stat st = {0};
    if (stat("model_animations", &st) == -1) {
        mkdir("model_animations", 0755);
    }
    std::system("rm -f model_animations/anim_model*_frame_*.obj");
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " model1.obj model2.obj model3.obj\n";
        return -1;
    }
    float width = 640.0f, height = 480.0f;
    if (!glfwInit()) return -1;
    GLFWwindow* window = createWindow(width, height);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    glEnable(GL_DEPTH_TEST);
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
        // Adicione este bloco para debug:
        std::cout << "Modelo " << i << " (" << argv[i+1] << "): "
                << models[i].vertices.size() << " vértices, "
                << models[i].normals.size() << " normais, "
                << models[i].faces.size() / 3 << " faces" << std::endl;
    }
    PhongLight light = { glm::vec3(5.0f, 10.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f) };
    std::vector<PhongMaterial> materials = {
        { glm::vec3(1.0f, 0.0f, 0.0f), 0.1f, 0.5f, 32 }, // vermelho
        { glm::vec3(0.0f, 1.0f, 0.0f), 0.1f, 0.5f, 32 }, // verde
        { glm::vec3(0.0f, 0.0f, 1.0f), 0.1f, 0.5f, 32 }  // azul
    };
    // Eleva todos os modelos para começarem acima do chão
    for (int i = 0; i < 3; ++i) {
        for (auto& v : models[i].vertices) {
            v.x += -2.0f;
        }
    }

    // Agora inicialize a física
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

    int frame = 0;

    float restitution[3] = {0.0f, 0.5f, 0.95f}; // tecido, rígido, borracha

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

    // Defina a AABB do chão (como se fosse um ModelPhysics)
    glm::mat4 groundModel = glm::scale(
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, groundY, 0.0f)),
        glm::vec3(8.0f, 1.0f, 8.0f)
    );
    glm::vec3 min = glm::vec3(groundModel * glm::vec4(groundModelData.vertices[0], 1.0f));
    glm::vec3 max = min;
    for (const auto& v : groundModelData.vertices) {
        glm::vec3 vt = glm::vec3(groundModel * glm::vec4(v, 1.0f));
        min = glm::min(min, vt);
        max = glm::max(max, vt);
    }
    ModelPhysics groundPhysics;
    groundPhysics.aabbMin = min;
    groundPhysics.aabbMax = max;

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float dt = currentTime - lastTime;
        dt = std::min(dt, 0.02f);
        lastTime = currentTime;

        glClearColor(0.9f, 0.9f, 0.95f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);
        setViewProjection(shaderProgram, width, height);
        glm::vec3 viewPos = glm::vec3(0, 0, 5);

        // Atualiza física
        updateAllPhysics(physicsModels, dt, gravity, groundY, restitution, updateRigidBody);

        // Atualiza AABBs após a física
        for (auto& pm : physicsModels)
            updateAABB(pm);

        // Colisões
        handleCollisions(physicsModels);

        // Atualiza vértices dos modelos
        updateModelsFromPhysics(models, physicsModels);


        // Desenha chão
        drawGround(groundModelData, shaderProgram, light, viewPos, groundY);

        // Desenha modelos
        drawAllModels(models, shaderProgram, materials, light, viewPos);

        // Exporta animação
        exportAllModels(models, frame);
        frame++;

        // Supondo que você já tem view e projection (ou pode calcular de novo)
        glm::mat4 view, projection;
        setViewProjection(shaderProgram, width, height); // ou recalcule aqui
        // Para cada modelo:
        drawAABB(groundPhysics, shaderProgram, view, projection);
        for (int i = 0; i < 3; ++i) {
            drawAABB(physicsModels[i], shaderProgram, view, projection);
        }

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
    for (int i = 0; i < 3; ++i) {
        if (!models[i].vertices.empty()) {
            glm::vec3 avg(0.0f);
            for (const auto& v : models[i].vertices) avg += v;
            avg /= (float)models[i].vertices.size();
            std::cout << "Centro geométrico do modelo " << i << ": ("
                      << avg.x << ", " << avg.y << ", " << avg.z << ")\n";
        }
    }
}