#pragma once

#include <memory>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "NUGL/ShaderProgram.h"
#include "NUGL/Texture.h"
#include "NUGL/Buffer.h"
#include "NUGL/VertexArray.h"
#include "scene/Camera.h"
#include "scene/Light.h"

namespace scene {
    struct Material {
        // Colours.
        glm::vec3 colAmbient = {0.7, 0, 0.9};
        glm::vec3 colDiffuse = {0.7, 0, 0.9};
        glm::vec3 colSpecular = {0.7, 0, 0.9};
        glm::vec3 colTransparent = {0.7, 0, 0.9};

        // Opacity of material in [0, 1].
        float opacity = 1;

        // Exponent in phong-shading.
        float shininess = 1;

        // Scales specular color.
        float shininessStrength = 1;

        // Indicates whether backface culling must be disabled.
        bool twoSided = true;

        std::shared_ptr<NUGL::Texture> texDiffuse;
        std::shared_ptr<NUGL::Texture> texEnvironmentMap;

        // Summarises the types of data this material offers.
        NUGL::MaterialInfo materialInfo;
    };

    struct Mesh {
        int materialIndex;
        std::shared_ptr<Material> material;

        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> texCoords;
        std::vector<GLint> elements;

        std::unique_ptr<NUGL::Buffer> vertexBuffer;
        std::unique_ptr<NUGL::Buffer> elementBuffer;
        std::unique_ptr<NUGL::VertexArray> vertexArray;

        std::shared_ptr<NUGL::ShaderProgram> shaderProgram;

        inline bool isTextured() {
            return material->texDiffuse != nullptr && !texCoords.empty();
        }

        inline bool hasNormals() {
            return !normals.empty();
        }

        inline bool isEnvironmentMapped() {
            return material->texEnvironmentMap != nullptr;
        }
    };

    class Model {
    public:
        struct Node {
            std::vector<int> meshes;
            std::vector<Node> children;
            glm::mat4 transform;
        };

        std::vector<Mesh> meshes;
        std::vector<std::shared_ptr<Light>> lights;
        std::vector<std::shared_ptr<Material>> materials;
        Node rootNode;

        std::shared_ptr<NUGL::ShaderProgram> flatProgram;
        std::shared_ptr<NUGL::ShaderProgram> textureProgram;
        std::shared_ptr<NUGL::ShaderProgram> environmentMapProgram;

        glm::mat4 transform; // TODO: Break this into components (pos, rot, scale)?

        void createMeshBuffers();

        void createVertexArrays();

        void draw(Camera &camera, std::shared_ptr<Light> light);

        void drawNode(Model::Node &node, glm::mat4 parentModel, Camera &camera, std::shared_ptr<Light> light);

        static std::shared_ptr<Model>  loadFromFile(const std::string &fileName);

        void setEnvironmentMap(std::shared_ptr<NUGL::Texture> envMap);

        void setCameraUniformsOnShaderPrograms(Camera &camera, glm::mat4 model);

        void setLightUniformsOnShaderPrograms(std::shared_ptr<Light> light);

        void prepareMaterialShaderProgram(std::shared_ptr<Material> material, std::shared_ptr<NUGL::ShaderProgram> shaderProgram);
    };
}
