#pragma once
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace NUGL {
    class Renderbuffer {
    public:
        Renderbuffer() {
            glGenRenderbuffers(1, &bufferId);
        }

        ~Renderbuffer() {
            glDeleteRenderbuffers(1, &bufferId);
        }

        inline void bind(GLenum target) {
            glBindRenderbuffer(target, bufferId);
        }

        inline void setStorage(GLenum target, GLenum internalFormat, GLenum width, GLenum height) {
            bind(target);
            glRenderbufferStorage(target, internalFormat, width, height);
            checkForAndPrintGLError(__FILE__, __LINE__);
        }

        inline GLuint id() {
            return bufferId;
        }

    private:
        GLuint bufferId;
    };
}
