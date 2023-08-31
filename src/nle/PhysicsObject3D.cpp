#include "PhysicsObject3D.h"

namespace nle
{

    PhysicsObject3D::PhysicsObject3D()
    {
        m_type = ObjectType::PhysicsObject3D;
    }

    PhysicsObject3D::~PhysicsObject3D()
    {
    }

    void PhysicsObject3D::set_physics_enabled(bool enabled)
    {
        if(!(m_physics_enabled = enabled))
        {
            set_velocity(glm::vec3(0.0f));
        }
    }

    bool PhysicsObject3D::physics_enabled() const
    {
        return m_physics_enabled;
    }

    void PhysicsObject3D::add_child(Object3D *child)
    {
        if(!children().empty())
        {
            prerr("cannot add more than one children to a physics object");
        }
        else
        {
            auto * ro = dynamic_cast<RenderObject3D*>(child);
            if(ro)
            {
                m_render_object = ro;
                Object3D::add_child(child);
            }
        }
    }

    void PhysicsObject3D::set_velocity(glm::vec3 velocity)
    {
        if(!m_rigid)
        {
            Object3D::set_velocity(velocity);
        }
    }

    nlohmann::json PhysicsObject3D::to_json()
    {
        auto ret = Object3D::to_json();
        ret["physics_enabled"] = m_physics_enabled;
        ret["rigid"] = m_rigid;
        return ret;
    }

    void PhysicsObject3D::from_json(const nlohmann::json &j)
    {
        Object3D::from_json(j);
        m_physics_enabled = j["physics_enabled"];
        m_rigid = j["rigid"];
    }

    void PhysicsObject3D::set_rigid(bool rigid)
    {
        if(!rigid)
        {
            set_velocity(glm::vec3(0.f));
        }
        m_rigid = rigid;
    }

    bool PhysicsObject3D::rigid() const
    {
        return m_rigid;
    }

    void PhysicsObject3D::set_render_object(RenderObject3D *render_object)
    {
        for(auto i : children())
        {
            delete_child(i);
        }
        add_child(render_object);
    }

    RenderObject3D *PhysicsObject3D::render_object() const
    {
        return m_render_object;
    }

} // namespace nle
