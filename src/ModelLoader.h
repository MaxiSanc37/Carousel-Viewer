
#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <string>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "Mesh.h"

class ModelLoader {
public:
    ModelLoader(const std::string& path);
    void Draw(float horseTime, unsigned int shaderProgram, const glm::mat4& baseModel) const;
    const std::vector<glm::vec3>& GetBulbPositions() const { return bulbPositions; }

private:
    std::vector<Mesh> meshes;
    std::string directory;
    std::vector<glm::vec3> bulbPositions;
    std::vector<std::string> meshNames;
    void loadModel(const std::string& path);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    unsigned int loadMaterialTexture(aiMaterial* mat, aiTextureType type);
};

#endif
