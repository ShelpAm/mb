#pragma once
#include <mb/check-gl-errors.h>
#include <mb/shader-program.h>
#include <mb/texture.h>

#include <cassert>
#include <cstdint>
#include <glad/gl.h>
#include <vector>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
};

// For rendering, containing vertices of models, vao, vbo, ebo, and textures.
//
// A mesh doesn't own the texture, while a model does.
class Mesh {
  public:
    Mesh(Mesh const &) = delete;
    Mesh(Mesh &&other) noexcept
        : vao_(other.vao_), vbo_(other.vbo_), ebo_(other.ebo_),
          vertices_(std::move(other.vertices_)),
          indices_(std::move(other.indices_)),
          // textures_{std::move(other.textures_)}
          diffuse_{other.diffuse_}, specular_{other.specular_}
    {
        other.vao_ = other.vbo_ = other.ebo_ = 0;
        // other.textures_.clear();
    }
    Mesh &operator=(Mesh const &) = delete;
    Mesh &operator=(Mesh &&other) noexcept
    {
        vao_ = other.vao_;
        vbo_ = other.vbo_;
        ebo_ = other.ebo_;
        other.vao_ = other.vbo_ = other.ebo_ = 0;
        vertices_ = std::move(other.vertices_);
        indices_ = std::move(other.indices_);
        diffuse_ = other.diffuse_;
        specular_ = other.specular_;
        return *this;
    }

    Mesh(std::vector<Vertex> vertices, std::vector<std::uint32_t> indices,
         Texture_view diffuse_map, Texture_view specular_map);

    ~Mesh();

    void render(Shader_program const &shader) const;

    [[deprecated("this->vertices_ will be removed in the future, so this "
                 "function will too be removed.")]] [[nodiscard]]
    auto const &vertices() const;

    [[deprecated("this->indices_ will be removed in the future, so this "
                 "function will too be removed.")]] [[nodiscard]]
    auto const &indices() const;

  private:
    [[deprecated("Don't depend too much on this, as this may be sign of bad "
                 "smell of code..")]]
    void bind_diffuse_and_specular(int diff, int spec) const;

    // Initial invalid value for error checking: if it's 0 , then it indicates
    // an error or the Mesh object doesn't own the model.
    GLuint vao_{};
    GLuint vbo_{};
    GLuint ebo_{};
    std::vector<Vertex> vertices_;
    std::vector<std::uint32_t> indices_;
    // std::vector<Texture> textures_;
    Texture_view diffuse_;
    Texture_view specular_;
};
