#include <mb/mesh.h>

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<std::uint32_t> indices,
           Texture_view diffuse_map, Texture_view specular_map)
    : vertices_(std::move(vertices)), indices_(std::move(indices)),
      // textures_{std::move(textures)}
      diffuse_{diffuse_map}, specular_{specular_map}
{
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    glBindVertexArray(vao_);
    // vbo and ebo are all bound to vao
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);

    glVertexAttribPointer(0, sizeof(Vertex::position) / sizeof(float), GL_FLOAT,
                          GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void *>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, sizeof(Vertex::normal) / sizeof(float), GL_FLOAT,
                          GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void *>(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, sizeof(Vertex::texcoord) / sizeof(float), GL_FLOAT,
                          GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void *>(offsetof(Vertex, texcoord)));
    glEnableVertexAttribArray(2);

    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices_.size() * sizeof(Vertex)),
                 vertices_.data(), GL_STATIC_DRAW);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(indices_.size() * sizeof(std::uint32_t)),
        indices_.data(), GL_STATIC_DRAW);

    // Unbind vao
    glBindVertexArray(0);

    spdlog::debug("Mesh initialized: vao={}, vbo={}, ebo={}, indices={}", vao_,
                  vbo_, ebo_, indices_.size());

    assert(vao_ != 0);
    assert(vbo_ != 0);
    assert(ebo_ != 0);

    check_gl_errors();
}

Mesh::~Mesh()
{
    if (vao_ != 0) {
        glDeleteVertexArrays(1, &vao_);
    }
    if (vbo_ != 0) {
        glDeleteBuffers(1, &vbo_);
    }
    if (ebo_ != 0) {
        glDeleteBuffers(1, &ebo_);
    }
}

void Mesh::render(Shader_program const &shader) const
{
    spdlog::trace("Mesh vao={}, vbo={}, ebo={}, size of indices={}", vao_, vbo_,
                  ebo_, indices_.size());

    assert(vao_ != 0);
    assert(vbo_ != 0);
    assert(ebo_ != 0);

    if (indices_.empty()) {
        spdlog::warn("Mesh indices is empty, may cause error");
        return;
    }

    shader.use_program();
    if (!diffuse_.is_null()) {
        diffuse_.bind_to_slot(0);
        shader.uniform_1i("material.diffuse", 0);
        shader.uniform_1i("material.num_diff", 1);
    }
    else {
        shader.uniform_1i("material.num_diff", 0);
    }
    if (!specular_.is_null()) {
        specular_.bind_to_slot(1);
        shader.uniform_1i("material.specular", 1);
        shader.uniform_1i("material.num_spec", 1);
    }
    else {
        shader.uniform_1i("material.num_spec", 0);
    }
    shader.uniform_1f("material.shininess", 64);
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

auto const &Mesh::vertices() const
{
    return vertices_;
}

auto const &Mesh::indices() const
{
    return indices_;
}

void Mesh::bind_diffuse_and_specular(int diff, int spec) const {}
