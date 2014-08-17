#pragma once
#include <iostream>
#include <vector>
#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "utility/debug.h"
#include "NUGL/Shader.h"
#include "NUGL/Texture.h"

namespace NUGL {
    inline void printProgramDebugInfo(GLuint programId, const std::string programName) {
        GLint deleteStatus;
        glGetProgramiv(programId, GL_DELETE_STATUS, &deleteStatus);
        GLint linkStatus;
        glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus);
        GLint validateStatus;
        glGetProgramiv(programId, GL_VALIDATE_STATUS, &validateStatus);
        GLint infoLogLength;
        glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);
        GLint attachedShaders;
        glGetProgramiv(programId, GL_ATTACHED_SHADERS, &attachedShaders);
        GLint activeAttributes;
        glGetProgramiv(programId, GL_ACTIVE_ATTRIBUTES, &activeAttributes);
        GLint activeAttributeMaxLength;
        glGetProgramiv(programId, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &activeAttributeMaxLength);
        GLint activeUniforms;
        glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &activeUniforms);
        GLint activeUniformMaxLength;
        glGetProgramiv(programId, GL_ACTIVE_UNIFORM_MAX_LENGTH, &activeUniformMaxLength);
        char infoLogBuff[512];
        glGetProgramInfoLog(programId, 512, nullptr, infoLogBuff);
        std::cout << "printProgramDebugInfo(" << programId << ", " << programName <<  ") {" << std::endl;
        std::cout << "  GL_DELETE_STATUS: " << (deleteStatus == GL_TRUE ? "Flagged for deletion" : "False") << "," << std::endl;
        std::cout << "  GL_LINK_STATUS: " << (linkStatus == GL_TRUE ? "Success" : "Failure") << "," << std::endl;
        std::cout << "  GL_VALIDATE_STATUS: " << (validateStatus == GL_TRUE ? "Success" : "Failure") << "," << std::endl;
        std::cout << "  GL_INFO_LOG_LENGTH: " << infoLogLength << "," << std::endl;
        std::cout << "  GL_ATTACHED_SHADERS: " << attachedShaders << "," << std::endl;
        std::cout << "  GL_ACTIVE_ATTRIBUTES: " << activeAttributes << "," << std::endl;
        std::cout << "  GL_ACTIVE_ATTRIBUTE_MAX_LENGTH: " << activeAttributeMaxLength << "," << std::endl;
        std::cout << "  GL_ACTIVE_UNIFORMS: " << activeUniforms << "," << std::endl;
        std::cout << "  GL_ACTIVE_UNIFORM_MAX_LENGTH: " << activeUniformMaxLength << "," << std::endl;
        std::cout << "  glGetProgramInfoLog: " << infoLogBuff << "," << std::endl;
        std::cout << "}\n" << std::endl;

        if (linkStatus != GL_TRUE) {
            std::stringstream errMsg;
            errMsg << __func__ << ": Link error in shader program: " << infoLogBuff << ".";
            throw std::logic_error(errMsg.str());
        }
    }

    class ShaderProgram {
    public:
        ShaderProgram() {
            programId = glCreateProgram();
        }

        ShaderProgram(const std::string& name) : ShaderProgram() {
            programName = name;
        }

        static inline ShaderProgram createFromFiles(std::vector<std::pair<GLenum, std::string>> shaders) {
            return createFromFiles("NO_NAME", shaders);
        }

        static inline ShaderProgram createFromFiles(const std::string& name, std::vector<std::pair<GLenum, std::string>> shaders) {
            ShaderProgram program(name);
            for (auto& pair : shaders) {
                auto shader = std::make_shared<Shader>(pair.first, pair.second);
                shader->compile();
                shader->printDebugInfo();
                program.attachShader(shader);
            }
            return program;
        }

        ~ShaderProgram() {
            glDeleteProgram(programId);
        }

        inline void attachShader(std::shared_ptr<Shader> shader) {
            shaders.push_back(shader);
            glAttachShader(programId, shader->id());
        }

        inline void bindFragDataLocation(GLuint colorNumber, const std::string& name) {
            glBindFragDataLocation(programId, colorNumber, name.c_str());
        }

        inline void link() {
            glLinkProgram(programId);
        }

        inline void use() {
            glUseProgram(programId);
        }

        inline GLint getUniformLocation(const std::string& name) {
            GLint uniLoc = glGetUniformLocation(programId, name.c_str());

            if (uniLoc == -1) {
                std::stringstream errMsg;
                errMsg << __func__ << ", " << programName
                       << ": The named uniform '" << name << "' does not exist.";
                throw std::logic_error(errMsg.str());
            }

            return uniLoc;
        }

        inline GLint getAttribLocation(const std::string& name) {
            GLint attribLoc = glGetAttribLocation(programId, name.c_str());

            if (attribLoc == -1) {
                std::stringstream errMsg;
                errMsg << __func__ << ", " << programName
                       << ": The named attribute '" << name << "' does not exist.";
//                throw std::logic_error(errMsg.str());
            }

            return attribLoc;
        }

        inline void printDebugInfo() {
            printProgramDebugInfo(programId, programName);
        }

        inline GLuint id() {
            return programId;
        }

        inline void validate() {
            glValidateProgram(programId);
        }

        template<typename T>
        inline void setUniform(const std::string& name, const T& value) {
            GLint uniLoc = getUniformLocation(name);
            setUniform(uniLoc, value);
        }
        inline void setUniform(GLint uniLoc, const glm::mat4& value, GLboolean transpose = GL_FALSE) {
            glUniformMatrix4fv(uniLoc, 1, transpose, glm::value_ptr(value));
            checkForAndPrintGLError(__FILE__, __LINE__, programName);
        }

        inline void setUniform(GLint uniLoc, const glm::vec3& value) {
            glUniform3fv(uniLoc, 1, glm::value_ptr(value));
            checkForAndPrintGLError(__FILE__, __LINE__, programName);
        }

        inline void setUniform(GLint uniLoc, GLint value) {
            glUniform1i(uniLoc, value);
            checkForAndPrintGLError(__FILE__, __LINE__, programName);
        }

        inline void setUniform(GLint uniLoc, std::shared_ptr<NUGL::Texture> value) {
            glUniform1i(uniLoc, value->unit() - GL_TEXTURE0);
            checkForAndPrintGLError(__FILE__, __LINE__, programName);
        }

    private:
        GLuint programId;
        std::vector<std::shared_ptr<Shader>> shaders;

        std::string programName; //!< Identifies the program in debug messages
    };
}
