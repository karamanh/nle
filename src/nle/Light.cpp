#include "Light.h"

namespace nle
{
    Light::Light()
        : m_color(glm::vec3(1.f)), m_ambient(glm::vec3(1.f)), m_diffuse(glm::vec3(1.f)), m_specular(glm::vec3(1.f))
    {
        m_type = ObjectType::Light;
    }

    Light::Light(glm::vec3 color, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular)
        : m_color(color), m_ambient(ambient), m_diffuse(diffuse), m_specular(specular)
    {
    }

    Light::~Light()
    {
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
        Object3D::set_id(id);
    }

    nlohmann::json Light::to_json()
    {
        auto ret = Object3D::to_json();
        ret["color"]["r"] = m_color.r;
        ret["color"]["g"] = m_color.g;
        ret["color"]["b"] = m_color.b;
        
        ret["ambient"]["x"] = m_ambient.x;
        ret["ambient"]["y"] = m_ambient.y;
        ret["ambient"]["z"] = m_ambient.z;

        ret["diffuse"]["x"] = m_diffuse.x;
        ret["diffuse"]["y"] = m_diffuse.y;
        ret["diffuse"]["z"] = m_diffuse.z;

        ret["specular"]["x"] = m_specular.x;
        ret["specular"]["y"] = m_specular.y;
        ret["specular"]["z"] = m_specular.z;

        ret["enabled"] = m_enabled;
        return ret;
    }

    void Light::from_json(const nlohmann::json &j)
    {
        Object3D::from_json(j);
        m_color.r = j["color"]["r"];
        m_color.g = j["color"]["g"];
        m_color.b = j["color"]["b"];

        m_ambient.x = j["ambient"]["x"];
        m_ambient.y = j["ambient"]["y"];
        m_ambient.z = j["ambient"]["z"];

        m_diffuse.x = j["diffuse"]["x"];
        m_diffuse.y = j["diffuse"]["y"];
        m_diffuse.z = j["diffuse"]["z"];

        m_specular.x = j["specular"]["x"];
        m_specular.y = j["specular"]["y"];
        m_specular.z = j["specular"]["z"];

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

    void Light::set_ambient(glm::vec3 ambient)
    {
        m_ambient = ambient;
    }

    glm::vec3 Light::ambient() const
    {
        return m_ambient;
    }

    void Light::set_diffuse(glm::vec3 diffuse)
    {
        m_diffuse = diffuse;
    }

    glm::vec3 Light::diffuse() const
    {
        return m_diffuse;
    }

    void Light::set_specular(glm::vec3 specular)
    {
        m_specular = specular;
    }

    glm::vec3 Light::specular() const
    {
        return m_specular;
    }

} // namespace nle
