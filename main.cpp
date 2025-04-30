#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

GLFWwindow* createWindow(float width, float height) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, "Bunny", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        return nullptr;
    }
    return window;
}

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

void configureShaders(GLuint& vs, GLuint& fs, GLuint& shaderProgram) {
    const char* vertex_shader =
        "#version 330 core\n"
        "layout(location = 0) in vec3 position;\n"
        "layout(location = 1) in vec3 normal;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "out vec3 Normal;\n"
        "void main() {\n"
        "   gl_Position = projection * view * model * vec4(position, 1.0);\n"
        "   Normal = normal;\n"
        "}";
    const char* fragment_shader =
        "#version 330 core\n"
        "out vec4 frag_color;"
        "void main() {"
        "   frag_color = vec4(0.5, 0.6, 0.0, 1.0);"
        "}";
    glShaderSource(vs, 1, &vertex_shader, nullptr);
    glCompileShader(vs);
    glShaderSource(fs, 1, &fragment_shader, nullptr);
    glCompileShader(fs);
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);
}

void setupBuffers(GLuint& VAO, GLuint& VBO_vertices, GLuint& VBO_normals, GLuint& EBO, const std::vector<glm::vec3>& vertices, const std::vector<glm::vec3>& normals, const std::vector<unsigned int>& indices) {
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO_vertices);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_vertices);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &VBO_normals);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_normals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
}

void renderScene(GLuint& shaderProgram, GLuint VAO, GLuint EBO, GLuint facesCount, GLFWwindow* window) {
    glClearColor(0.2f, 0.2f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // Get matrix uniform locations
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");

    // Set up matrices
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(2.0f)); // Scale 2x bigger

    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 3.0f), // Camera position
        glm::vec3(0.0f, 0.0f, 0.0f),  // Look at center
        glm::vec3(0.0f, 1.0f, 0.0f)); // Up vector

    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),          // FOV
        640.0f/480.0f,                // Aspect ratio
        0.1f, 100.0f);                // Near/far planes

    // Send matrices to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Draw
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glDrawElements(GL_TRIANGLES, facesCount, GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
}

int main(int argc, char* argv[]) {
    GLFWwindow* window;
    GLuint VAO, VBO_vertices, VBO_normals, EBO, vs, fs, shaderProgram;
    std::vector<glm::vec3> vertices, normals;
    std::vector<unsigned int> faces;
    float width = 640.0f, height = 480.0f;
    if (!glfwInit()) {
        return -1;
    }
    window = createWindow(width, height);
    if (!window) {
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) {
        std::cerr << "ERROR: GLEW Initialization Failed\n";
        return -1;
    }

    if (!carregarObj(argv[1], vertices, normals, faces)) {
        std::cerr << "Failed to load OBJ file: " << argv[1] << std::endl;
        return -1;
    }
    setupBuffers(VAO, VBO_vertices, VBO_normals, EBO, vertices, normals, faces);
    vs = glCreateShader(GL_VERTEX_SHADER);
    fs = glCreateShader(GL_FRAGMENT_SHADER);
    shaderProgram = glCreateProgram();
    configureShaders(vs, fs, shaderProgram);
    while (!glfwWindowShouldClose(window)) {
        renderScene(shaderProgram, VAO, EBO, static_cast<GLuint>(faces.size()), window);
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO_vertices);
    glDeleteBuffers(1, &VBO_normals);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glDeleteShader(vs);
    glDeleteShader(fs);
    glfwTerminate();
    return 0;
}
