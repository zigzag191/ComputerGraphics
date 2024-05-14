#pragma once

#include <string_view>
#include <fstream>

#include "GL/glew.h"
#include "spdlog/spdlog.h"
#include "glm/glm.hpp"

struct Shader
{
    static constexpr GLuint InvalidProgram = 0;

    static GLuint LoadProgram(std::string_view vertexShaderPath, std::string_view fragmentShaderPath)
    {
        GLuint vertexShader = LoadShader(vertexShaderPath, GL_VERTEX_SHADER);
        GLuint fragmentShader = LoadShader(fragmentShaderPath, GL_FRAGMENT_SHADER);

        if (vertexShader == 0 || fragmentShader == 0)
        {
            return 0;
        }

        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);

        glLinkProgram(program);

        GLint isLinked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
        if (isLinked == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

            std::string errorLog;
            errorLog.resize(maxLength);
            glGetProgramInfoLog(program, maxLength, &maxLength, &errorLog[0]);

            spdlog::critical("Unable to link program: {}", errorLog);

            glDeleteProgram(program);
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);

            return InvalidProgram;
        }

        glDetachShader(program, vertexShader);
        glDetachShader(program, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return program;
    }

    static void SetMat4Uniform(GLuint program, std::string_view uniformName, const glm::mat4& value)
    {
        GLint location = glGetUniformLocation(program, uniformName.data());
        glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
    }

    static void SetVec3Uniform(GLuint program, std::string_view uniformName, const glm::vec3& value)
    {
        GLint location = glGetUniformLocation(program, uniformName.data());
        glUniform3fv(location, 1, &value[0]);
    }

    static void SetTextureUniform(GLuint program, std::string_view uniformName, GLuint value)
    {
        GLint location = glGetUniformLocation(program, uniformName.data());
        glUniform1i(location, value);
    }

private:
    static GLuint LoadShader(std::string_view shaderPath, GLenum shaderType)
    {
        std::ifstream file(shaderPath.data());
        if (!file.is_open())
        {
            spdlog::critical("Unable to open file: \"{}\"", shaderPath);
            return 0;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string fileContent = buffer.str();
        const char* contentPtr = fileContent.c_str();

        GLuint shader = glCreateShader(shaderType);
        glShaderSource(shader, 1, &contentPtr, nullptr);
        glCompileShader(shader);

        GLint isCompiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

            std::string errorLog;
            errorLog.resize(maxLength);
            glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

            spdlog::critical("Unable to compile shader \"{}\". {}", shaderPath, errorLog);

            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }
};
