#include <iostream>

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
#include "SceneModel.h"

static auto camera = std::make_unique<Camera>();

void errorCallback(int error, const char* description) {
    std::cerr << "GLFW ERROR: " << description << std::endl;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    if (height == 0) {
        height = 1;
    }

    camera->proj = glm::perspective(camera->fov, width / float(height), 1.0f, 100.0f);
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

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL", nullptr, nullptr); // Windowed
//    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL", glfwGetPrimaryMonitor(), nullptr); // Fullscreen

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

    // Load shaders:
    auto flatProgram = NUGL::ShaderProgram::createFromFiles({
            {GL_VERTEX_SHADER, "src/shaders/vs_pos_mvp.gl"},
            {GL_FRAGMENT_SHADER, "src/shaders/fs_uniform.gl"},
    });
    flatProgram.bindFragDataLocation(0, "outColor");
    flatProgram.link();


    auto textureProgram = NUGL::ShaderProgram::createFromFiles({
            {GL_VERTEX_SHADER, "src/shaders/vs_pos_tex_mvp.gl"},
            {GL_FRAGMENT_SHADER, "src/shaders/fs_tex.gl"},
    });
    textureProgram.bindFragDataLocation(0, "outColor");
    textureProgram.link();


    // Load assets:
    auto eagle5Model = SceneModel::loadFromFile("assets/eagle 5 transport/eagle 5 transport landed.obj");
//    auto eagle5Model = SceneModel::loadFromFile("assets/galaxy_cruiser_3ds.3DS");
//    auto eagle5Model = SceneModel::loadFromFile("assets/cube.obj");
    eagle5Model.flatProgram = std::make_shared<NUGL::ShaderProgram>(flatProgram);
    eagle5Model.textureProgram = std::make_shared<NUGL::ShaderProgram>(textureProgram);
    eagle5Model.createMeshBuffers();
    eagle5Model.createVertexArrays();
    checkForAndPrintGLError(__FILE__, __LINE__);


    // Setup Camera:
    camera->pos = glm::vec3(0.0f, 0.0f, 10.0f);
    camera->fov = 45;
    camera->speed = 10;
    camera->lookSpeed = 0.5;
    camera->up = glm::vec3(0.0f, 0.0f, 1.0f);
    camera->proj = glm::perspective(camera->fov, 800.0f / 600.0f, 1.0f, 100.0f);
    camera->lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
    camera->lastUpdateTime = glfwGetTime();


    glEnable(GL_DEPTH_TEST);

//    // Backface culling:
//    glEnable (GL_CULL_FACE); // cull face
//    glCullFace (GL_BACK); // cull back face
//    glFrontFace (GL_CCW); // GL_CCW for counter clock-wise

    while (!glfwWindowShouldClose(window)) {
        /* Render here */
        glClearColor(0.5, 0.5, 0.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 model;
        model = glm::rotate(model, float(M_PI_2), glm::vec3(1.0f, 0.0f, 0.0f));
//        float scale = 10;
//        float scale = 1.0;
        float scale = 0.2;
        model = glm::scale(model, glm::vec3(scale, scale, scale));

        checkForAndPrintGLError(__FILE__, __LINE__);

        eagle5Model.draw(model, *camera);

        checkForAndPrintGLError(__FILE__, __LINE__);
        /* Swap front and back buffers */
//        glfwSwapInterval(1); // v-sync
        glfwSwapBuffers(window);

        /* Poll for and process events */
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

