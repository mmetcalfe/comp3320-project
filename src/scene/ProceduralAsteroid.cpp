#include "scene/ProceduralAsteroid.h"

#include <glm/glm.hpp>
#include <iostream>
#include <utility>
#include <map>
#include <random>
#include <cmath>
#include <cstdlib>

namespace scene {

static int getMidpointVertex(std::mt19937 &gen, scene::Mesh &mesh,
    std::map<std::tuple<GLint, GLint>, GLint> &lines, GLint a, GLint b) {

    std::uniform_real_distribution<float> distrib(-1, 1);

    auto search = lines.find(std::make_tuple(a, b));
    if (search != lines.end()) {
        return search->second;
    } else {
        search = lines.find(std::make_tuple(b, a));
        if (search != lines.end()) {
            return search->second;
        } else {
            auto midpoint = glm::normalize((mesh.vertices[a] + mesh.vertices[b]) / 2.f);

            float a_height = glm::length(mesh.vertices[a]);
            float b_height = glm::length(mesh.vertices[b]);
            float dist = glm::distance(mesh.vertices[a], mesh.vertices[b]);
            float ab_height = ((a_height + b_height) / 2.f) + 0.05f * dist * distrib(gen);

            GLint ab = mesh.vertices.size();
            mesh.vertices.push_back(midpoint * ab_height);
            mesh.normals.push_back(midpoint);

            lines[std::make_tuple(a, b)] = ab;
            return ab;
        }
    }
}

static void subdivide(std::mt19937 &gen, scene::Mesh &mesh) {
    std::map<std::tuple<GLint, GLint>, GLint> lines;
    std::vector<GLint> newElements;

    for (unsigned faceIndex = 0; faceIndex < mesh.elements.size(); faceIndex += 3) {
        GLint a = mesh.elements[faceIndex];
        GLint b = mesh.elements[faceIndex + 1];
        GLint c = mesh.elements[faceIndex + 2];

        GLint ab = getMidpointVertex(gen, mesh, lines, a, b);
        GLint bc = getMidpointVertex(gen, mesh, lines, b, c);
        GLint ac = getMidpointVertex(gen, mesh, lines, a, c);

        newElements.push_back(a); newElements.push_back(ab); newElements.push_back(ac);
        newElements.push_back(ab); newElements.push_back(b); newElements.push_back(bc);
        newElements.push_back(ab); newElements.push_back(bc); newElements.push_back(ac);
        newElements.push_back(ac); newElements.push_back(bc); newElements.push_back(c);
    }

    mesh.elements = newElements;
}

std::shared_ptr<Model> createAsteroid() {
    auto model = scene::Model::createIcosahedron();
    Mesh &mesh = model->meshes[0];

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> distrib(-1, 1);

    for (unsigned i = 0; i < mesh.vertices.size(); i ++)
        mesh.vertices[i] = mesh.vertices[i] + 0.1f * distrib(mt);

    for (int i = 0; i < 4; i ++)
        subdivide(mt, mesh);

    for (unsigned i = 0; i < mesh.vertices.size(); i ++) {
        glm::vec3 normal = { 0, 0, 0 };

        for (unsigned faceIndex = 0; faceIndex < mesh.elements.size(); faceIndex += 3) {
            GLint a = mesh.elements[faceIndex];
            GLint b = mesh.elements[faceIndex + 1];
            GLint c = mesh.elements[faceIndex + 2];

            if (i == a || i == b || i == c)
                normal += glm::cross(mesh.vertices[b] - mesh.vertices[a],
                                     mesh.vertices[c] - mesh.vertices[a]);
        }

        mesh.normals[i] = glm::normalize(normal);
    }

    return model;
}

}
