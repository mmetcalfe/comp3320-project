#pragma once

#include <iostream>
#include <string>
#include <sstream>

namespace utility {

    class FrameTimer {
    public:
        inline FrameTimer(double currentTime) : lastTime(currentTime), nbFrames(0), fps(1), timeStr("") { }

        inline bool frameUpdate(double currentTime) {
            nbFrames++;
            if (currentTime - lastTime >= 1.0) { // If last print was more than 1 sec ago
                fps = double(nbFrames);
                std::stringstream title;
                title << "OpenGL | " << (1000.0 / fps) << " ms/frame (" << fps << "fps)";
                nbFrames = 0;
                lastTime += 1.0;
//                glfwSetWindowTitle(window, title.str().c_str());
                timeStr = title.str();
                return true;
            }

            return false;
        }

        double lastTime;
        int nbFrames;
        double fps;
        std::string timeStr;
    };

}
