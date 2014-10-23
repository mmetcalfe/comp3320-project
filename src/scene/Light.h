#pragma once
#include <memory>
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
        glm::vec3 pos = glm::vec3(0);
        glm::vec3 dir = {1, 0, 0};
        float angleConeInner = 1;
        float angleConeOuter = 2;
        glm::vec3 colDiffuse = glm::vec3(1000);
        glm::vec3 colSpecular = glm::vec3(1000);
        glm::vec3 colAmbient = glm::vec3(0);
        float attenuationConstant = 0;
        float attenuationLinear = 0;
        float attenuationQuadratic = 1;


        static std::shared_ptr<Light> makeSpotlight(
                glm::vec3 pos = glm::vec3(0, 0, 0),
                glm::vec3 dir = glm::vec3(0, 0, -1),
                float angleConeOuter = 2,
                float angleConeInner = 1,
                glm::vec3 colDiffuse = glm::vec3(1000),
                glm::vec3 colSpecular = glm::vec3(1000),
                glm::vec3 colAmbient = glm::vec3(0)) {
            auto light = std::make_shared<Light>();

            light->type = scene::Light::Type::spot;
            light->pos = pos;
            light->dir = dir;
            light->angleConeInner = angleConeInner;
            light->angleConeOuter = angleConeOuter;
            light->colDiffuse = colDiffuse;
            light->colSpecular = colSpecular;
            light->colAmbient = colAmbient;

            return light;
        }
    };

}
