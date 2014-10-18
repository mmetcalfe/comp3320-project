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

        prepareFramebuffer(windowSize);
        prepareShadowMapFramebuffer(shadowMapSize);
        prepareReflectionFramebuffer(reflectionMapSize);
        prepareGBuffer(windowSize);

        screen = std::make_unique<utility::PostprocessingScreen>(screenProgram);
    }

    void Scene::prepareFramebuffer(glm::ivec2 size) {
        auto tex = NUGL::Texture::createTexture(GL_TEXTURE0, GL_TEXTURE_2D, size.x, size.y, GL_RGB);

        auto rbo  = std::make_unique<NUGL::Renderbuffer>();
        rbo->setStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y);

        framebuffer = std::make_unique<NUGL::Framebuffer>();
        framebuffer->attach(std::move(tex));
        framebuffer->attach(std::move(rbo));
    }

    void Scene::prepareGBuffer(glm::ivec2 size) {
        /*
         * G-Buffer format:
         * |--------+--------+--------+--------|--------|
         * |   R8   |   G8   |   B8   |   A8   | attach |
         * |--------+--------+--------+--------|--------|
         * |          depth           | stencil|   DS   |
         * |--------+--------+--------+--------|--------|
         * | normal x (FP16) | normal y (FP16) |   RT0  | outNormal
         * |--------+--------+--------+--------|--------|
         * |      diffuse albedo      | rough  |   RT1  | outAlbedoRoughness
         * |--------+--------+--------+--------|--------|
         * |      env map col         | spec in|   RT2  | outEnvMapColSpecIntensity
         * |--------+--------+--------+--------|--------|
         * |                          |        |   RT3  |
         * |--------+--------+--------+--------|--------|
         */

        GLenum unit = GL_TEXTURE8;
        auto depthTex = NUGL::Texture::createTexture(unit++, GL_TEXTURE_2D, size.x, size.y, GL_DEPTH24_STENCIL8, GL_DEPTH_COMPONENT);
        auto normalsTex = NUGL::Texture::createTexture(unit++, GL_TEXTURE_2D, size.x, size.y, GL_RG16F);
        auto albedoTex = NUGL::Texture::createTexture(unit++, GL_TEXTURE_2D, size.x, size.y, GL_RGBA8);
        auto envMapColTex = NUGL::Texture::createTexture(unit++, GL_TEXTURE_2D, size.x, size.y, GL_RGBA8);

        checkForAndPrintGLError(__FILE__, __LINE__);

        GLenum attach = GL_COLOR_ATTACHMENT0;
        auto gBuffer = std::make_unique<NUGL::Framebuffer>();
        gBuffer->attach(std::move(depthTex), GL_TEXTURE_2D, GL_DEPTH_STENCIL_ATTACHMENT);
        gBuffer->attach(std::move(normalsTex), GL_TEXTURE_2D, attach++);
        gBuffer->attach(std::move(albedoTex), GL_TEXTURE_2D, attach++);
        gBuffer->attach(std::move(envMapColTex), GL_TEXTURE_2D, attach++);

        checkForAndPrintGLError(__FILE__, __LINE__);


        this->gBuffer = std::move(gBuffer);

        std::cerr << __FILE__ << ", " << __LINE__ << ": "
        <<  getFramebufferStatusString(glCheckFramebufferStatus(GL_FRAMEBUFFER))
        << std::endl;

        NUGL::Framebuffer::useDefault();
    }

    void Scene::prepareShadowMapFramebuffer(int size) {
        auto tex = std::make_unique<NUGL::Texture>(GL_TEXTURE1, GL_TEXTURE_2D);
        tex->setTextureData(GL_TEXTURE_2D, size, size, nullptr, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT);
        checkForAndPrintGLError(__FILE__, __LINE__);
        tex->setParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        tex->setParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        tex->setParam(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        tex->setParam(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        shadowMapFramebuffer = std::make_unique<NUGL::Framebuffer>();
        shadowMapFramebuffer->attach(std::move(tex), GL_TEXTURE_2D, GL_DEPTH_ATTACHMENT);

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
        tex->setDefaultParams();

        checkForAndPrintGLError(__func__, __LINE__);

        return tex;
    }

    void Scene::prepareReflectionFramebuffer(int size) {
        auto rbo  = std::make_unique<NUGL::Renderbuffer>();
        rbo->setStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size, size);

        reflectionFramebuffer = std::make_unique<NUGL::Framebuffer>();
        reflectionFramebuffer->attach(std::move(rbo));

//        auto depthTex = std::make_unique<NUGL::Texture>(GL_TEXTURE15, GL_TEXTURE_2D);
//        depthTex->setTextureData(GL_TEXTURE_2D, windowSize.x, windowSize.y, nullptr, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT);
//        depthTex->setDefaultParams();
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

            reflectionFramebuffer->bind();
            glViewport(0, 0, mapCamera.frameWidth, mapCamera.frameHeight);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            for (auto drawModel : models) {
                if (model == drawModel)
                    continue;

                drawModel->draw(mapCamera, sharedLight, nullptr);
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
        glEnable(GL_DEPTH_TEST);

        if (useDeferredRendering)
            deferredRender();
        else
            forwardRender();

        profiler.printEvery(1);
    }


    void Scene::deferredRender() {
        // Requires:
        //  + A g-buffer class to store framebuffers for each attribute;
        //  + Code to bind each framebuffer to a separate fragdatalocation;
        //  + A g-buffer shader program that populates the g-buffer when rendering geometry;
        //  + A VAO for each mesh to support the g-buffer shader program;
        //  - Deferred shading programs that take input from the g-buffer instead of the vertex shader, etc.;

        // Render all geometry to g-buffer:
        gBuffer->bind();
        // TODO: Refactor enabling of framebuffer colour attachments.
        GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(3,  attachments);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, camera->frameWidth, camera->frameHeight);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        drawModels(gBufferProgram);
        glDisable(GL_CULL_FACE);

        profiler.split("render g-buffer");

        // Attach g-buffer uniforms to the deferred shaders:
        gBuffer->bindTextures();
        deferredShadingProgram->use();
        deferredShadingProgram->setUniform("texDepthStencil", gBuffer->textureAttachments[GL_DEPTH_STENCIL_ATTACHMENT]);
        deferredShadingProgram->setUniform("texNormal", gBuffer->textureAttachments[GL_COLOR_ATTACHMENT0]);
        deferredShadingProgram->setUniform("texAlbedoRoughness", gBuffer->textureAttachments[GL_COLOR_ATTACHMENT1]);
        deferredShadingProgram->setUniform("texEnvMapColSpecIntensity", gBuffer->textureAttachments[GL_COLOR_ATTACHMENT2]);
        glm::mat4 projInverse = glm::inverse(camera->proj);
        deferredShadingProgram->setUniform("projInverse", projInverse);
        glm::mat4 viewInverse = glm::inverse(camera->view);
        deferredShadingProgram->setUniform("viewInverse", viewInverse);
        deferredShadingProgram->setUniform("view", camera->view);

        // Clear the framebuffer:
        framebuffer->bind();
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_CULL_FACE);


        // Render the environment map:
        framebuffer->bind();
        glViewport(0, 0, camera->frameWidth, camera->frameHeight);
        screen->setTexture(gBuffer->textureAttachments[GL_COLOR_ATTACHMENT2]);
        screen->render();
        screen->removeTexture();

        // Run the deferred shader over the framebuffer for each light:
        int lightNum = 1;
        for (auto light : lights) {
            auto sharedLight = light.lock();

            auto lightCamera = prepareShadowMap(lightNum, sharedLight);

            framebuffer->bind();
            glViewport(0, 0, camera->frameWidth, camera->frameHeight);
//            glClear(GL_COLOR_BUFFER_BIT);

            gBuffer->bindTextures();
            screen->setTexture(framebuffer->textureAttachments[GL_COLOR_ATTACHMENT0]);

            Model::setLightUniformsOnShaderProgram(deferredShadingProgram, sharedLight, lightCamera);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            screen->render(deferredShadingProgram);
            glDisable(GL_BLEND);

            profiler.split("deferred light ", lightNum);

//            // Add the light's contribution to the screen:
//            addFramebufferToScreen();

            // Render a tiny shadow map:
            if (sharedLight->type == scene::Light::Type::spot)
                drawShadowMapThumbnail(lightNum - 1);

            profiler.split("drawShadowMapThumbnail ", lightNum);
            lightNum++;
        }


        // Draw skybox and transparent meshes:
        // (use the depth buffer from the g-buffer)
        framebuffer->bind();
        glViewport(0, 0, camera->frameWidth, camera->frameHeight);
        framebuffer->attach(gBuffer->textureAttachments[GL_DEPTH_STENCIL_ATTACHMENT], GL_TEXTURE_2D, GL_DEPTH_STENCIL_ATTACHMENT);

        // Draw skybox first:
        skyBox->draw(*camera, skyBox->environmentMapProgram);

        // Draw all transparent meshes:
        lightNum = 1;
        for (auto light : lights) {
            auto sharedLight = light.lock();

            auto lightCamera = prepareShadowMap(lightNum, sharedLight);

            framebuffer->bind();
            glViewport(0, 0, camera->frameWidth, camera->frameHeight);

            glEnable(GL_BLEND);
//            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            checkForAndPrintGLError(__FILE__, __LINE__);

            drawModels(sharedLight, lightCamera, true);
            glDisable(GL_BLEND);

            profiler.split("transparent: light ", lightNum);
            lightNum++;
        }

        // Restore the framebuffer's depth attachment:
        framebuffer->attach(std::move(framebuffer->renderbufferAttachment));


        profiler.split("deferred lighting");


        addFramebufferToScreen();


        // Render g-buffer thumbnails:
        drawGBufferThumbnails();
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
            drawShadowMapThumbnail(lightNum - 1);

            profiler.split("light ", lightNum);
            lightNum++;
        }
    }

    void Scene::drawGBufferThumbnails() {
        NUGL::Framebuffer::useDefault();
        glViewport(0, 0, framebufferSize.x, framebufferSize.y);

        int texNum = 0;
        for (auto& pair : gBuffer->textureAttachments) {
            auto tex = pair.second;

            screen->setTexture(tex);
            screen->render(4, texNum, 0);
            screen->removeTexture();

            texNum++;
        }

        profiler.split("g-buffer thumbnails");
    }

    void Scene::drawShadowMapThumbnail(int lightNum) {
        NUGL::Framebuffer::useDefault();
        glViewport(0, 0, framebufferSize.x, framebufferSize.y);

        screen->setTexture(shadowMapFramebuffer->textureAttachments[GL_DEPTH_ATTACHMENT]);
        screen->render(4, lightNum, 3);
        screen->removeTexture();
    }

    void Scene::addFramebufferToScreen() {
        NUGL::Framebuffer::useDefault();
        glViewport(0, 0, framebufferSize.x, framebufferSize.y);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        screen->setTexture(framebuffer->textureAttachments[GL_COLOR_ATTACHMENT0]);
        if (useDeferredRendering)
            screen->render(2, 0.5, 0.5);
        else
            screen->render();
//        screen->render(4, 3, 1);
        screen->removeTexture();

        glDisable(GL_BLEND);
    }

    std::shared_ptr<LightCamera> Scene::prepareShadowMap(int lightNum, std::shared_ptr<Light> sharedLight) {
        std::shared_ptr<LightCamera> lightCamera;

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
//                    model->shadowMapProgram = shadowMapProgram;
                model->draw(*lightCamera, shadowMapProgram);
            }

            glDisable(GL_CULL_FACE);

            lightCamera->shadowMap = shadowMapFramebuffer->textureAttachments[GL_DEPTH_ATTACHMENT];

            profiler.split("shadow map ", lightNum);
        }

        return lightCamera;
    }

    void Scene::drawModels(std::shared_ptr<Light> sharedLight, std::shared_ptr<LightCamera> lightCamera, bool transparentOnly) {
        for (auto model : models) {
            model->draw(*camera, sharedLight, lightCamera, transparentOnly);
        }
    }

    void Scene::drawModels(std::shared_ptr<NUGL::ShaderProgram> program) {
        for (auto model : models) {
            model->draw(*camera, program);
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
