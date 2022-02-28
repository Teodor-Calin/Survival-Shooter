#pragma once

#include "utils/glm_utils.h"


namespace transform2D
{
    // Translate matrix
    inline glm::mat3 Translate(float tx, float ty)
    {
        // TODO(student): Implement the translation matrix
        return glm::mat3(
            1, 0, 0, 
            0, 1, 0,     
            tx, ty, 1);

    }

    // Scale matrix
    inline glm::mat3 Scale(float scaleX, float scaleY)
    {
        // TODO(student): Implement the scaling matrix
        return glm::mat3(
            scaleX, 0, 0,
            0, scaleY, 0,
            0, 0, 1);

    }

    // Rotate matrix
    inline glm::mat3 Rotate(float radians)
    {
        // TODO(student): Implement the rotation matrix
        return glm::mat3(
            cos(radians), sin(radians), 0,
            -1 * sin(radians), cos(radians), 0,
            0, 0, 1);

    }
}   // namespace transform2D
