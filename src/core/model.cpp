#include "core/model.h"
#include "physics/animation.h"
#include "graphics/mesh.h"
#include "core/objexporter.h"
#include "core/objloader.h"
#include <iostream>

bool loadModel(const char* filename, ModelData& model) {
    if (!loadObj(filename, model.vertices, model.normals, model.faces))
        return false;
    setupBuffers(model.VAO, model.VBO_vertices, model.VBO_normals, model.EBO,
                 model.vertices, model.normals, model.faces);
    return true;
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

bool loadAllModels(const std::vector<std::string>& filenames, std::vector<ModelData>& models) {
    models.resize(filenames.size());
    for (size_t i = 0; i < filenames.size(); ++i) {
        if (!loadModel(filenames[i].c_str(), models[i])) {
            std::cerr << "Failed to load OBJ file: " << filenames[i] << std::endl;
            return false;
        }
    }
    return true;
}
