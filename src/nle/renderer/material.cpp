#include "material.h"

namespace nle
{

    material::material()
    {
        m_ambient = glm::vec3(1.0f);
        m_diffuse = glm::vec3(1.0f);
        m_specular = glm::vec3(1.0f);
        m_specular_intensity = 0.0f;
        m_shininess = 4.0f;
        m_dissolve = 1.0f;
        m_accept_light = true;
    }

    material::~material()
    {
    }

    void material::set_ambient(glm::vec3 ambient)
    {
        m_ambient = ambient;
    }

    glm::vec3 material::ambient() const
    {
        return m_ambient;
    }

    void material::set_diffuse(glm::vec3 diffuse)
    {
        m_diffuse = diffuse;
    }

    glm::vec3 material::diffuse() const
    {
        return m_diffuse;
    }

    void material::set_specular(glm::vec3 specular)
    {
        m_specular = specular;
    }

    glm::vec3 material::specular() const
    {
        return m_specular;
    }

    void material::set_shininess(float shininess)
    {
        m_shininess = shininess;
    }

    float material::shininess() const
    {
        return m_shininess;
    }

    void material::set_dissolve(float dissolve)
    {
        m_dissolve = dissolve;
    }

    float material::dissolve() const
    {
        return m_dissolve;
    }

    void material::set_accept_light(bool accept)
    {
        m_accept_light = accept;
    }

    bool material::accept_light() const
    {
        return m_accept_light;
    }

    void material::set_id(const std::string &id)
    {
        m_id = id;
    }

    std::string material::id() const
    {
        return m_id;
    }

} // namespace nle
