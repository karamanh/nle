#include "Material.h"

namespace nle
{
    Material::Material()
        : m_specular_intensity(0.0f),
          m_shininess(32.0f),
          m_accept_light(true),
          m_ambient({1.f, 1.f, 1.f}),
          m_diffuse({1.f, 1.f, 1.f}),
          m_specular({1.f, 1.f, 1.f}),
          m_dissolve(1.f)
    {
    }

    Material::Material(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float shininess)
        : m_ambient(ambient), m_diffuse(diffuse), m_specular(specular), m_shininess(shininess)
    {
    }

    Material::~Material()
    {
    }

    void Material::set_ambient(glm::vec3 ambient)
    {
        m_ambient = ambient;
    }

    glm::vec3 Material::ambient() const
    {
        return m_ambient;
    }

    void Material::set_diffuse(glm::vec3 diffuse)
    {
        m_diffuse = diffuse;
    }

    glm::vec3 Material::diffuse() const
    {
        return m_diffuse;
    }

    void Material::set_specular(glm::vec3 specular)
    {
        m_specular = specular;
    }

    glm::vec3 Material::specular() const
    {
        return m_specular;
    }

    void Material::set_shininess(float shininess)
    {
        m_shininess = shininess;
    }

    float Material::shininess() const
    {
        return m_shininess;
    }

    float Material::dissolve() const
    {
        return m_dissolve;
    }

    void Material::set_dissolve(float dissolve)
    {
        m_dissolve = dissolve;
    }

    void Material::set_accept_light(bool accept)
    {
        m_accept_light = accept;
    }

    bool Material::accept_light() const
    {
        return m_accept_light;
    }
} // namespace nle
