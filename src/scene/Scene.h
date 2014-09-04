#pragma once

#include <memory>
#include <vector>
#include "scene/Model.h"
#include "scene/Camera.h"

namespace scene {

    class Scene {
    public:
        std::vector<std::shared_ptr<Model>> models;

        std::unique_ptr<Camera> camera;
    };

}
