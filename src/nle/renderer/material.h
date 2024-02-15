/**
 * @file material.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief 
 * @version 0.1
 * @date 2024-02-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include <glm/glm.hpp>

#include <string>

namespace nle
{

class material
{
public:
    material();
    virtual ~material();

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

    void set_id(const std::string& id);
    std::string id() const;

private:
    std::string m_id;
    glm::vec3 m_ambient;
    glm::vec3 m_diffuse;
    glm::vec3 m_specular;
    float m_specular_intensity;
    float m_shininess;
    float m_dissolve;
    bool m_accept_light;
};

} // namespace nle
