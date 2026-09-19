/**
 * @file light.h
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
#include "../renderer/render_context.h"

namespace nle
{

    enum class light_type
    {
        directional,
        point
    };

    /**
     * @brief A directional light, and the base for every other light.
     *
     * Direction is taken from the object's orientation (object_3d::front()), so
     * a directional light is aimed with set_rotation().
     */
    class light : public object_3d
    {
    public:
        light();
        light(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular);
        virtual ~light();

        virtual enum light_type light_type() const;

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

        /// Fills the per-frame description consumed by the renderer.
        directional_light_data to_directional_light_data() const;

    private:
        glm::vec3 m_color;
        glm::vec3 m_ambient;
        glm::vec3 m_diffuse;
        glm::vec3 m_specular;

        bool m_enabled = true;
    };

    /**
     * @brief An omnidirectional light that falls off with distance.
     *
     * Attenuation follows the usual constant/linear/quadratic form:
     *
     *     attenuation = 1 / (constant + linear * d + quadratic * d * d)
     *
     * @p range is an additional hard cutoff. It lets the renderer discard
     * lights that cannot affect what is on screen, and keeps a light from
     * leaking across a whole level because its quadratic term is small.
     *
     * The light is positioned like any other object_3d, which means adding one
     * as a child of a moving object makes it follow that object.
     */
    class point_light : public light
    {
    public:
        point_light();
        point_light(glm::vec3 color, float range);
        virtual ~point_light();

        enum light_type light_type() const override;

        /// Sets attenuation from a target range using the values tabulated in
        /// the classic Ogre3D attenuation table. Convenience over correctness:
        /// it just picks a sane curve for a given reach.
        void set_range(float range);
        float range() const;

        void set_constant(float constant);
        float constant() const;

        void set_linear(float linear);
        float linear() const;

        void set_quadratic(float quadratic);
        float quadratic() const;

        /// Attenuation factor at distance @p distance, ignoring the cutoff.
        float attenuation_at(float distance) const;

        point_light_data to_point_light_data() const;

    private:
        float m_range;
        float m_constant;
        float m_linear;
        float m_quadratic;
    };

} // namespace nle
