#ifndef NLE_H
#define NLE_H

#include "Shader.h"
#include "Renderer3D.h"
#include "PhysicsEngine.h"
#include "TextureFactory.h"
#include "Globals.h"

namespace nle
{   
    class Nle
    {
    private:
        Window *m_window;
        Renderer3D *m_renderer;
        TextureFactory *m_texture_factory;
        PhysicsEngine *m_physics_engine;

        void initialize();
        void cleanup();
    public:
        Nle(/* args */);
        ~Nle();
        Window *window();
        Renderer3D *renderer();
        TextureFactory *texture_factory();
        PhysicsEngine *physics_engine();
        Scene *current_scene();
        void set_current_scene(Scene *s);
        void run();
    };

} // namespace nle

#endif