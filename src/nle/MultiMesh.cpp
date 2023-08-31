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
        // m_aabb_max.x = std::max(m_aabb_max.x, m->m_aabb_max.x);
        // m_aabb_max.y = std::max(m_aabb_max.y, m->m_aabb_max.y);
        // m_aabb_max.z = std::max(m_aabb_max.z, m->m_aabb_max.z);
        // m_aabb_min.x = std::min(m_aabb_min.x, m->m_aabb_min.x);
        // m_aabb_min.y = std::min(m_aabb_min.y, m->m_aabb_min.y);
        // m_aabb_min.z = std::min(m_aabb_min.z, m->m_aabb_min.z);

        m_aabb = AABB(
            {
                std::min(m_aabb.min().x, m->aabb().min().x),
                std::min(m_aabb.min().y, m->aabb().min().y),
                std::min(m_aabb.min().z, m->aabb().min().z)
            },
            {
                std::max(m_aabb.max().x, m->aabb().max().x),
                std::max(m_aabb.max().y, m->aabb().max().y),
                std::max(m_aabb.max().z, m->aabb().max().z)
            }
        );
    }

    MultiMeshInstance *MultiMesh::create_instance()
    {
        return new MultiMeshInstance(this);
    }

    float MultiMesh::bounding_sphere_radius() const
    {
        return m_bounding_sphere_radius;
    }

    AABB MultiMesh::aabb() const
    {
        return m_aabb;
    }

    MultiMeshInstance::MultiMeshInstance(MultiMesh *mm)
        : m_multimesh(mm)
    {
        m_type = ObjectType::MultiMeshInstance;

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

    void MultiMeshInstance::update()
    {
        Object3D::update();
        m_model_matrix = glm::mat4(1.f);
        m_model_matrix = glm::translate(m_model_matrix, this->position());
        m_model_matrix = glm::rotate(m_model_matrix, glm::radians(this->rotation().x), glm::vec3(1.f, 0.f, 0.f));
        m_model_matrix = glm::rotate(m_model_matrix, glm::radians(this->rotation().y), glm::vec3(0.f, 1.f, 0.f));
        m_model_matrix = glm::rotate(m_model_matrix, glm::radians(this->rotation().z), glm::vec3(0.f, 0.f, 1.f));
        m_model_matrix = glm::scale(m_model_matrix, this->scale());
        m_aabb = transform_aabb(m_multimesh->aabb(), m_model_matrix);
    }

    AABB MultiMeshInstance::transform_aabb(AABB aabb, glm::mat4 m)
    {
        AABB res;
        glm::vec3 corners[8];

        corners[0] = aabb.min();
        corners[1] = glm::vec3(aabb.min().x, aabb.max().y, aabb.min().z);
        corners[2] = glm::vec3(aabb.min().x, aabb.max().y, aabb.max().z);
        corners[3] = glm::vec3(aabb.min().x, aabb.min().y, aabb.max().z);
        corners[4] = glm::vec3(aabb.max().x, aabb.min().y, aabb.min().z);
        corners[5] = glm::vec3(aabb.max().x, aabb.max().y, aabb.min().z);
        corners[6] = aabb.max();
        corners[7] = glm::vec3(aabb.max().x, aabb.min().y, aabb.max().z);


        glm::vec4 tmp  = (m * glm::vec4(corners[0],1.0));
        glm::vec3 tmin = glm::vec3(tmp.x, tmp.y, tmp.z);
        glm::vec3 tmax = tmin;

        // transform the other 7 corners and compute the result AABB
        for(int i = 1; i < 8; i++)
        {
            tmp = (m * glm::vec4(corners[i],1.0));
            glm::vec3 point = glm::vec3(tmp.x, tmp.y, tmp.z);

            tmin = glm::min(tmin, point);
            tmax = glm::max(tmax, point);
        }        
        
        res.set_min(tmin);
        res.set_max(tmax);
        
        return res;
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
        ret["source"] = this->m_source;
        return ret;
    }

    void MultiMeshInstance::from_json(const nlohmann::json &j)
    {
        Object3D::from_json(j);
        m_source = j["source"];
    }

    float MultiMeshInstance::scaled_radius() const
    {
        float max = fmaxf(scale().x, fmaxf(scale().y, scale().z));
        return m_multimesh->bounding_sphere_radius() * max;
    }

    AABB MultiMeshInstance::aabb() const
    {
        return m_aabb;
    }

    glm::mat4 MultiMeshInstance::model_matrix() const
    {
        return m_model_matrix;
    }

} // namespace nle
