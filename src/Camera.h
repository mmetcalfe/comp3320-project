#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "utility/debug.h"
#include "utility/math/coordinates.h"

class Camera {
public:
    inline void lookAt(glm::vec3 lookAtPos) {
        view = glm::lookAt(pos, lookAtPos, up);
    };

    inline void processKeyboardInput(GLFWwindow* window, float deltaTime) {
        glm::vec3 move = {0, 0, 0};
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            move.x += 1;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            move.x -= 1;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            move.y += 1;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            move.y -= 1;
        }

        if (glm::length(move) > 0) {
            move = glm::normalize(move);
        }

        glm::vec3 right = glm::cross(dir, up);
        pos += dir * move.x * speed * deltaTime;
        pos += right * move.y * speed * deltaTime;
    }

    inline void processMouseInput(GLFWwindow* window, float deltaTime) {
        double xPos, yPos;
        glfwGetCursorPos(window, &xPos, &yPos);
        glm::vec2 cursor(xPos, yPos);
        static glm::vec2 lastCursor = cursor;
        glm::vec2 deltaMouse = cursor - lastCursor;
        lastCursor = cursor;

        // Note: glfwSetCursorPos does not work on OSX.
        // As a workaround we have set GLFW_CURSOR_DISABLED.
//        glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);

        horizontalAngle -= float(deltaMouse[0]) * lookSpeed * deltaTime;
        verticalAngle -= float(deltaMouse[1]) * lookSpeed * deltaTime;
        verticalAngle = glm::clamp<float>(verticalAngle, -M_PI_2 * 0.9, M_PI_2 * 0.9);

        dir = utility::math::coordinates::sphericalToCartesian(glm::vec3(1, horizontalAngle, verticalAngle));
    }

    inline void processPlayerInput(GLFWwindow* window) {
        float currentTime = glfwGetTime();
        float deltaTime = float(currentTime - lastUpdateTime);
        lastUpdateTime = currentTime;

        processKeyboardInput(window, deltaTime);
        processMouseInput(window, deltaTime);

        lookAt(pos + dir);
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
    float lookSpeed = 0.5;
    float lastUpdateTime = 0;
};

