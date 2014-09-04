#include "scene/Scene.h"

namespace scene {

    void Scene::render() {
        for (auto model : models) {
            model->draw(*camera);
        }
    }

}
