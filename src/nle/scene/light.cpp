#include "light.h"

#include <algorithm>

namespace nle
{
    light::light()
    {
        m_ambient = glm::vec3(1.0f);
        m_diffuse = glm::vec3(1.0f);
        m_specular = glm::vec3(1.0f);
        m_color = glm::vec3(1.0f);
    }

    light::light(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular)
    {
        m_ambient = ambient;
        m_diffuse = diffuse;
        m_specular = specular;
        m_color = glm::vec3(1.0f);
    }

    light::~light()
    {
    }

    enum light_type light::light_type() const
    {
        return light_type::directional;
    }

    void light::set_color(glm::vec3 color)
    {
        m_color = color;
    }

    glm::vec3 light::color() const
    {
        return m_color;
    }

    void light::set_ambient(glm::vec3 ambient)
    {
        m_ambient = ambient;
    }

    glm::vec3 light::ambient() const
    {
        return m_ambient;
    }

    void light::set_diffuse(glm::vec3 diffuse)
    {
        m_diffuse = diffuse;
    }

    glm::vec3 light::diffuse() const
    {
        return m_diffuse;
    }

    void light::set_specular(glm::vec3 specular)
    {
        m_specular = specular;
    }

    glm::vec3 light::specular() const
    {
        return m_specular;
    }

    void light::set_enabled(bool enabled)
    {
        m_enabled = enabled;
    }

    bool light::enabled() const
    {
        return m_enabled;
    }

    void light::set_intensity(float intensity)
    {
        m_intensity = intensity;
    }

    float light::intensity() const
    {
        return m_intensity;
    }

    directional_light_data light::to_directional_light_data() const
    {
        directional_light_data data;
        data.direction = front();
        data.color = m_color * m_intensity;
        data.ambient = m_ambient;
        data.diffuse = m_diffuse;
        data.specular = m_specular;
        data.enabled = m_enabled;
        return data;
    }

    point_light::point_light()
    {
        /// a lamp-sized light by default.
        set_ambient(glm::vec3(0.0f));
        set_range(50.0f);
    }

    point_light::point_light(glm::vec3 color, float range)
    {
        set_ambient(glm::vec3(0.0f));
        set_color(color);
        set_range(range);
    }

    point_light::~point_light()
    {
    }

    enum light_type point_light::light_type() const
    {
        return light_type::point;
    }

    void point_light::set_range(float range)
    {
        m_range = std::max(range, 0.0001f);

        /// Ogre3D's attenuation table, condensed to a continuous fit. At
        /// m_range the light has decayed to roughly 1/80 of its peak.
        m_constant = 1.0f;
        m_linear = 4.5f / m_range;
        m_quadratic = 75.0f / (m_range * m_range);
    }

    float point_light::range() const
    {
        return m_range;
    }

    void point_light::set_constant(float constant)
    {
        m_constant = constant;
    }

    float point_light::constant() const
    {
        return m_constant;
    }

    void point_light::set_linear(float linear)
    {
        m_linear = linear;
    }

    float point_light::linear() const
    {
        return m_linear;
    }

    void point_light::set_quadratic(float quadratic)
    {
        m_quadratic = quadratic;
    }

    float point_light::quadratic() const
    {
        return m_quadratic;
    }

    float point_light::attenuation_at(float distance) const
    {
        const float denominator = m_constant + m_linear * distance + m_quadratic * distance * distance;
        if (denominator <= 0.0f)
        {
            return 0.0f;
        }
        return 1.0f / denominator;
    }

    point_light_data point_light::to_point_light_data() const
    {
        point_light_data data;
        data.position = position();
        data.color = color() * intensity();
        data.ambient = ambient();
        data.diffuse = diffuse();
        data.specular = specular();
        data.constant = m_constant;
        data.linear = m_linear;
        data.quadratic = m_quadratic;
        data.range = m_range;
        return data;
    }

} // namespace nle
