#include "Scene.h"
#include "MultiMesh.h"

namespace nle
{

    Scene::Scene()
        : m_default_camera(new Camera(glm::vec3(0.f, 0.f, 0.f))), m_light(new Light(glm::vec3(1.f), glm::vec3(1.f), glm::vec3(1.f), glm::vec3(1.f)))
    {
        m_type = ObjectType::Scene;
        m_camera = m_default_camera;
    }

    Scene::~Scene()
    {
        delete m_light;
        delete m_default_camera;
    }

    Camera *Scene::camera()
    {
        return m_camera ? m_camera : m_default_camera;
    }

    void Scene::set_camera(Camera *c)
    {
        m_camera = c;
    }

    Light *Scene::light()
    {
        return m_light;
    }

    void Scene::register_render_object(RenderObject3D *object)
    {
        auto it = std::find(m_render_objects.begin(), m_render_objects.end(), object);
        if(it == m_render_objects.end())
        {
            m_render_objects.push_back(object);
            for(auto * c : object->children())
            {
                auto *ro = dynamic_cast<RenderObject3D*>(c);
                if(ro)
                {
                    register_render_object(ro);
                }
            }
        }
    }

    void Scene::delete_render_object(RenderObject3D *object)
    {
        auto it = std::find(m_render_objects.begin(), m_render_objects.end(), object);
        if(it != m_render_objects.end())
        {
            for(auto * c : object->children())
            {
                auto *ro = dynamic_cast<RenderObject3D*>(c);
                if(ro)
                {
                    delete_render_object(ro);
                }
            }
            m_render_objects.erase(it);
        }
    }

    std::vector<RenderObject3D *> Scene::render_objects()
    {
        return m_render_objects;
    }

    void Scene::add_child(Object3D *child)
    {
        Object3D::add_child(child);

        auto * ro = dynamic_cast<RenderObject3D*>(child);
        if(ro)
        {
            register_render_object(ro);
        }
    }

    void Scene::delete_child(Object3D *child, bool destroy)
    {
        auto * ro = dynamic_cast<RenderObject3D*>(child);
        if(ro)
        {
            delete_render_object(ro);
        }
        Object3D::delete_child(child, destroy);
    }

    void Scene::set_id(const std::string &id)
    {
        Object3D::set_id(id);
    }

    nlohmann::json Scene::serialize(Object3D *o)
    {
    }

    nlohmann::json Scene::to_json()
    {
        auto ret = Object3D::to_json();
        ret["camera"] = m_camera->to_json();
        ret["light"] = m_light->to_json();
        return ret;
    }

    void Scene::from_json(const nlohmann::json &j)
    {
        Object3D::from_json(j);
        m_camera->from_json(j["camera"]);
        m_light->from_json(j["light"]);
    }
} // namespace nle
