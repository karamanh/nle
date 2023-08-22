#ifndef NLE_MATERIAL_H
#define NLE_MATERIAL_H

#include <GL/glew.h>

namespace nle
{
    class Material
    {
    private:
        float m_specular_intensity;
        float m_shininess;
        bool m_accept_light = true;
    public:
        Material();
        Material(float specular_intensity, float shininess);
        ~Material();
        void use(float specular_intensity_location, float shininess_location);
        void set_specular_intensity(float specular_intensity); 
        float specular_intensity() const;
        void set_shininess(float shininess); 
        float shininess() const;
        void set_accept_light(bool accept);
        bool accept_light() const;
    };
} // namespace nle


#endif