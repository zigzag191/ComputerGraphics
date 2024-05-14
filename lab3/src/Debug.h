#pragma once

#include <string_view>

#include "GL/glew.h"
#include "spdlog/spdlog.h"

inline std::string_view GetGlErrorSource(GLenum code)
{
    switch (code)
    {
    case GL_DEBUG_SOURCE_API: return "Source:API";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "Source:Window System";
    case GL_DEBUG_SOURCE_SHADER_COMPILER: return "Source:Shader Compiler";
    case GL_DEBUG_SOURCE_THIRD_PARTY: return "Source:Third Party";
    case GL_DEBUG_SOURCE_APPLICATION: return "Source:Application";
    case GL_DEBUG_SOURCE_OTHER: return "Source:Other";
    };
    return "Source:Unknown";
}

inline std::string_view GetGlErrorType(GLenum code)
{
    switch (code)
    {
    case GL_DEBUG_TYPE_ERROR: return "Type:Error";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Type:Deprecated Behaviour";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "Type:Undefined Behaviour";
    case GL_DEBUG_TYPE_PORTABILITY: return "Type:Portability";
    case GL_DEBUG_TYPE_PERFORMANCE: return "Type:Performance";
    case GL_DEBUG_TYPE_MARKER: return "Type:Marker";
    case GL_DEBUG_TYPE_PUSH_GROUP: return "Type:Push Group";
    case GL_DEBUG_TYPE_POP_GROUP: return "Type:Pop Group";
    case GL_DEBUG_TYPE_OTHER: return "Type:Other";
    }
    return "Type:Unknown";
}

inline void GLDebugMessageCallback(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
    {
        spdlog::critical("GL ERROR [{}] {} {}: {}", id, GetGlErrorSource(source), GetGlErrorType(type), message);
        __debugbreak();
        break;
    }
    case GL_DEBUG_SEVERITY_MEDIUM:
    {
        spdlog::error("GL ERROR [{}] {} {}: {}", id, GetGlErrorSource(source), GetGlErrorType(type), message);
        __debugbreak();
        break;
    }
    case GL_DEBUG_SEVERITY_LOW:
    {
        spdlog::warn("GL WARN [{}] {} {}: {}", id, GetGlErrorSource(source), GetGlErrorType(type), message);
        __debugbreak();
        break;
    }
    case GL_DEBUG_SEVERITY_NOTIFICATION:
    {
        //spdlog::info("GL INFO [{}] {} {}: {}", id, GetGlErrorSource(source), GetGlErrorType(type), message);
        break;
    }
    }
}
