#ifndef SCENE_H
#define SCENE_H

#include "Camera.h"
#include "Light.h"

#include "vendor/nlohmann/json.hpp"

#include <vector>

namespace nle
{
    class Scene : public Object3D
    {
    private:
        Camera *m_camera;
        Light *m_light;
        std::vector<Object3D*> m_render_objects;
        nlohmann::json serialize(Object3D *o);

    public:
        Scene();
        virtual ~Scene();
        Camera *camera();
        void set_camera(Camera *c);
        Light *light();
        void register_render_object(Object3D *object);
        std::vector<Object3D*> render_objects();
        void add_child(Object3D *child) override;
        void set_id(const std::string& id) override;
        nlohmann::json to_json();

        friend class Renderer3D;
    };
} // namespace nle

#endif