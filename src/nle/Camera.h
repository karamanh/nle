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

        bool m_locked_to_mouse = false;
        bool m_free_roam = false;

        // glm::vec3 m_front;
        // glm::vec3 m_up;
        // glm::vec3 m_right;
        // glm::vec3 m_up_world;

        glm::mat4 get_view_matrix();
        void on_mouse_motion(int dx, int dy);
        void on_key_pressed(int key);

    public:
        Camera();
        Camera(glm::vec3 position);
        virtual ~Camera();
        // void update();
        // void set_speed(float speed);
        // float speed() const;
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

        friend class Renderer3D;
        friend class Nle;
    };
} // namespace nle

#endif