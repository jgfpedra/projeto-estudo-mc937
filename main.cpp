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

void drawAllModels(
    const std::vector<ModelData>& models,
    GLuint shaderProgram,
    const std::vector<PhongMaterial>& materials,
    const PhongLight& light,
    const glm::vec3& viewPos)
{
    for (int i = 0; i < 3; ++i) {
        setPhongUniforms(shaderProgram, light, materials[i], viewPos);
        glm::mat4 model = glm::mat4(1.0f);
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
    updatePhysics(physicsModels[0], dt, gravity, groundY, restitution[0]);
    updateRigidBody(physicsModels[1], dt, gravity, groundY, restitution[1]);
    updatePhysics(physicsModels[2], dt, gravity, groundY, restitution[2]);
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
};

void processInput(GLFWwindow* window, float deltaTime,
                 glm::vec3& cameraPos, glm::vec3& cameraFront, glm::vec3& cameraUp) {
    float cameraSpeed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos,
                    float& yaw, float& pitch, float& lastX, float& lastY, bool& firstMouse,
                    glm::vec3& cameraFront) {
    static float sensitivity = 0.1f;
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;
    if(pitch > 89.0f) pitch = 89.0f;
    if(pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

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
        std::cout << "Modelo " << i << " (" << argv[i+1] << "): "
                << models[i].vertices.size() << " vértices, "
                << models[i].normals.size() << " normais, "
                << models[i].faces.size() / 3 << " faces" << std::endl;
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

    std::vector<ModelPhysics> physicsModels(3);
    float masses[3] = { 10.0f, 2.0f, 50.0f };
    float initialYPositions[3] = {0.0f, 0.0f, 1.0f};
    float initialXPositions[3] = {-1.0f, 0.0f, 0.0f};

    for (int i = 0; i < 3; ++i) {
        for (const auto& v : models[i].vertices) {
            VertexPhysics vp;
            vp.position = v + glm::vec3(initialXPositions[i], initialYPositions[i], 0.0f);
            vp.velocity = glm::vec3(0.0f);
            vp.fixed = false;
            vp.mass = masses[i];
            physicsModels[i].vertices.push_back(vp);
        }
    }

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
            handleCollisions(physicsModels, restitution);
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