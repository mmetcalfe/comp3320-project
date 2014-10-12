#pragma once

#include <memory>
#include <glm/glm.hpp>

#include "NUGL/Texture.h"
#include "NUGL/ShaderProgram.h"

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
}
