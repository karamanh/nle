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

    nlohmann::json Light::to_json()
    {
        auto ret = Object3D::to_json();
        ret["type"] = 3;
        ret["color"]["r"] = m_color.r;
        ret["color"]["g"] = m_color.g;
        ret["color"]["b"] = m_color.b;
        ret["ambient_intensity"] = m_ambient_intensity;
        ret["diffuse_intensity"] = m_diffuse_intensity;
        ret["enabled"] = m_enabled;
        return ret;
    }

    void Light::from_json(const nlohmann::json &j)
    {
        Object3D::from_json(j);
        m_color.r = j["color"]["r"];
        m_color.g = j["color"]["g"];
        m_color.b = j["color"]["b"];
        m_ambient_intensity = j["ambient_intensity"];
        m_diffuse_intensity = j["diffuse_intensity"];
        m_enabled = j["enabled"];
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
