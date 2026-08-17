#include "axpch.hpp"

#include "Model.hpp"
#include "Renderer/Meshes/Mesh.hpp"
#include "Renderer/Textures/Texture.hpp"
#include "Core/Error/Panic.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Resource/ResourceManager.hpp"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/mesh.h"
#include "assimp/material.h"
#include "assimp/postprocess.h"
#include "assimp/types.h"

#include <tracy/Tracy.hpp>

namespace Axle {
    struct Model::InternalMethods {
        static void ProcessNode(aiNode* node, const aiScene* scene, Model* model);
        static Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene, Model* model);
        static Ref<Texture2D>
        LoadMaterialTexture(aiMaterial* mat, aiTextureType aiType, TextureType type, const std::string& directory);
    };

    Model::Model(const std::string& path) {
        ZoneScopedN("Create model");

        Assimp::Importer import;
        const aiScene* scene =
            import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

        if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr) {
            AX_CORE_ERROR(LogChannel::Renderer,
                          "Couldn't import model from file: {0}. Error: {1}",
                          path,
                          import.GetErrorString());
            return;
        }
        m_Directory = path.substr(0, path.find_last_of('/'));

        AX_CORE_INFO(LogChannel::Renderer,
                     "Loading model with: meshes={0} materials={1} nodes={2}",
                     scene->mNumMeshes,
                     scene->mNumMaterials,
                     scene->mRootNode->mNumChildren);

        InternalMethods::ProcessNode(scene->mRootNode, scene, this);
    }

    void Model::Draw(const glm::mat4& transform) {
        ZoneScopedN("Draw model");

        for (u32 i = 0; i < m_Meshes.size(); ++i) {
            m_Meshes[i].Draw(transform);
        }
    }

    void Model::InternalMethods::ProcessNode(aiNode* node, const aiScene* scene, Model* model) {
        ZoneScopedN("Process model node");

        // Process node's meshes
        for (u32 i = 0; i < node->mNumMeshes; ++i) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            model->m_Meshes.push_back(ProcessMesh(mesh, scene, model));
        }

        // Recursion
        for (u32 i = 0; i < node->mNumChildren; ++i) {
            ProcessNode(node->mChildren[i], scene, model);
        }
    }

    Mesh Model::InternalMethods::ProcessMesh(aiMesh* mesh, const aiScene* scene, Model* model) {
        ZoneScopedN("Process mesh model");

        std::vector<Vertex> vertices;
        std::vector<u32> indices;
        std::array<Ref<Texture2D>, static_cast<u32>(TextureType::Unknown)> textures;

        // Vertex data
        for (u32 i = 0; i < mesh->mNumVertices; ++i) {
            Vertex vertex;
            vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

            // tangents
            if (mesh->mTangents != nullptr) {
                glm::vec3 tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
                glm::vec3 bitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);

                // Handedness: does Assimp's bitangent agree with cross(N, T)?
                float handedness = (glm::dot(glm::cross(vertex.normal, tangent), bitangent) < 0.0f) ? -1.0f : 1.0f;

                vertex.tangent = glm::vec4(tangent, handedness);
            } else {
                // No UVs / tangent generation failed for this mesh — flag it so normal mapping
                // is skipped or falls back to a per-triangle tangent computed post-load.
                vertex.tangent = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
                AX_ASSERT(false, LogChannel::Renderer, "No tangent found in import");
            }

            // uv's
            if (mesh->mTextureCoords[0] != nullptr)
                vertex.textureCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            else
                vertex.textureCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }

        // Indices
        for (u32 i = 0; i < mesh->mNumFaces; ++i) {
            aiFace face = mesh->mFaces[i];
            for (u32 j = 0; j < face.mNumIndices; ++j)
                indices.push_back(face.mIndices[j]);
        }

        // Material
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

#define LOAD_MATERIAL_TEXTURE(aiType, aiType2, type)                        \
    temp = LoadMaterialTexture(material, aiType, type, model->m_Directory); \
    if (temp)                                                               \
        textures[static_cast<u32>(type)] = std::move(temp);                 \
    else                                                                    \
        textures[static_cast<u32>(type)] = LoadMaterialTexture(material, aiType2, type, model->m_Directory);

        // Textures
        Ref<Texture2D> temp;
        LOAD_MATERIAL_TEXTURE(aiTextureType_DIFFUSE, aiTextureType_BASE_COLOR, TextureType::BaseColor);
        LOAD_MATERIAL_TEXTURE(aiTextureType_NORMALS, aiTextureType_NORMAL_CAMERA, TextureType::Normal);
        LOAD_MATERIAL_TEXTURE(
            aiTextureType_GLTF_METALLIC_ROUGHNESS, aiTextureType_METALNESS, TextureType::MetallicRoughness);
        LOAD_MATERIAL_TEXTURE(aiTextureType_AMBIENT_OCCLUSION, aiTextureType_LIGHTMAP, TextureType::AO);
        LOAD_MATERIAL_TEXTURE(aiTextureType_EMISSIVE, aiTextureType_EMISSION_COLOR, TextureType::Emissive);
        LOAD_MATERIAL_TEXTURE(aiTextureType_HEIGHT, aiTextureType_DISPLACEMENT, TextureType::Height);
        LOAD_MATERIAL_TEXTURE(aiTextureType_OPACITY, aiTextureType_OPACITY, TextureType::Opacity);

        // POD
        MaterialPOD pod; // struct defaults as a last-resort fallback

        aiColor4D baseColor;
        if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_BASE_COLOR, &baseColor))
            pod.BaseColorFactor = glm::vec3(baseColor.r, baseColor.g, baseColor.b);

        f32 metallic;
        if (AI_SUCCESS == aiGetMaterialFloat(material, AI_MATKEY_METALLIC_FACTOR, &metallic))
            pod.MetallicFactor = metallic;

        f32 roughness;
        if (AI_SUCCESS == aiGetMaterialFloat(material, AI_MATKEY_ROUGHNESS_FACTOR, &roughness))
            pod.RoughnessFactor = roughness;

        return Mesh(vertices, indices, std::move(textures), pod);
    }

    Ref<Texture2D> Model::InternalMethods::LoadMaterialTexture(aiMaterial* mat,
                                                               aiTextureType aiType,
                                                               TextureType type,
                                                               const std::string& directory) {
        ZoneScopedN("Load material textures");
        Ref<Texture2D> texture;

        AX_ASSERT(mat->GetTextureCount(aiType) <= 1,
                  LogChannel::Renderer,
                  "Can't support multiple textures of the same type");
        if (mat->GetTextureCount(aiType) > 0) {
            aiString str;
            mat->GetTexture(aiType, 0, &str);

            std::string filename = directory + "/" + std::string(str.C_Str());

            texture = Texture2D::Create(filename, -1, type == TextureType::BaseColor, type);
        }

        return texture;
    }
} // namespace Axle
