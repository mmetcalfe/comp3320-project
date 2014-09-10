#include "scene/Scene.h"
#include "NUGL/Framebuffer.h"
#include "NUGL/Renderbuffer.h"
#include "utility/PostprocessingScreen.h"

namespace scene {

    Scene::Scene(std::shared_ptr<NUGL::ShaderProgram> screenProgram, int width, int height) : camera(std::make_unique<Camera>()) {
        prepareFramebuffer(width, height);

        screen = std::make_unique<utility::PostprocessingScreen>(screenProgram);
    }

    void Scene::prepareFramebuffer(int width, int height) {
        auto tex = std::make_unique<NUGL::Texture>(GL_TEXTURE0, GL_TEXTURE_2D);
        tex->setTextureData(GL_TEXTURE_2D, width, height, nullptr);
        checkForAndPrintGLError(__FILE__, __LINE__);
        tex->setParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        tex->setParam(GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        auto rbo  = std::make_unique<NUGL::Renderbuffer>();
        rbo->setStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

        framebuffer = std::make_unique<NUGL::Framebuffer>();
        framebuffer->attach(std::move(tex));
        framebuffer->attach(std::move(rbo));
    }

    void Scene::render() {
        glClearColor(0, 0, 0, 1.0);

        // Clear the screen:
        NUGL::Framebuffer::useDefault();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (auto light : lights) {
            auto sharedLight = light.lock();

            std::shared_ptr<NUGL::Texture> shadowMap;
            if (sharedLight->type == Light::Type::point) {
                // Render light's perspective into shadowMap.
            }


            // Render the light's contribution to the framebuffer:
            framebuffer->bind();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


            for (auto model : models) {
                model->draw(*camera, sharedLight);
            }

            // Add the light's contribution to the screen:
            NUGL::Framebuffer::useDefault();
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            framebuffer->textureAttachment->bind();
            screen->render();
            glDisable(GL_BLEND);
        }
    }

    void Scene::addModel(std::shared_ptr<Model> model) {
        models.push_back(model);

        for (auto light : model->lights) {
            std::weak_ptr<Light> weak(light);
            lights.push_back(weak);
        }
    }
}
