#pragma once

#include <iomanip>
#include <sstream>

inline const std::string getGLErrorName(GLenum error) {
    switch (error) {
        case GL_NO_ERROR: return "GL_NO_ERROR";
        case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
        case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
        default:
            std::stringstream errMsg;
            errMsg << "Unknown GL Error (" << error << ")";
            return errMsg.str();
//            throw std::invalid_argument(errMsg.str());
    }
}

inline void checkForAndPrintGLError(const std::string& label) {
    auto err = glGetError();
    if (err) {
        std::cerr << "[" << label << "] GL Error: " << getGLErrorName(err) << std::endl;
    }
}

inline void checkForAndPrintGLError(int num) {
    auto err = glGetError();
    if (err) {
        std::cerr << "[" << num << "] " << getGLErrorName(err) << std::endl;
    }
}

inline void checkForAndPrintGLError(const std::string& file, int line) {
    auto err = glGetError();
    if (err) {
        std::cerr << "[" << file << ", " << line << "] " << getGLErrorName(err) << std::endl;
    }
}

inline void checkForAndPrintGLError(const std::string& file, int line, const std::string& info) {
    auto err = glGetError();
    if (err) {
        std::cerr << "[" << file << ", " << line << ": " << info << "] " << getGLErrorName(err) << std::endl;
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
