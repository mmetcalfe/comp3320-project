#pragma once

#include <memory>
#include <vector>
#include "scene/Model.h"
#include "scene/Camera.h"
#include "scene/Light.h"
#include "utility/make_unique.h"
#include "utility/PostprocessingScreen.h"
#include "utility/Profiler.h"
#include "NUGL/Framebuffer.h"

namespace scene {

    class Scene {
    public:
        Scene(std::shared_ptr<NUGL::ShaderProgram> screenProgram, glm::ivec2 windowSize, glm::ivec2 framebufferSize);

        void render();
        void forwardRender();
        void deferredRender();

        void addModel(std::shared_ptr<Model>);

        void prepareFramebuffer(glm::ivec2 windowSize);
        void prepareShadowMapFramebuffer(int size);
        void prepareReflectionFramebuffer(int size);

        std::vector<std::shared_ptr<Model>> models;
        std::shared_ptr<Model> skyBox;

        /**
         * Weak pointers to all lights attached to all models in the scene.
         */
        std::vector<std::weak_ptr<Light>> lights;
        std::unique_ptr<PlayerCamera> camera;
        std::unique_ptr<NUGL::Framebuffer> framebuffer;
        std::unique_ptr<NUGL::Framebuffer> shadowMapFramebuffer;
        std::unique_ptr<NUGL::Framebuffer> reflectionFramebuffer;
        std::unique_ptr<NUGL::Framebuffer> gBuffer;
        std::unique_ptr<utility::PostprocessingScreen> screen;
        std::shared_ptr<NUGL::ShaderProgram> shadowMapProgram;
        std::shared_ptr<NUGL::ShaderProgram> gBufferProgram;
        std::shared_ptr<NUGL::ShaderProgram> deferredShadingProgram;

        Profiler profiler;

        int shadowMapSize = 1024;
        int reflectionMapSize = 128;
        glm::ivec2 windowSize = {800, 600};
        glm::ivec2 framebufferSize = {800, 600};
        bool useDeferredRendering = true;

        void renderReflectionMap(std::shared_ptr<Model> shared_ptr);
        void renderDynamicReflectionMaps();

        void drawModels(std::shared_ptr<Light> sharedLight, std::shared_ptr<LightCamera> lightCamera, bool transparentOnly = false);

        std::shared_ptr<LightCamera> prepareShadowMap(int lightNum, std::shared_ptr<Light> sharedLight);

        void addFramebufferToScreen();

        void drawShadowMapThumbnail(int lightNum);

        void prepareGBuffer(glm::ivec2 ivec2);

        void drawModels(std::shared_ptr<NUGL::ShaderProgram> shared_ptr);

        void drawGBufferThumbnails();
    };

}
