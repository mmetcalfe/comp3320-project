#pragma once
#include <exception>
#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <png++/png.hpp>
#include <jpeglib.h>

#include "utility/debug.h"

namespace NUGL {
    class Texture {
    public:
        Texture() = delete;

        Texture(GLenum unit, GLenum target) {
            glGenTextures(1, &textureId);
            textureUnit = unit;
            textureTarget = target;
        }

        ~Texture() {
            glDeleteTextures(1, &textureId);
        }

        inline void bind() {
            glActiveTexture(textureUnit);
            glBindTexture(textureTarget, textureId);
        }

        inline void loadFromPNG(const std::string& fileName) {
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

            // Set texture data
            bind();
            glTexImage2D(textureTarget, 0, GL_RGB,
                    image.get_width(), image.get_height(),
                    0, GL_RGB, GL_UNSIGNED_BYTE, imgData.data());
        }

        inline void loadFromJPEG(const std::string& fileName) {
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
            if (!pFile)
                throw std::invalid_argument("loadFromJPEG: Invalid fileName");
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
                samples += numSamples * cinfo.image_width * cinfo.num_components;
            }

            // Clean up
            jpeg_finish_decompress(&cinfo);
            jpeg_destroy_decompress(&cinfo);

            // Set texture data
            bind();
            glTexImage2D(textureTarget, 0, GL_RGB,
                    cinfo.image_width, cinfo.image_height,
                    0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
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
