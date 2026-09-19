#include "skeleton.h"

#include <glm/gtc/matrix_transform.hpp>

namespace nle
{

glm::mat4 skeleton_node::local_matrix() const
{
    return glm::translate(glm::mat4(1.0f), translation)
         * glm::mat4_cast(rotation)
         * glm::scale(glm::mat4(1.0f), scale);
}

int skeleton::find_node(const std::string& name) const
{
    for(size_t i = 0; i < m_nodes.size(); ++i)
    {
        if(m_nodes[i].name == name)
        {
            return static_cast<int>(i);
        }
    }

    return -1;
}

} // namespace nle
