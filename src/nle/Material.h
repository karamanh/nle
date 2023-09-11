#ifndef NLE_MATERIAL_H
#define NLE_MATERIAL_H

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <string>

namespace nle
{
    class Material
    {
    private:
        glm::vec3 m_ambient;
        glm::vec3 m_diffuse;
        glm::vec3 m_specular;
        float m_specular_intensity;
        float m_shininess;
        float m_dissolve;
        bool m_accept_light = true;
        std::string m_name;
    public:
        Material();
        Material(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float shininess);
        ~Material();

        void set_ambient(glm::vec3 ambient); 
        glm::vec3 ambient() const;

        void set_diffuse(glm::vec3 diffuse); 
        glm::vec3 diffuse() const;

        void set_specular(glm::vec3 specular); 
        glm::vec3 specular() const;

        void set_shininess(float shininess); 
        float shininess() const;

        void set_dissolve(float dissolve);
        float dissolve() const;

        void set_accept_light(bool accept);
        bool accept_light() const;

        void set_name(const std::string& name);
        std::string name() const;
    };
} // namespace nle


#endif