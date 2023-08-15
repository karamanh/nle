#include "Light.h"

namespace nle
{
    Light::Light()
        : m_color(glm::vec3(1.f)), m_ambient_intensity(1.f), m_diffuse_intensity(1.f)
    {
    }

    Light::Light(GLfloat red, GLfloat green, GLfloat blue, GLfloat ambient_intensity,
                 GLfloat diffuse_intensity)
    {
        m_color = glm::vec3(red, green, blue);
        m_ambient_intensity = ambient_intensity;
        m_diffuse_intensity = diffuse_intensity;
    }

    Light::~Light()
    {
    }

    void Light::use(GLfloat ambient_intensity_location, GLfloat ambient_color_location,
                    GLfloat diffuse_intensity_location, GLfloat diffuse_direction_location)
    {
        glUniform3f(ambient_color_location, m_color.x, m_color.y, m_color.z);
        glUniform1f(ambient_intensity_location, m_ambient_intensity);
        glUniform3f(diffuse_direction_location, glm::radians(this->rotation().x), glm::radians(this->rotation().y), glm::radians(this->rotation().z));
        glUniform1f(diffuse_intensity_location, m_diffuse_intensity);
    }

    void Light::set_ambient_intensity(GLfloat intensity)
    {
        m_ambient_intensity = intensity;
    }

    float Light::ambient_intensity()
    {
        return m_ambient_intensity;
    }

    void Light::set_diffuse_intensity(GLfloat intensity)
    {
        m_diffuse_intensity = intensity;
    }

    float Light::diffuse_intensity()
    {
        return m_diffuse_intensity;
    }

    void Light::set_specular_intensity(GLfloat intensity)
    {
        m_specular_intensity = intensity;
    }

    float Light::specular_intensity()
    {
        return m_specular_intensity;
    }

    void Light::set_enabled(bool enabled)
    {
        m_enabled = enabled;
    }

    bool Light::enabled()
    {
        return m_enabled;
    }

    void Light::set_id(const std::string &id)
    {
        Object3D::set_id("light_" + id);
    }

    void Light::set_color(GLfloat red, GLfloat green, GLfloat blue)
    {
        m_color.r = red;
        m_color.g = green;
        m_color.b = blue;
    }

    glm::vec3 Light::color()
    {
        return m_color;
    }

} // namespace nle
