#pragma once
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "utility/debug.h"
#include "NUGL/Texture.h"
#include "NUGL/Renderbuffer.h"

namespace NUGL {
    class Framebuffer {
    public:
        Framebuffer() {
            glGenFramebuffers(1, &bufferId);
        }

        ~Framebuffer() {
            glDeleteFramebuffers(1, &bufferId);
        }

        inline void bind(GLenum target = GL_FRAMEBUFFER) {
            glBindFramebuffer(target, bufferId);
            checkForAndPrintGLError(__FILE__, __LINE__);
        }

        static inline void useDefault() {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        inline GLuint id() {
            return bufferId;
        }

        inline void attach(std::shared_ptr<Texture> tex) {
            bind(GL_FRAMEBUFFER);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex->id(), 0);
            checkForAndPrintGLError(__FILE__, __LINE__);
        }

        inline void attach(Renderbuffer rbo) {
            bind(GL_FRAMEBUFFER);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo.id());
            checkForAndPrintGLError(__FILE__, __LINE__);
        }

    private:
        GLuint bufferId;
    };
}
