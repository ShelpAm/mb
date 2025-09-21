#pragma once
#include <glad/gl.h>
#include <source_location>
#include <spdlog/spdlog.h>
#include <stdexcept>

#ifdef NDEBUG // Release mode
#define check_gl_errors()
#else // Debug mode
inline char const *glErrorToString(GLenum errorCode)
{
    switch (errorCode) {
    case GL_NO_ERROR:
        return "GL_NO_ERROR";
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_STACK_OVERFLOW:
        return "GL_STACK_OVERFLOW";
    case GL_STACK_UNDERFLOW:
        return "GL_STACK_UNDERFLOW";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";
    case GL_CONTEXT_LOST:
        return "GL_CONTEXT_LOST";
    default:
        return "Unknown OpenGL Error";
    }
}
#define check_gl_errors()                                                      \
    {                                                                          \
        if (GLenum err = glGetError(); err != GL_NO_ERROR) {                   \
            spdlog::error("OpenGL error in line {}, function `{}`: {} {}",     \
                          std::source_location::current().line(),              \
                          std::source_location::current().function_name(),     \
                          err, glErrorToString(err));                          \
            throw std::runtime_error("opengl error, check in log");            \
        }                                                                      \
    }
#endif
