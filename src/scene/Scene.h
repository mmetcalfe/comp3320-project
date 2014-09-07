#pragma once

#include <memory>
#include <vector>
#include "scene/Model.h"
#include "scene/Camera.h"
#include "utility/make_unique.h"

namespace scene {

    class Scene {
    public:
        Scene() : camera(std::make_unique<Camera>()) { }

        void render(std::shared_ptr<NUGL::ShaderProgram> anIf);
        void addModel(std::shared_ptr<Model>);

        std::vector<std::shared_ptr<Model>> models;
        /**
        * Weak pointers to all lights attached to all models in the scene.
        */
        std::vector<std::weak_ptr<Light>> lights;
        std::unique_ptr<Camera> camera;
    };

}
