#include <iostream>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "utility/debug.h"
#include "utility/make_unique.h"

#include "NUGL/Shader.h"
#include "NUGL/ShaderProgram.h"
#include "NUGL/Buffer.h"
#include "NUGL/VertexArray.h"
#include "NUGL/Texture.h"
#include "scene/Model.h"

static auto camera = std::make_unique<Camera>();

void errorCallback(int error, const char* description) {
    std::cerr << "GLFW ERROR: " << description << std::endl;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    if (height == 0) {
        height = 1;
    }

    glViewport(0, 0, width, height);
    camera->proj = glm::perspective(camera->fov, width / float(height), 1.0f, 300.0f);
}

//void cursorPositionCallback(GLFWwindow* window, double x, double y) {
////    camera->processMouseInput(window, x, y);
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    // Windowed:
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL", nullptr, nullptr);

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
            {GL_VERTEX_SHADER, "src/glsl/pos_mvp.vert"},
            {GL_FRAGMENT_SHADER, "src/glsl/uniform.frag"},
    });
    flatProgram.bindFragDataLocation(0, "outColor");
    flatProgram.link();
    flatProgram.updateMaterialInfo();
    flatProgram.printDebugInfo();

    auto textureProgram = NUGL::ShaderProgram::createFromFiles("textureProgram", {
            {GL_VERTEX_SHADER, "src/glsl/pos_tex_mvp.vert"},
            {GL_FRAGMENT_SHADER, "src/glsl/tex.frag"},
    });
    textureProgram.bindFragDataLocation(0, "outColor");
    textureProgram.link();
    textureProgram.updateMaterialInfo();
    textureProgram.printDebugInfo();

    auto flatReflectProgram = NUGL::ShaderProgram::createFromFiles("flatReflectProgram", {
            {GL_VERTEX_SHADER, "src/glsl/flat_reflect.vert"},
            {GL_FRAGMENT_SHADER, "src/glsl/flat_reflect.frag"},
    });
    flatReflectProgram.bindFragDataLocation(0, "outColor");
    flatReflectProgram.link();
    flatReflectProgram.updateMaterialInfo();
    flatReflectProgram.printDebugInfo();

    auto reflectProgram = NUGL::ShaderProgram::createFromFiles("reflectProgram", {
            {GL_VERTEX_SHADER, "src/glsl/reflect_tex.vert"},
            {GL_FRAGMENT_SHADER, "src/glsl/reflect_tex.frag"},
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

    auto sharedFlatProgram = std::make_shared<NUGL::ShaderProgram>(flatProgram);
    auto sharedTextureProgram = std::make_shared<NUGL::ShaderProgram>(textureProgram);
    auto sharedFlatReflectProgram = std::make_shared<NUGL::ShaderProgram>(flatReflectProgram);
    auto sharedReflectProgram = std::make_shared<NUGL::ShaderProgram>(reflectProgram);
    auto sharedSkyboxProgram = std::make_shared<NUGL::ShaderProgram>(skyboxProgram);

//    // TODO: Find a way to manage texture units!
    auto cubeMap = std::make_shared<NUGL::Texture>(GL_TEXTURE1, GL_TEXTURE_CUBE_MAP);
    cubeMap->loadCubeMap({
            "assets/PereaBeach1/posx.jpg",
            "assets/PereaBeach1/negx.jpg",
            "assets/PereaBeach1/posy.jpg",
            "assets/PereaBeach1/negy.jpg",
            "assets/PereaBeach1/posz.jpg",
            "assets/PereaBeach1/negz.jpg",
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
//    auto eagle5Model = scene::Model::loadFromFile("assets/KingsTreasure_OBJ/KingsTreasure.obj");
//    auto eagle5Model = scene::Model::loadFromFile("assets/rc8c1qtjiygw-O/Organodron City/Organodron City.obj");
//    auto eagle5Model = scene::Model::loadFromFile("assets/Bedroom.3DS");
//    auto eagle5Model = scene::Model::loadFromFile("assets/Room 2013 New/Room 2013 New.c4d");
    eagle5Model.flatProgram = sharedFlatProgram;
    eagle5Model.textureProgram = sharedTextureProgram;
//    eagle5Model.environmentMapProgram = sharedFlatReflectProgram;
    eagle5Model.environmentMapProgram = sharedReflectProgram;
    eagle5Model.setEnvironmentMap(cubeMap);
    eagle5Model.createMeshBuffers();
    eagle5Model.createVertexArrays();

    auto houseModel = scene::Model::loadFromFile("assets/House01/House01.obj");
    houseModel.flatProgram = sharedFlatProgram;
    houseModel.textureProgram = sharedTextureProgram;
//    houseModel.environmentMapProgram = sharedFlatReflectProgram;
    houseModel.environmentMapProgram = sharedReflectProgram;
    houseModel.setEnvironmentMap(cubeMap);
    houseModel.createMeshBuffers();
    houseModel.createVertexArrays();

//
//    auto galaxyCruiserModel = scene::Model::loadFromFile("assets/galaxy_cruiser_3ds.3DS");
//    galaxyCruiserModel.flatProgram = sharedFlatProgram;
//    galaxyCruiserModel.textureProgram = sharedTextureProgram;
////    galaxyCruiserModel.environmentMapProgram = sharedFlatReflectProgram;
//    galaxyCruiserModel.environmentMapProgram = sharedReflectProgram;
//    galaxyCruiserModel.setEnvironmentMap(cubeMap);
//    galaxyCruiserModel.createMeshBuffers();
//    galaxyCruiserModel.createVertexArrays();
//
//    auto cubeModel = scene::Model::loadFromFile("assets/cube.obj");
//    cubeModel.flatProgram = sharedFlatProgram;
//    cubeModel.textureProgram = sharedTextureProgram;
////    cubeModel.environmentMapProgram = sharedFlatReflectProgram;
//    cubeModel.environmentMapProgram = sharedReflectProgram;
//    cubeModel.setEnvironmentMap(cubeMap);
//    cubeModel.createMeshBuffers();
//    cubeModel.createVertexArrays();

    auto skyBox = scene::Model::loadFromFile("assets/cube.obj");
    skyBox.flatProgram = sharedFlatProgram;
    skyBox.textureProgram = sharedTextureProgram;
    skyBox.environmentMapProgram = sharedSkyboxProgram;
    skyBox.setEnvironmentMap(cubeMap);
    skyBox.createMeshBuffers();
    skyBox.createVertexArrays();

    // Setup Camera:
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    camera->pos = glm::vec3(0.0f, 0.0f, 0.0f);
    camera->fov = 45;
    camera->speed = 10;
    camera->lookSpeed = 0.005;
    camera->up = glm::vec3(0.0f, 0.0f, 1.0f);
//    camera->up = glm::vec3(0.0f, 1.0f, 0.0f);
    camera->proj = glm::perspective(camera->fov, width / float(height), 0.1f, 300.0f);
    camera->lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
    camera->lastUpdateTime = glfwGetTime();

    glEnable(GL_DEPTH_TEST);

//    // Backface culling:
//    glEnable (GL_CULL_FACE); // cull face
//    glCullFace (GL_BACK); // cull back face
//    glFrontFace (GL_CCW); // GL_CCW for counter clock-wise

    double lastTime = glfwGetTime();
    int nbFrames = 0;
    while (!glfwWindowShouldClose(window)) {
        // TODO: Refactor frame timer into a utility class
        // Measure speed
        double currentTime = glfwGetTime();
        nbFrames++;
        if (currentTime - lastTime >= 1.0) { // If last print was more than 1 sec ago
            double fps = double(nbFrames);
            std::stringstream title;
            title << "OpenGL | " << (1000.0 / fps) << " ms/frame (" << fps << "fps)";
            glfwSetWindowTitle(window, title.str().c_str());
//            std::cout << (1000.0 / fps) << " ms/frame (" << fps << "fps)" << std::endl;
            nbFrames = 0;
            lastTime += 1.0;
        }

        // Begin rendering:
        glClearColor(0.5, 0.5, 0.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw Eagle 5:
        glm::mat4 shipTransform;
        shipTransform = glm::scale(shipTransform, glm::vec3(0.3));
        shipTransform = glm::rotate(shipTransform, float(M_PI_2), glm::vec3(1.0f, 0.0f, 0.0f)); // TODO: Make this rotation a boolean switch on Model.
        eagle5Model.transform = shipTransform;
        eagle5Model.draw(*camera);


        // Draw House:
        glm::mat4 houseTransform;
        houseTransform = glm::scale(houseTransform, glm::vec3(3));
        houseTransform = glm::rotate(houseTransform, float(M_PI_2), glm::vec3(1.0f, 0.0f, 0.0f));
        houseModel.transform = houseTransform;
        houseModel.draw(*camera);

//        // Draw Galaxy Cruiser:
//        glm::mat4 cruiserTransform;
////        cruiserTransform = glm::translate(cruiserTransform, glm::vec3(20));
//        cruiserTransform = glm::scale(cruiserTransform, glm::vec3(0.2));
//        cruiserTransform = glm::rotate(cruiserTransform, float(M_PI_2), glm::vec3(1.0f, 0.0f, 0.0f));
//        galaxyCruiserModel.transform = cruiserTransform;
//        galaxyCruiserModel.draw(*camera);
//
//        // Draw cube:
//        glm::mat4 mapTransform;
//        mapTransform = glm::translate(mapTransform, glm::vec3(10));
//        mapTransform = glm::scale(mapTransform, glm::vec3(10));
//        mapTransform = glm::rotate(mapTransform, float(M_PI_2), glm::vec3(1.0f, 0.0f, 0.0f));
//        cubeModel.transform = mapTransform;
//        cubeModel.draw(*camera);

        // Draw skybox:
        glm::mat4 skyboxTransform;
        skyboxTransform = glm::translate(skyboxTransform, camera->pos);
        skyboxTransform = glm::rotate(skyboxTransform, float(M_PI_2), glm::vec3(1.0f, 0.0f, 0.0f));
        skyBox.transform = skyboxTransform;
        skyBox.draw(*camera);

        // TODO: Add Framebuffer objects to NUGL and refactor rendering loop

        // Swap front and back buffers:
//        glfwSwapInterval(1); // v-sync
        glfwSwapBuffers(window);

        // Poll for and process events:
        glfwPollEvents();

        camera->processPlayerInput(window);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);

//        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Clean up GLFW:
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

