#include "scene/Scene.h"
#include "NUGL/Framebuffer.h"
#include "NUGL/Renderbuffer.h"
#include "utility/PostprocessingScreen.h"

namespace scene {

    Scene::Scene(std::shared_ptr<NUGL::ShaderProgram> screenProgram, int width, int height) : camera(std::make_unique<PlayerCamera>()) {
        shadowMapSize = 1024;

        prepareFramebuffer(width, height);
        prepareShadowMapFramebuffer(shadowMapSize);

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

    void Scene::prepareShadowMapFramebuffer(int size) {
        auto tex = std::make_unique<NUGL::Texture>(GL_TEXTURE1, GL_TEXTURE_2D);
        tex->setTextureData(GL_TEXTURE_2D, size, size, nullptr);
        checkForAndPrintGLError(__FILE__, __LINE__);
        tex->setParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        tex->setParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        tex->setParam(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        tex->setParam(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        auto rbo  = std::make_unique<NUGL::Renderbuffer>();
        rbo->setStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size, size);

        shadowMapFramebuffer = std::make_unique<NUGL::Framebuffer>();
        shadowMapFramebuffer->attach(std::move(tex));
        shadowMapFramebuffer->attach(std::move(rbo));
//        auto tex = std::make_unique<NUGL::Texture>(GL_TEXTURE0, GL_TEXTURE_2D);
//        tex->setTextureData(GL_TEXTURE_2D, size, size, nullptr, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT);
//        checkForAndPrintGLError(__FILE__, __LINE__);
//        tex->setParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//        tex->setParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//
//        shadowMapFramebuffer = std::make_unique<NUGL::Framebuffer>();
//        shadowMapFramebuffer->attach(std::move(tex), GL_DEPTH_ATTACHMENT);
//
//        shadowMapFramebuffer->bind(GL_FRAMEBUFFER);
//        glDrawBuffer(GL_NONE);
//        glReadBuffer(GL_NONE);
//
////        std::cerr << __FILE__ << ", " << __LINE__ << ": "
////                <<  getFramebufferStatusString(glCheckFramebufferStatus(GL_FRAMEBUFFER))
////                << std::endl;
//
//        NUGL::Framebuffer::useDefault();
    }

    void Scene::render() {
        glClearColor(0, 0, 0, 1.0);

        // Clear the screen:
        NUGL::Framebuffer::useDefault();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (auto light : lights) {
            auto sharedLight = light.lock();

            std::shared_ptr<LightCamera> lightCamera = LightCamera::fromLight(*sharedLight, shadowMapSize);
            if (sharedLight->type == Light::Type::spot) {
                // Render light's perspective into shadowMap.
                shadowMapFramebuffer->bind();
                glViewport(0, 0, lightCamera->frameWidth, lightCamera->frameHeight);
//                NUGL::Framebuffer::useDefault();

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                for (auto model : models) {
                    model->shadowMapProgram = shadowMapProgram;
                    model->drawDepth(*lightCamera);
                }

                lightCamera->shadowMap = shadowMapFramebuffer->textureAttachment;
//                return;
            }

            // Render the light's contribution to the framebuffer:
            framebuffer->bind();
            glViewport(0, 0, camera->frameWidth, camera->frameHeight);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            for (auto model : models) {
                model->draw(*camera, sharedLight, lightCamera);
            }

            // Add the light's contribution to the screen:
            NUGL::Framebuffer::useDefault();
            glViewport(0, 0, camera->frameWidth, camera->frameHeight);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            framebuffer->textureAttachment->bind();
            screen->screenProgram->use();
            screen->screenProgram->setUniform("texDiffuse", framebuffer->textureAttachment);
            screen->screenProgram->setUniform("model", glm::mat4());
            screen->render();
            glDisable(GL_BLEND);

            // Render a tiny shadow map:
            screen->screenProgram->use();
            screen->screenProgram->setUniform("texDiffuse", shadowMapFramebuffer->textureAttachment);
            glm::mat4 previewModel;
            previewModel = glm::scale(previewModel, glm::vec3(0.2f, 0.2, 1.0f));
            previewModel = glm::translate(previewModel, glm::vec3(-4.0f, -4.0f, 0.0f));
            screen->screenProgram->setUniform("model", previewModel);
            screen->render();
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
