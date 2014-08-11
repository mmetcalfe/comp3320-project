#pragma once
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace NUGL {
    class Buffer {
    public:
        Buffer() {
            glGenBuffers(1, &bufferId);
        }

        ~Buffer() {
            glDeleteBuffers(1, &bufferId);
        }

        inline void bind(GLenum target) {
            glBindBuffer(target, bufferId);
        }

        template<typename T>
        inline void setData(GLenum target, std::vector<T> data, GLenum usage) {
            bind(target);
            glBufferData(target, data.size() * sizeof(T), data.data(), usage);
        }

        inline GLuint id() {
            return bufferId;
        }

    private:
        GLuint bufferId;
    };
}
