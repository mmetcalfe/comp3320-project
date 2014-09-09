#pragma once

#include <memory>
#include <vector>
#include "scene/Model.h"
#include "scene/Camera.h"
#include "utility/make_unique.h"
#include "NUGL/Framebuffer.h"

namespace scene {

    class Scene {
    public:
        Scene();

        void render(std::shared_ptr<NUGL::ShaderProgram> anIf);
        void addModel(std::shared_ptr<Model>);

        void prepareFramebuffer();

        std::vector<std::shared_ptr<Model>> models;
        /**
        * Weak pointers to all lights attached to all models in the scene.
        */
        std::vector<std::weak_ptr<Light>> lights;
        std::unique_ptr<Camera> camera;
        std::unique_ptr<NUGL::Framebuffer> framebuffer;
    };

}
