#pragma once
#include <mb/check-gl-errors.h>
#include <mb/components.h>

#include <filesystem>
#include <fstream>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ranges>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string_view>
#include <unordered_map>

class Shader_program {
  public:
    Shader_program(Shader_program const &) = delete;
    Shader_program(Shader_program &&) = delete;
    Shader_program &operator=(Shader_program const &) = delete;
    Shader_program &operator=(Shader_program &&) = delete;

    Shader_program(std::filesystem::path const &vert,
                   std::filesystem::path const &frag)
        : program_(glCreateProgram())
    {
        spdlog::info("Initializing shader {} from {} and {}...", program_,
                     vert.string(), frag.string());
        check_gl_errors();
        auto vertex_shader = compile(read_file(vert), GL_VERTEX_SHADER);
        auto fragment_shader = compile(read_file(frag), GL_FRAGMENT_SHADER);
        attach_and_link(vertex_shader, fragment_shader);
        spdlog::info("Initialized shader {}", program_);
    }

    ~Shader_program()
    {
        glDeleteProgram(program_);
    }

    void use_program() const
    {
        spdlog::trace("use_program program={}", program_);
        glUseProgram(program_);
        check_gl_errors();
    }

    [[nodiscard]] [[deprecated("should be removed in the future")]] GLuint
    program() const
    {
        return program_;
    }

    void uniform_mat3(std::string const &name, glm::mat3 const &mat) const
    {
        use_program();
        GLint location = fetch_location(name);
        glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(mat));
    }

    void uniform_mat4(std::string const &name, glm::mat4 const &mat) const
    {
        use_program();
        GLint location = fetch_location(name);
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
    }

    void uniform_vec3(std::string const &name, glm::vec3 v) const
    {
        use_program();
        GLint location = fetch_location(name);
        glUniform3f(location, v.x, v.y, v.z);
    }

    void uniform_1i(std::string const &name, int value) const
    {
        use_program();
        GLint location = fetch_location(name);
        glUniform1i(location, value);
    }

    void uniform_1f(std::string const &name, float value) const
    {
        use_program();
        GLint location = fetch_location(name);
        glUniform1f(location, value);
    }

  private:
    static constexpr auto log_buf_size{512UZ};
    std::array<char, log_buf_size> info_log{};
    GLuint program_;
    mutable std::unordered_map<std::string, GLint> location_;

    static std::string read_file(std::filesystem::path const &path)
    {
        if (!std::filesystem::exists(path)) {
            throw std::runtime_error(
                std::format("{} doesn't exist", path.string()));
        }
        std::ifstream ifs(path);
        if (!ifs.is_open()) {
            throw std::runtime_error(
                std::format("can't read {}", path.string()));
        }
        std::ostringstream oss;
        oss << ifs.rdbuf();
        return oss.str();
    }
    static std::string_view shader_type2str(int type)
    {
        using namespace std::string_view_literals;
        switch (type) {
        case GL_VERTEX_SHADER:
            return "vertex"sv;
        case GL_FRAGMENT_SHADER:
            return "fragment"sv;
        default:
            return "unknown"sv;
        }
    }
    GLuint compile(std::string const &src, int type)
    {
        GLuint shader = glCreateShader(type);
        char const *src_ptr = src.c_str();
        glShaderSource(shader, 1, &src_ptr, nullptr);
        glCompileShader(shader);

        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (success == 0) {
            glGetShaderInfoLog(shader, log_buf_size, nullptr, info_log.data());
            throw std::runtime_error(
                std::format("shader::{}::compilation_failed: {}",
                            shader_type2str(type), std::string_view(info_log)));
        }
        return shader;
    }
    void attach_and_link(GLuint vertex_shader, GLuint fragment_shader)
    {
        glAttachShader(program_, vertex_shader);
        glAttachShader(program_, fragment_shader);
        glLinkProgram(program_);

        int success;
        glGetProgramiv(program_, GL_LINK_STATUS, &success);
        if (success == 0) {
            glGetProgramInfoLog(program_, log_buf_size, nullptr,
                                info_log.data());
            throw std::runtime_error(std::format("program::link_failed: {}",
                                                 std::string_view(info_log)));
        }
    }
    GLint fetch_location(std::string const &name) const
    {
        if (!location_.contains(name)) {
            location_.insert(
                {name, glGetUniformLocation(program_, name.c_str())});
        }

        return location_.at(name);
    }
};
