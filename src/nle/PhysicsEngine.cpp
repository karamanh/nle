#include "PhysicsEngine.h"
#include "MultiMesh.h"

#define NLE_PHYSICS_PROCESS_SLEEP_TIME 16666

namespace nle
{

    PhysicsEngine::PhysicsEngine()
        : m_realm(new PhysicsRealm)
    {
        m_physics_timeval = get_time_sec();
        m_thr_physics = std::thread(std::bind(&PhysicsEngine::runner, this));
    }

    PhysicsEngine::~PhysicsEngine()
    {
        m_killed = true;
        m_realm->bodies.clear();

        if (m_thr_physics.joinable())
            m_thr_physics.join();

        delete m_realm;
    }

    void PhysicsEngine::attach_physics_body(PhysicsObject3D *body)
    {
        std::lock_guard<std::mutex> lck(m_mtx_realm);
        attach_physics_body_thread_safe(body);
    }

    void PhysicsEngine::detach_physics_body(PhysicsObject3D *body)
    {
        std::lock_guard<std::mutex> lck(m_mtx_realm);
        detach_physics_body_thread_safe(body);
    }

    void PhysicsEngine::bind_physics_process_callback(std::function<void(Object3D *object, double delta)> callback)
    {
        sig_physics_process.bind_callback(callback);
    }

    void PhysicsEngine::bind_physics_tick_callback(std::function<void(double)> callback)
    {
        sig_physics_tick.bind_callback(callback);
    }

    Signal<Object3D *, Object3D *> &PhysicsEngine::collision()
    {
        return sig_collision;
    }

    void PhysicsEngine::process_collisions(const std::list<PhysicsObject3D *> &bodies)
    {
        for(auto it1 = bodies.begin(); it1 != bodies.end(); ++it1)
        {
            for(auto it2 = std::next(it1); it2 != bodies.end(); ++it2)
            {
                auto * mmi1 = dynamic_cast<MultiMeshInstance*>(*it1);
                auto * mmi2 = dynamic_cast<MultiMeshInstance*>(*it2);

                if(mmi1 && mmi2)
                {
                    if(mmi1->aabb().collides_with(mmi2->aabb()))
                    {
                        sig_collision.emit(mmi1, mmi2);
                        // auto tmp = mmi2->velocity();
                        auto a = mmi1->velocity() + mmi2->velocity();
                        mmi2->set_velocity(a);
                        mmi1->set_velocity(a);
                    }
                }
            }
        }
    }

    void PhysicsEngine::runner()
    {
        while(!m_killed)
        {
            double tsnow = get_time_sec();
            double tsdelta = tsnow - m_physics_timeval;

            for(auto it1 = m_realm->bodies.begin(); it1 != m_realm->bodies.end(); ++it1)
            {
                /** process collisions */ 
                {
                    std::lock_guard<std::mutex> lck(m_mtx_realm);

                    for(auto it2 = std::next(it1); it2 != m_realm->bodies.end(); ++it2)
                    {
                        auto * mmi1 = dynamic_cast<MultiMeshInstance*>((*it1)->render_object());
                        auto * mmi2 = dynamic_cast<MultiMeshInstance*>((*it2)->render_object());
                        // prdbg("%f %f %f - %f %f %f", mmi1->aabb().min().x, mmi1->aabb().min().y, mmi1->aabb().min().z, mmi1->aabb().max().x, mmi1->aabb().max().y, mmi1->aabb().max().z);

                        if(mmi1 && mmi2)
                        {
                            if(mmi1->aabb().collides_with(mmi2->aabb()))
                            {
                                sig_collision.emit(mmi1, mmi2);
                                // we have simulated a little vertical bounce
                                // i'm going to implement further.
                                (*it1)->set_velocity(-(*it1)->velocity());
                                (*it2)->set_velocity(-(*it2)->velocity());
                            }
                        }
                    }
                }

                if(!(*it1)->rigid())
                {
                    glm::vec3 dgrav = m_realm->gravity * m_realm->gravity_vector * (float)tsdelta;
                    /** apply gravity */ 
                    (*it1)->set_velocity((*it1)->velocity() + dgrav);

                    /** move body */
                    (*it1)->set_position((*it1)->position() + (*it1)->velocity() * (float)tsdelta);
                }
            }

            sig_physics_tick.emit(tsdelta);
            m_physics_timeval = get_time_sec();
            std::this_thread::sleep_for(std::chrono::microseconds(NLE_PHYSICS_PROCESS_SLEEP_TIME));
        }
    }

    void PhysicsEngine::attach_physics_body_thread_safe(PhysicsObject3D *body)
    {
        auto it = std::find(m_realm->bodies.begin(), m_realm->bodies.end(), body);
        if(it == m_realm->bodies.end())
        {
            m_realm->bodies.push_back(body);
        }
        body->set_physics_enabled(true);
    }

    void PhysicsEngine::detach_physics_body_thread_safe(PhysicsObject3D *body)
    {
        auto it = std::find(m_realm->bodies.begin(), m_realm->bodies.end(), body);
        if (it != m_realm->bodies.end())
        {
            m_realm->bodies.erase(it);
        }
        body->set_physics_enabled(false);
        body->set_velocity(glm::vec3(0.f));
    }
} // namespace nle
