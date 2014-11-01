#include "scene/Scene.h"
#include <tuple>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "NUGL/Framebuffer.h"
#include "NUGL/Renderbuffer.h"
#include "utility/PostprocessingScreen.h"

namespace scene {

    Scene::Scene(std::shared_ptr<NUGL::ShaderProgram> screenProgram,
            std::shared_ptr<NUGL::ShaderProgram> screenAlphaProgram,
            glm::ivec2 windowSize, glm::ivec2 framebufferSize)
            : camera(std::make_unique<PlayerCamera>()) {
        shadowMapSize = 2048;
        reflectionMapSize = 512;

        this->windowSize = windowSize;
        this->framebufferSize = framebufferSize;

        prepareFramebuffer(windowSize);
        prepareShadowMapFramebuffer(shadowMapSize);
        prepareReflectionFramebuffer(reflectionMapSize);
        prepareGBuffer(windowSize);

        screen = std::make_unique<utility::PostprocessingScreen>(screenProgram, screenAlphaProgram);
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
        std::shared_ptr<Camera> mapCamera = std::make_shared<Camera>();
        // TODO: Add actual renderReflectionMap origin offset.
        glm::vec4 pos = model->transform * glm::vec4(0, 0, 1.2, 1);
        mapCamera->pos = {pos.x, pos.y, pos.z};
        mapCamera->dir = {0, -1, 0};
        mapCamera->up = {0, 0, -1};
        mapCamera->fov = M_PI_2;
        mapCamera->frameWidth = reflectionMapSize;
        mapCamera->frameHeight = reflectionMapSize;
        mapCamera->near_ = 1;
        mapCamera->prepareTransforms();

        auto sharedLight = std::make_shared<scene::Light>();
        sharedLight->type = scene::Light::Type::point;
        sharedLight->colDiffuse = {0, 0, 0};
        sharedLight->colSpecular = {0, 0, 0};
        sharedLight->colAmbient = {1, 1, 1};

        std::vector<std::tuple<GLenum, glm::vec3, glm::vec3>> directions = {
//                std::make_tuple<GLenum, glm::vec3, glm::vec3>(GL_TEXTURE_CUBE_MAP_POSITIVE_X, {0, -1, 0}, {0, 0, -1}),
//                std::make_tuple<GLenum, glm::vec3, glm::vec3>(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, {0,  1, 0}, {0, 0, -1}),
//                std::make_tuple<GLenum, glm::vec3, glm::vec3>(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, {0, 0,  1}, {-1, 0, 0}),
//                std::make_tuple<GLenum, glm::vec3, glm::vec3>(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, {0, 0, -1}, {1, 0, 0}),
//                std::make_tuple<GLenum, glm::vec3, glm::vec3>(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, {-1, 0, 0}, {0, 0, -1}),
//                std::make_tuple<GLenum, glm::vec3, glm::vec3>(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, { 1, 0, 0}, {0, 0, -1}),
                std::make_tuple<GLenum, glm::vec3, glm::vec3>(GL_TEXTURE_CUBE_MAP_POSITIVE_X, { 1,  0,  0}, { 0,  0, -1}),
                std::make_tuple<GLenum, glm::vec3, glm::vec3>(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, {-1,  0,  0}, { 0,  0, -1}),
                std::make_tuple<GLenum, glm::vec3, glm::vec3>(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, { 0,  0,  1}, { 0, -1,  0}),
                std::make_tuple<GLenum, glm::vec3, glm::vec3>(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, { 0,  0, -1}, { 0,  1,  0}),
                std::make_tuple<GLenum, glm::vec3, glm::vec3>(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, { 0, -1,  0}, { 0,  0, -1}),
                std::make_tuple<GLenum, glm::vec3, glm::vec3>(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, { 0,  1,  0}, { 0,  0, -1}),
        };

//        for (int i = 0; i < directions.size(); i++) {
//            auto params = directions[i];

            // Render one face each frame:
            static int i = 0;
            i = (i + 1) % directions.size();
            auto params = directions[i];

            GLenum target;
            std::tie(target, mapCamera->dir, mapCamera->up) = params;

            mapCamera->prepareTransforms();
            reflectionFramebuffer->attach(model->texEnvironmentMap, target);

            reflectionFramebuffer->bind();
            glViewport(0, 0, mapCamera->frameWidth, mapCamera->frameHeight);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (forwardRenderReflections) {
                model->hidden = true;
                profiler.push("forwardRenderReflections");
                forwardRender(reflectionFramebuffer, {mapCamera->frameWidth, mapCamera->frameHeight}, mapCamera);
                profiler.pop();
                model->hidden = false;
            } else {
                for (auto drawModel : models) {
                    if (model == drawModel)
                        continue;

                    drawModel->draw(*mapCamera, sharedLight, nullptr);
                }
            }
//        }
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
            forwardRender(nullptr, framebufferSize, camera);

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
        drawModels(gBufferProgram, *camera);
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

            if (sharedLight->enabled) {
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
//            addFramebufferToTarget();

                // Render a tiny shadow map:
                if (!previewOptions.disable && (sharedLight->type == scene::Light::Type::spot ||
                        sharedLight->type == scene::Light::Type::directional)) {
                    drawShadowMapThumbnail(lightNum - 1);
                }

                profiler.split("drawShadowMapThumbnail ", lightNum);
            }

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
        // Disable depth buffer writes (for order invariant drawing):
        glDepthMask(GL_FALSE);
        lightNum = 1;
        for (auto light : lights) {
            auto sharedLight = light.lock();

            if (sharedLight->enabled) {
                auto lightCamera = prepareShadowMap(lightNum, sharedLight);

                framebuffer->bind();
                glViewport(0, 0, camera->frameWidth, camera->frameHeight);

                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
//            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                checkForAndPrintGLError(__FILE__, __LINE__);

                drawModels(sharedLight, lightCamera, true, *camera);
                glDisable(GL_BLEND);

                profiler.split("transparent: light ", lightNum);
            }

            lightNum++;
        }

        // Enable depth buffer writes:
        glDepthMask(GL_TRUE);

        // Restore the framebuffer's depth attachment:
        framebuffer->attach(std::move(framebuffer->renderbufferAttachment));


        profiler.split("deferred lighting");

        if (previewOptions.disable) {
            addFramebufferToTarget(framebufferSize, nullptr);
        } else {
            if (!previewOptions.fullscreen)
                addFramebufferToTarget(framebufferSize, nullptr, 2, 0.5, 0.5);
        }

        // Render g-buffer thumbnails:
        if (!previewOptions.disable)
            drawGBufferThumbnails();
    }

    void Scene::forwardRender(std::shared_ptr<NUGL::Framebuffer> target, glm::ivec2 targetSize, std::shared_ptr<Camera> camera) {
        int lightNum = 1;
        for (auto light : lights) {
            profiler.push("light ", lightNum);

            auto sharedLight = light.lock();

            if (sharedLight->enabled) {
                auto lightCamera = prepareShadowMap(lightNum, sharedLight);

                // Render the light's contribution to the framebuffer:
                framebuffer->bind();
                glViewport(0, 0, camera->frameWidth, camera->frameHeight);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                drawModels(sharedLight, lightCamera, false, *camera);

                profiler.split("drawModels");

                // Add the light's contribution to the screen:
                addFramebufferToTarget(targetSize, target);

                // Render a tiny shadow map:
                if (!previewOptions.disable)
                    drawShadowMapThumbnail(lightNum - 1);

                profiler.split("addFramebufferToTarget");
            }

            lightNum++;
            profiler.pop();
        }
    }

    void Scene::drawGBufferThumbnails() {
        NUGL::Framebuffer::useDefault();
        glViewport(0, 0, framebufferSize.x, framebufferSize.y);

        int texNum = 0;
        for (auto& pair : gBuffer->textureAttachments) {
            auto tex = pair.second;

            screen->setTexture(tex);
            if (previewOptions.fullscreen) {
                if (!previewOptions.shadowMap && previewOptions.index == texNum)
                    screen->render();
            } else {
                screen->render(4, texNum, 0);
            }
            screen->removeTexture();

            texNum++;
        }

        profiler.split("g-buffer thumbnails");
    }

    void Scene::drawShadowMapThumbnail(int lightNum) {
        NUGL::Framebuffer::useDefault();
        glViewport(0, 0, framebufferSize.x, framebufferSize.y);

        screen->setTexture(shadowMapFramebuffer->textureAttachments[GL_DEPTH_ATTACHMENT]);

        if (previewOptions.fullscreen && previewOptions.shadowMap) {
            if (previewOptions.index == lightNum)
                screen->render();
        } else {
            int gridSize = std::max(int(lights.size()), 4);
            screen->render(gridSize, lightNum, gridSize - 1);
        }

        screen->removeTexture();
    }

    void Scene::addFramebufferToTarget(glm::ivec2 targetSize, std::shared_ptr<NUGL::Framebuffer> target, float gridDim, float gridX, float gridY) {
        if (target == nullptr) {
            NUGL::Framebuffer::useDefault();
        } else {
            target->bind();
        }

//        glViewport(0, 0, targetSize.x, targetSize.y);
        glViewport(0, 0, framebufferSize.x, framebufferSize.y);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        screen->setTexture(framebuffer->textureAttachments[GL_COLOR_ATTACHMENT0]);

        screen->render(gridDim, gridX, gridY);

        screen->removeTexture();
        glDisable(GL_BLEND);
    }

    std::shared_ptr<LightCamera> Scene::prepareShadowMap(int lightNum, std::shared_ptr<Light> sharedLight) {
        std::shared_ptr<LightCamera> lightCamera;

        if (sharedLight->type == Light::Type::spot || sharedLight->type == scene::Light::Type::directional) {
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
                model->draw(*lightCamera, shadowMapProgram);
            }

            glDisable(GL_CULL_FACE);

            lightCamera->shadowMap = shadowMapFramebuffer->textureAttachments[GL_DEPTH_ATTACHMENT];

            profiler.split("shadow map ", lightNum);
        }

        return lightCamera;
    }

    void Scene::drawModels(std::shared_ptr<Light> sharedLight, std::shared_ptr<LightCamera> lightCamera, bool transparentOnly, Camera &camera) {
        for (auto model : models) {
            if (model->hidden)
                continue;

            model->draw(camera, sharedLight, lightCamera, transparentOnly);
        }
    }

    void Scene::drawModels(std::shared_ptr<NUGL::ShaderProgram> program, Camera &camera) {
        for (auto model : models) {
            if (model->hidden)
                continue;

            model->draw(camera, program);
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
            }

            renderReflectionMap(model);

            profiler.split("reflection map ", refMapNum++);
        }
    }

    void Scene::addModel(std::shared_ptr<Model> model) {
        models.push_back(model);

        for (auto light : model->lights) {
            light->dir = glm::normalize(light->dir);
            std::weak_ptr<Light> weak(light);
            lights.push_back(weak);
        }
    }
}
