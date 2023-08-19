#include "MultiMesh.h"

namespace nle
{

    MultiMesh::MultiMesh()
    {
    }

    MultiMesh::~MultiMesh()
    {
        for (auto *i : m_meshes)
            delete i;
    }

    std::vector<Mesh *> &MultiMesh::meshes()
    {
        return m_meshes;
    }

    void MultiMesh::add_mesh(Mesh *m)
    {
        m_meshes.push_back(m);
        m_aabb_max.x = std::max(m_aabb_max.x, m->m_aabb_max.x);
        m_aabb_max.y = std::max(m_aabb_max.y, m->m_aabb_max.y);
        m_aabb_max.z = std::max(m_aabb_max.z, m->m_aabb_max.z);
        m_aabb_min.x = std::min(m_aabb_min.x, m->m_aabb_min.x);
        m_aabb_min.y = std::min(m_aabb_min.y, m->m_aabb_min.y);
        m_aabb_min.z = std::min(m_aabb_min.z, m->m_aabb_min.z);
    }

    MultiMeshInstance *MultiMesh::create_instance()
    {
        return new MultiMeshInstance(this);
    }

    float MultiMesh::bounding_sphere_radius() const
    {
        return m_bounding_sphere_radius;
    }

    MultiMeshInstance::MultiMeshInstance(MultiMesh *mm)
        : m_multimesh(mm)
    {
        for (auto *c : m_multimesh->meshes())
        {
            add_child(c->create_instance());
        }
    }

    MultiMeshInstance::~MultiMeshInstance()
    {
    }

    MultiMesh *MultiMeshInstance::multimesh() const
    {
        return m_multimesh;
    }

    void MultiMeshInstance::set_material_for_meshes(Material *material)
    {
        for (auto *c : m_multimesh->meshes())
        {
            c->set_material(m_material);
        }
    }

    void MultiMeshInstance::update_model_matrix()
    {
        m_model_matrix = glm::mat4(1.f);
        m_model_matrix = glm::translate(m_model_matrix, this->position());
        m_model_matrix = glm::rotate(m_model_matrix, glm::radians(this->rotation().x), glm::vec3(1.f, 0.f, 0.f));
        m_model_matrix = glm::rotate(m_model_matrix, glm::radians(this->rotation().y), glm::vec3(0.f, 1.f, 0.f));
        m_model_matrix = glm::rotate(m_model_matrix, glm::radians(this->rotation().z), glm::vec3(0.f, 0.f, 1.f));
        m_model_matrix = glm::scale(m_model_matrix, this->scale());
    }

    void MultiMeshInstance::set_material(Material *material)
    {
        m_material = material;
        for (auto *c : m_multimesh->meshes())
        {
            c->set_material(m_material);
        }
    }

    Material *MultiMeshInstance::material() const
    {
        return m_material;
    }

    nlohmann::json MultiMeshInstance::to_json()
    {
        auto ret = Object3D::to_json();
        ret["type"] = 4;
        ret["source"] = this->m_source;
        return ret;
    }

    void MultiMeshInstance::from_json(const nlohmann::json &j)
    {
        Object3D::from_json(j);
        m_source = j["source"];
    }

    void MultiMeshInstance::set_position(glm::vec3 position)
    {
        Object3D::set_position(position);
        update_model_matrix();
    }

    void MultiMeshInstance::set_rotation(glm::vec3 rotation)
    {
        Object3D::set_rotation(rotation);
        update_model_matrix();
    }

    void MultiMeshInstance::set_scale(glm::vec3 scale)
    {
        Object3D::set_scale(scale);
        update_model_matrix();
    }

    float MultiMeshInstance::scaled_radius() const
    {
        float max = fmaxf(scale().x, fmaxf(scale().y, scale().z));
        return m_multimesh->bounding_sphere_radius() * max;
    }

    glm::vec3 MultiMeshInstance::aabb_max() const
    {
        return m_multimesh->m_aabb_max;
    }

    glm::vec3 MultiMeshInstance::aabb_min() const
    {
        return m_multimesh->m_aabb_min;
    }

    glm::mat4 MultiMeshInstance::model_matrix() const
    {
        return m_model_matrix;
    }

} // namespace nle
