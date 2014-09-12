#pragma once
#include <memory>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "NUGL/Buffer.h"
#include "NUGL/VertexArray.h"
#include "NUGL/ShaderProgram.h"

namespace utility {

    class PostprocessingScreen {
    public:

        inline PostprocessingScreen(std::shared_ptr<NUGL::ShaderProgram> screenProgram) {
            this->screenProgram = screenProgram;

            std::vector<GLfloat> quadVertices = {
                    -1.0f, 1.0f, 0.0f, 1.0f,
                    1.0f, 1.0f, 1.0f, 1.0f,
                    1.0f, -1.0f, 1.0f, 0.0f,

                    1.0f, -1.0f, 1.0f, 0.0f,
                    -1.0f, -1.0f, 0.0f, 0.0f,
                    -1.0f, 1.0f, 0.0f, 1.0f
            };
            quadBuffer = std::make_unique<NUGL::Buffer>();
            quadBuffer->setData(GL_ARRAY_BUFFER, quadVertices, GL_STATIC_DRAW);

            std::vector<NUGL::VertexAttribute> attribs = {
                    {"position", 2, GL_FLOAT, GL_FALSE, false},
                    {"texcoord", 2, GL_FLOAT, GL_FALSE, false},
            };

            vertexArray = std::make_unique<NUGL::VertexArray>();
            vertexArray->bind();
            vertexArray->setAttributePointers(*screenProgram, *quadBuffer, GL_ARRAY_BUFFER, attribs);
        }

        inline void render() {
            screenProgram->use();
            vertexArray->bind();
            quadBuffer->bind(GL_ARRAY_BUFFER);
            glDisable(GL_DEPTH_TEST);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glEnable(GL_DEPTH_TEST);
        }

        std::unique_ptr<NUGL::Buffer> quadBuffer;
        std::unique_ptr<NUGL::VertexArray> vertexArray;
        std::shared_ptr<NUGL::ShaderProgram> screenProgram;
    };

}