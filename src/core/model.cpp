#include "core/model.h"
#include "core/objloader.h"
#include "graphics/mesh.h"

bool loadModel(const char* filename, ModelData& model) {
    if (!carregarObj(filename, model.vertices, model.normals, model.faces))
        return false;
    setupBuffers(model.VAO, model.VBO_vertices, model.VBO_normals, model.EBO,
                 model.vertices, model.normals, model.faces);
    return true;
}