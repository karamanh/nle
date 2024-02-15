/**
 * @file camera.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief 
 * @version 0.1
 * @date 2024-02-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include "../object/object_3d.h"

namespace nle
{

class camera : public object_3d
{
public:
    camera();
    virtual ~camera();

    void set_turn_speed(float turn_speed);
    float turn_speed() const;

    void set_free_roam(bool free_roam);
    bool free_roam() const;
    
    void set_field_of_view(float fov);
    float field_of_view() const;

    void set_near(float near);
    float near() const;

    void set_far(float far);
    float far() const;

    glm::mat4 view_matrix() const;

    nlohmann::json to_json() const override;
    void from_json(const nlohmann::json& j) override;

private:
    float m_speed;
    float m_turn_speed;
    float m_field_of_view;
    float m_near;
    float m_far;
    bool m_free_roam;
};

} // namespace nle
