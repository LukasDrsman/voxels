#pragma once

#include <glm/common.hpp>
#include <glm/vec3.hpp>

namespace lp
{
    float lInfNorm(glm::vec3 eps)
    {
        const glm::vec3 epsilon = glm::abs(eps);
        return glm::max(glm::max(epsilon.x, epsilon.y), epsilon.z);
    }
}
