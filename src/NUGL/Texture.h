#pragma once
#include <stdexcept>
#include <vector>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <png++/png.hpp>
#include <jpeglib.h>

#include <boost/filesystem.hpp>

#include "utility/debug.h"

namespace NUGL {
    class Texture {
    public:
        Texture() = delete;

        inline Texture(GLenum unit, GLenum target) {
            glGenTextures(1, &textureId);
            textureUnit = unit;
            textureTarget = target;
        }

        inline ~Texture() {
            glDeleteTextures(1, &textureId);
        }

        inline void bind() {
            glActiveTexture(textureUnit);
            checkForAndPrintGLError(__FILE__, __LINE__);
            glBindTexture(textureTarget, textureId);
            checkForAndPrintGLError(__FILE__, __LINE__);
        }

        //! Face order is: +X, -X, +Y, -Y, +Z, -Z.
        inline void loadCubeMap(std::vector<std::string> faceFileNames) {
            if (faceFileNames.size() != 6) {
                std::stringstream errMsg;
                errMsg << __func__
                       << ": faceFileNames must have a size of 6. (faceFileNames.size() == "
                       << faceFileNames.size() << ").";
                throw std::invalid_argument(errMsg.str());
            }

            bind();
            for (int i = 0; i < 6; i++) {
                loadFromImage(faceFileNames[i], GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);
                checkForAndPrintGLError(__func__, __LINE__);
            }
        }

        inline void loadFromImage(const std::string& fileName) {
            loadFromImage(fileName, textureTarget);
        }

        inline void loadFromPNG(const std::string& fileName) {
            loadFromPNG(fileName, textureTarget);
        }

        inline void loadFromJPEG(const std::string& fileName) {
            loadFromJPEG(fileName, textureTarget);
        }

        inline void loadFromImage(const std::string& fileName, GLenum target) {
            if (!boost::filesystem::exists(fileName)) {
                std::stringstream errMsg;
                errMsg << __func__ << ": The file '" << fileName << "' does not exist.";
                throw std::invalid_argument(errMsg.str());
            }

            if (utility::strutil::checkFirstBytes(fileName, "\xFF\xD8\xFF")) {
                loadFromJPEG(fileName, target);
            } else if (utility::strutil::checkFirstBytes(fileName, "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A")) {
                loadFromPNG(fileName, target);
            } else {
                std::stringstream errMsg;
                errMsg << __func__
                    << ": The file '" << fileName << "' has unrecognised image file type.";
                throw std::invalid_argument(errMsg.str());
            }

            checkForAndPrintGLError(__FILE__, __LINE__);
        }

        inline void setTextureData(GLenum target, GLsizei width, GLsizei height, const GLvoid *pixels,
                GLenum format = GL_RGB, GLenum internalFormat = GL_RGB) {
            bind();
            if ((width % 4) == 0) {
                glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            } else {
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            }
            glTexImage2D(target, 0, internalFormat,
                    width, height,
                    0, format, GL_UNSIGNED_BYTE, pixels); // TODO: Check byte formats
        }

        inline void loadFromPNG(const std::string& fileName, GLenum target) {
            // Open image
            png::image<png::rgb_pixel> image(fileName.c_str());

            // Copy image data to buffer
            std::vector<unsigned char> imgData;
            imgData.reserve(image.get_width() * image.get_height() * 3);
            for (size_t y = 0; y < image.get_height(); y++) {
                for (size_t x = 0; x < image.get_width(); x++) {
                    auto& pixel = image[y][x];
                    imgData.push_back(pixel.red);
                    imgData.push_back(pixel.green);
                    imgData.push_back(pixel.blue);
                }
            }

            setTextureData(target, image.get_width(), image.get_height(), imgData.data());
        }

        inline void loadFromJPEG(const std::string& fileName, GLenum target) {
            struct jpeg_error_mgr err;
            struct jpeg_decompress_struct cinfo;
            std::memset(&cinfo, 0, sizeof(jpeg_decompress_struct));

            // Create decompressor
            jpeg_create_decompress(&cinfo);
            cinfo.err = jpeg_std_error(&err);

            // Set options
            cinfo.do_fancy_upsampling = false;
            cinfo.out_color_components = 3;
            cinfo.out_color_space = JCS_RGB;

            // Set source buffer
            FILE* pFile = fopen(fileName.c_str(), "rb");
            if (!pFile) {
                throw std::invalid_argument("loadFromJPEG: Invalid fileName");
            }
            jpeg_stdio_src(&cinfo, pFile);

            // Read jpeg header
            jpeg_read_header(&cinfo, true);

            // Decompress
            jpeg_start_decompress(&cinfo);

            // Read scanlines
            std::vector<char> data(cinfo.image_width * cinfo.image_height * cinfo.num_components);
            char* samples = data.data();
            while (cinfo.output_scanline < cinfo.output_height) {
                int numSamples = jpeg_read_scanlines(&cinfo, (JSAMPARRAY)&samples, 1);
                samples += numSamples * cinfo.output_width * cinfo.output_components;
            }

            // Clean up
            jpeg_finish_decompress(&cinfo);

            jpeg_destroy_decompress(&cinfo);

            setTextureData(target, cinfo.image_width, cinfo.image_height, data.data());
        }

        inline void setParam(GLenum param, GLint value) {
            glTexParameteri(textureTarget, param, value);
        }

        inline GLuint id() {
            return textureId;
        }

        inline GLenum target() {
            return textureTarget;
        }

        inline GLenum unit() {
            return textureUnit;
        }

    private:
        GLuint textureId;
        GLenum textureUnit;
        GLenum textureTarget;
    };
}
