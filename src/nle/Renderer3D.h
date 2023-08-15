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

        float m_max_render_distance;
        std::unordered_map<RenderLayer, RenderLayerAttributes> m_render_layer_attributes;
        bool m_time_strict_mode;

        double m_render_timestamp;
        // double m_input_timestamp;
        // std::thread m_thr_input;
        Shader *m_grid;

        void render_recursively(Object3D *root);
        void render_scene(Scene *scene);
        void initialize();

    public:
        Renderer3D(Window *render_target);
        virtual ~Renderer3D();
        void render(MeshInstance *mi);
        void set_root_scene(Scene *root);
        Scene *root_scene() const;
        void set_render_layer_attributes(RenderLayer layer, RenderLayerAttributes attributes);
        RenderLayerAttributes render_layer_attributes(RenderLayer layer = RenderLayer::_0);
        ImGUI_GLFW * gui() const;
    };
} // namespace nle

#endif