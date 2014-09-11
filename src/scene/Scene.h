#pragma once

#include <memory>
#include <vector>
#include "scene/Model.h"
#include "scene/Camera.h"
#include "scene/Light.h"
#include "utility/make_unique.h"
#include "utility/PostprocessingScreen.h"
#include "NUGL/Framebuffer.h"

namespace scene {

    class Scene {
    public:
        Scene(std::shared_ptr<NUGL::ShaderProgram> screenProgram, int width, int height);

        void render();
        void addModel(std::shared_ptr<Model>);

        void prepareFramebuffer(int width, int height);
        void prepareShadowMapFramebuffer(int size);

        std::vector<std::shared_ptr<Model>> models;
        /**
        * Weak pointers to all lights attached to all models in the scene.
        */
        std::vector<std::weak_ptr<Light>> lights;
        std::unique_ptr<PlayerCamera> camera;
        std::unique_ptr<NUGL::Framebuffer> framebuffer;
        std::unique_ptr<NUGL::Framebuffer> shadowMapFramebuffer;
        std::unique_ptr<utility::PostprocessingScreen> screen;
        std::shared_ptr<NUGL::ShaderProgram> shadowMapProgram;
    };

}
