#pragma once
#include <mb/check-gl-errors.h>

#include <filesystem>
#include <glad/gl.h>
#include <stb_image.h>

/// @brief Texture doesn't own the resource, but has reference to it.
class Texture {
  public:
    Texture(std::filesystem::path const &path) : texture_{gen_texture()}
    {
        int width;
        int height;
        int channels;
        stbi_set_flip_vertically_on_load(1);
        unsigned char *data =
            stbi_load(path.c_str(), &width, &height, &channels, 0);
        if (data == nullptr) {
            spdlog::error("Failed to find {}", path.string());
            throw std::runtime_error(
                std::format("failed to find {}", path.string()));
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format,
                     GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        check_gl_errors();
    }

    void bind_to_slot(int slot) const
    {
        assert(slot >= 0);
        glActiveTexture(GL_TEXTURE0 + slot);
        check_gl_errors();
        glBindTexture(GL_TEXTURE_2D, texture_);
        check_gl_errors();
    }

    [[nodiscard]] GLuint texture() const
    {
        return texture_;
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
        return texture;
    }

    GLuint texture_;
};
