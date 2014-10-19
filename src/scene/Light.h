#pragma once
#include <glm/glm.hpp>

namespace scene {

    struct Light {
        enum class Type {
            undefined,
            directional,
            point,
            spot,
        };

        Type type = Type::undefined;
        glm::vec3 pos = {0, 0, 0};
        glm::vec3 dir = {1, 0, 0};
        float attenuationConstant = 1;
        float attenuationLinear = 0;
        float attenuationQuadratic = 0;
        glm::vec3 colDiffuse = {1, 1, 1};
        glm::vec3 colSpecular = {1, 1, 1};
        glm::vec3 colAmbient = {0, 0, 0};
        float angleConeInner = 0.8;
        float angleConeOuter = 2;
    };

}
