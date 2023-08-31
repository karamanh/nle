#pragma once

#include "Object3D.h"
#include "RenderObject3D.h"

namespace nle
{
    class PhysicsObject3D : public Object3D
    {
    private:
        bool m_physics_enabled = false;
        bool m_rigid = false;

        RenderObject3D *m_render_object;
        
    public:
        PhysicsObject3D();
        ~PhysicsObject3D();
        void set_physics_enabled(bool enabled);
        bool physics_enabled() const;
        
        void set_rigid(bool rigid);
        bool rigid() const;

        void set_render_object(RenderObject3D *render_object);
        RenderObject3D* render_object() const;

        void add_child(Object3D *child) override;
        void set_velocity(glm::vec3 velocity) override;

        nlohmann::json to_json();
        void from_json(const nlohmann::json &j);
    };
} // namespace nle