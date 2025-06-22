#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

bool carregarObj(const char* filename, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<unsigned int>& faces);

bool carregarObj(const char* filename, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<unsigned int>& faces) {
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec3> temp_normals;
    std::vector<unsigned int> vertex_indices, normal_indices;

    FILE* file = fopen(filename, "r");
    if (!file) {
        std::cerr << "Cannot open file " << filename << std::endl;
        return false;
    }

    char line[128];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v' && line[1] == ' ') {
            glm::vec3 vertex;
            sscanf(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
        }
        else if (line[0] == 'v' && line[1] == 'n') {
            glm::vec3 normal;
            sscanf(line, "vn %f %f %f", &normal.x, &normal.y, &normal.z);
            temp_normals.push_back(normal);
        }
        else if (line[0] == 'f') {
            unsigned int vertexIndex[3], normalIndex[3];
            int matches = sscanf(line, "f %u//%u %u//%u %u//%u",
                                 &vertexIndex[0], &normalIndex[0],
                                 &vertexIndex[1], &normalIndex[1],
                                 &vertexIndex[2], &normalIndex[2]);

            if (matches != 6) {
                // Try format without normals if the first attempt fails
                matches = sscanf(line, "f %u %u %u",
                                 &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
                if (matches != 3) {
                    std::cerr << "Face format not recognized" << std::endl;
                    continue;
                }
                // If no normals, use zero normals
                normalIndex[0] = normalIndex[1] = normalIndex[2] = 0;
            }

            for (int i = 0; i < 3; i++) {
                vertex_indices.push_back(vertexIndex[i]);
                normal_indices.push_back(normalIndex[i]);
            }
        }
    }
    fclose(file);
    for (unsigned int i = 0; i < vertex_indices.size(); i++) {
        unsigned int vertexIndex = vertex_indices[i];
        unsigned int normalIndex = normal_indices[i];
        glm::vec3 vertex = temp_vertices[vertexIndex - 1];
        vertices.push_back(vertex);
        if (normalIndex > 0 && normalIndex <= temp_normals.size()) {
            normals.push_back(temp_normals[normalIndex - 1]);
        } else {
            normals.push_back(glm::vec3(0.0f));
        }

        faces.push_back(i);
    }

    return true;
}
