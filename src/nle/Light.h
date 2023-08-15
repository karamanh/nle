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
        GLfloat m_ambient_intensity;
        GLfloat m_diffuse_intensity;
        GLfloat m_specular_intensity;
        
        bool m_enabled = false;

    public:
        Light();
        Light(GLfloat red, GLfloat green, GLfloat blue, GLfloat ambientIntensity,
              GLfloat diffuseIntensity);
        ~Light();
        void use(GLfloat ambient_intensity_location, GLfloat ambient_color_location,
                 GLfloat diffuse_intensity_location, GLfloat diffuse_direction_location);
        void set_color(GLfloat red, GLfloat green, GLfloat blue);
        glm::vec3 color();
        void set_ambient_intensity(GLfloat intensity);
        float ambient_intensity();
        void set_diffuse_intensity(GLfloat intensity);
        float diffuse_intensity();
        void set_specular_intensity(GLfloat intensity);
        float specular_intensity();
        void set_enabled(bool enabled);
        bool enabled();
        void set_id(const std::string& id) override;
    };
} // namespace nle

#endif