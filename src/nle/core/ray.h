/**
 * @file ray.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief Rays, for picking things in the world with the mouse.
 * @version 0.1
 * @date 2026-09-19
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include <glm/glm.hpp>

#include <cmath>

namespace nle
{

struct ray
{
    glm::vec3 origin = glm::vec3(0.0f);

    /// expected to be normalised.
    glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);

    glm::vec3 at(float distance) const { return origin + direction * distance; }
};

/**
 * @brief Intersects @p r with the horizontal plane at @p height.
 *
 * @param distance receives the distance along the ray to the hit.
 * @return false when the ray is parallel to the plane or points away from it.
 */
inline bool intersect_horizontal_plane(const ray& r, float height, float& distance)
{
    // Parallel enough that the intersection would be meaningless or enormous.
    if(std::abs(r.direction.y) < 1e-6f)
    {
        return false;
    }

    distance = (height - r.origin.y) / r.direction.y;
    return distance >= 0.0f;
}

} // namespace nle
