#include "scene/Model.h"
#include <iostream>
#include <memory>

#include <boost/filesystem.hpp>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include "NUGL/ShaderProgram.h"
#include "utility/make_unique.h"
#include "utility/debug.h"
#include "utility/AssimpDebug.h"

namespace scene {

unsigned int getPostProcessingFlags() {
    unsigned int pFlags = 0x0000;
//    pFlags |= aiProcess_CalcTangentSpace;         // Calculates the tangents and bitangents for the imported meshes.
    pFlags |= aiProcess_JoinIdenticalVertices;    // Identifies and joins identical vertex data sets within all imported meshes.
//    pFlags |= aiProcess_MakeLeftHanded;           // Converts all the imported data to a left-handed coordinate space.
    pFlags |= aiProcess_Triangulate;              // Triangulates all faces of all meshes.
//    pFlags |= aiProcess_RemoveComponent;          // Removes some parts of the data structure (animations, materials, light sources, cameras, textures, vertex components).
//    pFlags |= aiProcess_GenNormals;               // Generates normals for all faces of all meshes.
    pFlags |= aiProcess_GenSmoothNormals;         // Generates smooth normals for all vertices in the mesh.
    pFlags |= aiProcess_SplitLargeMeshes;         // Splits large meshes into smaller sub-meshes.
//    pFlags |= aiProcess_PreTransformVertices;     // Removes the node graph and pre-transforms all vertices with the local transformation matrices of their nodes.
//    pFlags |= aiProcess_LimitBoneWeights;         // Limits the number of bones simultaneously affecting a single vertex to a maximum value.
    pFlags |= aiProcess_ValidateDataStructure;    // Validates the imported scene data structure. This makes sure that all indices are valid, all animations and bones are linked correctly, all material references are correct .. etc.
//    pFlags |= aiProcess_ImproveCacheLocality;     // Reorders triangles for better vertex cache locality.
    pFlags |= aiProcess_RemoveRedundantMaterials; // Searches for redundant/unreferenced materials and removes them.
    pFlags |= aiProcess_FixInfacingNormals;       // This step tries to determine which meshes have normal vectors that are facing inwards and inverts them.
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

GLenum getGLTextureWrapForAiTextureMapMode(aiTextureMapMode mapMode) {
    switch (mapMode) {
        case aiTextureMapMode_Wrap:
            return GL_REPEAT;
        case aiTextureMapMode_Clamp:
            return GL_CLAMP_TO_EDGE;
        case aiTextureMapMode_Decal:
            return GL_CLAMP_TO_BORDER;
        case aiTextureMapMode_Mirror:
            return GL_MIRRORED_REPEAT;
        default: {
            std::stringstream errMsg;
            errMsg << __func__
                    << ": Invaid mapMode " << mapMode << ".";
            throw std::invalid_argument(errMsg.str().c_str());
        };
    }
}

Light copyAiLight(const std::string &fileName, const aiLight *srcLight) {
    Light light;
    switch (srcLight->mType) {
        case aiLightSource_SPOT:
            light.type = Light::Type::spot;
            break;
        case aiLightSource_DIRECTIONAL:
            light.type = Light::Type::directional;
            break;
        case aiLightSource_POINT:
            light.type = Light::Type::point;
            break;
        default:
            light.type = Light::Type::undefined;
            break;
    }

    light.pos.x = srcLight->mPosition.x;
    light.pos.y = srcLight->mPosition.y;
    light.pos.z = srcLight->mPosition.z;

    light.dir.x = srcLight->mDirection.x;
    light.dir.y = srcLight->mDirection.y;
    light.dir.z = srcLight->mDirection.z;

    light.colAmbient.r = srcLight->mColorAmbient.r;
    light.colAmbient.g = srcLight->mColorAmbient.g;
    light.colAmbient.b = srcLight->mColorAmbient.b;

    light.colDiffuse.r = srcLight->mColorDiffuse.r;
    light.colDiffuse.g = srcLight->mColorDiffuse.g;
    light.colDiffuse.b = srcLight->mColorDiffuse.b;

    light.colSpecular.r = srcLight->mColorSpecular.r;
    light.colSpecular.g = srcLight->mColorSpecular.g;
    light.colSpecular.b = srcLight->mColorSpecular.b;

    light.attenuationConstant = srcLight->mAttenuationConstant;
    light.attenuationLinear = srcLight->mAttenuationLinear;
    light.attenuationQuadratic = srcLight->mAttenuationQuadratic;

    light.angleConeInner = srcLight->mAngleInnerCone;
    light.angleConeOuter = srcLight->mAngleOuterCone;

    return light;
}

Material copyAiMaterial(const std::string &fileName, const aiMaterial *srcMaterial) {
    // TODO: Support remaining material properties from http://assimp.sourceforge.net/lib_html/materials.html
    Material material;
    material.materialInfo.bitSet = 0;

    aiColor3D aiColAmbient;
    if (!srcMaterial->Get(AI_MATKEY_COLOR_AMBIENT, aiColAmbient)) {
        material.materialInfo.has.colAmbient = true;
        material.colAmbient.r = aiColAmbient.r;
        material.colAmbient.g = aiColAmbient.g;
        material.colAmbient.b = aiColAmbient.b;
    }

    aiColor3D aiColDiffuse;
    if (!srcMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColDiffuse)) {
        material.materialInfo.has.colDiffuse = true;
        material.colDiffuse.r = aiColDiffuse.r;
        material.colDiffuse.g = aiColDiffuse.g;
        material.colDiffuse.b = aiColDiffuse.b;
    }

    aiColor3D aiColSpecular;
    if (!srcMaterial->Get(AI_MATKEY_COLOR_SPECULAR, aiColSpecular)) {
        material.materialInfo.has.colSpecular = true;
        material.colSpecular.r = aiColSpecular.r;
        material.colSpecular.g = aiColSpecular.g;
        material.colSpecular.b = aiColSpecular.b;
    }

    aiColor3D aiColTransparent;
    if (!srcMaterial->Get(AI_MATKEY_COLOR_TRANSPARENT, aiColTransparent)) {
        material.materialInfo.has.colTransparent = true;
        material.colTransparent.r = aiColTransparent.r;
        material.colTransparent.g = aiColTransparent.g;
        material.colTransparent.b = aiColTransparent.b;
    }

    float aiOpacity;
    if (!srcMaterial->Get(AI_MATKEY_OPACITY, aiOpacity)) {
        material.materialInfo.has.opacity = true;
        material.opacity = aiOpacity;
    }

    float aiShininess;
    if (!srcMaterial->Get(AI_MATKEY_SHININESS, aiShininess)) {
        material.materialInfo.has.shininess = true;
        material.shininess = aiShininess;
    }

    float aiShininessStrength;
    if (!srcMaterial->Get(AI_MATKEY_SHININESS_STRENGTH, aiShininessStrength)) {
        material.materialInfo.has.shininessStrength = true;
        material.shininessStrength = aiShininessStrength;
    }

    int aiTwoSided;
    if (!srcMaterial->Get(AI_MATKEY_TWOSIDED, aiTwoSided)) {
        material.twoSided = (aiTwoSided != 0);
    }

    // TODO: Support multiple illumination models:
    //  - http://en.wikipedia.org/wiki/List_of_common_shading_algorithms
    //  - http://www1.cs.columbia.edu/CAVE/projects/oren/
    //  - http://www1.cs.columbia.edu/CAVE/software/curet/
    //  - http://renderman.pixar.com/view/cook-torrance-shader
    aiShadingMode shadingMode;
    if (!srcMaterial->Get(AI_MATKEY_SHADING_MODEL, shadingMode)) {
        std::cerr << __func__ << "Shading mode: " << utility::getAiShadingModeName(shadingMode) << std::endl;
    }

    auto diffTexCount = srcMaterial->GetTextureCount(aiTextureType_DIFFUSE);

    for (unsigned int t = 0; t < diffTexCount; t++) {
        material.materialInfo.has.texDiffuse = true;

        aiString path;
        auto mapModes = std::vector<aiTextureMapMode>(3);
        srcMaterial->GetTexture(aiTextureType_DIFFUSE, t, &path, nullptr, nullptr, nullptr, nullptr,
                mapModes.data());
        boost::filesystem::path p(fileName);
        boost::filesystem::path dir = p.parent_path();
        dir += "/";
        dir += path.C_Str();
//        std::cout << __FILE__ << ", " << __LINE__ << ": " << dir.string() << std::endl;

        material.texDiffuse = std::make_unique<NUGL::Texture>(GL_TEXTURE0 + t, GL_TEXTURE_2D);
        material.texDiffuse->loadFromImage(dir.string());
//      TODO: Read texture settings from Assimp (+ Check for other texture types/layers).
//      TODO: Support 3D textures.
        material.texDiffuse->setParam(GL_TEXTURE_WRAP_S, getGLTextureWrapForAiTextureMapMode(mapModes[0]));
        material.texDiffuse->setParam(GL_TEXTURE_WRAP_T, getGLTextureWrapForAiTextureMapMode(mapModes[1]));
//        material.texDiffuse->setParam(GL_TEXTURE_WRAP_R, getGLTextureWrapForAiTextureMapMode(mapModes[2]));
        material.texDiffuse->setParam(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        material.texDiffuse->setParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break; // Only use first texture. TODO: Support multiple textures.
    }

    return std::move(material);
}

void copyAiNode(const aiNode *pNode, Model::Node &node) {
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

std::shared_ptr<Model> Model::loadFromFile(const std::string &fileName) {
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(fileName, getPostProcessingFlags());

    if (!scene) {
        std::stringstream errMsg;
        errMsg << __func__
                << ": Could not import model from '" << fileName << "': "
                << importer.GetErrorString() << "'.";
        throw std::invalid_argument(errMsg.str().c_str());
    }

    utility::printAiSceneInfo(scene);

    std::shared_ptr<Model> sceneModel = std::make_shared<Model>();

    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        auto *material = scene->mMaterials[i];
        sceneModel->materials.push_back(std::make_shared<Material>(copyAiMaterial(fileName, material)));
    }

    for (unsigned int i = 0; i < scene->mNumLights; i++) {
        auto *light = scene->mLights[i];
        sceneModel->lights.push_back(std::make_shared<Light>(copyAiLight(fileName, light)));
    }

    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        auto *sceneMesh = scene->mMeshes[m];
        Mesh mesh;
        mesh.materialIndex = sceneMesh->mMaterialIndex;
        mesh.material = sceneModel->materials[mesh.materialIndex];

        for (unsigned int i = 0; i < sceneMesh->mNumVertices; i++) {
            auto &meshVertex = sceneMesh->mVertices[i];
            glm::vec3 vertex = {meshVertex.x, meshVertex.y, meshVertex.z};
            mesh.vertices.push_back(vertex);
        }

        for (unsigned int i = 0; i < sceneMesh->mNumVertices; i++) {
            auto &meshNormal = sceneMesh->mNormals[i];
            glm::vec3 normal = {meshNormal.x, meshNormal.y, meshNormal.z};
            mesh.normals.push_back(normal);
        }

        if (sceneMesh->HasTextureCoords(0)) {
            for (unsigned int i = 0; i < sceneMesh->mNumVertices; i++) {
                auto &meshTexCoord = sceneMesh->mTextureCoords[0][i];
                glm::vec2 texCoord = {meshTexCoord.x, meshTexCoord.y};
                mesh.texCoords.push_back(texCoord);
            }
        }

        for (unsigned int f = 0; f < sceneMesh->mNumFaces; f++) {
            auto &meshFace = sceneMesh->mFaces[f];
            for (unsigned int i = 0; i < meshFace.mNumIndices; i++)
                mesh.elements.push_back(meshFace.mIndices[i]);
        }

        sceneModel->meshes.push_back(std::move(mesh));
    }

    copyAiNode(scene->mRootNode, sceneModel->rootNode);

    return sceneModel;
}

void Model::createMeshBuffers() {
    for (auto &mesh : meshes) {
        std::vector<GLfloat> vertices;
//        for (auto& vertex : mesh.vertices) {
        for (unsigned int i = 0; i < mesh.vertices.size(); i++) {
            auto &vertex = mesh.vertices[i];
            vertices.push_back(vertex.x);
            vertices.push_back(vertex.y);
            vertices.push_back(vertex.z);

            if (mesh.hasNormals()) {
                auto &normal = mesh.normals[i];
                vertices.push_back(normal.x);
                vertices.push_back(normal.y);
                vertices.push_back(normal.z);
            }

            if (mesh.isTextured()) {
                auto &texCoord = mesh.texCoords[i];
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

void Model::createVertexArrays() {
    for (auto &mesh : meshes) {
        std::vector<NUGL::VertexAttribute> attribs = {
                {"position", 3, GL_FLOAT, GL_FALSE},
        };

        if (mesh.hasNormals()) {
            attribs.push_back({"normal", 3, GL_FLOAT, GL_FALSE});
        }

        auto program = flatProgram;
        if (mesh.isTextured()) {
            attribs.push_back({"texcoord", 2, GL_FLOAT, GL_FALSE});
            program = textureProgram;
        }

        // TODO: Handle the case of environment mappped but untextured meshes correctly.
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

void Model::draw(Camera &camera, std::shared_ptr<Light> light) {
    drawNode(rootNode, transform, camera, light);
}

void Model::drawNode(Model::Node &node, glm::mat4 parentNodeTransform, Camera &camera, std::shared_ptr<Light> light) {
    glm::mat4 model = parentNodeTransform * node.transform;

    // TODO: Find a better way of managing shader programs!
    setCameraUniformsOnShaderPrograms(camera, model);
    setLightUniformsOnShaderPrograms(light);

    for (int index : node.meshes) {
        auto &mesh = meshes[index];
        mesh.shaderProgram->use();

        prepareMaterialShaderProgram(mesh.material, mesh.shaderProgram);

        mesh.vertexArray->bind();

        mesh.vertexBuffer->bind(GL_ARRAY_BUFFER);
        mesh.elementBuffer->bind(GL_ELEMENT_ARRAY_BUFFER);
        glDrawElements(GL_TRIANGLES, mesh.elements.size(), GL_UNSIGNED_INT, 0);
        checkForAndPrintGLError(__FILE__, __LINE__);
    }

    for (auto &child : node.children) {
        drawNode(child, model, camera, light);
    }
}

void Model::prepareMaterialShaderProgram(std::shared_ptr<Material> material,
        std::shared_ptr<NUGL::ShaderProgram> shaderProgram) {
    if (material->materialInfo.has.colAmbient && shaderProgram->materialInfo.has.colAmbient) {
        shaderProgram->setUniform("colAmbient", material->colAmbient);
    }

    if (material->materialInfo.has.colDiffuse && shaderProgram->materialInfo.has.colDiffuse) {
        shaderProgram->setUniform("colDiffuse", material->colDiffuse);
    }

    if (material->materialInfo.has.colSpecular && shaderProgram->materialInfo.has.colSpecular) {
        shaderProgram->setUniform("colSpecular", material->colSpecular);
    }

    if (material->materialInfo.has.colTransparent && shaderProgram->materialInfo.has.colTransparent) {
        shaderProgram->setUniform("colTransparent", material->colTransparent);
    }

    if (material->materialInfo.has.opacity && shaderProgram->materialInfo.has.opacity) {
        shaderProgram->setUniform("opacity", material->opacity);
    }

    if (material->materialInfo.has.shininess && shaderProgram->materialInfo.has.shininess) {
        shaderProgram->setUniform("shininess", material->shininess);
    }

    if (material->materialInfo.has.shininessStrength && shaderProgram->materialInfo.has.shininessStrength) {
        shaderProgram->setUniform("shininessStrength", material->shininessStrength);
    }

//    if (material->materialInfo.has.reserved_value && shaderProgram->materialInfo.has.reserved_value) {
//        shaderProgram->setUniform("reserved_value", material->reserved_value);
//    }

    if (material->materialInfo.has.texEnvironmentMap && shaderProgram->materialInfo.has.texEnvironmentMap) {
        material->texEnvironmentMap->bind();
        shaderProgram->setUniform("texEnvironmentMap", material->texEnvironmentMap);
    }

    if (material->materialInfo.has.texDiffuse && shaderProgram->materialInfo.has.texDiffuse) {
        material->texDiffuse->bind();
        shaderProgram->setUniform("texDiffuse", material->texDiffuse);
    }

//    if (material->materialInfo.has.texSpecular && shaderProgram->materialInfo.has.texSpecular) {
//        material->texSpecular->bind();
//        shaderProgram->setUniform("texSpecular", material->texSpecular);
//    }
//
//    if (material->materialInfo.has.texAmbient && shaderProgram->materialInfo.has.texAmbient) {
//        material->texAmbient->bind();
//        shaderProgram->setUniform("texAmbient", material->texAmbient);
//    }
//
//    if (material->materialInfo.has.texHeight && shaderProgram->materialInfo.has.texHeight) {
//        material->texHeight->bind();
//        shaderProgram->setUniform("texHeight", material->texHeight);
//    }
//
//    if (material->materialInfo.has.texNormals && shaderProgram->materialInfo.has.texNormals) {
//        material->texNormals->bind();
//        shaderProgram->setUniform("texNormals", material->texNormals);
//    }
//
//    if (material->materialInfo.has.texShininess && shaderProgram->materialInfo.has.texShininess) {
//        material->texShininess->bind();
//        shaderProgram->setUniform("texShininess", material->texShininess);
//    }
//
//    if (material->materialInfo.has.texOpacity && shaderProgram->materialInfo.has.texOpacity) {
//        material->texOpacity->bind();
//        shaderProgram->setUniform("texOpacity", material->texOpacity);
//    }

}

void Model::setCameraUniformsOnShaderPrograms(Camera &camera, glm::mat4 model) {
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
        if (environmentMapProgram != nullptr) {
            environmentMapProgram->use();
            environmentMapProgram->setUniform("model", model);
            environmentMapProgram->setUniform("view", camera.view);
            environmentMapProgram->setUniform("proj", camera.proj);

            if (environmentMapProgram->uniformIsActive("modelViewInverse")) {
                // We don't invert the transforms relating to the model's internal structure.
                glm::mat4 modelViewInverse = glm::inverse(camera.view * transform);
                environmentMapProgram->setUniform("modelViewInverse", modelViewInverse);
            }
        }
    }
}

void Model::setLightUniformsOnShaderPrograms(std::shared_ptr<Light> light) {
//    if (textureProgram != nullptr) {
//        textureProgram->use();
//        textureProgram->setUniform("model", model);
//    }
//
//    if (flatProgram != nullptr) {
//        flatProgram->use();
//        flatProgram->setUniform("model", model);
//    }

    if (environmentMapProgram != nullptr) {
        if (environmentMapProgram->uniformIsActive("light.pos")) {
            environmentMapProgram->use();
            environmentMapProgram->setUniform("light.pos", light->pos);
            environmentMapProgram->setUniform("light.dir", light->dir);
            environmentMapProgram->setUniform("light.attenuationConstant", light->attenuationConstant);
            environmentMapProgram->setUniform("light.attenuationLinear", light->attenuationLinear);
            environmentMapProgram->setUniform("light.attenuationQuadratic", light->attenuationQuadratic);
            environmentMapProgram->setUniform("light.colDiffuse", light->colDiffuse);
            environmentMapProgram->setUniform("light.colSpecular", light->colSpecular);
            environmentMapProgram->setUniform("light.colAmbient", light->colAmbient);
            environmentMapProgram->setUniform("light.angleConeInner", light->angleConeInner);
            environmentMapProgram->setUniform("light.angleConeOuter", light->angleConeOuter);
        }
    }
}

void Model::setEnvironmentMap(std::shared_ptr<NUGL::Texture> envMap) {
    // TODO: Improve environment map management.
    for (auto &mesh : meshes) {
        mesh.material->texEnvironmentMap = envMap;
        mesh.material->materialInfo.has.texEnvironmentMap = true;
    }
}

}