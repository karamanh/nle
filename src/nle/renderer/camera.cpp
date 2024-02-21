#include "camera.h"

namespace nle
{
    camera::camera()
    {
        m_field_of_view = 75.0f;
        m_near = 0.1f;
        m_far = 100.0f;
    }

    camera::~camera()
    {
    }

    void camera::set_turn_speed(float turn_speed)
    {
        m_turn_speed = turn_speed;
    }

    float camera::turn_speed() const
    {
        return m_turn_speed;
    }

    void camera::set_free_roam(bool free_roam)
    {
        m_free_roam = free_roam;
    }

    bool camera::free_roam() const
    {
        return m_free_roam;
    }

    void camera::set_field_of_view(float fov)
    {
        m_field_of_view = fov;
    }

    float camera::field_of_view() const
    {
        return m_field_of_view;
    }

    void camera::set_near(float near)
    {
        m_near = near;
    }

    float camera::near() const
    {
        return m_near;
    }

    void camera::set_far(float far)
    {
        m_far = far;
    }

    glm::mat4 camera::view_matrix() const
    {
        return glm::lookAt(position(), position() + front(), up());
    }

    nlohmann::json camera::to_json() const
    {
        auto ret = object_3d::to_json();
        ret["turn_speed"] = m_turn_speed;
        ret["speed"] = m_speed;
        ret["free_roam"] = m_free_roam;
        return ret;
    }

    void camera::from_json(const nlohmann::json &j)
    {
        object_3d::from_json(j);
        m_turn_speed = j["turn_speed"];
        m_speed = j["speed"];
        m_free_roam = j["free_roam"];
    }

    float camera::far() const
    {
        return m_far;
    }

} // namespace nle
