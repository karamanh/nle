#include "PhysicsEngine.h"

#define NLE_PHYSICS_PROCESS_SLEEP_TIME 16666

namespace nle
{

    PhysicsEngine::PhysicsEngine()
        : m_realm(new PhysicsRealm)
    {
        m_physics_timeval = get_time_sec();
        m_thr_physics = std::thread(&PhysicsEngine::runner, this);
    }

    PhysicsEngine::~PhysicsEngine()
    {
        m_killed = true;
        
        if(m_thr_physics.joinable())
            m_thr_physics.join();

        delete m_realm;
    }

    void PhysicsEngine::attach_physics_body(Object3D *body)
    {
        m_realm->bodies.push_back(body);
        for(auto * c : body->children())
        {
            attach_physics_body(c);
        }
        body->set_physics_enabled(true);
    }

    void PhysicsEngine::detach_physics_body(Object3D *body)
    {
        auto it = std::find(m_realm->bodies.begin(), m_realm->bodies.end(), body);
        if(it != m_realm->bodies.end())
        {
            for(auto *c : body->children())
            {
                detach_physics_body(c);
            }
            m_realm->bodies.erase(it);
        }
        body->set_physics_enabled(false);
        body->set_velocity(glm::vec3(0.f));
    }

    void PhysicsEngine::bind_physics_process_callback(std::function<void(Object3D * object, double delta)> callback)
    {
        m_on_physics_process.bind_callback(callback);
    }

    void PhysicsEngine::bind_physics_tick_callback(std::function<void(double)> callback)
    {
        m_on_physics_tick.bind_callback(callback);
    }

    void PhysicsEngine::process(Object3D *body, double delta_time)
    {
        glm::vec3 d = m_realm->gravity * m_realm->gravityVector * (float)delta_time;
        body->set_velocity(body->velocity() + d);

        // @TODO: check for collisions and move if there's no obstacle

        body->set_position(body->position() + body->velocity());
        
        // fake collision
        auto pos = body->position();
        if(pos.y < 0.f)
        {
            pos.y = 0.f;
            auto vel = body->velocity();
            vel.y = 0.0;
            body->set_velocity(vel);
        }
        body->set_position(pos);
        // fake collision

        m_on_physics_process.emit(body, delta_time);
    }

    void PhysicsEngine::process_recursively(Object3D *root, double delta_time)
    {
        process(root, delta_time);

        for(auto * c : root->children())
        {
            process_recursively(c, delta_time);
        }
    }

    void PhysicsEngine::runner(void *_this)
    {
        PhysicsEngine *p = static_cast<PhysicsEngine*>(_this);
        if(p)
        {

            while(!p->m_killed)
            {
                double dtime = get_time_sec() - p->m_physics_timeval;
                for(auto * i : p->m_realm->bodies)
                {
                    p->process_recursively(i, dtime);
                }

                p->m_on_physics_tick.emit(dtime);
                p->m_physics_timeval = get_time_sec();
                // 60 hertz constant physics processing.
                std::this_thread::sleep_for(std::chrono::microseconds(NLE_PHYSICS_PROCESS_SLEEP_TIME));
            }
        }
    }
} // namespace nle
