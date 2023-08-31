#include "Object3D.h"

#include <algorithm>

namespace nle
{
    Object3D::Object3D(const std::string &id)
        : m_position(0),
          m_rotation(0),
          m_scale(1.f),
          m_velocity(0.f),
          m_speed(1.f),
          m_root(this),
          m_up({0.f, 1.f, 0.f}),
          m_front({0.f, 0.f, -1.f}),
          m_right({1.f, 0.f, 0.f})
    {
        m_id = id.empty() ? generate_random_alphanumeric_string() : id;
    }

    Object3D::~Object3D()
    {
        for (auto *i : m_children)
        {
            delete i;
        }
    }

    void Object3D::set_position(glm::vec3 pos)
    {
        m_position = pos;
        for (auto *c : m_children)
        {
            c->set_position(pos);
        }
        update();
    }

    glm::vec3 Object3D::position() const
    {
        return m_position;
    }

    void Object3D::set_rotation(glm::vec3 rotation)
    {
        m_rotation = rotation;
        for (auto *c : m_children)
        {
            c->set_rotation(rotation);
        }
        update();
    }

    glm::vec3 Object3D::rotation() const
    {
        return m_rotation;
    }

    void Object3D::set_scale(glm::vec3 scale)
    {
        m_scale = scale;
        for (auto *c : m_children)
        {
            c->set_scale(scale);
        }
        update();
    }

    glm::vec3 Object3D::velocity() const
    {
        return m_velocity;
    }

    void Object3D::set_velocity(glm::vec3 velocity)
    {
        m_velocity = velocity;
    }

    glm::vec3 Object3D::scale() const
    {
        return m_scale;
    }

    std::vector<Object3D *> Object3D::children() const
    {
        return m_children;
    }

    Signal<Object3D *> Object3D::sig_child_added() const
    {
        return m_sig_child_added;
    }

    void Object3D::move_forward()
    {
        glm::vec3 dpos = m_front * m_speed;
        set_position(position() + dpos);
    }

    void Object3D::move_backwards()
    {
        glm::vec3 dpos = -(m_front * m_speed);
        set_position(position() + dpos);
    }

    void Object3D::move_right()
    {
        glm::vec3 dpos = m_right * m_speed;
        set_position(position() + dpos);
    }

    void Object3D::move_left()
    {
        glm::vec3 dpos = -(m_right * m_speed);
        set_position(position() + dpos);
    }

    void Object3D::move_up()
    {
        glm::vec3 dpos = m_up * m_speed;
        set_position(position() + dpos);
    }

    void Object3D::move_down()
    {
        glm::vec3 dpos = -(m_up * m_speed);
        set_position(position() + dpos);
    }

    glm::vec3 Object3D::front() const
    {
        return m_front;
    }

    glm::vec3 Object3D::right() const
    {
        return m_right;
    }

    glm::vec3 Object3D::up() const
    {
        return m_up;
    }

    ObjectType Object3D::type() const
    {
        return m_type;
    }

    void Object3D::set_speed(float speed)
    {
        m_speed = speed;
    }

    float Object3D::speed() const
    {
        return m_speed;
    }

    void Object3D::update()
    {
        m_front.x = cos(glm::radians(rotation().y)) * cos(glm::radians(rotation().x));
        m_front.y = sin(glm::radians(rotation().x));
        m_front.z = sin(glm::radians(rotation().y)) * cos(glm::radians(rotation().x));
        m_front = glm::normalize(m_front);

        m_right = glm::normalize(glm::cross(m_front, WORLD_UP));
        m_up = glm::normalize(glm::cross(m_right, m_front));
    }

    void Object3D::add_child(Object3D *child)
    {
        if (!child)
            prerr("child cannot be null");

        if (std::find(m_children.begin(), m_children.end(), child) == m_children.end())
        {
            m_children.push_back(child);
            child->set_parent(this);
            m_sig_child_added.emit(child);
        }
        else
            prerr("child already added");
    }

    void Object3D::delete_child(Object3D *child, bool destroy)
    {
        auto it = std::find(m_children.begin(), m_children.end(), child);
        if (it != m_children.end())
        {
            if(destroy)
            {
                delete *it;
            }
            m_children.erase(it);
        }
    }

    void Object3D::set_id(const std::string &id)
    {
        m_id = id;
    }

    std::string Object3D::id() const
    {
        return m_id;
    }

    // void Object3D::set_physics_enabled(bool enabled)
    // {
    //     m_physics_enabled = enabled;
    // }

    // bool Object3D::physics_enabled() const
    // {
    //     return m_physics_enabled;
    // }

    void Object3D::set_parent(Object3D *parent)
    {
        m_parent = parent;
        set_root(parent->root());
    }

    nlohmann::json Object3D::to_json()
    {
        auto ret = nlohmann::json();

        ret["type"] = static_cast<int>(this->m_type);
        ret["id"] = this->id();
        ret["position"]["x"] = this->position().x;
        ret["position"]["y"] = this->position().y;
        ret["position"]["z"] = this->position().z;
        ret["rotation"]["x"] = this->rotation().x;
        ret["rotation"]["y"] = this->rotation().y;
        ret["rotation"]["z"] = this->rotation().z;
        ret["scale"]["x"] = this->scale().x;
        ret["scale"]["y"] = this->scale().y;
        ret["scale"]["z"] = this->scale().z;

        for (auto *c : m_children)
        {
            ret["children"].push_back(c->to_json());
        }

        return ret;
    }

    void Object3D::from_json(const nlohmann::json &j)
    {
        this->m_type = static_cast<ObjectType>(j["type"]);
        this->set_id(j["id"]);
        this->set_position({j["position"]["x"], j["position"]["y"], j["position"]["z"]});
        this->set_rotation({j["rotation"]["x"], j["rotation"]["y"], j["rotation"]["z"]});
        this->set_scale({j["scale"]["x"], j["scale"]["y"], j["scale"]["z"]});
    }

    Object3D *Object3D::parent() const
    {
        return m_parent;
    }

    void Object3D::set_root(Object3D *root)
    {
        m_root = root;
        for (auto *c : m_children)
        {
            c->set_root(root);
        }
    }

    Object3D *Object3D::root() const
    {
        return m_root;
    }
} // namespace nle
