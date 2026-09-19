#include "camera.h"

namespace nle
{
    camera::camera()
    {
        m_field_of_view = 45.0f;
        m_near = 0.1f;
        m_far = 1000.0f;
        m_turn_speed = 1.0f;
        m_free_roam = false;
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

    glm::mat4 camera::projection_matrix(float aspect_ratio) const
    {
        if(aspect_ratio <= 0.0f)
        {
            aspect_ratio = 1.0f;
        }

        return glm::perspective(m_field_of_view, aspect_ratio, m_near, m_far);
    }

    ray camera::screen_ray(const glm::vec2& pixel, const glm::vec2& resolution) const
    {
        const glm::vec2 size = glm::max(resolution, glm::vec2(1.0f));

        // Pixels (top-left origin) to normalised device coordinates, where y
        // points the other way and the range is [-1, 1].
        const float x = (2.0f * pixel.x) / size.x - 1.0f;
        const float y = 1.0f - (2.0f * pixel.y) / size.y;

        const glm::mat4 inverse_view_projection =
            glm::inverse(projection_matrix(size.x / size.y) * view_matrix());

        // Unproject the near and far plane points and join them.
        glm::vec4 near_point = inverse_view_projection * glm::vec4(x, y, -1.0f, 1.0f);
        glm::vec4 far_point = inverse_view_projection * glm::vec4(x, y, 1.0f, 1.0f);

        near_point /= near_point.w;
        far_point /= far_point.w;

        ray result;
        result.origin = glm::vec3(near_point);
        result.direction = glm::normalize(glm::vec3(far_point - near_point));
        return result;
    }

    bool camera::world_to_screen(const glm::vec3& world, const glm::vec2& resolution, glm::vec2& out) const
    {
        const glm::vec2 size = glm::max(resolution, glm::vec2(1.0f));

        const glm::vec4 clip = projection_matrix(size.x / size.y) * view_matrix() * glm::vec4(world, 1.0f);

        // w is the view-space depth; behind the camera it is zero or negative
        // and the divide would mirror the point back onto the screen.
        if(clip.w <= 0.0f)
        {
            return false;
        }

        const glm::vec3 ndc = glm::vec3(clip) / clip.w;

        out.x = (ndc.x * 0.5f + 0.5f) * size.x;
        out.y = (1.0f - (ndc.y * 0.5f + 0.5f)) * size.y;
        return true;
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
