#include "scene/Mesh.h"
#include <iostream>
#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "NUGL/ShaderProgram.h"
#include "utility/make_unique.h"
#include "utility/debug.h"
#include "utility/AssimpDebug.h"
#include "scene/Camera.h"

namespace scene {

void Mesh::draw(std::shared_ptr<NUGL::ShaderProgram> program) {
    program->use();
    prepareMaterialShaderProgram(program);

    if (!vertexArrayMap.count(*program)) {
        prepareVertexArrayForShaderProgram(program);
    }

    if (!vertexArrayMap.count(*program)) {
        std::stringstream errMsg;
        errMsg << __FILE__ << ", " << __LINE__ << ", " << __func__ << ": "
                << "Mesh has no vertex array for shader program: '" << program->name() << "'"
                << ".";
        throw std::runtime_error(errMsg.str().c_str());
    }

    if (vertexBuffer == nullptr) {
        std::stringstream errMsg;
        errMsg << __FILE__ << ", " << __LINE__ << ", " << __func__ << ": "
                << "Mesh has a null vertexBuffer.";
        throw std::runtime_error(errMsg.str().c_str());
    }

    vertexArrayMap[*program]->bind();
    vertexBuffer->bind(GL_ARRAY_BUFFER);
    elementBuffer->bind(GL_ELEMENT_ARRAY_BUFFER);
    glDrawElements(GL_TRIANGLES, elements.size(), GL_UNSIGNED_INT, 0);
    checkForAndPrintGLError(__FILE__, __LINE__);
}

void Mesh::prepareVertexArrayForShaderProgram(std::shared_ptr<NUGL::ShaderProgram> program) {
    if (vertexBuffer == nullptr) {
        std::stringstream errMsg;
        errMsg << __FILE__ << ", " << __LINE__ << ", " << __func__ << ": "
                << "Mesh has a null vertexBuffer.";
        throw std::runtime_error(errMsg.str().c_str());
    }

    std::vector<NUGL::VertexAttribute> attribs = {
            {"position", 3, GL_FLOAT, GL_FALSE, !program->attributeIsActive("position")},
    };

    if (hasNormals()) {
        attribs.push_back({"normal", 3, GL_FLOAT, GL_FALSE, !program->attributeIsActive("normal")});
    }

    if (isTextured()) {
        attribs.push_back({"texcoord", 2, GL_FLOAT, GL_FALSE, !program->attributeIsActive("texcoord")});
    }

    program->use();

    auto vertexArray = std::make_unique<NUGL::VertexArray>();
    vertexArray->bind();
    vertexArray->setAttributePointers(*program, *vertexBuffer, GL_ARRAY_BUFFER, attribs);

    vertexArrayMap[*program] = move(vertexArray);
}

void Mesh::prepareMaterialShaderProgram(std::shared_ptr<NUGL::ShaderProgram> program) {
    if (material->materialInfo.has.colAmbient && program->materialInfo.has.colAmbient) {
        program->setUniform("colAmbient", material->colAmbient);
    }

    if (material->materialInfo.has.colDiffuse && program->materialInfo.has.colDiffuse) {
        program->setUniform("colDiffuse", material->colDiffuse);
    }

    if (material->materialInfo.has.colSpecular && program->materialInfo.has.colSpecular) {
        program->setUniform("colSpecular", material->colSpecular);
    }

    if (material->materialInfo.has.colTransparent && program->materialInfo.has.colTransparent) {
        program->setUniform("colTransparent", material->colTransparent);
    }

    if (material->materialInfo.has.opacity && program->materialInfo.has.opacity) {
        program->setUniform("opacity", material->opacity);
    }

    if (material->materialInfo.has.shininess && program->materialInfo.has.shininess) {
        program->setUniform("shininess", material->shininess);
    }

    if (material->materialInfo.has.reflectivity && program->materialInfo.has.reflectivity) {
        program->setUniform("reflectivity", material->reflectivity);
    } else {
        program->setUniformIfActive("reflectivity", 0.0f);
    }

    if (material->materialInfo.has.emissive && program->materialInfo.has.emissive) {
        program->setUniform("emissive", material->emissive);
    } else {
        program->setUniformIfActive("emissive", 0.0f);
    }

    if (material->materialInfo.has.shininessStrength && program->materialInfo.has.shininessStrength) {
        program->setUniform("shininessStrength", material->shininessStrength);
    }

//    if (material->materialInfo.has.reserved_value && program->materialInfo.has.reserved_value) {
//        program->setUniform("reserved_value", material->reserved_value);
//    }


    program->setUniformIfActive("hasTexEnvironmentMap", false);
    if (material->materialInfo.has.texEnvironmentMap && program->materialInfo.has.texEnvironmentMap) {
        if (material->texEnvironmentMap != nullptr) {
            material->texEnvironmentMap->bind();
            program->setUniform("texEnvironmentMap", material->texEnvironmentMap);
            program->setUniformIfActive("hasTexEnvironmentMap", true);
        } else {
            std::cerr << "WARNING: material->materialInfo.has.texEnvironmentMap was true, but texEnvironmentMap was null.";
        }
    }

    program->setUniformIfActive("hasTexDiffuse", false);
    if (material->materialInfo.has.texDiffuse && program->materialInfo.has.texDiffuse) {
        if (material->texDiffuse != nullptr) {
            material->texDiffuse->bind();
            program->setUniform("texDiffuse", material->texDiffuse);
            program->setUniformIfActive("hasTexDiffuse", true);
        } else {
            std::cerr << "WARNING: material->materialInfo.has.texDiffuse was true, but texDiffuse was null.";
        }
    }

    program->setUniformIfActive("hasTexHeight", false);
    if (material->materialInfo.has.texHeight && program->materialInfo.has.texHeight) {
        if (material->texHeight != nullptr) {
            material->texHeight->bind();
            program->setUniform("texHeight", material->texHeight);
            program->setUniform("hasTexHeight", true);
        } else {
            std::cerr << "WARNING: material->materialInfo.has.texHeight was true, but texHeight was null.";
        }
    }

//    if (material->materialInfo.has.texSpecular && program->materialInfo.has.texSpecular) {
//        material->texSpecular->bind();
//        program->setUniform("texSpecular", material->texSpecular);
//    }
//
//    if (material->materialInfo.has.texAmbient && program->materialInfo.has.texAmbient) {
//        material->texAmbient->bind();
//        program->setUniform("texAmbient", material->texAmbient);
//    }
//
//    if (material->materialInfo.has.texNormals && program->materialInfo.has.texNormals) {
//        material->texNormals->bind();
//        program->setUniform("texNormals", material->texNormals);
//    }
//
//    if (material->materialInfo.has.texShininess && program->materialInfo.has.texShininess) {
//        material->texShininess->bind();
//        program->setUniform("texShininess", material->texShininess);
//    }
//
//    if (material->materialInfo.has.texOpacity && program->materialInfo.has.texOpacity) {
//        material->texOpacity->bind();
//        program->setUniform("texOpacity", material->texOpacity);
//    }
}

void Mesh::generateBuffers(bool forceTexcoords) {
    // Verify element buffer correctness:
    for (unsigned int e : elements) {
        if (e >= vertices.size()) {
            std::stringstream errMsg;
            errMsg << __FILE__ << ", " << __LINE__ << ", " << __func__ << ": "
                    << "Element buffer of mesh contains out of range elements"
                    << " (request for vertex " << e << " out of " << vertices.size() << " vertices).";
            throw std::runtime_error(errMsg.str().c_str());
        }
    }

    std::vector<GLfloat> vertexBufferData;
    for (unsigned int i = 0; i < vertices.size(); i++) {
        auto &vertex = vertices[i];
        vertexBufferData.push_back(vertex.x);
        vertexBufferData.push_back(vertex.y);
        vertexBufferData.push_back(vertex.z);

        if (hasNormals()) {
            auto &normal = normals[i];
            vertexBufferData.push_back(normal.x);
            vertexBufferData.push_back(normal.y);
            vertexBufferData.push_back(normal.z);
        }

        if (isTextured() || forceTexcoords) {
            auto &texCoord = texCoords[i];
//                vertexBufferData.push_back(1 - texCoord.x);
//                vertexBufferData.push_back(1 - texCoord.y);
            vertexBufferData.push_back(texCoord.x);
            vertexBufferData.push_back(texCoord.y);
        }
    }

    vertexBuffer = std::make_unique<NUGL::Buffer>();
    vertexBuffer->setData(GL_ARRAY_BUFFER, vertexBufferData, GL_STATIC_DRAW);

    elementBuffer = std::make_unique<NUGL::Buffer>();
    elementBuffer->setData(GL_ELEMENT_ARRAY_BUFFER, elements, GL_STATIC_DRAW);
}


}
