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

    std::cout << "Resize to: windowSize = (" << width << ", " << height << "), fbSize = (" << fbWidth << ", " << fbHeight << ")." << std::endl;

    glViewport(0, 0, fbWidth, fbHeight);

    mainScene->prepareFramebuffer(width, height);

    mainScene->camera->frameWidth = fbWidth;
    mainScene->camera->frameHeight = fbHeight;
    mainScene->camera->prepareTransforms();
}

//void cursorPositionCallback(GLFWwindow* window, double x, double y) {
////    mainScene->camera->processMouseInput(window, x, y);
//    std::cout << __func__ << ": [" << x << ", " << y << "]" << std::endl;
//}

//void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
//    std::cout << __func__ << ", Key: {" << std::endl;
//    std::cout << "  key: " << key << std::endl;
//    std::cout << "  scancode: " << scancode << std::endl;
//    std::cout << "  action: " << action << std::endl;
//    std::cout << "  mods: " << mods << std::endl;
//    std::cout << "  }" << std::endl;
//}

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
    int screenWidth = 800;
    int screenHeight = 600;
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "OpenGL", nullptr, nullptr);
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);

//    // Fullscreen:
//    const GLFWvidmode* videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
//    GLFWwindow* window = glfwCreateWindow(videoMode->width, videoMode->height, "OpenGL", glfwGetPrimaryMonitor(), nullptr);

    glfwMakeContextCurrent(window);

    // Disable cursor to allow correct mouse input for camera controls on OSX.
    // See: http://stackoverflow.com/questions/14468039/glfw-glfwsetmousepos-bug-on-mac-os-x-10-7-with-opengl-camera
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
//    glfwSetCursorPosCallback(window, cursorPositionCallback);
//    glfwSetKeyCallback(window, keyCallback);

    checkForAndPrintGLError(__FILE__, __LINE__);


    // Initialise GLEW:
    glewExperimental = GL_TRUE;
    glewInit();
    checkForAndPrintGLError(__FILE__, __LINE__);


    // Load glsl:
    auto flatProgram = NUGL::ShaderProgram::createFromFiles("flatProgram", {
            {GL_VERTEX_SHADER, "src/glsl/position.vert"},
            {GL_FRAGMENT_SHADER, "src/glsl/uniform.frag"},
    });
    flatProgram.bindFragDataLocation(0, "outColor");
    flatProgram.link();
    flatProgram.updateMaterialInfo();
    flatProgram.printDebugInfo();

    auto textureProgram = NUGL::ShaderProgram::createFromFiles("textureProgram", {
            {GL_VERTEX_SHADER, "src/glsl/textured.vert"},
            {GL_FRAGMENT_SHADER, "src/glsl/textured.frag"},
    });
    textureProgram.bindFragDataLocation(0, "outColor");
    textureProgram.link();
    textureProgram.updateMaterialInfo();
    textureProgram.printDebugInfo();

    auto reflectProgram = NUGL::ShaderProgram::createFromFiles("reflectProgram", {
            {GL_VERTEX_SHADER, "src/glsl/shadow.vert"},
            {GL_FRAGMENT_SHADER, "src/glsl/shadow.frag"},
//            {GL_FRAGMENT_SHADER, "src/glsl/lit_rfl_tex.frag"},
    });
    reflectProgram.bindFragDataLocation(0, "outColor");
    reflectProgram.link();
    reflectProgram.updateMaterialInfo();
    reflectProgram.printDebugInfo();

    auto skyboxProgram = NUGL::ShaderProgram::createFromFiles("skyboxProgram", {
            {GL_VERTEX_SHADER, "src/glsl/skybox.vert"},
            {GL_FRAGMENT_SHADER, "src/glsl/skybox.frag"},
    });
    skyboxProgram.bindFragDataLocation(0, "outColor");
    skyboxProgram.link();
    skyboxProgram.updateMaterialInfo();
    skyboxProgram.printDebugInfo();

    auto screenProgram = NUGL::ShaderProgram::createFromFiles("screenProgram", {
            {GL_VERTEX_SHADER, "src/glsl/screen.vert"},
            {GL_FRAGMENT_SHADER, "src/glsl/textured.frag"},
    });
    screenProgram.bindFragDataLocation(0, "outColor");
    screenProgram.link();
    screenProgram.updateMaterialInfo();
    screenProgram.printDebugInfo();

    auto shadowMapProgram = NUGL::ShaderProgram::createFromFiles("shadowMapProgram", {
            {GL_VERTEX_SHADER, "src/glsl/shadow_map.vert"},
            {GL_FRAGMENT_SHADER, "src/glsl/shadow_map.frag"},
    });
    shadowMapProgram.bindFragDataLocation(0, "outColor");
    shadowMapProgram.link();
    shadowMapProgram.updateMaterialInfo();
    shadowMapProgram.printDebugInfo();

    auto sharedFlatProgram = std::make_shared<NUGL::ShaderProgram>(flatProgram);
    auto sharedTextureProgram = std::make_shared<NUGL::ShaderProgram>(textureProgram);
    auto sharedReflectProgram = std::make_shared<NUGL::ShaderProgram>(reflectProgram);
    auto sharedSkyboxProgram = std::make_shared<NUGL::ShaderProgram>(skyboxProgram);
    auto sharedScreenProgram = std::make_shared<NUGL::ShaderProgram>(screenProgram);
    auto sharedShadowMapProgram = std::make_shared<NUGL::ShaderProgram>(shadowMapProgram);

    // Create Scene:
    mainScene = std::make_unique<scene::Scene>(sharedScreenProgram,
            glm::ivec2({screenWidth, screenHeight}),
            glm::ivec2({fbWidth, fbHeight})
    );
    mainScene->shadowMapProgram = sharedShadowMapProgram;

    // Add some lights:
    auto lightModel = std::make_shared<scene::Model>();
    auto light = std::make_shared<scene::Light>();
    light->type = scene::Light::Type::spot;
    light->pos = {15, 120, 40};
    light->dir = {1, 0, 0};
//    light->pos = {10, 120, 15};
//    light->dir = {0, 0, -1};
    light->attenuationConstant = 1;
    light->attenuationLinear = 0.1;
    light->attenuationQuadratic = 0;
    light->angleConeOuter = 2;
    light->angleConeInner = 1;
    light->colDiffuse = {5, 5, 5};
    light->colSpecular = {2, 2, 2};
    light->colAmbient = {0, 0, 0};
    lightModel->lights.push_back(light);
    mainScene->addModel(lightModel);

    lightModel = std::make_shared<scene::Model>();
    light = std::make_shared<scene::Light>();
    light->type = scene::Light::Type::point;
    light->pos = {0, 0, 40};
    light->dir = {1, 0, 0};
    light->attenuationConstant = 0;
    light->attenuationLinear = 0.5;
    light->attenuationQuadratic = 0;
    light->colDiffuse = {1, 1, 1};
    light->colSpecular = {1, 1, 1};
    light->colAmbient = {0.2, 0.2, 0.2};
    light->angleConeInner = 1;
    light->angleConeOuter = 1;
    lightModel->lights.push_back(light);
    mainScene->addModel(lightModel);

    lightModel = std::make_shared<scene::Model>();
    light = std::make_shared<scene::Light>();
    light->type = scene::Light::Type::point;
    light->pos = {0, 200, 60};
    light->dir = {1, 0, 0};
    light->attenuationConstant = 0;
    light->attenuationLinear = 0.05;
    light->attenuationQuadratic = 0;
    light->colDiffuse = {1, 1, 1};
    light->colSpecular = {1, 1, 1};
    light->colAmbient = {0, 0, 0};
    light->angleConeInner = 1;
    light->angleConeOuter = 1;
    lightModel->lights.push_back(light);
    mainScene->addModel(lightModel);

    // TODO: Find a way to manage texture units!
    auto cubeMap = std::make_shared<NUGL::Texture>(GL_TEXTURE2, GL_TEXTURE_CUBE_MAP);
    cubeMap->loadCubeMap({
           "assets/skybox_right1.png",
           "assets/skybox_left2.png",
           "assets/skybox_top3.png",
           "assets/skybox_bottom4.png",
           "assets/skybox_front5.png",
           "assets/skybox_back6.png"
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
    eagle5Model->flatProgram = sharedFlatProgram;
    eagle5Model->textureProgram = sharedTextureProgram;
//    eagle5Model->environmentMapProgram = sharedFlatReflectProgram;
    eagle5Model->environmentMapProgram = sharedReflectProgram;
    eagle5Model->setEnvironmentMap(cubeMap);
    eagle5Model->createMeshBuffers();
    eagle5Model->createVertexArrays();
    eagle5Model->pos = {50, 100, 9};
    eagle5Model->dir = {0, 1, 0};
    eagle5Model->scale = glm::vec3(0.1);
    mainScene->addModel(eagle5Model);

    auto houseModel = scene::Model::loadFromFile("assets/House01/House01.obj");
    houseModel->flatProgram = sharedFlatProgram;
    houseModel->textureProgram = sharedTextureProgram;
//    houseModel->environmentMapProgram = sharedFlatReflectProgram;
    houseModel->environmentMapProgram = sharedReflectProgram;
    houseModel->setEnvironmentMap(cubeMap);
    houseModel->createMeshBuffers();
    houseModel->createVertexArrays();
    houseModel->dir = {0, 1, 0};
    houseModel->scale = glm::vec3(3);
    mainScene->addModel(houseModel);

    auto cityModel = scene::Model::loadFromFile("assets/rc8c1qtjiygw-O/Organodron City/Organodron City.obj");
    cityModel->flatProgram = sharedFlatProgram;
    cityModel->textureProgram = sharedTextureProgram;
//    cityModel->environmentMapProgram = sharedFlatReflectProgram;
    cityModel->environmentMapProgram = sharedReflectProgram;
    cityModel->setEnvironmentMap(cubeMap);
    cityModel->createMeshBuffers();
    cityModel->createVertexArrays();
    cityModel->dir = {0, 1, 0};
    mainScene->addModel(cityModel);

    auto cubeModel = scene::Model::loadFromFile("assets/cube.obj");
    cubeModel->flatProgram = sharedFlatProgram;
    cubeModel->textureProgram = sharedTextureProgram;
//    cubeModel->environmentMapProgram = sharedFlatReflectProgram;
    cubeModel->environmentMapProgram = sharedReflectProgram;
    cubeModel->setEnvironmentMap(cubeMap);
    cubeModel->createMeshBuffers();
    cubeModel->createVertexArrays();
    cubeModel->pos = {15, 120, 40};
    cubeModel->scale = glm::vec3(3);
    mainScene->addModel(cubeModel);

    auto asteroidModel = scene::createAsteroid();
    asteroidModel->flatProgram = sharedFlatProgram;
    asteroidModel->textureProgram = sharedTextureProgram;
    asteroidModel->environmentMapProgram = sharedReflectProgram;
    asteroidModel->setEnvironmentMap(cubeMap);
    asteroidModel->createMeshBuffers();
    asteroidModel->createVertexArrays();
    asteroidModel->pos = {15, 130, 40};
    asteroidModel->scale = glm::vec3(3);
    mainScene->addModel(asteroidModel);

    auto skyBox = scene::Model::loadFromFile("assets/cube.obj");
    skyBox->flatProgram = sharedFlatProgram;
    skyBox->textureProgram = sharedTextureProgram;
    skyBox->environmentMapProgram = sharedSkyboxProgram;
    skyBox->setEnvironmentMap(cubeMap);
    skyBox->createMeshBuffers();
    skyBox->createVertexArrays();
    mainScene->addModel(skyBox);

    // Setup Camera:
//    mainScene->camera->pos = {15, 120, 40};
    mainScene->camera->pos = {37.2, 156, 38.1};
    mainScene->camera->dir = {0.415, -0.646, -0.64};
    mainScene->camera->fov = 45;
    mainScene->camera->speed = 10;
    mainScene->camera->lookSpeed = 0.005;
    mainScene->camera->up = glm::vec3(0.0f, 0.0f, 1.0f);
//    mainScene->camera->up = glm::vec3(0.0f, 1.0f, 0.0f);
    mainScene->camera->lastUpdateTime = glfwGetTime();
    mainScene->camera->frameWidth = screenWidth;
    mainScene->camera->frameHeight = screenHeight;
    mainScene->camera->prepareTransforms();
    mainScene->camera->initializeAngles();

    glEnable(GL_DEPTH_TEST);

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

        mainScene->render();

        // Swap front and back buffers:
//        glfwSwapInterval(1); // v-sync
        glfwSwapBuffers(window);

        // Poll for and process events:
        glfwPollEvents();
//        std::cout << mainScene->camera->dir << std::endl;
//        std::cout << mainScene->camera->pos << std::endl;
        mainScene->camera->processPlayerInput(window);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);
//        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Clean up GLFW:
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

