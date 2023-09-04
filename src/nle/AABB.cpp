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
    
    AABB AABB::get_transformed(glm::mat4 m) const
    {
        AABB res;
        glm::vec3 corners[8];

        corners[0] = m_min;
        corners[1] = glm::vec3(m_min.x, m_max.y, m_min.z);
        corners[2] = glm::vec3(m_min.x, m_max.y, m_max.z);
        corners[3] = glm::vec3(m_min.x, m_min.y, m_max.z);
        corners[4] = glm::vec3(m_max.x, m_min.y, m_min.z);
        corners[5] = glm::vec3(m_max.x, m_max.y, m_min.z);
        corners[6] = m_max;
        corners[7] = glm::vec3(m_max.x, m_min.y, m_max.z);


        glm::vec4 tmp  = (m * glm::vec4(corners[0],1.0));
        glm::vec3 tmin = glm::vec3(tmp.x, tmp.y, tmp.z);
        glm::vec3 tmax = tmin;

        // transform the other 7 corners and compute the result AABB
        for(int i = 1; i < 8; i++)
        {
            tmp = (m * glm::vec4(corners[i],1.0));
            glm::vec3 point = glm::vec3(tmp.x, tmp.y, tmp.z);

            tmin = glm::min(tmin, point);
            tmax = glm::max(tmax, point);
        }        
        
        res.set_min(tmin);
        res.set_max(tmax);
        
        return res;
    }

} // namespace nle
