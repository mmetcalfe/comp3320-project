#pragma once

#include <memory>
#include <glm/glm.hpp>

#include "NUGL/Buffer.h"
#include "NUGL/Texture.h"
#include "NUGL/VertexArray.h"
#include "NUGL/ShaderProgram.h"
#include "scene/Mesh.h"
#include "scene/Light.h"
#include "scene/Camera.h"
#include "scene/Material.h"

namespace scene {
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
//        std::shared_ptr<NUGL::ShaderProgram> shadowMapProgram;

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

        void draw(Camera &camera, std::shared_ptr<Light> light, std::shared_ptr<LightCamera> lightCamera, bool transparentOnly = false);
        void drawNode(Model::Node &node, glm::mat4 parentModel, Camera &camera, std::shared_ptr<Light> light,
                std::shared_ptr<LightCamera> lightCamera, bool transparentOnly = false);

        void draw(Camera &camera, std::shared_ptr<NUGL::ShaderProgram> program, bool transparentOnly = false);
        void drawNodeWithProgram(Model::Node &node, glm::mat4 parentModel, Camera &camera, std::shared_ptr<NUGL::ShaderProgram> program, bool transparentOnly = false);

        static std::shared_ptr<Model> loadFromFile(const std::string &fileName);

        static std::shared_ptr<Model> createIcosahedron();

        void setEnvironmentMap(std::shared_ptr<NUGL::Texture> envMap);

        void setCameraUniformsOnShaderPrograms(Camera &camera, glm::mat4 model);

        static void setLightUniformsOnShaderProgram(std::shared_ptr<NUGL::ShaderProgram> program, std::shared_ptr<Light> light, std::shared_ptr<LightCamera> lightCamera);

        void setCameraUniformsOnShaderProgram(std::shared_ptr<NUGL::ShaderProgram> program, Camera &camera, glm::mat4 model);
    };
}
