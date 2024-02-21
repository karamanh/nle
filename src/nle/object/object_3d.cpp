#include "object_3d.h"

namespace nle
{
    const glm::vec3 WORLD_UP = {0.f, 1.f, 0.f};

    object_3d::object_3d(const std::string &id)
    {
        m_id = id.empty() ? nle::utils::generate_random_alphanumeric_string() : id;

        m_position = glm::vec3(0.0f);
        m_rotation = glm::vec3(0.0f);
        m_scale = glm::vec3(1.0f);
        m_velocity = glm::vec3(0.0f);
        m_right = glm::vec3(1.0f, 0.0f, 0.0f);
        m_up = glm::vec3(0.0f, 1.0f, 0.0f);
        m_front = glm::vec3(0.0f, 0.0f, 1.0f);
    }

    object_3d::~object_3d()
    {
    }

    void object_3d::set_position(glm::vec3 position)
    {
        m_position = position;
        update();
    }

    glm::vec3 object_3d::position() const
    {
        return m_position;
    }

    void object_3d::set_rotation(glm::vec3 rotation)
    {
        m_rotation = rotation;
        update();
    }

    glm::vec3 object_3d::rotation() const
    {
        return m_rotation;
    }

    void object_3d::set_scale(glm::vec3 scale)
    {
        m_scale = scale;
        update();
    }

    glm::vec3 object_3d::scale() const
    {
        return m_scale;
    }

    void object_3d::set_velocity(glm::vec3 velocity)
    {
        m_velocity = velocity;
    }

    glm::vec3 object_3d::velocity() const
    {
        return m_velocity;
    }

    std::set<ref<object_3d>, std::owner_less<ref<object_3d>>> object_3d::children() const
    {
        return m_children;
    }

    void object_3d::add_child(ref<object_3d> child)
    {
        if(!m_children.insert(child).second)
        {
            utils::prdebug("could not add child (already exists)");
        }
    }

    void object_3d::delete_child(ref<object_3d> child)
    {
        m_children.erase(child);
    }

    void object_3d::set_id(const std::string &id)
    {
        m_id = id;
    }

    const std::string &object_3d::id() const
    {
        return m_id;
    }

    void object_3d::set_speed(float speed)
    {
        m_speed = speed;
    }

    float object_3d::speed() const
    {
        return m_speed;
    }

    void object_3d::set_parent(ref<object_3d> parent)
    {
        m_parent = parent;
    }

    ref<object_3d> object_3d::parent() const
    {
        return m_parent;
    }

    void object_3d::set_root(ref<object_3d> root)
    {
        m_root = root;
        for(auto it : m_children)
        {
            it->set_root(root);
        }
    }

    ref<object_3d> object_3d::root() const
    {
        return m_root;
    }
    
    nlohmann::json object_3d::to_json() const
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

        for (auto c : m_children)
        {
            ret["children"].push_back(c->to_json());
        }

        return ret;
    }

    void object_3d::from_json(const nlohmann::json &j)
    {
        this->m_type = static_cast<object_3d_type>(j["type"]);
        this->set_id(j["id"]);
        this->set_position({j["position"]["x"], j["position"]["y"], j["position"]["z"]});
        this->set_rotation({j["rotation"]["x"], j["rotation"]["y"], j["rotation"]["z"]});
        this->set_scale({j["scale"]["x"], j["scale"]["y"], j["scale"]["z"]});
    }

    glm::vec3 object_3d::front() const
    {
        return m_front;
    }

    glm::vec3 object_3d::right() const
    {
        return m_right;
    }

    glm::vec3 object_3d::up() const
    {
        return m_up;
    }

    void object_3d::move_forward()
    {
        glm::vec3 dpos = m_front * m_speed;
        set_position(position() + dpos);
    }

    void object_3d::move_backwards()
    {
        glm::vec3 dpos = -(m_front * m_speed);
        set_position(position() + dpos);
    }

    void object_3d::move_right()
    {
        glm::vec3 dpos = m_right * m_speed;
        set_position(position() + dpos);
    }

    void object_3d::move_left()
    {
        glm::vec3 dpos = -(m_right * m_speed);
        set_position(position() + dpos);
    }

    void object_3d::move_up()
    {
        glm::vec3 dpos = m_up * m_speed;
        set_position(position() + dpos);
    }

    void object_3d::move_down()
    {
        glm::vec3 dpos = -(m_up * m_speed);
        set_position(position() + dpos);
    }

    void object_3d::update()
    {
        m_front.x = cos(glm::radians(rotation().y)) * cos(glm::radians(rotation().x));
        m_front.y = sin(glm::radians(rotation().x));
        m_front.z = sin(glm::radians(rotation().y)) * cos(glm::radians(rotation().x));
        m_front = glm::normalize(m_front);

        m_right = glm::normalize(glm::cross(m_front, WORLD_UP));
        m_up = glm::normalize(glm::cross(m_right, m_front));
    }
} // namespace nle
