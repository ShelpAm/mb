#include <mb/font.h>

#include <mb/game.h>

Font::Font(std::filesystem::path const &path)
{
    FT_Library library;
    auto error = FT_Init_FreeType(&library);
    if (error != 0) {
        spdlog::error("Failed to init libfreetype");
        throw std::runtime_error(
            std::source_location::current().function_name());
    }

    FT_Face face;
    error = FT_New_Face(library, path.string().c_str(), 0, &face);
    if (error == FT_Err_Unknown_File_Format) {
        spdlog::error("Unsupported file format");
    }
    else if (error != 0) {
        spdlog::error("Unknown error");
    }

    FT_Set_Pixel_Sizes(face, 0, 24);

    glPixelStorei(GL_UNPACK_ALIGNMENT,
                  1); // disable byte-alignment restriction

    for (unsigned char c = 0; c < 128; c++) {
        // load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER) != 0) {
            spdlog::error("Failed to load glyph '{}'", c);
            continue;
        }
        // Texture texture(face->glyph->bitmap.width, face->glyph->bitmap.rows,
        //                 GL_RED, face->glyph->bitmap.buffer);
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width,
                     face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
                     face->glyph->bitmap.buffer);
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        Character character{.texture_id = texture,
                            .size = glm::ivec2(face->glyph->bitmap.width,
                                               face->glyph->bitmap.rows),
                            .bearing = glm::ivec2(face->glyph->bitmap_left,
                                                  face->glyph->bitmap_top),
                            .advance = face->glyph->advance.x};
        maxbearingy_ = std::max(maxbearingy_, character.bearing.y);
        characters_.insert({c, character});
    }
    FT_Done_Face(face);
    FT_Done_FreeType(library);
}

Ui::Ui(float width, float height, Font const *font,
       Shader_program const *shader)
    : font_{font}, shader_{shader},
      projection_{glm::ortho(0.0F, width, 0.0F, height)}, width_{width},
      height_{height}
{
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(Font_vertex), nullptr,
                 GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Font_vertex),
                          (void *)offsetof(Font_vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Font_vertex),
                          (void *)offsetof(Font_vertex, texcoord));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    check_gl_errors();
}

Ui::~Ui()
{
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
}

void Ui::render_text(std::string_view s, glm::vec2 pos, float scale,
                     glm::vec3 color)
{
    // 启用混合以支持透明
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader_->use_program();
    shader_->uniform_mat4("projection", projection_);
    shader_->uniform_vec3("textColor", color);
    shader_->uniform_1i("text", 0);

    glBindVertexArray(vao_);
    float x = pos.x;

    // 逐字符渲染
    for (char c : s) {
        auto it = font_->characters_.find(c);
        if (it == font_->characters_.end()) {
            spdlog::error("Can't render {}: it doesn't exist in characters");
            continue;
        }

        Character const &ch = it->second;
        float xpos = x + ch.bearing.x * scale;
        float ypos =
            height_ - pos.y - (font_->maxbearingy_ - ch.bearing.y) * scale;
        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        // 顶点数据：位置和纹理坐标
        std::array<Font_vertex, 6> vertices{
            Font_vertex{.position = {xpos, ypos - h}, .texcoord = {0.0f, 1.0f}},
            Font_vertex{.position = {xpos, ypos}, .texcoord = {0.0f, 0.0f}},
            Font_vertex{.position = {xpos + w, ypos}, .texcoord = {1.0f, 0.0f}},

            Font_vertex{.position = {xpos, ypos - h}, .texcoord = {0.0f, 1.0f}},
            Font_vertex{.position = {xpos + w, ypos}, .texcoord = {1.0f, 0.0f}},
            Font_vertex{.position = {xpos + w, ypos - h},
                        .texcoord = {1.0f, 1.0f}},
        };

        spdlog::trace("Rendering char '{}' at {},{}", c, xpos, ypos);

        // 绑定纹理，更新VBO，绘制
        glActiveTexture(GL_TEXTURE0); // bind to slot0
        glBindTexture(GL_TEXTURE_2D, ch.texture_id);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices.data());
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        check_gl_errors();

        // 移动到下一个字符（advance单位为1/64像素）
        x += (ch.advance >> 6) * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
}
