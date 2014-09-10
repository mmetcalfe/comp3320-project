#pragma once

#include <memory>
#include <vector>
#include "scene/Model.h"
#include "scene/Camera.h"
#include "utility/make_unique.h"
#include "utility/PostprocessingScreen.h"
#include "NUGL/Framebuffer.h"

namespace scene {

    class Scene {
    public:
        Scene(std::shared_ptr<NUGL::ShaderProgram> screenProgram);

        void render();
        void addModel(std::shared_ptr<Model>);

        void prepareFramebuffer(int width, int height);

        std::vector<std::shared_ptr<Model>> models;
        /**
        * Weak pointers to all lights attached to all models in the scene.
        */
        std::vector<std::weak_ptr<Light>> lights;
        std::unique_ptr<Camera> camera;
        std::unique_ptr<NUGL::Framebuffer> framebuffer;
        std::unique_ptr<utility::PostprocessingScreen> screen;
    };

}