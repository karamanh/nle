/**
 * @file frustum.h
 * @brief The six planes of what a camera can see, and what falls inside them.
 *
 * Drawing something the camera is not pointing at costs exactly as much as
 * drawing something it is. A render distance alone cannot tell the two apart:
 * it keeps a sphere around the eye, and most of that sphere is behind you.
 */

#pragma once

#include <glm/glm.hpp>

namespace nle
{

/**
 * @brief The volume a camera can see, as six inward-facing planes.
 *
 * Each plane is stored as xyz = its normal and w = its distance from the
 * origin, normalised so that dot(normal, point) + w is the point's signed
 * distance from the plane in world units. Inside is positive, which is what
 * lets a sphere be tested by its centre and its radius alone.
 */
struct frustum
{
    enum side { left, right, bottom, top, near_side, far_side, sides };

    glm::vec4 planes[sides] = {};

    /**
     * @brief The planes of @p clip, which is projection multiplied by view.
     *
     * Taken from the matrix rather than from the camera's angles so that it
     * is right for whatever the projection happens to be, and so there is
     * one description of what is visible rather than two that can disagree.
     */
    static frustum of(const glm::mat4& clip);

    /**
     * @brief Whether any part of that sphere is inside.
     *
     * Generous on purpose: a sphere straddling a plane is inside, and a
     * sphere is bigger than the thing it stands for. Culling something that
     * should have been drawn is a hole in the world; drawing something that
     * was not needed costs a draw.
     */
    bool holds(const glm::vec3& centre, float radius) const;
};

} // namespace nle
