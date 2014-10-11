#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <cstdint>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "utility/debug.h"
#include "NUGL/Shader.h"
#include "NUGL/Texture.h"

namespace NUGL {
    union MaterialInfo {
        uint16_t bitSet = 0;
        struct {
            uint16_t colAmbient        : 1;
            uint16_t colDiffuse        : 1;
            uint16_t colSpecular       : 1;
            uint16_t colTransparent    : 1;
            uint16_t opacity           : 1;
            uint16_t shininess         : 1;
            uint16_t shininessStrength : 1;
            uint16_t reserved_value    : 1; // Reserved for future use.
            uint16_t texEnvironmentMap : 1;
            uint16_t texDiffuse        : 1;
            uint16_t texSpecular       : 1;
            uint16_t texAmbient        : 1;
            uint16_t texHeight         : 1;
            uint16_t texNormals        : 1;
            uint16_t texShininess      : 1;
            uint16_t texOpacity        : 1;
        } has;
    };

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
        char infoLogBuff[512] = {0};
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
            errMsg << __func__ << ": Link error in shader program '" << programName << "': " << infoLogBuff << ".";
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

        static inline std::shared_ptr<ShaderProgram> createSharedFromFiles(const std::string& name, std::vector<std::pair<GLenum, std::string>> shaders) {
            auto program = std::make_shared<ShaderProgram>(name);

            for (auto& pair : shaders) {
                auto shader = std::make_shared<Shader>(pair.first, pair.second);
                shader->compile();
                shader->printDebugInfo();
                program->attachShader(shader);
            }
            return program;
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
            checkForAndPrintGLError(__FILE__, __LINE__, programName);
        }

        inline void bindFragDataLocation(GLuint colorNumber, const std::string& name) {
            glBindFragDataLocation(programId, colorNumber, name.c_str());
            checkForAndPrintGLError(__FILE__, __LINE__, programName);
        }

        inline void link() {
            glLinkProgram(programId);
            checkForAndPrintGLError(__FILE__, __LINE__, programName);

            // TODO: After linking, detach all shaders and remove them from the shaders list.
        }

        inline void use() {
            glUseProgram(programId);
            checkForAndPrintGLError(__FILE__, __LINE__, programName);
        }

        inline bool uniformIsActive(const std::string& name) {
            GLint uniLoc = glGetUniformLocation(programId, name.c_str());
            return (uniLoc != -1);
        }

        inline GLint getUniformLocation(const std::string& name) {
            GLint uniLoc = glGetUniformLocation(programId, name.c_str());
            checkForAndPrintGLError(__FILE__, __LINE__, programName);

            if (uniLoc == -1) {
                std::stringstream errMsg;
                errMsg << __func__ << ", " << programName
                       << ": The named uniform '" << name << "' does not exist or is not active.";
                throw std::logic_error(errMsg.str());
            }

            return uniLoc;
        }

        inline bool attributeIsActive(const std::string& name) {
            GLint attribLoc = glGetAttribLocation(programId, name.c_str());
            return (attribLoc != -1);
        }

        inline GLint getAttribLocation(const std::string& name) {
            GLint attribLoc = glGetAttribLocation(programId, name.c_str());
            checkForAndPrintGLError(__FILE__, __LINE__, programName);

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

        inline GLuint id() const {
            return programId;
        }

        inline void validate() {
            glValidateProgram(programId);
            checkForAndPrintGLError(__FILE__, __LINE__, programName);
        }

        template<typename T>
        inline void setUniformIfActive(const std::string& name, const T& value) {
            if (uniformIsActive(name))
                setUniform(name, value);
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

        inline void setUniform(GLint uniLoc, GLfloat value) {
            glUniform1f(uniLoc, value);
            checkForAndPrintGLError(__FILE__, __LINE__, programName);
        }

        inline void setUniform(GLint uniLoc, std::shared_ptr<NUGL::Texture> value) {
            glUniform1i(uniLoc, value->unit() - GL_TEXTURE0);
            checkForAndPrintGLError(__FILE__, __LINE__, programName);
        }

        inline void setUniform(GLint uniLoc, NUGL::Texture value) {
            glUniform1i(uniLoc, value.unit() - GL_TEXTURE0);
//            std::cout << uniLoc << " " << (value.unit() - GL_TEXTURE0);
            checkForAndPrintGLError(__FILE__, __LINE__, programName);
        }

        //! Populates the program's material info.
        inline void updateMaterialInfo() {
            materialInfo.bitSet = 0;
            materialInfo.has.colAmbient = uniformIsActive("colAmbient");
            materialInfo.has.colDiffuse = uniformIsActive("colDiffuse");
            materialInfo.has.colSpecular = uniformIsActive("colSpecular");
            materialInfo.has.colTransparent = uniformIsActive("colTransparent");
            materialInfo.has.opacity = uniformIsActive("opacity");
            materialInfo.has.shininess = uniformIsActive("shininess");
            materialInfo.has.shininessStrength = uniformIsActive("shininessStrength");
//            materialInfo.has.reserved_value_1 = uniformIsActive("reserved_value");
            materialInfo.has.texEnvironmentMap = uniformIsActive("texEnvironmentMap");
            materialInfo.has.texDiffuse = uniformIsActive("texDiffuse");
            materialInfo.has.texSpecular = uniformIsActive("texSpecular");
            materialInfo.has.texAmbient = uniformIsActive("texAmbient");
            materialInfo.has.texHeight = uniformIsActive("texHeight");
            materialInfo.has.texNormals = uniformIsActive("texNormals");
            materialInfo.has.texShininess = uniformIsActive("texShininess");
            materialInfo.has.texOpacity = uniformIsActive("texOpacity");
        }

        bool operator==(const NUGL::ShaderProgram &other) const {
            return id() == other.id();
        }

        std::string name() {
            return programName;
        }

    private:
        GLuint programId;
        std::vector<std::shared_ptr<Shader>> shaders;

        std::string programName; //!< Identifies the program in debug messages

    public:
        // TODO: Consider moving this (and MaterialInfo) outside of ShaderProgram (and NUGL?).
        //! Describes which uniforms the shader program accepts.
        MaterialInfo materialInfo;
    };
}

namespace std {
    template<>
    struct hash<NUGL::ShaderProgram> {
        std::size_t operator()(const NUGL::ShaderProgram &k) const {
            return std::hash<GLuint>()(k.id());
        }
    };
}
