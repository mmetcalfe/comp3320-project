#pragma once
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "utility/debug.h"

namespace NUGL {
    class Framebuffer {
    public:
        Framebuffer() {
            glGenFramebuffers(1, &bufferId);
        }

        ~Framebuffer() {
            glDeleteFramebuffers(1, &bufferId);
        }

        inline void bind(GLenum target) {
            glBindFramebuffer(target, bufferId);
        }

        static inline void useDefault() {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        inline GLuint id() {
            return bufferId;
        }

    private:
        GLuint bufferId;
    };
}
