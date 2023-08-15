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
        m_render_objects.push_back(object);
        for(auto * c : object->children())
        {
            m_render_objects.push_back(c);
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

    void Scene::set_id(const std::string &id)
    {
        Object3D::set_id("scene_" + id);
    }

    nlohmann::json Scene::serialize(Object3D *o)
    {
        auto ret = nlohmann::basic_json();
        Scene *s = dynamic_cast<Scene*>(o);
        if(s)
        {
            ret["camera"]["turn_speed"] = s->camera()->turn_speed();
            ret["camera"]["speed"] = s->camera()->speed();
            ret["camera"]["free_roam"] = s->camera()->free_roam();
            ret["camera"]["id"] = o->id();
            ret["camera"]["position"]["x"] = o->position().x;
            ret["camera"]["position"]["y"] = o->position().y;
            ret["camera"]["position"]["z"] = o->position().z;
            ret["camera"]["rotation"]["x"] = o->rotation().x;
            ret["camera"]["rotation"]["y"] = o->rotation().y;
            ret["camera"]["rotation"]["z"] = o->rotation().z;
            ret["camera"]["scale"]["x"] = o->scale().x;
            ret["camera"]["scale"]["y"] = o->scale().y;
            ret["camera"]["scale"]["z"] = o->scale().z;
            ret["light"]["color"]["r"] = s->light()->color().r;
            ret["light"]["color"]["g"] = s->light()->color().g;
            ret["light"]["color"]["b"] = s->light()->color().b;
            ret["light"]["ambient_intensity"] = s->light()->ambient_intensity();
            ret["light"]["diffuse_intensity"] = s->light()->diffuse_intensity();
            ret["light"]["enabled"] = s->light()->enabled();
            ret["light"]["id"] = o->id();
            ret["light"]["position"]["x"] = o->position().x;
            ret["light"]["position"]["y"] = o->position().y;
            ret["light"]["position"]["z"] = o->position().z;
            ret["light"]["rotation"]["x"] = o->rotation().x;
            ret["light"]["rotation"]["y"] = o->rotation().y;
            ret["light"]["rotation"]["z"] = o->rotation().z;
            ret["light"]["scale"]["x"] = o->scale().x;
            ret["light"]["scale"]["y"] = o->scale().y;
            ret["light"]["scale"]["z"] = o->scale().z;
        }
        else
        {
            Light *l = dynamic_cast<Light*>(o);
            if(l)
            {
                ret["color"]["r"] = l->color().r;
                ret["color"]["g"] = l->color().g;
                ret["color"]["b"] = l->color().b;
                ret["ambient_intensity"] = l->ambient_intensity();
                ret["diffuse_intensity"] = l->diffuse_intensity();
                ret["enabled"] = l->enabled();
            }
            Camera *c = dynamic_cast<Camera*>(o);
            if(c)
            {
                ret["turn_speed"] = c->turn_speed();
                ret["speed"] = c->speed();
                ret["free_roam"] = c->free_roam();
            }
        }

        ret["id"] = o->id();
        ret["position"]["x"] = o->position().x;
        ret["position"]["y"] = o->position().y;
        ret["position"]["z"] = o->position().z;
        ret["rotation"]["x"] = o->rotation().x;
        ret["rotation"]["y"] = o->rotation().y;
        ret["rotation"]["z"] = o->rotation().z;
        ret["scale"]["x"] = o->scale().x;
        ret["scale"]["y"] = o->scale().y;
        ret["scale"]["z"] = o->scale().z;

        for(auto * c : o->children())
        {
            ret["children"].push_back(serialize(c));
        }

        return ret;
    }

    nlohmann::json Scene::to_json()
    {
        // std::function<nlohmann::json(Object3D*)> serialize = [&](Object3D *o) -> nlohmann::json {
        // };

        return serialize(this);
    }
} // namespace nle
