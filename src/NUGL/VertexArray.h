#pragma once
#include <stdexcept>
#include <vector>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "utility/debug.h"

namespace NUGL {
    inline int getSizeOfOpenGlType(GLenum type) {
        switch (type) {
            case GL_BYTE: return sizeof(GLbyte);
            case GL_UNSIGNED_BYTE: return sizeof(GLubyte);
            case GL_SHORT: return sizeof(GLshort);
            case GL_UNSIGNED_SHORT: return sizeof(GLushort);
            case GL_INT: return sizeof(GLint);
            case GL_UNSIGNED_INT: return sizeof(GLuint);
            case GL_FLOAT: return sizeof(GLfloat);
            case GL_DOUBLE: return sizeof(GLdouble);
            default: {
                std::stringstream errMsg;
                errMsg << __func__ << ": Unknown type enum value " << type << ".";
                throw std::invalid_argument(errMsg.str());
            }
        }
    }

    struct VertexAttribute {
        std::string name;
        GLint size;
        GLenum type;
        GLboolean normalized;
//        GLsizei stride;
//        const GLvoid* offset;
        bool isPadding;
    };

    class VertexArray {
    public:
        VertexArray() {
            glGenVertexArrays(1, &arrayId);
        }

        ~VertexArray() {
            glDeleteVertexArrays(1, &arrayId);
        }

        inline void bind() {
            glBindVertexArray(arrayId);
        }

        // Assumes sequential, packed vertices.
        inline void setAttributePointers(ShaderProgram& program, Buffer& buffer, GLenum target, std::vector<VertexAttribute> attribs) {
            int stride = 0;
            for (auto& attrib : attribs) {
                stride += attrib.size * getSizeOfOpenGlType(attrib.type);
            }

            bind();
            checkForAndPrintGLError(__func__, __LINE__);

            buffer.bind(target);
            checkForAndPrintGLError(__func__, __LINE__);

            size_t offset = 0;
            for (auto& attrib : attribs) {
                if (!attrib.isPadding) {
                    GLint location = program.getAttribLocation(attrib.name);
                    checkForAndPrintGLError(__func__, __LINE__);

                    glEnableVertexAttribArray(location);
                    checkForAndPrintGLError(__func__, __LINE__, attrib.name);

                    glVertexAttribPointer(
                            location,
                            attrib.size,
                            attrib.type,
                            attrib.normalized,
                            stride,
                            (void *) offset);
                    checkForAndPrintGLError(__func__, __LINE__);
                }

                offset += attrib.size * getSizeOfOpenGlType(attrib.type);
            }
        }

        inline GLuint id() {
            return arrayId;
        }

    private:
        GLuint arrayId;
    };
}
