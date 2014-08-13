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
#include "Camera.h"

class SceneModel {
public:
    struct Light {
        glm::vec3 position;
    };

    struct Material {
        glm::vec3 colDiffuse;
        std::unique_ptr<NUGL::Texture> texDiffuse;
    };

    struct Mesh {
        int materialIndex;
        std::shared_ptr<Material> material;

        std::vector<glm::vec3> vertices;
        std::vector<glm::vec2> texCoords;
        std::vector<GLint> elements;

        std::unique_ptr<NUGL::Buffer> vertexBuffer;
        std::unique_ptr<NUGL::Buffer> elementBuffer;
        std::unique_ptr<NUGL::VertexArray> vertexArray;

        std::shared_ptr<NUGL::ShaderProgram> shaderProgram;

        inline bool isTextured() {
            return material->texDiffuse != nullptr && !texCoords.empty();
        }
    };

    struct Node {
        std::vector<int> meshes;
        std::vector<Node> children;
        glm::mat4 transform;
    };

    std::vector<Mesh> meshes;
    std::vector<std::shared_ptr<Material>> materials;
    Node rootNode;

    std::shared_ptr<NUGL::ShaderProgram> flatProgram;
    std::shared_ptr<NUGL::ShaderProgram> textureProgram;

//    std::unique_ptr<NUGL::VertexArray> vertexArray;

    void createMeshBuffers();
    void createVertexArrays();
    void draw(glm::mat4& model, Camera& camera);
    void drawNode(SceneModel::Node& node, glm::mat4 &parentModel, Camera &camera);
    static SceneModel loadFromFile(const std::string& fileName);
};

