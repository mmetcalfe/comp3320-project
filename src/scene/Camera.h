#pragma once

#include <memory>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "utility/debug.h"
#include "utility/math/coordinates.h"
#include "scene/Light.h"
#include "NUGL/Texture.h"

namespace scene {

    class Camera {
    public:
        inline void lookAt(glm::vec3 lookAtPos) {
            view = glm::lookAt(pos, lookAtPos, up);
        };

        inline void prepareTransforms() {
            lookAt(pos + dir);
            if (useOrtho) {
                float ratio = frameWidth / float(frameHeight);
                float right = orthoWidth * 0.5;
                float top = right / ratio;
                proj = glm::ortho(-right, right, -top, top, near_, far_);
            } else {
                proj = glm::perspective(fov, frameWidth / float(frameHeight), near_, far_);
            }
        }

        glm::vec3 pos;
        glm::vec3 dir;
        glm::vec3 up;
        float fov = M_PI_4;
        float near_ = 5.0f;
        float far_ = 300.0f;

        int frameWidth = 800;
        int frameHeight = 600;
        bool useOrtho = false;
        float orthoWidth = 5;

        glm::mat4 view;
        glm::mat4 proj;
    };

    class LightCamera : public Camera {
    public:
        static inline std::shared_ptr<LightCamera> fromLight(scene::Light &light, int frameSize) {
            auto camera = std::make_shared<LightCamera>();
            camera->pos = light.pos;
            camera->dir = light.dir;
            camera->up = {0, 0, 1};
            if (std::abs(glm::dot(camera->dir, camera->up)) > 0.9) {
                camera->up = {0, 1, 0};
            }
            camera->fov = light.angleConeOuter;
            camera->frameWidth = frameSize;
            camera->frameHeight = frameSize;

            if (light.type == Light::Type::directional) {
                camera->useOrtho = true;
                camera->orthoWidth = light.orthoSize;
            }

            camera->prepareTransforms();

            return camera;
        }

        std::shared_ptr<NUGL::Texture> shadowMap;
    };

    class PlayerCamera : public Camera {
    public:
        inline void processKeyboardInput(GLFWwindow *window, float deltaTime) {
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

            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
                move *= 5;

            if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL))
                move *= 0.2;

            glm::vec3 right = glm::cross(dir, up);
            pos += dir * move.x * speed * deltaTime;
            pos += right * move.y * speed * deltaTime;
        }

        inline void processMouseInput(GLFWwindow *window, float deltaTime) {
            double xPos, yPos;
            glfwGetCursorPos(window, &xPos, &yPos);
            glm::vec2 cursor(xPos, yPos);
            static glm::vec2 lastCursor = cursor;
            glm::vec2 deltaMouse = cursor - lastCursor;
            lastCursor = cursor;

            // Note: glfwSetCursorPos does not work on OSX.
            // As a workaround we have set GLFW_CURSOR_DISABLED.
//        glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);

            horizontalAngle -= float(deltaMouse[0]) * lookSpeed;
            verticalAngle -= float(deltaMouse[1]) * lookSpeed;
            verticalAngle = glm::clamp<float>(verticalAngle, -M_PI_2 * 0.9, M_PI_2 * 0.9);

            dir = utility::math::coordinates::sphericalToCartesian(glm::vec3(1, horizontalAngle, verticalAngle));
        }

        inline void processPlayerInput(GLFWwindow *window) {
            float currentTime = glfwGetTime();
            float deltaTime = float(currentTime - lastUpdateTime);
            lastUpdateTime = currentTime;

            processKeyboardInput(window, deltaTime);
            processMouseInput(window, deltaTime);

            lookAt(pos + dir);
        }

        inline void initializeAngles() {
            glm::vec3 spherical = utility::math::coordinates::cartesianToSpherical(dir);

            horizontalAngle = spherical[1];
            verticalAngle = spherical[2];
        }

        float horizontalAngle = 0;
        float verticalAngle = 0;

        float speed = 10;
        float lookSpeed = 0.005;
        float lastUpdateTime = 0;
    };

}

