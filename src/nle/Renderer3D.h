#ifndef RENDERER_H
#define RENDERER_H

#include "Scene.h"
#include "Window.h"
#include "Mesh.h"
#include "MultiMesh.h"
#include "ImGUI_GLFW.h"

#include <thread>
#include <unordered_map>

namespace nle
{
    struct RenderLayerAttributes
    {
        int render_distance = 100;
        bool visible = true;
    };

    class Renderer3D
    {
    private:
        Scene *m_root_scene;
        Window *m_parent_window;
        ImGUI_GLFW *m_imgui;
        std::unordered_map<RenderLayer, RenderLayerAttributes> m_render_layer_attributes;
        Shader *m_grid;
        float m_max_render_distance;
        bool m_time_strict_mode;
        double m_render_timestamp;
        Signal <MultiMeshInstance*> sig_object_intersects;

        void render_recursively(Object3D *root);
        void render_scene(Scene *scene);
        void initialize();
        glm::vec3 get_mouse_ray(float mouse_x, float mouse_y, int screen_w, int screen_h, const glm::mat4 &projection, const glm::mat4 &view);
        bool test_ray_obb_inersection(glm::vec3 ray_origin, glm::vec3 ray_direction, glm::vec3 aabb_min, glm::vec3 aabb_max, glm::mat4 model, float &distance);
        Object3D *get_mouse_ray_target(int mouse_x, int mouse_y);

    public:
        Renderer3D(Window *render_target);
        virtual ~Renderer3D();
        void render(MeshInstance *mi);
        void set_root_scene(Scene *root);
        Scene *root_scene() const;
        void set_render_layer_attributes(RenderLayer layer, RenderLayerAttributes attributes);
        RenderLayerAttributes render_layer_attributes(RenderLayer layer = RenderLayer::_0);
        ImGUI_GLFW * gui() const;
        Signal <MultiMeshInstance*>& object_intersects();
    };
} // namespace nle

#endif