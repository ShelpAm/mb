#pragma once
#include <mb/mesh.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <unordered_map>
#include <utility>

class Model {
  public:
    Model(std::filesystem::path const &path)
    {
        spdlog::info("Loading model {}", path.string());

        std::array<unsigned char, 4> default_grey{100, 100, 100, 255};
        textures_.insert(
            {"path:default", Texture(1, 1, GL_RGBA, default_grey.data())});

        Assimp::Importer importer;
        // A few other useful options are:

        // aiProcess_GenNormals:
        //   creates normal vectors for each vertex if the model doesn't contain
        //   normal vectors.
        // aiProcess_SplitLargeMeshes:
        //   splits large meshes into smaller sub-meshes which is useful if your
        //   rendering has a maximum number of vertices allowed and can only
        //   process smaller meshes.
        // aiProcess_OptimizeMeshes: does the reverse by trying to join several
        //   meshes into one larger mesh, reducing drawing calls for
        //   optimization.
        aiScene const *scene{importer.ReadFile(
            path.string(), aiProcess_Triangulate | aiProcess_FlipUVs)};
        if (scene == nullptr ||
            static_cast<bool>(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) ||
            scene->mRootNode == nullptr) {
            spdlog::error("Failed to load model {}: {}", path.string(),
                          importer.GetErrorString());
            throw std::runtime_error("check last error");
        }

        auto model_parent = path.parent_path();
        process_assimp_node(scene->mRootNode, scene, model_parent);
        spdlog::info("Loaded model {}", path.string());
    }

    Model(std::vector<Vertex> vertices, std::vector<std::uint32_t> indices,
          Texture diffuse_map, Texture specular_map)
    {
        textures_.insert({"path:diffuse", std::move(diffuse_map)});
        textures_.insert({"path:specular", std::move(specular_map)});
        meshes_.emplace_back(std::move(vertices), std::move(indices),
                             Texture_view(textures_.at("path:diffuse")),
                             Texture_view(textures_.at("path:specular")));
    }

    void render(Shader_program const &shader) const
    {
        for (auto const &mesh : meshes_) {
            mesh.render(shader);
        }
    }

  private:
    void process_assimp_node(aiNode const *node, aiScene const *scene,
                             std::filesystem::path const &model_parent)
    {
        for (unsigned int i{}; i != node->mNumMeshes; ++i) {
            spdlog::info("Loading mesh {} of node {}", i,
                         static_cast<void const *>(node));
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            auto mesh = extract_mesh(scene->mMeshes[node->mMeshes[i]], scene,
                                     model_parent);
            meshes_.push_back(std::move(mesh));
        }
        for (unsigned int i{}; i != node->mNumChildren; ++i) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            process_assimp_node(node->mChildren[i], scene, model_parent);
        }
    }

    Mesh extract_mesh(aiMesh const *mesh, aiScene const *scene,
                      std::filesystem::path const &model_parent)
    {
        std::vector<Vertex> vertices;
        std::vector<std::uint32_t> indices;

        for (unsigned int i{}; i != mesh->mNumVertices; i++) {
            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            glm::vec3 pos{mesh->mVertices[i].x, mesh->mVertices[i].y,
                          mesh->mVertices[i].z};
            glm::vec3 normal{mesh->mNormals[i].x, mesh->mNormals[i].y,
                             mesh->mNormals[i].z};
            // A aiMesh can have at most 8 texcoord, but we only care the first
            // one.
            glm::vec2 texcoord{mesh->mTextureCoords[0] != nullptr
                                   ? glm::vec2{mesh->mTextureCoords[0][i].x,
                                               mesh->mTextureCoords[0][i].y}
                                   : glm::vec2{}};
            // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

            vertices.push_back(
                {.position = pos, .normal = normal, .texcoord = texcoord});
        }

        for (unsigned int i{}; i != mesh->mNumFaces; i++) {
            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
            // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }

        if (mesh->mMaterialIndex >= scene->mNumMaterials) {
            spdlog::warn("Model doesn't have any material");
            throw std::runtime_error("check last error");
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        aiMaterial *material{scene->mMaterials[mesh->mMaterialIndex]};
        Texture_view diffuse{load_material_texture(
            scene, material, aiTextureType_DIFFUSE, model_parent)};
        Texture_view specular{load_material_texture(
            scene, material, aiTextureType_SPECULAR, model_parent)};

        return Mesh{std::move(vertices), std::move(indices), diffuse, specular};
    }

    Texture_view
    load_material_texture(aiScene const *scene, aiMaterial const *mat,
                          aiTextureType type,
                          std::filesystem::path const &model_parent) const
    {
        unsigned int num_textures{mat->GetTextureCount(type)};
        for (unsigned int i{}; i != num_textures; i++) {
            aiString rel_path; // ?? Why retrieving path here? Where should the
                               // path be for the mesh??
            mat->GetTexture(type, i, &rel_path);
            aiTexture const *texture{
                scene->GetEmbeddedTexture(rel_path.C_Str())};
            if (texture != nullptr) { // Embedded texture
                auto const *data =
                    reinterpret_cast<unsigned char const *>(texture->pcData);
                int width = texture->mWidth;
                int height = texture->mHeight;

                // BGRA format
                if (texture->achFormatHint[0] == '\0') {
                    textures_.insert({rel_path.C_Str(),
                                      Texture(width, height, GL_BGRA, data)});
                    return Texture_view(textures_.at(rel_path.C_Str()));
                }

                // Compressed image format
                assert(height == 0 && "height should be zero because now ought "
                                      "to be in compressed image mode");
                int channels;
                unsigned char *rgba_data = stbi_load_from_memory(
                    data, width, &width, &height, &channels, 4);
                if (rgba_data == nullptr) {
                    spdlog::error(
                        "Failed to read image from memory: MEMORY LAYOUT:");
                    spdlog::error("{}", reinterpret_cast<char const *>(data));
                    throw std::runtime_error("check last error");
                }
                textures_.insert({rel_path.C_Str(),
                                  Texture(width, height, GL_RGBA, rgba_data)});
                stbi_image_free(rgba_data);
                return Texture_view(textures_.at(rel_path.C_Str()));
            }

            auto path = model_parent / rel_path.C_Str();
            spdlog::info("path={}", path.string());
            if (!textures_.contains(path)) {
                textures_.insert({path, Texture(path)});
            }
            return Texture_view{textures_.at(path)};
        }
        spdlog::warn(
            "can't find desired texture type {}, using default grey texture",
            static_cast<int>(type));
        return Texture_view(textures_.at("path:default"));
    }

    std::vector<Mesh> meshes_;
    // Cached
    mutable std::unordered_map<std::filesystem::path, Texture> textures_;
    float scale_{1};
};
