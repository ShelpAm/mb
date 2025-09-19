#pragma once
#include <mb/check-gl-errors.h>
#include <mb/shader-program.h>

#include <cassert>
#include <cstdint>
#include <glad/gl.h>
#include <source_location>
#include <vector>

// For rendering, containing vertices of models, vao, vbo and ebo.
//
// Now only support position of model
class Mesh {
  public:
    Mesh(Mesh const &) = delete;
    Mesh(Mesh &&other) noexcept
        : vao_(other.vao_), vbo_(other.vbo_), ebo_(other.ebo_),
          vertices_(std::move(other.vertices_)),
          indices_(std::move(other.indices_))

    {
        other.vao_ = other.vbo_ = other.ebo_ = -1U;
    }
    Mesh &operator=(Mesh const &) = delete;
    Mesh &operator=(Mesh &&) = delete;
    Mesh(std::vector<float> vertices, std::vector<std::uint32_t> indices)
        : vertices_(std::move(vertices)), indices_(std::move(indices))
    {
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
        glGenBuffers(1, &ebo_);

        glBindVertexArray(vao_);
        // vbo and ebo are all bound to vao
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                              nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                              reinterpret_cast<void *>(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                              (void *)(6 * sizeof(float))); // TexCoord
        glEnableVertexAttribArray(2);

        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(vertices_.size() * sizeof(float)),
                     vertices_.data(), GL_STATIC_DRAW);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(indices_.size() * sizeof(std::uint32_t)),
            indices_.data(), GL_STATIC_DRAW);

        // Unbind vao
        glBindVertexArray(0);

        spdlog::debug("Mesh initialized: vao={}, vbo={}, ebo={}, indices={}",
                      vao_, vbo_, ebo_, indices_.size());

        assert(vao_ != 0);
        assert(vbo_ != 0);
        assert(ebo_ != 0);

        check_gl_errors();
    }
    [[deprecated("vertices is no longer 3 per group, but 8. If you don't know "
                 "what this means, please don't use this function.")]]
    Mesh(std::vector<glm::vec3> vertices, std::vector<std::uint32_t> indices)
        : Mesh(
              // Converts to std::vector<float>
              [&vertices]() {
                  std::vector<float> verts(vertices.size() * 3);
                  for (int i{}; i != vertices.size(); ++i) {
                      verts[(3 * i) + 0] = vertices[i].x;
                      verts[(3 * i) + 1] = vertices[i].y;
                      verts[(3 * i) + 2] = vertices[i].z;
                  }
                  return verts;
              }(),
              std::move(indices))
    {
    }

    ~Mesh()
    {
        if (vao_ != -1U) {
            glDeleteVertexArrays(1, &vao_);
        }
        if (vbo_ != -1U) {
            glDeleteBuffers(1, &vbo_);
        }
        if (ebo_ != -1U) {
            glDeleteBuffers(1, &ebo_);
        }
    }

    void render(Shader_program const &shader) const
    {
        spdlog::trace("Mesh vao={}, vbo={}, ebo={}, size of indices={}", vao_,
                      vbo_, ebo_, indices_.size());

        assert(vao_ != 0);
        assert(vbo_ != 0);
        assert(ebo_ != 0);

        if (indices_.empty()) {
            spdlog::warn("Mesh indices is empty, may cause error");
            return;
        }

        shader.use_program();
        check_gl_errors();
        glBindVertexArray(vao_);
        assert(indices_.size() <= std::numeric_limits<GLsizei>::max());
        check_gl_errors();
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices_.size()),
                       GL_UNSIGNED_INT, nullptr);
        check_gl_errors();
        glBindVertexArray(0);
        check_gl_errors();
    }

    [[deprecated("this->vertices_ will be removed in the future, so this "
                 "function will too be removed.")]] [[nodiscard]]
    auto const &vertices() const
    {
        return vertices_;
    }

    [[deprecated("this->indices_ will be removed in the future, so this "
                 "function will too be removed.")]] [[nodiscard]]
    auto const &indices() const
    {
        return indices_;
    }

  private:
    // Initial invalid value for error checking: if it's -1U (very big signed),
    // then it indicates an error or the Mesh object doesn't own the model.
    GLuint vao_{};
    GLuint vbo_{};
    GLuint ebo_{};
    std::vector<float> vertices_;
    std::vector<std::uint32_t> indices_;
};
