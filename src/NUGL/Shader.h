#pragma once
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "utility/debug.h"
#include "utility/strutil.h"

namespace NUGL {
    inline void printShaderDebugInfo(GLuint shaderId) {
        GLint compileStatus;
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus);
        GLint shaderType;
        glGetShaderiv(shaderId, GL_SHADER_TYPE, &shaderType);
        GLint deleteStatus;
        glGetShaderiv(shaderId, GL_DELETE_STATUS, &deleteStatus);
        GLint sourceLength;
        glGetShaderiv(shaderId, GL_SHADER_SOURCE_LENGTH, &sourceLength);
        GLint infoLogLength;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
        char infoLogBuff[512];
        glGetShaderInfoLog(shaderId, 512, nullptr, infoLogBuff);
        // char srcBuff[512];
        // glGetShaderSource(shaderId, 512, nullptr, srcBuff);
        printf("printShaderDebugInfo(%d) {\n", shaderId);
        printf("  GL_SHADER_TYPE: ");
        switch (shaderType) {
            case GL_VERTEX_SHADER: printf("GL_VERTEX_SHADER"); break;
            case GL_FRAGMENT_SHADER: printf("GL_FRAGMENT_SHADER"); break;
            default: printf("INVALID (%d)", shaderType); break;
        }
        printf(",\n");
        printf("  GL_DELETE_STATUS: %s,\n", deleteStatus == GL_TRUE ? "Flagged for deletion" : "False");
        printf("  GL_COMPILE_STATUS: %s,\n", compileStatus == GL_TRUE ? "Success" : "Failure");
        printf("  GL_INFO_LOG_LENGTH: %d,\n", infoLogLength);
        printf("  GL_SHADER_SOURCE_LENGTH: %d,\n", sourceLength);
        printf("  glGetShaderInfoLog: %s,\n", infoLogBuff);
        // printf("  glGetShaderSource: %s,\n", srcBuff);
        printf("}\n");
    }

    class Shader {
    public:
        Shader() = delete;

        explicit Shader(GLenum shaderType) {
            shaderId = glCreateShader(shaderType);
        }

        Shader(GLenum shaderType, const std::string& sourceFileName) {
            shaderId = glCreateShader(shaderType);
            setSourceFromFile(sourceFileName);
        }

        ~Shader() {
            glDeleteShader(shaderId);
        }

        inline void setSource(std::string sourceString) {
            auto shaderSource = sourceString.c_str();
            glShaderSource(shaderId, 1, &shaderSource, nullptr);
        }

        inline void setSourceFromFile(std::string fileName) {
            auto fileString = utility::strutil::getFileString(fileName);
            auto shaderSource = fileString.c_str();
            glShaderSource(shaderId, 1, &shaderSource, nullptr);
        }

        inline void compile() {
            glCompileShader(shaderId);
        }

        inline void printDebugInfo() {
            printShaderDebugInfo(shaderId);
        }

        inline GLuint id() const {
            return shaderId;
        }

    private:
        GLuint shaderId;
    };
}
