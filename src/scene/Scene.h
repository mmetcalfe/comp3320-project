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

        void render();

        std::vector<std::shared_ptr<Model>> models;
        std::unique_ptr<Camera> camera;
    };

}
