#include "SceneModel.h"
#include <iostream>

#include <boost/filesystem.hpp>

// assimp (asset importer library)
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include "NUGL/ShaderProgram.h"

#include "utility/make_unique.h"
#include "utility/debug.h"
#include "utility/AssimpDebug.h"

unsigned int getPostProcessingFlags() {
    unsigned int pFlags = 0x0000;
//    pFlags |= aiProcess_CalcTangentSpace;         // Calculates the tangents and bitangents for the imported meshes.
    pFlags |= aiProcess_JoinIdenticalVertices;    // Identifies and joins identical vertex data sets within all imported meshes.
//    pFlags |= aiProcess_MakeLeftHanded;           // Converts all the imported data to a left-handed coordinate space.
    pFlags |= aiProcess_Triangulate;              // Triangulates all faces of all meshes.
//    pFlags |= aiProcess_RemoveComponent;          // Removes some parts of the data structure (animations, materials, light sources, cameras, textures, vertex components).
//    pFlags |= aiProcess_GenNormals;               // Generates normals for all faces of all meshes.
//    pFlags |= aiProcess_GenSmoothNormals;         // Generates smooth normals for all vertices in the mesh.
    pFlags |= aiProcess_SplitLargeMeshes;         // Splits large meshes into smaller sub-meshes.
//    pFlags |= aiProcess_PreTransformVertices;     // Removes the node graph and pre-transforms all vertices with the local transformation matrices of their nodes.
//    pFlags |= aiProcess_LimitBoneWeights;         // Limits the number of bones simultaneously affecting a single vertex to a maximum value.
    pFlags |= aiProcess_ValidateDataStructure;    // Validates the imported scene data structure. This makes sure that all indices are valid, all animations and bones are linked correctly, all material references are correct .. etc.
//    pFlags |= aiProcess_ImproveCacheLocality;     // Reorders triangles for better vertex cache locality.
    pFlags |= aiProcess_RemoveRedundantMaterials; // Searches for redundant/unreferenced materials and removes them.
//    pFlags |= aiProcess_FixInfacingNormals;       // This step tries to determine which meshes have normal vectors that are facing inwards and inverts them.
    pFlags |= aiProcess_SortByPType;              // This step splits meshes with more than one primitive type in homogeneous sub-meshes.
    pFlags |= aiProcess_FindDegenerates;          // This step searches all meshes for degenerate primitives and converts them to proper lines or points.
    pFlags |= aiProcess_FindInvalidData;          // This step searches all meshes for invalid data, such as zeroed normal vectors or invalid UV coords and removes/fixes them. This is intended to get rid of some common exporter errors.
    pFlags |= aiProcess_GenUVCoords;              // This step converts non-UV mappings (such as spherical or cylindrical mapping) to proper texture coordinate channels.
//    pFlags |= aiProcess_TransformUVCoords;        // This step applies per-texture UV transformations and bakes them into stand-alone vtexture coordinate channels.
//    pFlags |= aiProcess_FindInstances;            // This step searches for duplicate meshes and replaces them with references to the first mesh.
    pFlags |= aiProcess_OptimizeMeshes;           // A postprocessing step to reduce the number of meshes.
    pFlags |= aiProcess_OptimizeGraph;            // A postprocessing step to optimize the scene hierarchy.
    pFlags |= aiProcess_FlipUVs;                  // This step flips all UV coordinates along the y-axis and adjusts material settings and bitangents accordingly.
//    pFlags |= aiProcess_FlipWindingOrder;         // This step adjusts the output face winding order to be CW.
//    pFlags |= aiProcess_SplitByBoneCount;         // This step splits meshes with many bones into sub-meshes so that each su-bmesh has fewer or as many bones as a given limit.
//    pFlags |= aiProcess_Debone;                   // This step removes bones losslessly or according to some threshold.
    return pFlags;
}

SceneModel::Material copyAiMaterial(const std::string& fileName, const aiMaterial* srcMaterial) {
    SceneModel::Material material;

    aiColor3D aiCol;
    srcMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiCol);
    material.colDiffuse.r = aiCol.r;
    material.colDiffuse.g = aiCol.g;
    material.colDiffuse.b = aiCol.b;

    auto diffTexCount = srcMaterial->GetTextureCount(aiTextureType_DIFFUSE);
    for (unsigned int t = 0; t < diffTexCount; t++) {
        aiString path;
        srcMaterial->GetTexture(aiTextureType_DIFFUSE, t, &path);
        boost::filesystem::path p(fileName);
        boost::filesystem::path dir = p.parent_path();
        dir += path.C_Str();
//        std::cout << __FILE__ << ", " << __LINE__ << ": " << dir.string() << std::endl;

        material.texDiffuse = std::make_unique<NUGL::Texture>(GL_TEXTURE0 + t, GL_TEXTURE_2D);
        material.texDiffuse->loadFromImage(dir.string());
        material.texDiffuse->setParam(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        material.texDiffuse->setParam(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        material.texDiffuse->setParam(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        material.texDiffuse->setParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break; // Only use first texture. TODO: Support multiple textures.
    }

    return std::move(material);
}

void copyAiNode(const aiNode *pNode, SceneModel::Node& node) {
    node.meshes.assign(pNode->mMeshes, pNode->mMeshes + pNode->mNumMeshes);

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            node.transform[col][row] = pNode->mTransformation[row][col];
        }
    }

    for (unsigned int i = 0; i < pNode->mNumChildren; i++) {
        node.children.emplace_back();
        copyAiNode(pNode->mChildren[i], node.children.back());
    }
}

SceneModel SceneModel::loadFromFile(const std::string& fileName) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(fileName, getPostProcessingFlags());

    if(!scene) {
        std::stringstream errMsg;
        errMsg << __func__
                << ": Could not import model from '" << fileName << "': "
                << importer.GetErrorString() << "'.";
        throw std::invalid_argument(errMsg.str().c_str());
    }

//    utility::printAiSceneInfo(scene);

    SceneModel sceneModel;

    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        auto* material = scene->mMaterials[i];
        sceneModel.materials.push_back(std::make_shared<Material>(copyAiMaterial(fileName, material)));
    }

    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        auto* sceneMesh = scene->mMeshes[m];
        Mesh mesh;
        mesh.materialIndex = sceneMesh->mMaterialIndex;
        mesh.material = sceneModel.materials[mesh.materialIndex];

        for (unsigned int v = 0; v < sceneMesh->mNumVertices; v++) {
            auto& meshVertex = sceneMesh->mVertices[v];
            glm::vec3 vertex = {meshVertex.x, meshVertex.y, meshVertex.z};
            mesh.vertices.push_back(vertex);
        }

        if (sceneMesh->HasTextureCoords(0)) {
            for (unsigned int i = 0; i < sceneMesh->mNumVertices; i++) {
                auto& meshTexCoord = sceneMesh->mTextureCoords[0][i];
                glm::vec2 texCoord = {meshTexCoord.x, meshTexCoord.y};
                mesh.texCoords.push_back(texCoord);
            }
        }

        for (unsigned int f = 0; f < sceneMesh->mNumFaces; f++) {
            auto& meshFace = sceneMesh->mFaces[f];
            for (unsigned int i = 0; i < meshFace.mNumIndices; i++)
                mesh.elements.push_back(meshFace.mIndices[i]);
        }

        sceneModel.meshes.push_back(std::move(mesh));
    }

    copyAiNode(scene->mRootNode, sceneModel.rootNode);

    return sceneModel;
}

void SceneModel::createMeshBuffers() {
    for (auto& mesh : meshes) {
        std::vector<GLfloat> vertices;
//        for (auto& vertex : mesh.vertices) {
        for (unsigned int i = 0; i < mesh.vertices.size(); i++) {
            auto& vertex = mesh.vertices[i];
            vertices.push_back(vertex.x);
            vertices.push_back(vertex.y);
            vertices.push_back(vertex.z);

            if (mesh.isTextured()) {
                auto& texCoord = mesh.texCoords[i];
//#warning Texcoords are inverted!
//                vertices.push_back(1 - texCoord.x); // TODO: fix texcoords ??
//                vertices.push_back(1 - texCoord.y);
                vertices.push_back(texCoord.x);
                vertices.push_back(texCoord.y);
            }
        }
        mesh.vertexBuffer = std::make_unique<NUGL::Buffer>();
        mesh.vertexBuffer->setData(GL_ARRAY_BUFFER, vertices, GL_STATIC_DRAW);

        mesh.elementBuffer = std::make_unique<NUGL::Buffer>();
        mesh.elementBuffer->setData(GL_ELEMENT_ARRAY_BUFFER, mesh.elements, GL_STATIC_DRAW);
    }
}

void SceneModel::createVertexArrays() {
    for (auto& mesh : meshes) {
        std::vector<NUGL::VertexAttribute> attribs = {
                {"position", 3, GL_FLOAT, GL_FALSE},
        };

        auto program = flatProgram;
        if (mesh.isTextured()) {
            attribs.push_back({"texcoord", 2, GL_FLOAT, GL_FALSE});
            program = textureProgram;
        }

        if (mesh.isEnvironmentMapped()) {
            program = environmentMapProgram;
        }

        mesh.shaderProgram = program;

        mesh.shaderProgram->use();
        mesh.vertexArray = std::make_unique<NUGL::VertexArray>();
        mesh.vertexArray->bind();
        mesh.vertexArray->setAttributePointers(*program, *mesh.vertexBuffer, GL_ARRAY_BUFFER, attribs);
    }
}

void SceneModel::draw(glm::mat4& model, Camera& camera) {
    drawNode(rootNode, model, camera);
}

void SceneModel::drawNode(SceneModel::Node &node, glm::mat4 &parentModel, Camera& camera) {
    glm::mat4 model = parentModel * node.transform;

    // TODO: Find a better way of managing shader programs!
    if (textureProgram != nullptr) {
        textureProgram->use();
        textureProgram->setUniform("model", model);
        textureProgram->setUniform("view", camera.view);
        textureProgram->setUniform("proj", camera.proj);
    }

    if (flatProgram != nullptr) {
        flatProgram->use();
        flatProgram->setUniform("model", model);
        flatProgram->setUniform("view", camera.view);
        flatProgram->setUniform("proj", camera.proj);
    }

    if (environmentMapProgram != nullptr) {
        environmentMapProgram->use();
        environmentMapProgram->setUniform("model", model);
        environmentMapProgram->setUniform("view", camera.view);
        environmentMapProgram->setUniform("proj", camera.proj);

        glm::mat4 modelViewInverse = glm::inverse(camera.view * model);
        environmentMapProgram->setUniform("modelViewInverse", modelViewInverse);
    }

    checkForAndPrintGLError(__FILE__, __LINE__);

    for (int index : node.meshes) {
        auto& mesh = meshes[index];
        mesh.shaderProgram->use();

        if (mesh.isEnvironmentMapped()) {
            checkForAndPrintGLError(__FILE__, __LINE__);
            mesh.material->environmentMap->bind();
            checkForAndPrintGLError(__FILE__, __LINE__);
            // TODO: Make setting textures as uniforms cleaner.
            mesh.shaderProgram->setUniform("environmentMap", mesh.material->environmentMap->unit() - GL_TEXTURE0);
        } else if (mesh.isTextured()) {
            mesh.material->texDiffuse->bind();
            mesh.shaderProgram->setUniform("tex", mesh.material->texDiffuse->unit() - GL_TEXTURE0);
        } else {
            mesh.shaderProgram->setUniform("materialColour", mesh.material->colDiffuse);
        }

        mesh.vertexArray->bind();

        mesh.vertexBuffer->bind(GL_ARRAY_BUFFER);
        mesh.elementBuffer->bind(GL_ELEMENT_ARRAY_BUFFER);
        glDrawElements(GL_TRIANGLES, mesh.elements.size(), GL_UNSIGNED_INT, 0);


        checkForAndPrintGLError(__FILE__, __LINE__);
    }

    for (auto& child : node.children) {
        drawNode(child, model, camera);
    }
}
