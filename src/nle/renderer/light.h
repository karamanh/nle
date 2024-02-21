/**
 * @file light.cpp
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief
 * @version 0.1
 * @date 2024-02-20
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include "../object/object_3d.h"

namespace nle
{

    class light : public object_3d
    {
    public:
        light();
        light(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular);
        virtual ~light();

        void set_color(glm::vec3 color);
        glm::vec3 color() const;

        void set_ambient(glm::vec3 ambient);
        glm::vec3 ambient() const;

        void set_diffuse(glm::vec3 diffuse);
        glm::vec3 diffuse() const;

        void set_specular(glm::vec3 specular);
        glm::vec3 specular() const;

        void set_enabled(bool enabled);
        bool enabled() const;

    private:
        glm::vec3 m_color;
        glm::vec3 m_ambient;
        glm::vec3 m_diffuse;
        glm::vec3 m_specular;

        bool m_enabled = true;
    };

} // namespace nle
