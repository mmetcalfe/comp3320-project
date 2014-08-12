#pragma once

#include <iomanip>

inline void printGLError(GLenum error) {
    printf("%s", "GL Error: ");
    switch (error) {
        case GL_NO_ERROR: printf("%s\n", "GL_NO_ERROR"); break;
        case GL_INVALID_ENUM: printf("%s\n", "GL_INVALID_ENUM"); break;
        case GL_INVALID_VALUE: printf("%s\n", "GL_INVALID_VALUE"); break;
        case GL_INVALID_OPERATION: printf("%s\n", "GL_INVALID_OPERATION"); break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: printf("%s\n", "GL_INVALID_FRAMEBUFFER_OPERATION"); break;
        case GL_OUT_OF_MEMORY: printf("%s\n", "GL_OUT_OF_MEMORY"); break;
        default: printf("%s (%d)\n", "Unknown error", error); break;
    }
}

inline void checkForAndPrintGLError(const std::string label) {
    auto err = glGetError();
    if (err) {
        printf("[%s] ", label.c_str());
        printGLError(err);
    }
}

inline void checkForAndPrintGLError(int num) {
    auto err = glGetError();
    if (err) {
        printf("[%d] ", num);
        printGLError(err);
    }
}

inline void checkForAndPrintGLError(const char* file, int line) {
    auto err = glGetError();
    if (err) {
        printf("[%s, %d] ", file, line);
        printGLError(err);
    }
}

// GLM printing:
inline std::ostream &operator<< (std::ostream &out, const glm::vec2 &vec) {
    out << "[" << vec.x << ", " << vec.y << "]";
    return out;
}
inline std::ostream &operator<< (std::ostream &out, const glm::vec3 &vec) {
    out << "[" << vec.x << ", " << vec.y << ", "<< vec.z << "]";
    return out;
}
inline std::ostream &operator<< (std::ostream &out, const glm::vec4 &vec) {
    out << "[" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << "]";
    return out;
}

inline std::ostream &operator<< (std::ostream &out, const glm::mat3 &mat) {
    out << std::endl;
    for (int row = 0; row < 3; row++) {
        out << "    ";
        out << "[";
        for (int col = 0; col < 3; col++) {
            out << std::setw(6) << std::setprecision(3) << mat[col][row];
            if (col < 2)
                out << ", ";
        }
        out << "]";
        if (row < 2)
            out << ",";
        out << std::endl;
    }

    return out;
}

inline std::ostream &operator<< (std::ostream &out, const glm::mat4 &mat) {
    out << std::endl;
    for (int row = 0; row < 4; row++) {
        out << "    ";
        out << "[";
        for (int col = 0; col < 4; col++) {
            out << std::setw(6) << std::setprecision(3) << mat[col][row];
            if (col < 3)
                out << ", ";
        }
        out << "]";
        if (row < 3)
            out << ",";
        out << std::endl;
    }

    return out;
}
