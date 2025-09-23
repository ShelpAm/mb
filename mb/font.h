#pragma once
#include <mb/shader-program.h>
#include <mb/texture.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <filesystem>
#include <glm/glm.hpp>
#include <source_location>
#include <spdlog/spdlog.h>
#include <stdexcept>

struct Character {
    unsigned int texture_id;
    glm::ivec2 size;    // Size of glyph
    glm::ivec2 bearing; // Offset from baseline to left/top of glyph
    long advance;       // Offset to advance to next glyph
};

class Font {
    friend class Ui;

  public:
    Font(std::filesystem::path const &path);

  private:
    std::unordered_map<char, Character> characters_;
    int maxbearingy_{};
};

struct Font_vertex {
    glm::vec2 position;
    glm::vec2 texcoord;
};

class Game;
class Ui {
  public:
    Ui(float width, float height, Font const *font,
       Shader_program const *shader);
    ~Ui();

    void render_text(std::string_view s, glm::vec2 pos, float scale,
                     glm::vec3 color = {0, 0, 0}) const;

  private:
    Font const *font_;
    Shader_program const *shader_;
    glm::mat4 projection_;

    GLuint vao_, vbo_;
    float width_, height_;
};
