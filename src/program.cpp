#include <iostream>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "utility/debug.h"
#include "utility/make_unique.h"
#include "utility/FrameTimer.h"

#include "NUGL/Shader.h"
#include "NUGL/ShaderProgram.h"
#include "NUGL/Buffer.h"
#include "NUGL/VertexArray.h"
#include "NUGL/Texture.h"
#include "scene/ProceduralAsteroid.h"
#include "scene/Model.h"
#include "scene/Scene.h"

static std::unique_ptr<scene::Scene> mainScene;

void errorCallback(int error, const char* description) {
    std::cerr << "GLFW ERROR: " << description << std::endl;
}

void framebufferSizeCallback(GLFWwindow* window, int fbWidth, int fbHeight) {
    if (fbWidth == 0) {
        fbWidth = 1;
    }

    // Ignore high-dpi screens? (i.e. always use virtual pixel sizes)
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    std::cerr << "Resize to: windowSize = (" << width << ", " << height << "), fbSize = (" << fbWidth << ", " << fbHeight << ")." << std::endl;

    mainScene->framebufferSize = {fbWidth, fbHeight};

    mainScene->windowSize = {width, height};
    mainScene->prepareFramebuffer(mainScene->windowSize);
    mainScene->prepareGBuffer(mainScene->windowSize);

    mainScene->camera->frameWidth = width;
    mainScene->camera->frameHeight = height;
    mainScene->camera->prepareTransforms();
}

//void cursorPositionCallback(GLFWwindow* window, double x, double y) {
////    mainScene->camera->processMouseInput(window, x, y);
//    std::cout << __func__ << ": [" << x << ", " << y << "]" << std::endl;
//}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS && key == GLFW_KEY_TAB)
        mainScene->useDeferredRendering = !mainScene->useDeferredRendering;

    if (action == GLFW_PRESS && key == GLFW_KEY_T)
        mainScene->profiler.glFinishEnabled = !mainScene->profiler.glFinishEnabled;

    if (action == GLFW_PRESS && key == GLFW_KEY_SPACE)
        mainScene->fpsMode = !mainScene->fpsMode;

    if (action == GLFW_PRESS && key == GLFW_KEY_R)
        mainScene->forwardRenderReflections = !mainScene->forwardRenderReflections;

    if (action == GLFW_PRESS && key == GLFW_KEY_F)
        mainScene->flashlightOn = !mainScene->flashlightOn;

    // previewOptions keys:
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_GRAVE_ACCENT)
            mainScene->previewOptions.disable = !mainScene->previewOptions.disable;

        if (key == GLFW_KEY_EQUAL)
            mainScene->previewOptions.fullscreen = !mainScene->previewOptions.fullscreen;

        if (key == GLFW_KEY_MINUS)
            mainScene->previewOptions.shadowMap = !mainScene->previewOptions.shadowMap;

        if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9)
            mainScene->previewOptions.index = key - GLFW_KEY_0 - 1;
    }
}

int main(int argc, char** argv) {
    glfwInit();

    glfwSetErrorCallback(errorCallback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
//    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    // Windowed:
    int screenWidth = 700;
    int screenHeight = 700;
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "OpenGL", nullptr, nullptr);
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

//    // Fullscreen:
//    const GLFWvidmode* videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
//    GLFWwindow* window = glfwCreateWindow(videoMode->width, videoMode->height, "OpenGL", glfwGetPrimaryMonitor(), nullptr);

    glfwMakeContextCurrent(window);

    // Disable cursor to allow correct mouse input for camera controls on OSX.
    // See: http://stackoverflow.com/questions/14468039/glfw-glfwsetmousepos-bug-on-mac-os-x-10-7-with-opengl-camera
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
//    glfwSetCursorPosCallback(window, cursorPositionCallback);
    glfwSetKeyCallback(window, keyCallback);

    checkForAndPrintGLError(__FILE__, __LINE__);


    // Initialise GLEW:
    glewExperimental = GL_TRUE;
    glewInit();
    checkForAndPrintGLError(__FILE__, __LINE__);


    // Load deferred rendering shaders:
    auto gBufferProgram = NUGL::ShaderProgram::createSharedFromFiles("gBufferProgram", {
            {GL_VERTEX_SHADER, "src/glsl/g-buffer.vert"},
            {GL_FRAGMENT_SHADER, "src/glsl/g-buffer.frag"},
    });
    gBufferProgram->bindFragDataLocation(0, "outNormal");
    gBufferProgram->bindFragDataLocation(1, "outAlbedoRoughness");
    gBufferProgram->bindFragDataLocation(2, "outEnvMapColSpecIntensity");
    gBufferProgram->link();
    gBufferProgram->updateMaterialInfo();
    gBufferProgram->printDebugInfo();

    auto deferredShadingProgram = NUGL::ShaderProgram::createSharedFromFiles("deferredShadingProgram", {
            {GL_VERTEX_SHADER, "src/glsl/deferredShading.vert"},
            {GL_FRAGMENT_SHADER, "src/glsl/deferredShading.frag"},
    });
    deferredShadingProgram->bindFragDataLocation(0, "outColor");
    deferredShadingProgram->link();
    deferredShadingProgram->updateMaterialInfo();
    deferredShadingProgram->printDebugInfo();

    // Load forward rendering shaders:
    auto flatProgram = NUGL::ShaderProgram::createSharedFromFiles("flatProgram", {
            {GL_VERTEX_SHADER, "src/glsl/position.vert"},
            {GL_FRAGMENT_SHADER, "src/glsl/uniform.frag"},
    });
    flatProgram->bindFragDataLocation(0, "outColor");
    flatProgram->link();
    flatProgram->updateMaterialInfo();
    flatProgram->printDebugInfo();

    auto textureProgram = NUGL::ShaderProgram::createSharedFromFiles("textureProgram", {
            {GL_VERTEX_SHADER, "src/glsl/textured.vert"},
            {GL_FRAGMENT_SHADER, "src/glsl/textured.frag"},
    });
    textureProgram->bindFragDataLocation(0, "outColor");
    textureProgram->link();
    textureProgram->updateMaterialInfo();
    textureProgram->printDebugInfo();

    auto reflectProgram = NUGL::ShaderProgram::createSharedFromFiles("reflectProgram", {
            {GL_VERTEX_SHADER, "src/glsl/shadow.vert"},
            {GL_FRAGMENT_SHADER, "src/glsl/shadow.frag"},
    });
    reflectProgram->bindFragDataLocation(0, "outColor");
    reflectProgram->link();
    reflectProgram->updateMaterialInfo();
    reflectProgram->printDebugInfo();

    auto skyboxProgram = NUGL::ShaderProgram::createSharedFromFiles("skyboxProgram", {
            {GL_VERTEX_SHADER, "src/glsl/skybox.vert"},
            {GL_FRAGMENT_SHADER, "src/glsl/skybox.frag"},
    });
    skyboxProgram->bindFragDataLocation(0, "outColor");
    skyboxProgram->link();
    skyboxProgram->updateMaterialInfo();
    skyboxProgram->printDebugInfo();

    auto shadowMapProgram = NUGL::ShaderProgram::createSharedFromFiles("shadowMapProgram", {
            {GL_VERTEX_SHADER, "src/glsl/shadow_map.vert"},
            {GL_FRAGMENT_SHADER, "src/glsl/shadow_map.frag"},
    });
    shadowMapProgram->bindFragDataLocation(0, "outColor");
    shadowMapProgram->link();
    shadowMapProgram->updateMaterialInfo();
    shadowMapProgram->printDebugInfo();

    // Load compositing shaders:
    auto screenProgram = NUGL::ShaderProgram::createSharedFromFiles("screenProgram", {
            {GL_VERTEX_SHADER, "src/glsl/screen.vert"},
            {GL_FRAGMENT_SHADER, "src/glsl/textured.frag"},
    });
    screenProgram->bindFragDataLocation(0, "outColor");
    screenProgram->link();
    screenProgram->updateMaterialInfo();
    screenProgram->printDebugInfo();

    auto screenAlphaProgram = NUGL::ShaderProgram::createSharedFromFiles("screenAlphaProgram", {
            {GL_VERTEX_SHADER, "src/glsl/screen.vert"},
            {GL_FRAGMENT_SHADER, "src/glsl/alpha.frag"},
    });
    screenAlphaProgram->bindFragDataLocation(0, "outColor");
    screenAlphaProgram->link();
    screenAlphaProgram->updateMaterialInfo();
    screenAlphaProgram->printDebugInfo();

    // Create Scene:
    mainScene = std::make_unique<scene::Scene>(screenProgram, screenAlphaProgram,
            glm::ivec2({screenWidth, screenHeight}),
            glm::ivec2({fbWidth, fbHeight})
    );
    mainScene->shadowMapProgram = shadowMapProgram;
    mainScene->gBufferProgram = gBufferProgram;
    mainScene->deferredShadingProgram = deferredShadingProgram;

    // Add some lights:
    glm::vec3 sunlightCol = glm::vec3(1, 1, 0.7);
    glm::vec3 downlightCol = glm::vec3(1, 1, 1) * 100.0f;
    glm::vec3 tubelightCol = glm::vec3(0.4, 0.4, 1) * 50.0f;
    glm::vec3 flashlightCol = glm::vec3(1, 1, 1) * 50.0f;

    auto lightModel = std::make_shared<scene::Model>("sun");
    auto light = std::make_shared<scene::Light>();
    glm::vec3 sunDir = glm::normalize(glm::vec3(-10, -50, -5));
    glm::vec3 sunPos = glm::vec3(30, 50, 12) + sunDir * 0.0f;
    lightModel->lights.push_back(scene::Light::makeDirectional(sunPos, sunDir, 125, sunlightCol, sunlightCol));
    mainScene->addModel(lightModel);

    lightModel = std::make_shared<scene::Model>("downlight 1");
    lightModel->lights.push_back(scene::Light::makeSpotlight({10, 0, 14}, {0, 0, -1}, 2, 1, downlightCol, downlightCol));
    mainScene->addModel(lightModel);

    lightModel = std::make_shared<scene::Model>("downlight 2");
    lightModel->lights.push_back(scene::Light::makeSpotlight({-10, 0, 14}, {0, 0, -1}, 2, 1, downlightCol, downlightCol));
    mainScene->addModel(lightModel);

    lightModel = std::make_shared<scene::Model>("tube light");
    light = scene::Light::makeSpotlight({0, 0, 0}, {0, 0, 1}, 1.5, 1.5, tubelightCol, tubelightCol);
    light->colAmbient = glm::vec3(0.1);
    lightModel->lights.push_back(light);
    mainScene->addModel(lightModel);

    lightModel = std::make_shared<scene::Model>("flash light");
    auto flashlight = scene::Light::makeSpotlight({0, 0, 0}, {0, 0, 1}, 0.5, 0.2, flashlightCol, flashlightCol);
    flashlight->colAmbient = glm::vec3(0.1);
    lightModel->lights.push_back(flashlight);
    mainScene->addModel(lightModel);


    // TODO: Find a way to manage texture units!
    auto cubeMap = std::make_shared<NUGL::Texture>(GL_TEXTURE2, GL_TEXTURE_CUBE_MAP);
    cubeMap->loadCubeMap({
            "assets/default_right1.png",
            "assets/default_left2.png",
            "assets/default_top3.png",
            "assets/default_bottom4.png",
            "assets/default_front5.png",
            "assets/default_back6.png"

//            "assets/PereaBeach1/posx.jpg",
//            "assets/PereaBeach1/negx.jpg",
//            "assets/PereaBeach1/posy.jpg",
//            "assets/PereaBeach1/negy.jpg",
//            "assets/PereaBeach1/posz.jpg",
//            "assets/PereaBeach1/negz.jpg"
    });
    cubeMap->setParam(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    cubeMap->setParam(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    cubeMap->setParam(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    cubeMap->setParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    cubeMap->setParam(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    cubeMap->setParam(GL_TEXTURE_BASE_LEVEL, 0);
    cubeMap->setParam(GL_TEXTURE_MAX_LEVEL, 0);
////    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // Load assets:
    auto eagle5Model = scene::Model::loadFromFile("assets/eagle 5 transport/eagle 5 transport landed.obj");
//    auto eagle5Model = scene::Model::loadFromFile("assets/Ship/Ship Room.obj");
//    auto eagle5Model = scene::Model::loadFromFile("assets/textured_cube.obj");
//    auto eagle5Model = scene::Model::loadFromFile("assets/KingsTreasure_OBJ/KingsTreasure.obj");
    eagle5Model->flatProgram = flatProgram;
    eagle5Model->textureProgram = textureProgram;
//    eagle5Model->environmentMapProgram = sharedFlatReflectProgram;
    eagle5Model->environmentMapProgram = reflectProgram;
    eagle5Model->setEnvironmentMap(cubeMap);
//    eagle5Model->setEnvironmentMap(nullptr); // TODO: Improve environment map management.
//    eagle5Model->dynamicReflections = true;
    eagle5Model->createMeshBuffers();
    eagle5Model->createVertexArrays();
    eagle5Model->pos = {60, 0, -5};
    eagle5Model->dir = {-1, 1, 0};
    eagle5Model->scale = glm::vec3(0.2);
    mainScene->addModel(eagle5Model);

//    auto houseModel = scene::Model::loadFromFile("assets/House01/House01.obj");
//    houseModel->flatProgram = flatProgram;
//    houseModel->textureProgram = textureProgram;
////    houseModel->environmentMapProgram = sharedFlatReflectProgram;
//    houseModel->environmentMapProgram = reflectProgram;
//    houseModel->setEnvironmentMap(cubeMap);
//    houseModel->createMeshBuffers();
//    houseModel->createVertexArrays();
//    houseModel->dir = {0, 1, 0};
//    houseModel->scale = glm::vec3(3);
//    mainScene->addModel(houseModel);

    auto spaceshipModel = scene::Model::loadFromFile("assets/spaceship/spaceship.obj");
//    auto spaceshipModel = scene::Model::loadFromFile("assets/spaceship/spaceship.3ds");
    spaceshipModel->flatProgram = flatProgram;
    spaceshipModel->textureProgram = textureProgram;
//    spaceshipModel->environmentMapProgram = sharedFlatReflectProgram;
    spaceshipModel->environmentMapProgram = reflectProgram;
    spaceshipModel->setEnvironmentMap(cubeMap);
    spaceshipModel->createMeshBuffers();
    spaceshipModel->createVertexArrays();
    spaceshipModel->dir = {0, 1, 0};
//    spaceshipModel->pos = {50, 150, 30};
    spaceshipModel->scale = glm::vec3(20);
    mainScene->addModel(spaceshipModel);

    auto robotModel = scene::Model::loadFromFile("assets/graph-robot.obj");
    robotModel->flatProgram = flatProgram;
    robotModel->textureProgram = textureProgram;
    robotModel->environmentMapProgram = reflectProgram;
//    robotModel->setEnvironmentMap(cubeMap);
    robotModel->setEnvironmentMap(nullptr); // TODO: Improve environment map management.
    robotModel->dynamicReflections = true;
    robotModel->createMeshBuffers();
    robotModel->createVertexArrays();
    robotModel->dir = {0, 1, 0};
    robotModel->pos = {0, 1, 0};
    robotModel->scale = glm::vec3(1);

    // Make the robot's light globes emissive:
    robotModel->materials[2]->materialInfo.has.emissive = true;
    robotModel->materials[2]->emissive = 1;

    // Make the robot metallic:
    for (auto material : robotModel->materials) {
        material->materialInfo.has.reflectivity = true;
        material->reflectivity = 1;
    }

    glm::vec3 robotLightCol = glm::vec3(0.6, 0.6, 1) * 50.0f;
    robotModel->lights.push_back(scene::Light::makeSpotlight({0, 0, 0}, {0, 0, 0}, 1.5, 0.6, robotLightCol, robotLightCol));
    robotModel->lights.push_back(scene::Light::makeSpotlight({0, 0, 0}, {0, 0, 0}, 1.5, 0.6, robotLightCol, robotLightCol));
    mainScene->addModel(robotModel);

//    auto cubeModel = scene::Model::loadFromFile("assets/cube.obj");
//    cubeModel->flatProgram = flatProgram;
//    cubeModel->textureProgram = textureProgram;
////    cubeModel->environmentMapProgram = sharedFlatReflectProgram;
//    cubeModel->environmentMapProgram = reflectProgram;
//    cubeModel->setEnvironmentMap(cubeMap);
////    cubeModel->setEnvironmentMap(nullptr); // TODO: Improve environment map management.
////    cubeModel->dynamicReflections = true;
//    cubeModel->createMeshBuffers();
//    cubeModel->createVertexArrays();
////    cubeModel->pos = {15, 120, 40};
////    cubeModel->scale = glm::vec3(3);
//    cubeModel->pos = {15, 120, 60};
//    cubeModel->scale = glm::vec3(10);
//    cubeModel->materials[0]->materialInfo.has.shininess = true;
//    cubeModel->materials[0]->shininess = 1;
//    cubeModel->materials[0]->colSpecular = glm::vec3(1);
//    mainScene->addModel(cubeModel);

    auto asteroidModel = scene::createAsteroid(0.3, 0.2, 4);
//    auto asteroidModel = scene::createAsteroid(0, 0, 4);
    asteroidModel->flatProgram = flatProgram;
    asteroidModel->textureProgram = textureProgram;
    asteroidModel->environmentMapProgram = reflectProgram;
    asteroidModel->setEnvironmentMap(cubeMap);
//    asteroidModel->setEnvironmentMap(nullptr); // TODO: Improve environment map management.
//    asteroidModel->dynamicReflections = true;
    asteroidModel->createMeshBuffers();
    asteroidModel->createVertexArrays();
    asteroidModel->pos = {0, 0, 5};
    asteroidModel->scale = glm::vec3(1.2);
//    asteroidModel->scale = glm::vec3(1.5);
    mainScene->addModel(asteroidModel);

    std::vector<std::shared_ptr<scene::Model>> asteroids;
    for (int i = 0; i < 10; i++) {
        auto asteroid = scene::createAsteroid(0.2, 0.2, 2);
        asteroids.push_back(asteroid);

        asteroid->flatProgram = flatProgram;
        asteroid->textureProgram = textureProgram;
        asteroid->environmentMapProgram = reflectProgram;
        asteroid->setEnvironmentMap(cubeMap);
//    asteroid->setEnvironmentMap(nullptr); // TODO: Improve environment map management.
//    asteroid->dynamicReflections = true;
        asteroid->createMeshBuffers();
        asteroid->createVertexArrays();
        asteroid->pos = {0, 30, 0};
        asteroid->scale = glm::vec3(5);
        mainScene->addModel(asteroid);
    }

    auto skyBox = scene::Model::loadFromFile("assets/cube.obj");
    skyBox->flatProgram = flatProgram;
    skyBox->textureProgram = textureProgram;
    skyBox->environmentMapProgram = skyboxProgram;
    skyBox->setEnvironmentMap(cubeMap);
    skyBox->createMeshBuffers();
    skyBox->createVertexArrays();
    mainScene->addModel(skyBox);
    mainScene->skyBox = skyBox;

    // Setup Camera:
//    mainScene->camera->pos = {15, 120, 40};
    mainScene->camera->near_ = 1;
    mainScene->camera->pos = {-20, 0, 5};
    mainScene->camera->dir = {1, 0, 0};
//    mainScene->camera->pos = {37.2, 156, 38.1};
//    mainScene->camera->dir = {0.415, -0.646, -0.64};
    mainScene->camera->fov = M_PI_4;
    mainScene->camera->speed = 10;
    mainScene->camera->lookSpeed = 0.005;
    mainScene->camera->up = glm::vec3(0.0f, 0.0f, 1.0f);
//    mainScene->camera->up = glm::vec3(0.0f, 1.0f, 0.0f);
    mainScene->camera->lastUpdateTime = glfwGetTime();
    mainScene->camera->frameWidth = screenWidth;
    mainScene->camera->frameHeight = screenHeight;
    mainScene->camera->prepareTransforms();
    mainScene->camera->initializeAngles();

    // Backface culling:
//    glEnable (GL_CULL_FACE); // cull face
//    glCullFace (GL_BACK); // cull back face
//    glFrontFace (GL_CCW); // GL_CCW for counter clock-wise

    utility::FrameTimer frameTimer(glfwGetTime());
    while (!glfwWindowShouldClose(window)) {
        if (frameTimer.frameUpdate(glfwGetTime())) {
            glfwSetWindowTitle(window, frameTimer.timeStr.c_str());
        }

        skyBox->pos = mainScene->camera->pos;

        // Tube-rock movement:
        float t = glfwGetTime();
        asteroidModel->pos = glm::vec3(0,0,8) + glm::vec3(0,0,3) * std::sin(t);
        asteroidModel->dir = glm::normalize(glm::vec3(std::sin(2 * t) + std::cos(3 * t), std::cos(2 * t), std::cos(3 * t)));

        // Asteroid movement:
        srand(1031);
        for (int i = 0; i < asteroids.size(); i++) {
            auto asteroid = asteroids[i];

            float k = rand() / (float)RAND_MAX;

            float t = glfwGetTime() + k * 1097;
            float radius = (80 + k * 80);
            float tilt = ((k - 0.5) + 0.5) * 0.5;
            float speed = 1600.0 / (radius * radius);

            glm::vec4 pos = glm::vec4(std::cos(t * speed), std::sin(t * speed), 0, 1);
            glm::mat4 rot;
            rot = glm::rotate(rot, tilt, glm::normalize(glm::vec3(0,1,0)));
            pos = rot * pos;
            asteroid->pos = glm::vec3(pos.x, pos.y, pos.z) * radius;
            asteroid->dir = glm::vec3(std::sin(k*t*5), std::cos(k*t*5), 0);
        }

        // Robot movement:
        {
            float t = glfwGetTime();
            float radius = 11. + (std::sin(t / 1.5) * 3.);
            glm::vec3 lastPos = robotModel->pos;
            robotModel->pos = glm::vec3(std::cos(t / 6.), std::sin(t / 6.), 0) * radius;
            robotModel->dir = glm::normalize(robotModel->pos - lastPos);

            glm::mat4 transform = robotModel->modelTransform();
            robotModel->lights[0]->pos = glm::vec3(transform * glm::vec4(1.00111, 1.45769, -0.98452, 1));
            robotModel->lights[0]->dir = glm::normalize(glm::vec3(transform * glm::vec4(0, 0, -1, 0)));
            robotModel->lights[1]->pos = glm::vec3(transform * glm::vec4(-1.00111, 1.45769, -0.98452, 1));
            robotModel->lights[1]->dir = glm::normalize(glm::vec3(transform * glm::vec4(0, 0, -1, 0)));
        }

        mainScene->render();

        // Swap front and back buffers:
//        glfwSwapInterval(1); // v-sync
        glfwSwapBuffers(window);
        mainScene->profiler.split("glfwSwapBuffers");

        // Poll for and process events:
        glfwPollEvents();
        mainScene->profiler.split("glfwPollEvents");

        mainScene->camera->processPlayerInput(window);
        if (mainScene->fpsMode) {
            mainScene->camera->pos.z = 6;
            mainScene->camera->prepareTransforms();
        }
        flashlight->enabled = mainScene->flashlightOn;
        flashlight->pos = mainScene->camera->pos;
        flashlight->dir = mainScene->camera->dir;
        mainScene->profiler.split("processPlayerInput");

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);
    }

    // Clean up GLFW:
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

