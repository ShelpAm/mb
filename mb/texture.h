#pragma once
#include <mb/check-gl-errors.h>

#include <filesystem>
#include <glad/gl.h>
#include <stb_image.h>

/// @brief Texture owns the resource, and has reference to it.
class Texture {
  public:
    Texture(Texture const &) = delete;
    Texture(Texture &&other) noexcept : texture_{other.texture_}
    {
        other.texture_ = 0;
    }
    Texture &operator=(Texture const &) = delete;
    Texture &operator=(Texture &&) = delete;
    Texture(std::filesystem::path const &path) : texture_{gen_texture()}
    {
        int width;
        int height;
        int channels;
        stbi_set_flip_vertically_on_load(1);
        unsigned char *data =
            stbi_load(path.string().c_str(), &width, &height, &channels, 0);
        if (data == nullptr) {
            spdlog::error("Failed to load texture {}", path.string());
            throw std::runtime_error("check last error");
        }
        GLenum format = channels == 3 ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format,
                     GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        check_gl_errors();
        stbi_image_free(data);
    }

    Texture(int width, int height, int format, unsigned char const *data)
        : texture_{gen_texture()}
    {
        if (data == nullptr) {
            throw std::invalid_argument("data is nullptr");
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format,
                     GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        check_gl_errors();
    }

    ~Texture()
    {
        if (texture_ != 0) {
            spdlog::debug("Deleting texture {}", texture_);
            glDeleteTextures(1, &texture_);
        }
    }

    void bind_to_slot(int slot) const
    {
        spdlog::trace("Binding texture {} to slot {}", texture_, slot);
        assert(!is_null() && "Using possibly `std::move`d texture");
        assert(slot >= 0);
        GLint max_units;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_units);
        if (slot >= max_units) {
            spdlog::error("Slot exceeds maximum texture units");
        }
        glActiveTexture(GL_TEXTURE0 + slot);
        check_gl_errors();
        glBindTexture(GL_TEXTURE_2D, texture_);
        check_gl_errors();
    }

    [[nodiscard]] GLuint texture() const
    {
        return texture_;
    }

    [[nodiscard]] bool is_null() const
    {
        return texture_ == 0;
    }

  private:
    static GLuint gen_texture()
    {
        GLuint texture;
        glGenTextures(1, &texture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        check_gl_errors();
        spdlog::debug("Generated texture {}", texture);
        return texture;
    }

    GLuint texture_;
    int slot_;
};

// Be ware of reference dangling.
class Texture_view {
  public:
    explicit Texture_view(Texture const &texture) : texture_{&texture} {}

    void bind_to_slot(int slot) const
    {
        assert(texture_ != nullptr);
        texture_->bind_to_slot(slot);
    }

    [[nodiscard]] GLuint texture() const
    {
        assert(texture_ != nullptr);
        return texture_->texture();
    }

    [[nodiscard]] bool is_null() const
    {
        return texture_->is_null();
    }

  private:
    Texture const *texture_;
};
