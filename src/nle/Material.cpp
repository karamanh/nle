#include "Material.h"

namespace nle
{
    Material::Material()
        : m_specular_intensity(0.0f),
          m_shininess(0.0f),
          m_accept_light(true)
    {
    }

    Material::Material(float specular_intensity, float shininess)
    {
        m_specular_intensity = specular_intensity;
        m_shininess = shininess;
    }

    Material::~Material()
    {
    }

    void Material::use(float specular_intensity_location, float shininess_location)
    {
        glUniform1f(specular_intensity_location, m_specular_intensity);
        glUniform1f(shininess_location, m_shininess);
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

    void Material::set_specular_intensity(float specular_intensity)
    {
        m_specular_intensity = specular_intensity;
    }

    float Material::specular_intensity() const
    {
        return m_specular_intensity;
    }

    void Material::set_shininess(float shininess)
    {
        m_shininess = shininess;
    }

    float Material::shininess() const
    {
        return m_shininess;
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
