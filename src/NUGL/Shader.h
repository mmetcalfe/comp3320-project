#pragma once
#include <iostream>
#include <stdexcept>
#include <sstream>

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
        std::cout << "printShaderDebugInfo(" << shaderId << ") {" << std::endl;
        std::cout << "  GL_SHADER_TYPE: ";
        switch (shaderType) {
            case GL_VERTEX_SHADER: std::cout << "GL_VERTEX_SHADER"; break;
            case GL_FRAGMENT_SHADER: std::cout << "GL_FRAGMENT_SHADER"; break;
            default: std::cout << "INVALID (" << shaderType << ")"; break;
        }
        std::cout << "," << std::endl;
        std::cout << "  GL_DELETE_STATUS: " << (deleteStatus == GL_TRUE ? "Flagged for deletion" : "False") << std::endl;
        std::cout << "  GL_COMPILE_STATUS: " << (compileStatus == GL_TRUE ? "Success" : "Failure") << std::endl;
        std::cout << "  GL_INFO_LOG_LENGTH: " << infoLogLength << std::endl;
        std::cout << "  GL_SHADER_SOURCE_LENGTH: " << sourceLength << std::endl;
        std::cout << "  glGetShaderInfoLog: " << infoLogBuff << std::endl;
        // std::cout << "  glGetShaderSource: %s,\n", srcBuff << std::endl;
        std::cout << "}" << std::endl;

        if (compileStatus != GL_TRUE) {
            std::stringstream errMsg;
            errMsg << __func__ << ": Compile error in shader: " << infoLogBuff << ".";
            throw std::logic_error(errMsg.str());
        }
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

        inline void setSource(const std::string& sourceString) {
            auto shaderSource = sourceString.c_str();
            glShaderSource(shaderId, 1, &shaderSource, nullptr);
            checkForAndPrintGLError(__FILE__, __LINE__);
        }

        inline void setSourceFromFile(const std::string& fileName) {
            auto fileString = utility::strutil::getFileString(fileName);
            auto shaderSource = fileString.c_str();
            glShaderSource(shaderId, 1, &shaderSource, nullptr);
            checkForAndPrintGLError(__FILE__, __LINE__);
        }

        inline void compile() {
            glCompileShader(shaderId);
            checkForAndPrintGLError(__FILE__, __LINE__);
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
