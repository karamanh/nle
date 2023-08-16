#include "Scene.h"

namespace nle
{

    Scene::Scene()
        : m_camera(new Camera(glm::vec3(0.f, 0.f, 0.f))), m_light(new Light(1.f, 1.f, 1.f, 1.f, 1.f))
    {
        // this->sig_child_added().bind_callback([this](const Object3D*& msg){
        //     this->register_render_object(c);
        // });
    }

    Scene::~Scene()
    {
        delete m_light;
        delete m_camera;
    }

    Camera *Scene::camera()
    {
        return m_camera;
    }

    void Scene::set_camera(Camera *c)
    {
        m_camera = c;
    }

    Light *Scene::light()
    {
        return m_light;
    }

    void Scene::register_render_object(Object3D *object)
    {
        auto it = std::find(m_render_objects.begin(), m_render_objects.end(), object);
        if(it == m_render_objects.end())
        {
            m_render_objects.push_back(object);
            for(auto * c : object->children())
            {
                register_render_object(c);
            }
        }
    }

    void Scene::delete_render_object(Object3D *object)
    {
        auto it = std::find(m_render_objects.begin(), m_render_objects.end(), object);
        if(it != m_render_objects.end())
        {
            for(auto * c : object->children())
            {
                delete_render_object(c);
            }
            m_render_objects.erase(it);
        }
    }

    std::vector<Object3D *> Scene::render_objects()
    {
        return m_render_objects;
    }

    void Scene::add_child(Object3D *child)
    {
        Object3D::add_child(child);
        register_render_object(child);
    }

    void Scene::delete_child(Object3D *child)
    {
        delete_render_object(child);
        Object3D::delete_child(child);
    }

    void Scene::set_id(const std::string &id)
    {
        Object3D::set_id("scene_" + id);
    }

    nlohmann::json Scene::serialize(Object3D *o)
    {
    }

    nlohmann::json Scene::to_json()
    {
        auto ret = Object3D::to_json();
        ret["type"] = 1;
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
