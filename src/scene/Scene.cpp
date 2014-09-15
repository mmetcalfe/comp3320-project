#include "scene/Scene.h"
#include "NUGL/Framebuffer.h"
#include "NUGL/Renderbuffer.h"
#include "utility/PostprocessingScreen.h"

namespace scene {

    Scene::Scene(std::shared_ptr<NUGL::ShaderProgram> screenProgram, glm::ivec2 windowSize, glm::ivec2 framebufferSize) : camera(std::make_unique<PlayerCamera>()) {
        shadowMapSize = 1024;
        reflectionMapSize = 128;

        this->windowSize = windowSize;
        this->framebufferSize = framebufferSize;

        prepareFramebuffer(windowSize.x, windowSize.y);
        prepareShadowMapFramebuffer(shadowMapSize);
        prepareReflectionFramebuffer(reflectionMapSize);

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

    void Scene::prepareReflectionFramebuffer(int size) {
//        auto tex = std::make_unique<NUGL::Texture>(GL_TEXTURE3, GL_TEXTURE_2D);
//        tex->setTextureData(GL_TEXTURE_2D, size, size, nullptr);
//        checkForAndPrintGLError(__FILE__, __LINE__);
//        tex->setParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        tex->setParam(GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        auto rbo  = std::make_unique<NUGL::Renderbuffer>();
        rbo->setStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size, size);

        reflectionFramebuffer = std::make_unique<NUGL::Framebuffer>();
//        reflectionFramebuffer->attach(std::move(tex));
//        reflectionFramebuffer->bind(GL_FRAMEBUFFER);
//        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, tex->id(), 0);
        reflectionFramebuffer->attach(std::move(rbo));
    }

    void Scene::renderReflectionMap(std::shared_ptr<Model> model) {
        if (model->meshes.size() == 0)
            return;
        if (!model->meshes[0].material->materialInfo.has.texEnvironmentMap)
            return;

        Camera mapCamera;
        glm::vec4 pos = model->transform * glm::vec4(0, 0, 0, 1);
        mapCamera.pos = {pos.x, pos.y, pos.z};
        mapCamera.dir = {1, 0, 0};
        mapCamera.up = {0, 0, 1};
        mapCamera.fov = 90;
        mapCamera.frameWidth = reflectionMapSize;
        mapCamera.frameHeight = reflectionMapSize;
        mapCamera.prepareTransforms();

        model->meshes[0].material->texEnvironmentMap->bind();
//        model->meshes[0].material->texEnvironmentMap->setTextureData(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 128, 128, nullptr);

        reflectionFramebuffer->bind(GL_FRAMEBUFFER);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                model->meshes[0].material->texEnvironmentMap->id(), 0);

        reflectionFramebuffer->bind();
        glViewport(0, 0, mapCamera.frameWidth, mapCamera.frameHeight);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (auto model : models) {
            model->shadowMapProgram = shadowMapProgram;
            model->drawDepth(mapCamera);
        }
    }

    void Scene::render() {
        profiler.split("other");

        glClearColor(0, 0, 0, 1.0);

//        // Prepare dynamic reflection maps:
//        for (auto model : models) {
//            renderReflectionMap(model);
//        }

        // Clear the screen:
        NUGL::Framebuffer::useDefault();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int lightNum = 1;
        for (auto light : lights) {
            auto sharedLight = light.lock();

            std::shared_ptr<LightCamera> lightCamera;
            if (sharedLight->type == Light::Type::spot) {
                lightCamera = LightCamera::fromLight(*sharedLight, shadowMapSize);
                // Render light's perspective into shadowMap.
                shadowMapFramebuffer->bind();
                glViewport(0, 0, lightCamera->frameWidth, lightCamera->frameHeight);

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                for (auto model : models) {
                    model->shadowMapProgram = shadowMapProgram;
                    model->drawDepth(*lightCamera);
                }

                lightCamera->shadowMap = shadowMapFramebuffer->textureAttachment;
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
            glViewport(0, 0, framebufferSize.x, framebufferSize.y);
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

            profiler.split("light ", lightNum++);
        }

        profiler.printEvery(1);
    }

    void Scene::addModel(std::shared_ptr<Model> model) {
        models.push_back(model);

        for (auto light : model->lights) {
            std::weak_ptr<Light> weak(light);
            lights.push_back(weak);
        }
    }


}
