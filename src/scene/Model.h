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
            return material->materialInfo.has.texEnvironmentMap;
        }
    };

    class Model {
    public:
        struct Node {
            std::vector<int> meshes;
            std::vector<Node> children;
            glm::mat4 transform;
        };

        Model() = delete;

        Model(std::string modelName) {
            this->modelName = modelName;
        }

        std::string modelName;

        std::vector<Mesh> meshes;
        std::vector<std::shared_ptr<Light>> lights;
        std::vector<std::shared_ptr<Material>> materials;
        Node rootNode;

        std::shared_ptr<NUGL::ShaderProgram> flatProgram;
        std::shared_ptr<NUGL::ShaderProgram> textureProgram;
        std::shared_ptr<NUGL::ShaderProgram> environmentMapProgram;
        std::shared_ptr<NUGL::ShaderProgram> shadowMapProgram;

        // TODO: Improve environment map management.
        bool dynamicReflections = false;
        std::shared_ptr<NUGL::Texture> texEnvironmentMap;


        glm::vec3 pos = {0, 0, 0}; // The object's position in world space.
        glm::vec3 dir = {1, 0, 0}; // The object's x-axis in world space.
        glm::vec3 up  = {0, 0, 1}; // Along with 'dir', defines the plane containing the object's z-axis.
        glm::vec3 scale = {1, 1, 1}; // Scale along each of the object's axes.
        glm::mat4 transform; // Model transform generated from the above components.

        glm::mat4 buildModelTransform(glm::vec3 pos, glm::vec3 dir, glm::vec3 up, glm::vec3 scale);

        void createMeshBuffers();

        void createVertexArrays();

        void draw(Camera &camera, std::shared_ptr<Light> light, std::shared_ptr<LightCamera> lightCamera);
        void drawNode(Model::Node &node, glm::mat4 parentModel, Camera &camera, std::shared_ptr<Light> light,
                std::shared_ptr<LightCamera> lightCamera);

        void drawDepth(Camera &camera);
        void drawNodeDepth(Model::Node &node, glm::mat4 parentModel, Camera &camera);

        static std::shared_ptr<Model> loadFromFile(const std::string &fileName);

        static std::shared_ptr<Model> createIcosahedron();

        void setEnvironmentMap(std::shared_ptr<NUGL::Texture> envMap);

        void setCameraUniformsOnShaderPrograms(Camera &camera, glm::mat4 model);

        void setLightUniformsOnShaderPrograms(std::shared_ptr<Light> light, std::shared_ptr<LightCamera> lightCamera);

        void prepareMaterialShaderProgram(std::shared_ptr<Material> material, std::shared_ptr<NUGL::ShaderProgram> shaderProgram);
    };
}
