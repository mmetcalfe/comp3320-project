#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace utility {
namespace math {
namespace coordinates {

    // (r, phi, theta) = (distance, bearing, declination)
    inline glm::vec3 sphericalToCartesian(const glm::vec3 spherical) {
        auto r = spherical[0];
        auto sinTheta = std::sin(spherical[1]);
        auto cosTheta = std::cos(spherical[1]);
        auto sinPhi = std::sin(spherical[2]);
        auto cosPhi = std::cos(spherical[2]);

        glm::vec3 cartesian;
        cartesian.x = r * cosTheta * cosPhi;
        cartesian.y = r * sinTheta * cosPhi;
        cartesian.z = r * sinPhi;
        return cartesian;
    }
}
}
}