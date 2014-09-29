#include "scene/Scene.h"
#include <tuple>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "NUGL/Framebuffer.h"
#include "NUGL/Renderbuffer.h"
#include "utility/PostprocessingScreen.h"

namespace scene {

    Scene::Scene(std::shared_ptr<NUGL::ShaderProgram> screenProgram, glm::ivec2 windowSize, glm::ivec2 framebufferSize) : camera(std::make_unique<PlayerCamera>()) {
        shadowMapSize = 1024;
        reflectionMapSize = 512;

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
//        tex->setTextureData(GL_TEXTURE_2D, size, size, nullptr);
        tex->setTextureData(GL_TEXTURE_2D, size, size, nullptr, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT);
        checkForAndPrintGLError(__FILE__, __LINE__);
        tex->setParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        tex->setParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        tex->setParam(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        tex->setParam(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

//        auto rbo  = std::make_unique<NUGL::Renderbuffer>();
//        rbo->setStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size, size);

        shadowMapFramebuffer = std::make_unique<NUGL::Framebuffer>();
//        shadowMapFramebuffer->attach(std::move(tex));
        shadowMapFramebuffer->attach(std::move(tex), GL_TEXTURE_2D, GL_DEPTH_ATTACHMENT);
//        shadowMapFramebuffer->attach(std::move(rbo));

        shadowMapFramebuffer->bind(GL_FRAMEBUFFER);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

//        std::cerr << __FILE__ << ", " << __LINE__ << ": "
//                <<  getFramebufferStatusString(glCheckFramebufferStatus(GL_FRAMEBUFFER))
//                << std::endl;

        NUGL::Framebuffer::useDefault();
    }

    std::unique_ptr<NUGL::Texture> createCubeMapTexture(int size) {
        auto tex = std::make_unique<NUGL::Texture>(GL_TEXTURE3, GL_TEXTURE_CUBE_MAP);
        for (unsigned i = 0; i < 6; i++) {
            tex->setTextureData(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, size, size, nullptr);
            checkForAndPrintGLError(__func__, __LINE__);
        }
        checkForAndPrintGLError(__FILE__, __LINE__);
        tex->setParam(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        tex->setParam(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        tex->setParam(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        tex->setParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        tex->setParam(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        tex->setParam(GL_TEXTURE_BASE_LEVEL, 0);
        tex->setParam(GL_TEXTURE_MAX_LEVEL, 0);

        checkForAndPrintGLError(__func__, __LINE__);

        return tex;
    }

    void Scene::prepareReflectionFramebuffer(int size) {
//        auto tex = createCubeMapTexture(size);
        auto rbo  = std::make_unique<NUGL::Renderbuffer>();
        rbo->setStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size, size);

        reflectionFramebuffer = std::make_unique<NUGL::Framebuffer>();
//        reflectionFramebuffer->attach(std::move(tex), GL_TEXTURE_CUBE_MAP_POSITIVE_X);
        reflectionFramebuffer->attach(std::move(rbo));

//        std::cerr << __FILE__ << ", " << __LINE__ << ": "
//                <<  getFramebufferStatusString(glCheckFramebufferStatus(GL_FRAMEBUFFER))
//                << std::endl;

        NUGL::Framebuffer::useDefault();
    }

    void Scene::renderReflectionMap(std::shared_ptr<Model> model) {
        Camera mapCamera;
        glm::vec4 pos = model->transform * glm::vec4(0, 0, 0, 1);
        mapCamera.pos = {pos.x, pos.y, pos.z};
        mapCamera.dir = {0, -1, 0};
        mapCamera.up = {0, 0, -1};
        mapCamera.fov = M_PI_2;
        mapCamera.frameWidth = reflectionMapSize;
        mapCamera.frameHeight = reflectionMapSize;
        mapCamera.prepareTransforms();

        auto sharedLight = std::make_shared<scene::Light>();
        sharedLight->type = scene::Light::Type::point;
        sharedLight->colDiffuse = {0, 0, 0};
        sharedLight->colSpecular = {0, 0, 0};
        sharedLight->colAmbient = {1, 1, 1};

        std::vector<std::tuple<GLenum, glm::vec3, glm::vec3>> directions = {
                std::make_tuple<GLenum, glm::vec3, glm::vec3>(GL_TEXTURE_CUBE_MAP_POSITIVE_X, {0, -1, 0}, {0, 0, -1}),
                std::make_tuple<GLenum, glm::vec3, glm::vec3>(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, {0,  1, 0}, {0, 0, -1}),
                std::make_tuple<GLenum, glm::vec3, glm::vec3>(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, {0, 0,  1}, {-1, 0, 0}),
                std::make_tuple<GLenum, glm::vec3, glm::vec3>(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, {0, 0, -1}, {1, 0, 0}),
                std::make_tuple<GLenum, glm::vec3, glm::vec3>(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, {-1, 0, 0}, {0, 0, -1}),
                std::make_tuple<GLenum, glm::vec3, glm::vec3>(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, { 1, 0, 0}, {0, 0, -1}),
        };

        for (auto params : directions) {
            GLenum target;
            std::tie(target, mapCamera.dir, mapCamera.up) = params;
            mapCamera.prepareTransforms();
            reflectionFramebuffer->attach(model->texEnvironmentMap, target);
//            reflectionFramebuffer->attach(reflectionFramebuffer->textureAttachment, target);

//        model->meshes[0].material->texEnvironmentMap->bind();
//        model->meshes[0].material->texEnvironmentMap->setTextureData(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 128, 128, nullptr);

//        reflectionFramebuffer->bind(GL_FRAMEBUFFER);
//        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X,
//                model->meshes[0].material->texEnvironmentMap->id(), 0);

            reflectionFramebuffer->bind();
            glViewport(0, 0, mapCamera.frameWidth, mapCamera.frameHeight);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            for (auto drawModel : models) {
                if (model == drawModel)
                    continue;

                drawModel->shadowMapProgram = shadowMapProgram;
//                auto prog = drawModel->environmentMapProgram;
//                drawModel->environmentMapProgram = nullptr;
//                drawModel->drawDepth(mapCamera);
                drawModel->draw(mapCamera, sharedLight, nullptr);
//                drawModel->environmentMapProgram = prog;
            }
        }
    }

    void Scene::render() {
        glClearColor(0, 0, 0, 1.0);
        profiler.split("other");

        renderDynamicReflectionMaps();

        // Clear the screen:
        NUGL::Framebuffer::useDefault();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//        forwardRender();
        deferredRender();

        profiler.printEvery(1);
    }

    void Scene::deferredRender() {
        // Requires:
        //  - A g-buffer class to store framebuffers for each attribute;
        //  - Code to bind each framebuffer to a separate fragdatalocation;
        //  - A g-buffer shader program that populates the g-buffer when rendering geometry;
        //  - A VAO for each mesh to support the g-buffer shader program;
        //  - Deferred shading programs that take input from the g-buffer instead of the vertex shader, etc.;

        // Render all geometry to g-buffer:
//        for (auto model : models) {
//            model->draw(*camera, sharedLight, lightCamera);
//        }

        // Attach g-buffer uniforms to the deferred shaders:

        // Run the deferred shader over the framebuffer for each light:


        int lightNum = 1;
        for (auto light : lights) {
            auto sharedLight = light.lock();

            auto lightCamera = prepareShadowMap(lightNum, sharedLight);

            // Render the light's contribution to the framebuffer:
            framebuffer->bind();
            glViewport(0, 0, camera->frameWidth, camera->frameHeight);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            drawModels(sharedLight, lightCamera);

            profiler.split("framebuffer ", lightNum);

            // Add the light's contribution to the screen:
            addFramebufferToScreen();

            // Render a tiny shadow map:
            drawShadowMapThumbnail(lightNum);

            profiler.split("light ", lightNum);
            lightNum++;
        }
    }

    void Scene::forwardRender() {
        int lightNum = 1;
        for (auto light : lights) {
            auto sharedLight = light.lock();

            auto lightCamera = prepareShadowMap(lightNum, sharedLight);

            // Render the light's contribution to the framebuffer:
            framebuffer->bind();
            glViewport(0, 0, camera->frameWidth, camera->frameHeight);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            drawModels(sharedLight, lightCamera);

            profiler.split("framebuffer ", lightNum);

            // Add the light's contribution to the screen:
            addFramebufferToScreen();

            // Render a tiny shadow map:
            drawShadowMapThumbnail(lightNum);

            profiler.split("light ", lightNum);
            lightNum++;
        }
    }

    void Scene::drawShadowMapThumbnail(int lightNum) {
        screen->screenProgram->use();
        screen->screenProgram->setUniform("texDiffuse", shadowMapFramebuffer->textureAttachment);
        glm::mat4 previewModel;
        previewModel = glm::scale(previewModel, glm::vec3(0.2f, 0.2, 1.0f));
        previewModel = glm::translate(previewModel, glm::vec3(-4.0f + (lightNum - 1) * 2, -4.0f, 0.0f));
        screen->screenProgram->setUniform("model", previewModel);
        screen->render();
    }

    void Scene::addFramebufferToScreen() {
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
    }

    std::shared_ptr<LightCamera> Scene::prepareShadowMap(int lightNum, std::shared_ptr<Light> sharedLight) {
        std::__1::shared_ptr<LightCamera> lightCamera;
        if (sharedLight->type == Light::Type::spot) {
                lightCamera = LightCamera::fromLight(*sharedLight, shadowMapSize);
                // Render light's perspective into shadowMap.
                shadowMapFramebuffer->bind();
                glViewport(0, 0, lightCamera->frameWidth, lightCamera->frameHeight);

//                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glClear(GL_DEPTH_BUFFER_BIT);

                // Front-face culling:
                glEnable(GL_CULL_FACE);
                glCullFace(GL_FRONT);

                for (auto model : models) {
                    model->shadowMapProgram = shadowMapProgram;
                    model->drawDepth(*lightCamera);
                }

                glDisable(GL_CULL_FACE);

                lightCamera->shadowMap = shadowMapFramebuffer->textureAttachment;

                profiler.split("shadow map ", lightNum);
            }
        return lightCamera;
    }

    void Scene::drawModels(std::shared_ptr<Light> sharedLight, std::shared_ptr<LightCamera> lightCamera) {
        for (auto model : models) {
                model->draw(*camera, sharedLight, lightCamera);
            }
    }

    void Scene::renderDynamicReflectionMaps() {
        int refMapNum = 1;
        for (auto model : models) {
            if (!model->dynamicReflections)
                continue;

            if (model->texEnvironmentMap == nullptr) {
                auto tex = createCubeMapTexture(reflectionMapSize);
                model->setEnvironmentMap(move(tex));
                profiler.split("create reflection map ", refMapNum);
            }

            renderReflectionMap(model);
            profiler.split("reflection map ", refMapNum++);
//            model->setEnvironmentMap(reflectionFramebuffer->textureAttachment);
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
