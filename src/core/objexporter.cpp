#include "core/objexporter.h"
#include <fstream>

void exportObjFrame(const std::string& filename,
                    const std::vector<glm::vec3>& vertices,
                    const std::vector<glm::vec3>& normals,
                    const std::vector<unsigned int>& faces) {
    std::ofstream out(filename);
    if (!out.is_open()) return;

    for (const auto& v : vertices)
        out << "v " << v.x << " " << v.y << " " << v.z << "\n";
    for (const auto& n : normals)
        out << "vn " << n.x << " " << n.y << " " << n.z << "\n";
    for (size_t i = 0; i < faces.size(); i += 3)
        out << "f "
            << faces[i] + 1 << "//" << faces[i] + 1 << " "
            << faces[i+1] + 1 << "//" << faces[i+1] + 1 << " "
            << faces[i+2] + 1 << "//" << faces[i+2] + 1 << "\n";
    out.close();
}