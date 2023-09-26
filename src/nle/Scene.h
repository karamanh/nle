#ifndef SCENE_H
#define SCENE_H

#include "Camera.h"
#include "Light.h"
#include "Sky.h"
#include "RenderObject3D.h"

#include "../vendor/nlohmann/json.hpp"

#include <vector>

namespace nle
{
    class Scene : public Object3D
    {
    private:
        Camera *m_camera = nullptr;
        Camera *m_default_camera = nullptr;
        Sky *m_sky = nullptr;
        Sky *m_default_sky = nullptr;
        Light *m_light = nullptr;
        std::vector<RenderObject3D*> m_render_objects;
        nlohmann::json serialize(Object3D *o);

    public:
        Scene();
        virtual ~Scene();

        Camera *camera();
        void set_camera(Camera *c);

        Light *light();

        Sky *sky();
        void set_sky(Sky *sky);

        void register_render_object(RenderObject3D *object);
        void delete_render_object(RenderObject3D *object);
        std::vector<RenderObject3D*> render_objects();
        void add_child(Object3D *child) override;
        void delete_child(Object3D *child, bool destroy = true) override;
        void set_id(const std::string& id) override;
        nlohmann::json to_json() override;
        void from_json(const nlohmann::json& j) override;

        friend class Renderer3D;
    };
} // namespace nle

#endif