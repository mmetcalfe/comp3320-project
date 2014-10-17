#include "scene/Model.h"
#include <iostream>
#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <boost/filesystem.hpp>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include "NUGL/ShaderProgram.h"
#include "utility/make_unique.h"
#include "utility/debug.h"
#include "utility/AssimpDebug.h"
#include "scene/Camera.h"
#include "scene/Mesh.h"

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
//        std::cerr << __FILE__ << ", " << __LINE__ << ": opacity " << aiOpacity << " " << fileName << std::endl;
    } else {
        material.opacity = 1;
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
    std::cout << "Loading '" << fileName << "'..."<< std::endl;

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(fileName, getPostProcessingFlags());

    if (!scene) {
        std::stringstream errMsg;
        errMsg << __func__
                << ": Could not import model from '" << fileName << "': "
                << importer.GetErrorString() << "'.";
        throw std::invalid_argument(errMsg.str().c_str());
    }

//    utility::printAiSceneInfo(scene);

    std::shared_ptr<Model> sceneModel = std::make_shared<Model>(fileName);

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

std::shared_ptr<Model> Model::createIcosahedron() {
    static const double PHI = 1.61803398874989484820;

    std::shared_ptr<Model> sceneModel = std::make_shared<Model>("icosahedron");

    auto material = std::make_shared<Material>();
    material->colAmbient = {1, 1, 1};
    material->colDiffuse = {1, 1, 1};
    material->colSpecular = {1, 1, 1};
    material->colTransparent = {1, 1, 1};
    material->shininess = 1;
    material->shininessStrength = 1;
    material->twoSided = false;
    material->materialInfo.has.colAmbient = true;
    material->materialInfo.has.colDiffuse = true;
    material->materialInfo.has.colSpecular = true;
    material->materialInfo.has.colTransparent = true;
    material->materialInfo.has.opacity = true;
    material->materialInfo.has.shininess = true;
    material->materialInfo.has.shininessStrength = true;
    sceneModel->materials.push_back(material);

    Mesh mesh;
    mesh.materialIndex = 0;
    mesh.vertices = mesh.normals = {
        glm::normalize(glm::vec3(-1, 0, -PHI)),
        glm::normalize(glm::vec3(1, 0, -PHI)),
        glm::normalize(glm::vec3(1, 0, PHI)),
        glm::normalize(glm::vec3(-1, 0, PHI)),

        glm::normalize(glm::vec3(-PHI, -1, 0)),
        glm::normalize(glm::vec3(-PHI, 1, 0)),
        glm::normalize(glm::vec3(PHI, 1, 0)),
        glm::normalize(glm::vec3(PHI, -1, 0)),

        glm::normalize(glm::vec3(0, -PHI, 1)),
        glm::normalize(glm::vec3(0, -PHI, -1)),
        glm::normalize(glm::vec3(0, PHI, -1)),
        glm::normalize(glm::vec3(0, PHI, 1)),
    };

    mesh.elements = {
        1, 9, 0, 10, 1, 0, 5, 10, 0, 4, 5, 0, 9, 4, 0, 8, 2, 3, 4, 8, 3, 5, 4,
        3, 11, 5, 3, 2, 11, 3, 11, 2, 6, 10, 11, 6, 1, 10, 6, 7, 1, 6, 2, 7, 6,
        11, 10, 5, 9, 8, 4, 7, 2, 8, 9, 7, 8, 1, 7, 9
    };

    mesh.material = sceneModel->materials[0];
    sceneModel->meshes.push_back(std::move(mesh));
    sceneModel->rootNode.meshes.push_back(0);

    return sceneModel;
}

void Model::createMeshBuffers() {
    for (auto &mesh : meshes) {
        mesh.generateBuffers();
    }
}

void Model::createVertexArrays() {
    for (auto &mesh : meshes) {
        auto program = flatProgram;
        if (mesh.isTextured()) {
            program = textureProgram;
        }
        // TODO: Handle the case of environment mappped but untextured meshes correctly.
        if (mesh.isEnvironmentMapped()) {
            program = environmentMapProgram;
        }
        mesh.shaderProgram = program;

        mesh.prepareVertexArrayForShaderProgram(program);
    }
}

glm::mat4 Model::buildModelTransform(glm::vec3 pos, glm::vec3 dir, glm::vec3 up, glm::vec3 scale) {
    glm::mat4 model;
    model = glm::scale(model, scale);

    glm::mat4 orientation = glm::lookAt(pos, pos + dir, up);

    return glm::inverse(orientation) * model;
}

void Model::draw(Camera &camera, std::shared_ptr<NUGL::ShaderProgram> program, bool transparentOnly) {
    transform = buildModelTransform(pos, dir, up, scale);

    drawNodeWithProgram(rootNode, transform, camera, program, transparentOnly);
}

void Model::draw(Camera &camera, std::shared_ptr<Light> light, std::shared_ptr<LightCamera> lightCamera, bool transparentOnly) {
    transform = buildModelTransform(pos, dir, up, scale);

    drawNode(rootNode, transform, camera, light, lightCamera, transparentOnly);
}

void Model::drawNodeWithProgram(Model::Node &node, glm::mat4 parentNodeTransform, Camera &camera, std::shared_ptr<NUGL::ShaderProgram> program, bool transparentOnly) {
    glm::mat4 model = parentNodeTransform * node.transform;

    program->use();
    setCameraUniformsOnShaderProgram(program, camera, model);

    for (int index : node.meshes) {
        auto &mesh = meshes[index];

        if (transparentOnly == (mesh.material->opacity == 1))
            continue;

        mesh.draw(program);
    }

    for (auto &child : node.children) {
        drawNodeWithProgram(child, model, camera, program, transparentOnly);
    }
}

void Model::drawNode(Model::Node &node, glm::mat4 parentNodeTransform, Camera &camera, std::shared_ptr<Light> light,
                    std::shared_ptr<LightCamera> lightCamera, bool transparentOnly) {
    glm::mat4 model = parentNodeTransform * node.transform;

    // TODO: Find a better way of managing shader programs!
    setCameraUniformsOnShaderPrograms(camera, model);
    setLightUniformsOnShaderProgram(environmentMapProgram, light, lightCamera);

    for (int index : node.meshes) {
        auto &mesh = meshes[index];

        if (transparentOnly == (mesh.material->opacity == 1))
            continue;

        mesh.draw(mesh.shaderProgram);
    }

    for (auto &child : node.children) {
        drawNode(child, model, camera, light, lightCamera, transparentOnly);
    }
}


void Model::setCameraUniformsOnShaderPrograms(Camera &camera, glm::mat4 model) {
    if (textureProgram != nullptr) {
        setCameraUniformsOnShaderProgram(textureProgram, camera, model);
    }

    if (flatProgram != nullptr) {
        setCameraUniformsOnShaderProgram(flatProgram, camera, model);
    }

    if (environmentMapProgram != nullptr) {
        setCameraUniformsOnShaderProgram(environmentMapProgram, camera, model);
    }
}

void Model::setCameraUniformsOnShaderProgram(std::shared_ptr<NUGL::ShaderProgram> program, Camera &camera, glm::mat4 model) {
    glm::mat4 mvp = camera.proj * camera.view * model;

    program->use();
    program->setUniformIfActive("model", model);
    program->setUniformIfActive("view", camera.view);
    program->setUniformIfActive("proj", camera.proj);

    program->setUniformIfActive("mvp", mvp);

    // We don't invert the transforms relating to the model's internal structure.
    glm::mat4 modelViewInverse = glm::inverse(camera.view * transform);
    program->setUniformIfActive("modelViewInverse", modelViewInverse);

    glm::mat4 viewInverse = glm::inverse(camera.view);
    program->setUniformIfActive("viewInverse", viewInverse);
}

void Model::setLightUniformsOnShaderProgram(std::shared_ptr<NUGL::ShaderProgram> program, std::shared_ptr<Light> light, std::shared_ptr<LightCamera> lightCamera) {
    if (program != nullptr) {
        if (program->uniformIsActive("light.pos")) {
            program->use();
            program->setUniform("light.pos", light->pos);
            program->setUniform("light.dir", light->dir);
            program->setUniform("light.attenuationConstant", light->attenuationConstant);
            program->setUniform("light.attenuationLinear", light->attenuationLinear);
            program->setUniform("light.attenuationQuadratic", light->attenuationQuadratic);
            program->setUniform("light.colDiffuse", light->colDiffuse);
            program->setUniform("light.colSpecular", light->colSpecular);
            program->setUniform("light.colAmbient", light->colAmbient);
            program->setUniform("light.angleConeInner", light->angleConeInner);
            program->setUniform("light.angleConeOuter", light->angleConeOuter);

            if (program->uniformIsActive("light.texShadowMap")) {
                program->use();

                if (lightCamera != nullptr) {
                    program->setUniform("light.hasShadowMap", true);
                    program->setUniform("light.texShadowMap", lightCamera->shadowMap);
                    program->setUniform("light.view", lightCamera->view);
                    program->setUniform("light.proj", lightCamera->proj);
                } else {
                    program->setUniform("light.hasShadowMap", false);
                }
            }
        }
    }
}

void Model::setEnvironmentMap(std::shared_ptr<NUGL::Texture> envMap) {
    // TODO: Improve environment map management.
    texEnvironmentMap = envMap;

    for (auto &mesh : meshes) {
        mesh.material->texEnvironmentMap = envMap;
        mesh.material->materialInfo.has.texEnvironmentMap = true;
    }
}

}
