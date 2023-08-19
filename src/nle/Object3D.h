#ifndef SPATIAL_H
#define SPATIAL_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "nlohmann/json.hpp"

#include "Utils.h"
#include "Signal.hpp"

#include <vector>
#include <string>

const glm::vec3 WORLD_UP = {0.f, 1.f, 0.f};

namespace nle
{
    class Object3D
    {
    private:
        glm::vec3 m_position;
        glm::vec3 m_rotation;
        glm::vec3 m_scale;
        glm::vec3 m_velocity;
        glm::vec3 m_front;
        glm::vec3 m_up;
        glm::vec3 m_right;
        std::vector<Object3D *> m_children;
        std::string m_id;
        Object3D *m_parent;
        Object3D *m_root;
        Signal<Object3D *> m_sig_child_added;
        bool m_physics_enabled = false;
        float m_speed;

        void set_root(Object3D *root);
        void update();

    public:
        Object3D(const std::string &id = "");
        virtual ~Object3D();
        virtual void set_position(glm::vec3 position);
        virtual glm::vec3 position() const;
        virtual void set_rotation(glm::vec3 rotation);
        virtual glm::vec3 rotation() const;
        virtual void set_scale(glm::vec3 scale);
        virtual glm::vec3 velocity() const;
        virtual void set_velocity(glm::vec3 velocity);
        virtual glm::vec3 scale() const;
        virtual std::vector<Object3D *> children() const;
        virtual void add_child(Object3D *child);
        virtual void delete_child(Object3D *child);
        virtual void set_id(const std::string &id);
        virtual std::string id() const;
        virtual void set_physics_enabled(bool enabled);
        virtual bool physics_enabled() const;
        virtual void set_parent(Object3D *parent);
        virtual nlohmann::json to_json();
        virtual void from_json(const nlohmann::json &j);
        virtual Object3D *parent() const;
        virtual Object3D *root() const;
        Signal<Object3D *> sig_child_added() const;
        const std::type_info &type() const;
        virtual void move_forward();
        virtual void move_backwards();
        virtual void move_right();
        virtual void move_left();
        virtual void move_up();
        virtual void move_down();
        virtual glm::vec3 front() const;
        virtual glm::vec3 right() const;
        virtual glm::vec3 up() const;
        virtual void set_speed(float speed);
        virtual float speed() const;
    };
} // namespace nle

#endif