#include "Camera.h"
#include "Scene.h"

#include <GLFW/glfw3.h>

namespace nle
{
    Camera::Camera()
        : m_fov(45.f), m_near(0.1f), m_far(10000.f)
    {
        // m_up = glm::vec3(0.f, 1.f, 0.f);
        // m_front = glm::vec3(0.f, 0.f, -1.f);
        // m_right = glm::vec3(1.f, 0.f, 0.f);
        // m_up_world = glm::vec3(0.f, 1.f, 0.f);
        set_speed(1.f);
        set_turn_speed(1.f);
    }

    Camera::Camera(glm::vec3 position)
        : m_fov(45.f), m_near(0.1f), m_far(10000.f)
    {
        set_position(position);
        set_rotation(glm::vec3(0.f));
        // m_up = glm::vec3(0.f, 1.f, 0.f);
        // m_front = glm::vec3(0.f, 0.f, -1.f);
        // m_right = glm::vec3(1.f, 0.f, 0.f);
        // m_up_world = glm::vec3(0.f, 1.f, 0.f);
        set_speed(1.f);
        set_turn_speed(1.f);
    }

    Camera::~Camera()
    {
    }

    // void Camera::set_speed(float speed)
    // {
    //     m_speed = speed;
    // }

    // float Camera::speed() const
    // {
    //     return m_speed;
    // }

    void Camera::set_turn_speed(float turnSpeed)
    {
        m_turn_speed = turnSpeed;
    }

    float Camera::turn_speed() const
    {
        return m_turn_speed;
    }

    bool Camera::free_roam() const
    {
        return m_free_roam;
    }

    void Camera::look_at(glm::vec3 position)
    {
        glm::vec3 dir = glm::normalize(position - this->position());
        // // glm::
        glm::vec3 rot = {
            atan2(dir.x, 1.f),
            atan2(dir.y, 1.f),
            atan2(dir.z, -1.f),
        };

        this->set_rotation(glm::degrees(rot));
        ;
        // glm::euler(glm::lookAt(this->position(), this->position() + dir, m_up_world));
        // glm::euler<mat4>(glm::lookAt(this->position(), this->position() + dir, m_up_world));
    }

    void Camera::set_free_roam(bool free_roam)
    {
        m_free_roam = free_roam;
    }

    void Camera::set_rotation(glm::vec3 rotation)
    {
        Object3D::set_rotation(rotation);
        // update();
    }

    void Camera::set_position(glm::vec3 position)
    {
        Object3D::set_position(position);
        // update();
    }

    void Camera::set_id(const std::string &id)
    {
        Object3D::set_id(id);
    }

    nlohmann::json Camera::to_json()
    {
        auto ret = Object3D::to_json();
        ret["type"] = 2;
        ret["turn_speed"] = m_turn_speed;
        ret["speed"] = m_speed;
        ret["free_roam"] = m_free_roam;
        return ret;
    }

    void Camera::from_json(const nlohmann::json &j)
    {
        Object3D::from_json(j);
        m_turn_speed = j["turn_speed"];
        m_speed = j["speed"];
        m_free_roam = j["free_roam"];
    }

    void Camera::set_fov(float fov)
    {
        m_fov = fov;
    }

    float Camera::fov() const
    {
        return m_fov;
    }

    void Camera::set_near(float near)
    {
        m_near = near;
    }

    float Camera::near() const
    {
        return m_near;
    }

    void Camera::set_far(float far)
    {
        m_far = far;
    }

    float Camera::far() const
    {
        return m_far;
    }

    // void Camera::update()
    // {
    //     m_front.x = cos(glm::radians(rotation().y)) * cos(glm::radians(rotation().x));
    //     m_front.y = sin(glm::radians(rotation().x));
    //     m_front.z = sin(glm::radians(rotation().y)) * cos(glm::radians(rotation().x));
    //     m_front = glm::normalize(m_front);

    //     m_right = glm::normalize(glm::cross(m_front, m_up_world));
    //     m_up = glm::normalize(glm::cross(m_right, m_front));
    // }

    glm::mat4 Camera::get_view_matrix()
    {
        return glm::lookAt(position(), position() + front(), up());
    }

    void Camera::on_mouse_motion(int dx, int dy)
    {
        if(!m_free_roam)
        {
            return;
        }
        
        dx *= m_turn_speed;
        dy *= m_turn_speed;

        glm::vec3 camrot = rotation();

        float pitch = camrot.x + dy;
        float yaw = camrot.y + dx;

        if (pitch > 89.f)
        {
            pitch = 89.f;
        }
        if (pitch < -89.f)
        {
            pitch = -89.f;
        }

        set_rotation({pitch, yaw, 0.f});
    }

    void Camera::on_key_pressed(int key)
    {
        if (key == GLFW_KEY_LEFT_CONTROL)
        {
            m_free_roam = !m_free_roam;
        }
        
        if(m_free_roam)
        {
            if (key == GLFW_KEY_W)
            {
                move_forward();
            }
            if (key == GLFW_KEY_S)
            {
                move_backwards();
            }
            if (key == GLFW_KEY_D)
            {
                move_right();
            }
            if (key == GLFW_KEY_A)
            {
                move_left();
            }
            if (key == GLFW_KEY_E)
            {
                move_up();
            }
            if (key == GLFW_KEY_Q)
            {
                move_down();
            }
        }
    }
} // namespace nle
