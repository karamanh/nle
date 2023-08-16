#ifndef SPATIAL_H
#define SPATIAL_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Utils.h"
#include "Signal.hpp"

#include <vector>
#include <string>

namespace nle
{
    class Object3D
    {
    private:
        glm::vec3 m_position;
        glm::vec3 m_rotation;
        glm::vec3 m_scale;
        glm::vec3 m_velocity;

        std::vector<Object3D *> m_children;
        std::string m_id;

        Object3D *m_parent;
        Object3D *m_root;

        Signal<Object3D*> m_sig_child_added;

        bool m_physics_enabled = false;

        void set_root(Object3D *root);

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
        virtual void set_id(const std::string& id);
        virtual std::string id() const;
        virtual void set_physics_enabled(bool enabled);
        virtual bool physics_enabled() const;
        virtual void set_parent(Object3D *parent);
        virtual Object3D *parent() const;
        virtual Object3D *root() const;
        Signal<Object3D*> sig_child_added() const;
    };
} // namespace nle

#endif