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
//            checkForAndPrintGLError(__FILE__, __LINE__);
        }

        ~Framebuffer() {
            glDeleteFramebuffers(1, &bufferId);
//            checkForAndPrintGLError(__FILE__, __LINE__);
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

        inline void attach(std::unique_ptr<Texture> tex, GLenum attachment = GL_COLOR_ATTACHMENT0, GLenum target = GL_TEXTURE_2D) {
            bind(GL_FRAMEBUFFER);
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, target, tex->id(), 0);
            textureAttachment = std::move(tex);
            checkForAndPrintGLError(__FILE__, __LINE__);
        }

        inline void attach(std::unique_ptr<Renderbuffer> rbo) {
            bind(GL_FRAMEBUFFER);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo->id());
            renderbufferAttachment = std::move(rbo);
            checkForAndPrintGLError(__FILE__, __LINE__);

        }

        std::shared_ptr<Texture> textureAttachment;
    private:
        GLuint bufferId;
        std::unique_ptr<Renderbuffer> renderbufferAttachment;
    };
}
