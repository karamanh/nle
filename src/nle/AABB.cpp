#include "AABB.h"

namespace nle
{

    AABB::AABB(glm::vec3 min, glm::vec3 max)
        : m_min(min), m_max(max)
    {
    }

    void AABB::set_min(glm::vec3 min)
    {
        m_min = min;
    }

    glm::vec3 AABB::min() const
    {
        return m_min;
    }

    void AABB::set_max(glm::vec3 max)
    {
        m_max = max;
    }

    glm::vec3 AABB::max() const
    {
        return m_max;
    }

    bool AABB::collides_with(const AABB &other)
    {
        return (
            m_max.x > other.m_min.x &&
            m_min.x < other.m_max.x &&
            m_max.y > other.m_min.y &&
            m_min.y < other.m_max.y &&
            m_max.z > other.m_min.z &&
            m_min.z < other.m_max.z
        );
    }

} // namespace nle
