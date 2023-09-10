#ifndef LIGHT_H
#define LIGHT_H

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Object3D.h"

namespace nle
{
    class Light : public Object3D
    {
    private:
        glm::vec3 m_color;
        glm::vec3 m_ambient;
        glm::vec3 m_diffuse;
        glm::vec3 m_specular;
        
        bool m_enabled = false;

    public:
        Light();
        Light(glm::vec3 color, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular);
        ~Light();
        
        void set_color(GLfloat red, GLfloat green, GLfloat blue);
        glm::vec3 color();

        void set_ambient (glm::vec3 ambient);
        glm::vec3 ambient () const;

        void set_diffuse (glm::vec3 diffuse);
        glm::vec3 diffuse () const;

        void set_specular (glm::vec3 specular);
        glm::vec3 specular () const;

        void set_enabled(bool enabled);
        bool enabled();
        void set_id(const std::string& id) override;
        nlohmann::json to_json() override;
        void from_json(const nlohmann::json& j) override;
    };
} // namespace nle

#endif