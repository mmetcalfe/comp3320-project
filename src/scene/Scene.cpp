#include "scene/Scene.h"
#include "NUGL/Framebuffer.h"
#include "NUGL/Renderbuffer.h"

namespace scene {

    void Scene::render(std::shared_ptr<NUGL::ShaderProgram> screenProgram) {

        NUGL::Framebuffer fbo;

        std::shared_ptr<NUGL::Texture> tex = std::make_shared<NUGL::Texture>(GL_TEXTURE0, GL_TEXTURE_2D);
        tex->setTextureData(GL_TEXTURE_2D, 800, 600, nullptr);
        checkForAndPrintGLError(__FILE__, __LINE__);
        tex->setParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        tex->setParam(GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        fbo.attach(tex);

        NUGL::Renderbuffer rbo;
        rbo.setStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);

        fbo.attach(rbo);

//        std::cout << __FILE__ << ", " << __LINE__ << ": " << getFramebufferStatusString(glCheckFramebufferStatus(GL_FRAMEBUFFER)) << std::endl;

        NUGL::Framebuffer::useDefault();

        glClearColor(0.5, 0.5, 0.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (auto model : models) {
            model->draw(*camera);
        }

        fbo.bind();

        glClearColor(0.5, 0.5, 0.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (auto model : models) {
            model->draw(*camera);
        }

        NUGL::Framebuffer::useDefault();

        std::vector<GLfloat> quadVertices = {
            -1.0f,  1.0f,  0.0f, 1.0f,
            1.0f,  1.0f,  1.0f, 1.0f,
            1.0f, -1.0f,  1.0f, 0.0f,

            1.0f, -1.0f,  1.0f, 0.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
            -1.0f,  1.0f,  0.0f, 1.0f
        };
        NUGL::Buffer quadBuffer;
        quadBuffer.setData(GL_ARRAY_BUFFER, quadVertices, GL_STATIC_DRAW);

        std::vector<NUGL::VertexAttribute> attribs = {
                {"position", 2, GL_FLOAT, GL_FALSE},
                {"texcoord", 2, GL_FLOAT, GL_FALSE},
        };
//        checkForAndPrintGLError(__FILE__, __LINE__);


        tex->bind();
        checkForAndPrintGLError(__FILE__, __LINE__);

//        screenProgram->setUniform("texDiffuse", tex);
        screenProgram->use();

        auto vertexArray = std::make_unique<NUGL::VertexArray>();
        vertexArray->bind();
        vertexArray->setAttributePointers(*screenProgram, quadBuffer, GL_ARRAY_BUFFER, attribs);

        screenProgram->use();
        vertexArray->bind();
        quadBuffer.bind(GL_ARRAY_BUFFER);
        glDisable(GL_DEPTH_TEST);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glEnable(GL_DEPTH_TEST);
    }

    void Scene::addModel(std::shared_ptr<Model> model) {
        models.push_back(model);

        for (auto light : model->lights) {
            std::weak_ptr<Light> weak(light);
            lights.push_back(weak);
        }
    }
}
