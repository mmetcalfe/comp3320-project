#pragma once
#include <iostream>
#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "utility/debug.h"
#include "NUGL/Shader.h"

namespace NUGL {
    inline void printProgramDebugInfo(GLuint programId) {
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
        char buffer[512];
        glGetProgramInfoLog(programId, 512, nullptr, buffer);
        printf("printProgramDebugInfo(%d) {\n", programId);
        printf("  GL_DELETE_STATUS: %s,\n", deleteStatus == GL_TRUE ? "Flagged for deletion" : "False");
        printf("  GL_LINK_STATUS: %s,\n", linkStatus == GL_TRUE ? "Success" : "Failure");
        printf("  GL_VALIDATE_STATUS: %s,\n", validateStatus == GL_TRUE ? "Success" : "Failure");
        printf("  GL_INFO_LOG_LENGTH: %d,\n", infoLogLength);
        printf("  GL_ATTACHED_SHADERS: %d,\n", attachedShaders);
        printf("  GL_ACTIVE_ATTRIBUTES: %d,\n", activeAttributes);
        printf("  GL_ACTIVE_ATTRIBUTE_MAX_LENGTH: %d,\n", activeAttributeMaxLength);
        printf("  GL_ACTIVE_UNIFORMS: %d,\n", activeUniforms);
        printf("  GL_ACTIVE_UNIFORM_MAX_LENGTH: %d,\n", activeUniformMaxLength);
        printf("  glGetProgramInfoLog: %s,\n", buffer);
        printf("}\n");
    }

    class ShaderProgram {
    public:
        ShaderProgram() {
            programId = glCreateProgram();
        }

        static inline ShaderProgram createFromFiles(std::vector<std::pair<GLenum, const std::string&>> shaders) {
            ShaderProgram program;
            for (auto& pair : shaders) {
                std::cout << __func__ << ": " << pair.second << std::endl;
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

        inline void bindFragDataLocation(GLuint colorNumber, const std::string name) {
            glBindFragDataLocation(programId, colorNumber, name.c_str());
        }

        inline void link() {
            glLinkProgram(programId);
        }

        inline void use() {
            glUseProgram(programId);
        }

        inline GLint getUniformLocation(const std::string& name) {
            return glGetUniformLocation(programId, name.c_str());
        }

        inline GLint getAttribLocation(const std::string& name) {
            return glGetAttribLocation(programId, name.c_str());
        }

        inline void printDebugInfo() {
            printProgramDebugInfo(programId);
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
        }

        inline void setUniform(GLint uniLoc, GLint value) {
            glUniform1i(uniLoc, value);
        }

    private:
        GLuint programId;
        std::vector<std::shared_ptr<Shader>> shaders;
    };
}
