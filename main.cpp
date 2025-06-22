#include <iostream>
#include <vector>
#include <limits>
#include <sys/stat.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "window/window.h"
#include "core/model.h"
#include "physics/animation.h"
#include "graphics/shaders.h"
#include "graphics/light.h"
#include "graphics/camera.h"
#include "graphics/mesh.h"

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

    std::vector<std::string> modelFiles = { argv[1], argv[2], argv[3] };
    std::vector<ModelData> models;
    if (!loadAllModels(modelFiles, models)) {
        return -1;
    }
    PhongLight light = { glm::vec3(5.0f, 10.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f) };
    std::vector<PhongMaterial> materials = {
        { glm::vec3(1.0f, 0.0f, 0.0f), 0.1f, 0.5f, 32 },
        { glm::vec3(0.0f, 1.0f, 0.0f), 0.1f, 0.5f, 32 },
        { glm::vec3(0.0f, 0.0f, 1.0f), 0.1f, 0.5f, 32 }
    };
    for (int i = 0; i < 3; ++i) {
        for (auto& v : models[i].vertices) {
            v.x += -2.0f;
        }
    }

    std::vector<ModelPhysics> physicsModels;
    float masses[3] = { 10.0f, 2.0f, 50.0f };
    float initialYPositions[3] = {0.0f, 0.0f, 1.0f};
    float initialXPositions[3] = {-1.0f, 0.0f, 0.0f};
    createPhysicsModels(models, physicsModels, masses, initialYPositions, initialXPositions);

    if (!physicsModels[0].vertices.empty())
        physicsModels[0].vertices[0].fixed = true;

    float gravity = 9.8f;
    float groundY = -2.0f;
    static float lastTime = glfwGetTime();

    int frame = 0;

    float restitution[3] = {0.0f, 0.5f, 0.95f};

    ModelData groundModelData;
    groundModelData.vertices = {
        glm::vec3(-0.5f, 0.0f, -0.5f),
        glm::vec3( 0.5f, 0.0f, -0.5f),
        glm::vec3( 0.5f, 0.0f,  0.5f),
        glm::vec3(-0.5f, 0.0f,  0.5f)
    };
    groundModelData.normals = {
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    };
    groundModelData.faces = {
        0, 1, 2,
        2, 3, 0
    };
    setupBuffers(
        groundModelData.VAO,
        groundModelData.VBO_vertices,
        groundModelData.VBO_normals,
        groundModelData.EBO,
        groundModelData.vertices,
        groundModelData.normals,
        groundModelData.faces
    );

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

    glm::vec3 cameraPos   = glm::vec3(0, 2, 6);
    glm::vec3 cameraFront = glm::normalize(glm::vec3(0, -0.3f, -1));
    glm::vec3 cameraUp    = glm::vec3(0, 1, 0);
    float yaw = -135.0f, pitch = -30.0f;
    float lastX = 320, lastY = 240;
    bool firstMouse = true;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float dt = currentTime - lastTime;
        dt = std::min(dt, 0.02f);
        lastTime = currentTime;

        glClearColor(0.9f, 0.9f, 0.95f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);

        glm::vec3 viewPos = glm::vec3(0, 0, 5);
        processInput(window, dt, cameraPos, cameraFront, cameraUp);

        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        mouse_callback(window, xpos, ypos, yaw, pitch, lastX, lastY, firstMouse, cameraFront);

        int substeps = 5;
        float subdt = dt / substeps;
        for (int s = 0; s < substeps; ++s) {
            updateAllPhysics(physicsModels, subdt, gravity, groundY, restitution, updateRigidBody);
            for (auto& pm : physicsModels)
                updateAABB(pm);
            handleCollisions(physicsModels);
        }

        float minY = physicsModels[0].vertices[0].position.y;
        size_t minIdx = 0;
        for (size_t i = 0; i < physicsModels[0].vertices.size(); ++i) {
            if (physicsModels[0].vertices[i].position.y < minY) {
                minY = physicsModels[0].vertices[i].position.y;
                minIdx = i;
            }
        }
        static bool cordaTravada = false;
        if (!cordaTravada && minY <= 1.0f) {
            physicsModels[0].vertices[minIdx].fixed = true;
            cordaTravada = true;
        }

        updateModelsFromPhysics(models, physicsModels);

        drawGround(groundModelData, shaderProgram, light, viewPos, groundY);

        drawAllModels(models, shaderProgram, materials, light, viewPos);

        exportAllModels(models, frame);
        frame++;

        glm::mat4 view, projection;
        setViewProjection(shaderProgram, width, height, cameraPos, cameraFront, cameraUp);

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