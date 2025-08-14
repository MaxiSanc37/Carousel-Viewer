
#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    unsigned int textureID; // Diffuse
    unsigned int normalMapID = 0; // Normal Map
    unsigned int VAO;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, unsigned int textureID, unsigned int normalMapID = 0);
    void Draw() const;

private:
    unsigned int VBO, EBO;
    void setupMesh();
};

#endif
