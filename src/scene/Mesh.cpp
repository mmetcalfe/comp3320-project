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
        errMsg << __func__
                << ": Mesh has no vertex array for shader program: '" << program->name() << "'"
                << ".";
        throw std::runtime_error(errMsg.str().c_str());
    }

    vertexArrayMap[*program]->bind();
    vertexBuffer->bind(GL_ARRAY_BUFFER);
    elementBuffer->bind(GL_ELEMENT_ARRAY_BUFFER);
    glDrawElements(GL_TRIANGLES, elements.size(), GL_UNSIGNED_INT, 0);
    checkForAndPrintGLError(__FILE__, __LINE__);
}

void Mesh::prepareVertexArrayForShaderProgram(std::shared_ptr<NUGL::ShaderProgram> shadowMapProgram) {
    std::vector<NUGL::VertexAttribute> attribs = {
            {"position", 3, GL_FLOAT, GL_FALSE, !shadowMapProgram->attributeIsActive("position")},
    };

    if (hasNormals()) {
        attribs.push_back({"normal", 3, GL_FLOAT, GL_FALSE, !shadowMapProgram->attributeIsActive("normal")});
    }

    if (isTextured()) {
        attribs.push_back({"texcoord", 2, GL_FLOAT, GL_FALSE, !shadowMapProgram->attributeIsActive("texcoord")});
    }

    shadowMapProgram->use();

    auto vertexArray = std::make_unique<NUGL::VertexArray>();
    vertexArray->bind();
    vertexArray->setAttributePointers(*shadowMapProgram, *vertexBuffer, GL_ARRAY_BUFFER, attribs);

    vertexArrayMap[*shadowMapProgram] = move(vertexArray);
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

    if (material->materialInfo.has.shininessStrength && program->materialInfo.has.shininessStrength) {
        program->setUniform("shininessStrength", material->shininessStrength);
    }

//    if (material->materialInfo.has.reserved_value && program->materialInfo.has.reserved_value) {
//        program->setUniform("reserved_value", material->reserved_value);
//    }

    if (material->materialInfo.has.texEnvironmentMap && program->materialInfo.has.texEnvironmentMap) {
        if (material->texEnvironmentMap != nullptr) {
            material->texEnvironmentMap->bind();
            program->setUniform("texEnvironmentMap", material->texEnvironmentMap);
        } else {
            std::cerr << "WARNING: material->materialInfo.has.texEnvironmentMap was true, but texEnvironmentMap was null.";
        }
    }

    if (material->materialInfo.has.texDiffuse && program->materialInfo.has.texDiffuse) {
        material->texDiffuse->bind();
        program->setUniform("texDiffuse", material->texDiffuse);
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
//    if (material->materialInfo.has.texHeight && program->materialInfo.has.texHeight) {
//        material->texHeight->bind();
//        program->setUniform("texHeight", material->texHeight);
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

}
