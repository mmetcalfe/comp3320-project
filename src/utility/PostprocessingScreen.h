#pragma once
#include <memory>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "NUGL/Buffer.h"
#include "NUGL/VertexArray.h"
#include "NUGL/ShaderProgram.h"
#include "scene/Material.h"
#include "scene/Mesh.h"

namespace utility {

    class PostprocessingScreen {
    public:

        inline PostprocessingScreen(std::shared_ptr<NUGL::ShaderProgram> screenProgram) {
            this->screenProgram = screenProgram;

            auto mesh = std::make_unique<scene::Mesh>();
            mesh->materialIndex = 0;

            float s = 1;
            mesh->vertices = {
                glm::vec3(-s,  s, 0.0f),
                glm::vec3( s,  s, 0.0f),
                glm::vec3( s, -s, 0.0f),
                glm::vec3(-s, -s, 0.0f),
            };
            mesh->texCoords = {
                glm::vec2(0.0f, 1.0f),
                glm::vec2(1.0f, 1.0f),
                glm::vec2(1.0f, 0.0f),
                glm::vec2(0.0f, 0.0f),
            };
            mesh->elements = {
                    0, 1, 2,
                    2, 3, 0
            };

            auto material = std::make_shared<scene::Material>();
            material->twoSided = false;
            material->materialInfo.has.texDiffuse = false;
            mesh->material = material;

            mesh->generateBuffers(true);

            screenMesh = std::move(mesh);
        }

        inline void setTexture(std::shared_ptr<NUGL::Texture> tex) {
            screenMesh->material->texDiffuse = tex;
            screenMesh->material->materialInfo.has.texDiffuse = true;
        }

        inline void removeTexture() {
            screenMesh->material->texDiffuse = nullptr;
            screenMesh->material->materialInfo.has.texDiffuse = false;
        }

        inline void render(float gridDim = 1, float gridX = 0, float gridY = 0) {
            render(screenProgram, gridDim, gridX, gridY);
        }

        inline void render(std::shared_ptr<NUGL::ShaderProgram> program, float gridDim = 1, float gridX = 0, float gridY = 0) {
            if (!screenMesh->isTextured()) {
                std::stringstream errMsg;
                errMsg << "PostprocessingScreen::" << __func__
                        << ": screenMesh must have a texture"
                        << ".";
                throw std::runtime_error(errMsg.str().c_str());
            }

            program->use();

            float width = 1.0f / gridDim;
            glm::mat4 previewModel;
            previewModel = glm::scale(previewModel, glm::vec3(width, width, 1.0f));
            previewModel = glm::translate(previewModel, glm::vec3(gridX * 2 - (gridDim - 1), gridY * 2 - (gridDim - 1), 0.0f));
            program->setUniform("model", previewModel);

            glDisable(GL_DEPTH_TEST);
            screenMesh->draw(program);
            glEnable(GL_DEPTH_TEST);
        }

        std::shared_ptr<NUGL::ShaderProgram> screenProgram;

    private:
        std::unique_ptr<scene::Mesh> screenMesh;
    };
}