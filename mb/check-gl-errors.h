#pragma once
#include <glad/gl.h>
#include <source_location>
#include <spdlog/spdlog.h>
#include <stdexcept>

#ifdef NDEBUG // Release mode
#define check_gl_errors()
#else // Debug mode
#define check_gl_errors()                                                      \
    {                                                                          \
        if (GLenum err = glGetError(); err != GL_NO_ERROR) {                   \
            spdlog::error("OpenGL error in line {}, function {}: {}",          \
                          std::source_location::current().line(),              \
                          std::source_location::current().function_name(),     \
                          err);                                                \
            throw std::runtime_error("opengl error, check in log");            \
        }                                                                      \
    }
#endif
