#include "light.h"

namespace nle
{
    light::light()
    {
    }

    light::light(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular)
    {
        m_ambient = ambient;
        m_diffuse = diffuse;
        m_specular = specular;
    }

    light::~light()
    {
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

} // namespace nle
