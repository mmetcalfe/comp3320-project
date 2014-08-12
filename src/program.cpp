#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "utility/debug.h"
#include "NUGL/Shader.h"
#include "NUGL/ShaderProgram.h"
#include "NUGL/Buffer.h"
#include "NUGL/VertexArray.h"
#include "NUGL/Texture.h"
#include "SceneModel.h"

void errorCallback(int error, const char* description) {
    std::cerr << "GLFW ERROR: " << description << std::endl;
}

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

    checkForAndPrintGLError(__FILE__, __LINE__);

    // Initialise GLEW:
    glewExperimental = GL_TRUE;
    glewInit();
    checkForAndPrintGLError(__FILE__, __LINE__);


    auto flatProgram = NUGL::ShaderProgram::createFromFiles({
            {GL_VERTEX_SHADER, "src/shaders/vs_pos_mvp.gl"},
            {GL_FRAGMENT_SHADER, "src/shaders/fs_white.gl"},
    });
    flatProgram.bindFragDataLocation(0, "outColor");
    flatProgram.link();


    auto textureProgram = NUGL::ShaderProgram::createFromFiles({
            {GL_VERTEX_SHADER, "src/shaders/vs_pos_tex_mvp.gl"},
            {GL_FRAGMENT_SHADER, "src/shaders/fs_tex.gl"},
    });
    textureProgram.bindFragDataLocation(0, "outColor");
    textureProgram.link();


    auto eagle5Model = SceneModel::loadFromFile("assets/eagle_5_transport/eagle_5_transport_landed.obj");
//    auto eagle5Model = SceneModel::loadFromFile("assets/galaxy_cruiser_3ds.3DS");
//    auto eagle5Model = SceneModel::loadFromFile("assets/cube.obj");
    eagle5Model.flatProgram = std::make_shared<NUGL::ShaderProgram>(flatProgram);
    eagle5Model.textureProgram = std::make_shared<NUGL::ShaderProgram>(textureProgram);
    eagle5Model.createMeshBuffers();
    eagle5Model.createVertexArrays();
    checkForAndPrintGLError(__FILE__, __LINE__);


    Camera camera;
    // Projection transform: glm::perspective(FOV, Aspect, Near, Far);
    camera.pos = glm::vec3(50.0f, 50.0f, 30.0f);
    camera.fov = 45;
    camera.speed = 10;
    camera.lookSpeed = 10;
    camera.up = glm::vec3(0.0f, 0.0f, 1.0f);
    camera.proj = glm::perspective(camera.fov, 800.0f / 600.0f, 1.0f, 100.0f);
    camera.lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
    camera.lastUpdateTime = glfwGetTime();

    // Depth buffer:
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
        model = glm::rotate(model, 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
//        float scale = 10;
//        float scale = 1.0;
        float scale = 0.2;
        model = glm::scale(model, glm::vec3(scale, scale, scale));

        checkForAndPrintGLError(__FILE__, __LINE__);

        eagle5Model.draw(model, camera);


        checkForAndPrintGLError(__FILE__, __LINE__);
        /* Swap front and back buffers */
//        glfwSwapInterval(1); // v-sync
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);

//        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Clean up GLFW:
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

