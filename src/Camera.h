#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera {
public:
    inline void lookAt(glm::vec3 lookAtPos) {
        view = glm::lookAt(pos, lookAtPos, up);
    };

    inline void processKeyboardInput(GLFWwindow* window) {
        glm::vec3 move = {0, 0, 0};
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            move.z += 1;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            move.z -= 1;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            move.x += 1;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            move.x -= 1;
        }

        if (glm::length(move) > 0) {
            move = glm::normalize(move);
        }

        float currentTime = glfwGetTime();
        float deltaTime = float(currentTime - lastUpdateTime);
        lastUpdateTime = currentTime;

        pos += move * deltaTime * speed;
    }

    inline void processPlayerInput(GLFWwindow* window) {
        int windowWidth, windowHeight;
        glfwGetWindowSize(window, &windowWidth, &windowHeight);

        double xPos, yPos;
        glfwGetCursorPos(window, &xPos, &yPos);
//        glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);
        glm::vec2 deltaMouse = {xPos - (windowWidth / 2), yPos - (windowHeight / 2)};


//        int windowWidth, windowHeight;
//        glfwGetWindowSize(window, &windowWidth, &windowHeight);
//        glm::vec2 windowDimensions(windowWidth, windowHeight);
//
//        double xPos, yPos;
//        glfwGetCursorPos(window, &xPos, &yPos);
//        glm::vec2 cursor(xPos, yPos);
//
//        glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);
//        glm::vec2 deltaMouse = cursor - (windowDimensions / 2.0f;
//        std::cout << __func__ << ", windowDimensions: " << windowDimensions[0] << ", " << windowDimensions[1] << std::endl;
//        std::cout << __func__ << ", cursor: " << cursor[0] << ", " << cursor[1] << std::endl;
//        std::cout << __func__ << ", deltaMouse: " << deltaMouse[0] << ", " << deltaMouse[1] << std::endl;


//        glm::mat4 trans;
//        trans = glm::translate(trans, pos);
//
//        horizontalAngle -= float(deltaMouse[1]) * lookSpeed * deltaTime;
//        verticalAngle -= float(deltaMouse[0]) * lookSpeed * deltaTime;
////        horizontalAngle = glm::clamp(horizontalAngle, -180.0f, 180.0f);
////        verticalAngle = glm::clamp(verticalAngle, -80.0f, 80.0f);
//
//        glm::mat4 rot;
//        rot = glm::rotate(rot, horizontalAngle, glm::vec3(0.0f, 1.0f, 0.0f));
//        rot = glm::rotate(rot, verticalAngle, glm::vec3(0.0f, 0.0f, 1.0f));

////        std::cout << __func__ << ", mouse: " << xPos << ", " << yPos << std::endl;
//        std::cout << __func__ << ", deltaMouse: " << deltaMouse[0] << ", " << deltaMouse[1] << std::endl;
////        std::cout << __func__ << ", pos: " << pos << std::endl;
//        std::cout << __func__ << ", angles: " << horizontalAngle << ", " << verticalAngle << std::endl;
////        view = trans * rot;
////        std::cout << __func__ << ", view: " << view << std::endl;
//
//        glm::vec4 dir = rot * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
//        std::cout << __func__ << ", dir: " << dir << std::endl;
//        lookAt(pos + glm::vec3(dir.x, dir.y, dir.z));
    }


    glm::vec3 pos;
    glm::vec3 up;
    glm::vec3 dir;
    float horizontalAngle = 0;
    float verticalAngle = 0;
    float fov = 45;

    glm::mat4 view;
    glm::mat4 proj;

    float speed = 10;
    float lookSpeed = 10;
    float lastUpdateTime = 0;
};

