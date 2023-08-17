#include "Object3D.h"

#include <algorithm>

namespace nle
{
    Object3D::Object3D(const std::string &id)
        : m_position(0), m_rotation(0), m_scale(1.f), m_velocity(0.f), m_root(this)
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
        for(auto * c : m_children)
        {
            c->set_position(pos);
        }
    }

    glm::vec3 Object3D::position() const
    {
        return m_position;
    }

    void Object3D::set_rotation(glm::vec3 rotation)
    {
        m_rotation = rotation;
        for(auto * c : m_children)
        {
            c->set_rotation(rotation);
        }
    }

    glm::vec3 Object3D::rotation() const
    {
        return m_rotation;
    }

    void Object3D::set_scale(glm::vec3 scale)
    {
        m_scale = scale;
        for(auto * c : m_children)
        {
            c->set_scale(scale);
        }
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

    const std::type_info& Object3D::type() const
    {
        return typeid(*this);
    }

    void Object3D::add_child(Object3D *child)
    {
        if(!child)
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

    void Object3D::delete_child(Object3D *child)
    {
        auto it = std::find(m_children.begin(), m_children.end(), child);
        if(it != m_children.end())
        {
            delete *it;
            m_children.erase(it);
        }
    }

    void Object3D::set_id(const std::string& id)
    {
        m_id = id;
    }

    std::string Object3D::id() const
    {
        return m_id;
    }

    void Object3D::set_physics_enabled(bool enabled)
    {
        m_physics_enabled = enabled;
    }

    bool Object3D::physics_enabled() const
    {
        return m_physics_enabled;
    }

    void Object3D::set_parent(Object3D *parent)
    {
        m_parent = parent;
        set_root(parent->root());
    }

    nlohmann::json Object3D::to_json()
    {
        auto ret = nlohmann::json();
        
        ret["type"] = 0;
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

        for(auto * c : m_children)
        {
            ret["children"].push_back(c->to_json());
        }

        return ret;
    }

    void Object3D::from_json(const nlohmann::json &j)
    {
        m_id = j["id"];
        m_position.x = j["position"]["x"];
        m_position.y = j["position"]["y"];
        m_position.z = j["position"]["z"];
        m_rotation.x = j["rotation"]["x"];
        m_rotation.y = j["rotation"]["y"];
        m_rotation.z = j["rotation"]["z"];
        m_scale.x = j["scale"]["x"];
        m_scale.y = j["scale"]["y"];
        m_scale.z = j["scale"]["z"];
    }

    Object3D *Object3D::parent() const
    {
        return m_parent;
    }

    void Object3D::set_root(Object3D *root)
    {
        m_root = root;
        for(auto * c : m_children)
        {
            c->set_root(root);
        }
    }

    Object3D *Object3D::root() const
    {
        return m_root;
    }
} // namespace nle
