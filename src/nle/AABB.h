#pragma once

#include <glm/glm.hpp>

namespace nle
{
    class AABB
    {
    private:
        glm::vec3 m_min;
        glm::vec3 m_max;

    public:
        AABB() = default;
        AABB(glm::vec3 min, glm::vec3 max);

        void set_min(glm::vec3 min);
        glm::vec3 min() const;

        void set_max(glm::vec3 max);
        glm::vec3 max() const;

        bool collides_with(const AABB& other);
        AABB get_transformed(glm::mat4 m) const;
    };
} // namespace nle
