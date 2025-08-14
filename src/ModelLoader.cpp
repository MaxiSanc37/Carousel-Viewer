#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "ModelLoader.h"
#include <iostream>
#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

ModelLoader::ModelLoader(const std::string& path) {
    loadModel(path);
}

void ModelLoader::loadModel(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace);

    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    directory = std::filesystem::path(path).parent_path().string();

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[i];
        meshes.push_back(processMesh(mesh, scene));

        std::string meshName = mesh->mName.C_Str();
        std::transform(meshName.begin(), meshName.end(), meshName.begin(), ::tolower);

        meshNames.push_back(meshName);

        std::cout << "Mesh " << i << ": " << meshName << std::endl;

        if (meshName.find("bulb") != std::string::npos || meshName.find("light") != std::string::npos || meshName.find("lit") != std::string::npos) {
            std::vector<glm::vec3> clusterCenters;
            float clusterThreshold = 0.15f; // tweak if needed

            for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
                glm::vec3 pos(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z);
                bool foundCluster = false;

                for (auto& center : clusterCenters) {
                    if (glm::distance(center, pos) < clusterThreshold) {
                        // Already covered by this cluster
                        foundCluster = true;
                        break;
                    }
                }

                if (!foundCluster) {
                    clusterCenters.push_back(pos);
                    if (clusterCenters.size() >= 64) break;
                }
            }

            std::cout << "Extracted " << clusterCenters.size() << " light bulbs from mesh '" << meshName << "'." << std::endl;
            bulbPositions.insert(bulbPositions.end(), clusterCenters.begin(), clusterCenters.end());
        }
    }


    //find the total meshes of the model
    //std::cout << "Total meshes: " << meshes.size() << std::endl;
}

Mesh ModelLoader::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        vertex.position = glm::vec3(
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        );
        vertex.normal = glm::vec3(
            mesh->mNormals[i].x,
            mesh->mNormals[i].y,
            mesh->mNormals[i].z
        );
        vertex.texCoords = mesh->mTextureCoords[0] ?
            glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) :
            glm::vec2(0.0f);

        if (mesh->HasTangentsAndBitangents()) {
            vertex.tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
            vertex.bitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
        }
        else {
            vertex.tangent = glm::vec3(0.0f);
            vertex.bitangent = glm::vec3(0.0f);
        }

        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    unsigned int textureID = 0;
    unsigned int normalMapID = 0;

    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        textureID = loadMaterialTexture(material, aiTextureType_DIFFUSE);
        normalMapID = loadMaterialTexture(material, aiTextureType_NORMALS);

        if (!normalMapID) {
            std::cout << "[Fallback] Trying HEIGHT map instead of NORMAL map..." << std::endl;
            normalMapID = loadMaterialTexture(material, aiTextureType_HEIGHT);
        }

        std::cout << "Texture ID: " << textureID << ", NormalMap ID: " << normalMapID << std::endl;
    }

    return Mesh(vertices, indices, textureID, normalMapID);
}

unsigned int ModelLoader::loadMaterialTexture(aiMaterial* mat, aiTextureType type) {
    aiString str;
    if (mat->GetTexture(type, 0, &str) != AI_SUCCESS) {
        return 0;
    }

    std::filesystem::path texturePath = std::filesystem::path(directory).parent_path() / "textures" / std::filesystem::path(str.C_Str()).filename();
    std::string fullPath = texturePath.string();
    int width, height, nrComponents;
    std::cout << "Trying to load texture at path: " << fullPath << std::endl;
    std::cout << "Assimp texture name: " << str.C_Str() << std::endl;
    unsigned char* data = stbi_load(fullPath.c_str(), &width, &height, &nrComponents, 0);

    if (!data) {
        std::cerr << "Failed to load texture at path: " << fullPath << std::endl;
        return 0;
    }

    GLenum format = nrComponents == 3 ? GL_RGB : GL_RGBA;
    unsigned int textureID;
    glGenTextures(1, &textureID);

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return textureID;
}

// Draw method with vertical horse animation
void ModelLoader::Draw(float horseTime, unsigned int shaderProgram, const glm::mat4& baseModel) const {
    for (size_t i = 0; i < meshes.size(); ++i) {
        // Base transform matrix for each mesh
        glm::mat4 transform = baseModel;

        // Apply bouncing only to selected horse meshes by index
        if (i == 0) {
            float verticalOffset = sin(horseTime) * 2.8f;
            transform = glm::translate(transform, glm::vec3(0.0f, 0.0f, verticalOffset));//Z acts as Y, because the model
        //had to be rotated by -90 degrees in order to look good in the view.
        }
        else if (i == 1) {
            //pi<float> does the movement intercalation effect, the multiplier allows for further movement up and down
            float verticalOffset = sin(horseTime + glm::pi<float>()) * 2.8f;
            transform = glm::translate(transform, glm::vec3(0.0f, 0.0f, verticalOffset));
        }

        // Upload model matrix to shader
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(transform));

        const std::string& meshName = meshNames[i];
        if (meshName.find("lit") != std::string::npos ||
            meshName.find("bulb") != std::string::npos ||
            meshName.find("light") != std::string::npos) {
            glUniform1i(glGetUniformLocation(shaderProgram, "forceBulbColor"), 1);
        }
        else {
            glUniform1i(glGetUniformLocation(shaderProgram, "forceBulbColor"), 0);
        }

        meshes[i].Draw();

        //to see which meshes are the horses (the ones that are moving)
        //std::cout << "Drawing mesh " << i << std::endl;
    }
}
