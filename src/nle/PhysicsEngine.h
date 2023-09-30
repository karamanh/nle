#ifndef NLE_PHYSICS_ENGINE_H
#define NLE_PHYSICS_ENGINE_H

#include "Scene.h"
#include "Signal.hpp"
#include "PhysicsObject3D.h"
#include "AABB.h"

#include <list>
#include <mutex>
#include <thread>

namespace nle
{
    struct PhysicsRealm
    {
        float gravity = 9.81f;
        glm::vec3 gravity_vector = {0.f, -1.f, 0.f};
        std::list<PhysicsObject3D *> bodies;
    };

    class PhysicsEngine
    {
    private:
        Signal<Object3D*, double> sig_physics_process;
        Signal<double> sig_physics_tick;
        Signal<Object3D*, Object3D*> sig_collision;
        PhysicsRealm *m_realm; // single realm now, multiple in the future
        Scene *m_scene;
        bool m_paused = false;
        bool m_killed = false;
        double m_physics_timeval;
        std::thread m_thr_physics;
        std::mutex m_mtx_realm;

        void process_collisions(const std::list<PhysicsObject3D*>& bodies);
        void runner();
        void attach_physics_body_thread_safe(PhysicsObject3D *body);
        void detach_physics_body_thread_safe(PhysicsObject3D *body);

    public:
        PhysicsEngine();
        ~PhysicsEngine();
        void attach_physics_body(PhysicsObject3D *body);
        void detach_physics_body(PhysicsObject3D *body);
        void bind_physics_process_callback(std::function<void(Object3D* object, double delta)> callback);
        void bind_physics_tick_callback(std::function<void(double)> callback);
        Signal<Object3D*, Object3D*> &collision();
    };
} // namespace nle

#endif