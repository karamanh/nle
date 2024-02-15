#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../../vendor/nlohmann/json.hpp"

#include <set>

#include "../core/ref.h"
#include "../core/signal.hpp"
#include "../core/utils.h"

#include "object_3d_type.h"


namespace nle
{
    class object_3d
    {
    public:
        object_3d(const std::string &id = "");
        virtual ~object_3d();

        virtual void set_position(glm::vec3 position);
        virtual glm::vec3 position() const;

        virtual void set_rotation(glm::vec3 rotation);
        virtual glm::vec3 rotation() const;

        virtual void set_scale(glm::vec3 scale);
        virtual glm::vec3 scale() const;

        virtual void set_velocity(glm::vec3 velocity);
        virtual glm::vec3 velocity() const;

        virtual std::set<ref<object_3d>, std::owner_less<ref<object_3d>>> children() const;
        virtual void add_child(ref<object_3d> child);
        virtual void delete_child(ref<object_3d> child);

        virtual void set_id(const std::string &id);
        virtual const std::string &id() const;

        virtual void set_speed(float speed);
        virtual float speed() const;

        virtual void set_parent(ref<object_3d> parent);
        virtual ref<object_3d> parent() const;

        virtual void set_root(ref<object_3d> root);
        virtual ref<object_3d> root() const;

        virtual nlohmann::json to_json() const;
        virtual void from_json(const nlohmann::json &j);

        virtual glm::vec3 front() const;
        virtual glm::vec3 right() const;
        virtual glm::vec3 up() const;

        virtual void move_forward();
        virtual void move_backwards();
        virtual void move_right();
        virtual void move_left();
        virtual void move_up();
        virtual void move_down();

    private:

        virtual void update();
        
        std::string m_id;
        enum object_3d_type m_type;

        glm::vec3 m_position;
        glm::vec3 m_rotation;
        glm::vec3 m_scale;
        glm::vec3 m_velocity;
        glm::vec3 m_front;
        glm::vec3 m_up;
        glm::vec3 m_right;
        float m_speed;

        ref<object_3d> m_parent;
        ref<object_3d> m_root;
        std::set<ref<object_3d>, std::owner_less<ref<object_3d>>> m_children;

        signal<ref<object_3d>> sig_child_added;

    };
} // namespace nle
