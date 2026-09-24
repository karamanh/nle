#include "frustum.h"

namespace nle
{

frustum frustum::of(const glm::mat4& clip)
{
    frustum made;

    // Each plane is a row of the clip matrix added to or taken from the row
    // that produces w. glm is column-major, so a row is the same index taken
    // from each of the four columns.
    const glm::vec4 row_x(clip[0][0], clip[1][0], clip[2][0], clip[3][0]);
    const glm::vec4 row_y(clip[0][1], clip[1][1], clip[2][1], clip[3][1]);
    const glm::vec4 row_z(clip[0][2], clip[1][2], clip[2][2], clip[3][2]);
    const glm::vec4 row_w(clip[0][3], clip[1][3], clip[2][3], clip[3][3]);

    made.planes[left]      = row_w + row_x;
    made.planes[right]     = row_w - row_x;
    made.planes[bottom]    = row_w + row_y;
    made.planes[top]       = row_w - row_y;
    made.planes[near_side] = row_w + row_z;
    made.planes[far_side]  = row_w - row_z;

    // Normalised, so that what comes out of a plane is a distance in world
    // units rather than in whatever scale the matrix happens to carry. A
    // radius is in world units, and the two have to be comparable.
    for(auto& plane : made.planes)
    {
        const float length = glm::length(glm::vec3(plane));

        if(length > 0.0f)
        {
            plane /= length;
        }
    }

    return made;
}

bool frustum::holds(const glm::vec3& centre, float radius) const
{
    for(const auto& plane : planes)
    {
        // Entirely on the outside of any one plane is entirely outside.
        // Being outside several is no different from being outside one.
        if(glm::dot(glm::vec3(plane), centre) + plane.w < -radius)
        {
            return false;
        }
    }

    return true;
}

} // namespace nle
