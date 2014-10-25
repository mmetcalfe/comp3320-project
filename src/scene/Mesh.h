#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <unordered_map>

#include "NUGL/Buffer.h"
#include "NUGL/Texture.h"
#include "NUGL/VertexArray.h"
#include "NUGL/ShaderProgram.h"
#include "scene/Material.h"

namespace scene {
    struct Mesh {
        int materialIndex;
        std::shared_ptr<Material> material;

        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> texCoords;
        std::vector<GLint> elements;

        std::unique_ptr<NUGL::Buffer> vertexBuffer;
        std::unique_ptr<NUGL::Buffer> elementBuffer;

        std::shared_ptr<NUGL::ShaderProgram> shaderProgram;

        std::unordered_map<NUGL::ShaderProgram, std::unique_ptr<NUGL::VertexArray>> vertexArrayMap;

        inline bool isTextured() {
            return (bool)(material->materialInfo.has.texDiffuse || material->materialInfo.has.texHeight) && !texCoords.empty();
        }

        inline bool hasNormals() {
            return !normals.empty();
        }

        inline bool isEnvironmentMapped() {
            return (bool)material->materialInfo.has.texEnvironmentMap;
        }

        void generateBuffers(bool forceTexcoords = false);
        void draw(std::shared_ptr<NUGL::ShaderProgram> program);
        void prepareVertexArrayForShaderProgram(std::shared_ptr<NUGL::ShaderProgram> shadowMapProgram);
        void prepareMaterialShaderProgram(std::shared_ptr<NUGL::ShaderProgram> program);
    };
}
