#pragma once
#include "scene/Model.h"
#include <memory>

namespace scene {
    std::shared_ptr<Model> createAsteroid(float baseNoise, float subDivisionNoise, int subdivisions);
}
