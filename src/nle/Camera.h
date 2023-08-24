#ifndef CAMERA_H
#define CAMERA_H

#include "Object3D.h"

namespace nle
{
    class Camera : public Object3D
    {
    private:
        float m_speed;
        float m_turn_speed;
        float m_fov;
        float m_near;
        float m_far;
        bool m_locked_to_mouse = false;
        bool m_free_roam = false;

        glm::mat4 get_view_matrix();
        void on_mouse_motion(int dx, int dy);
        void on_key_pressed(int key);

    public:
        Camera();
        Camera(glm::vec3 position);
        virtual ~Camera();
        void set_turn_speed(float turnSpeed);
        float turn_speed() const;
        bool free_roam() const;
        void look_at(glm::vec3 position);
        void set_free_roam(bool free_roam);
        void set_rotation(glm::vec3 rotation) override;
        void set_position(glm::vec3 position) override;
        void set_id(const std::string& id) override;
        nlohmann::json to_json() override;
        void from_json(const nlohmann::json& j) override;
        void set_fov(float fov);
        float fov() const;
        void set_near(float near);
        float near() const;
        void set_far(float far);
        float far() const;

        friend class Renderer3D;
        friend class Nle;
    };
} // namespace nle

#endif