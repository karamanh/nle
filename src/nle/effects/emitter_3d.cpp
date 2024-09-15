#include "emitter_3d.h"
#include <glm/gtc/random.hpp>

namespace nle
{
    emitter_3d::emitter_3d(nle::ref<multimesh_3d> particle_multimesh, int number_of_particles)
        : m_particle_multimesh(particle_multimesh), m_emission_direction(0.0f, 0.0f, 1.0f), m_number_of_particles(number_of_particles)
    {
        this->generate_particles();
    }

    void emitter_3d::set_emission_radius(float radius)
    {
        m_emission_radius = radius;
    }

    float emitter_3d::emission_radius()
    {
        return m_emission_radius;
    }

    void emitter_3d::set_number_of_particles(int number_of_particles)
    {
        m_number_of_particles = number_of_particles;
    }

    int emitter_3d::number_of_particles()
    {
        return m_number_of_particles;
    }

    void emitter_3d::set_maximum_distance(float distance)
    {
        m_max_distance = distance;
    }

    float emitter_3d::maximum_distance()
    {
        return m_max_distance;
    }

    void emitter_3d::set_particle_multimesh(nle::ref<multimesh_3d> multimesh)
    {
        std::lock_guard<std::mutex> lck(m_mtx_multimesh);
        for(auto child : this->children())
        {
            this->delete_child(child);
        }
        m_particle_multimesh = multimesh;
        this->generate_particles();
    }

    ref<multimesh_3d> emitter_3d::particle_multimesh()
    {
        return m_particle_multimesh;
    }

    void emitter_3d::render()
    {
        for(auto& particle : render_objects())
        {
            particle->move_forward();
            auto diff = particle->position() - this->position();
            if(std::fabs(diff.x) > m_max_distance || std::fabs(diff.y) > m_max_distance || std::fabs(diff.z) > m_max_distance)
            {
                particle->set_position(this->position() + glm::sphericalRand(m_emission_radius));
                // particle->set_position(this->position() + glm::vec3(glm::circularRand(m_emission_radius), 0.0f));
            }
            particle->render();
        }
    }

    void emitter_3d::generate_particles()
    {
        struct timespec ts;
        for(int i = 0; i < m_number_of_particles; i++)
        {
            clock_gettime(CLOCK_MONOTONIC, &ts);

            srand((time_t)ts.tv_nsec);
            auto tickle = make_ref<multimesh_instance_3d>(m_particle_multimesh);
            tickle->set_speed((rand() % 100) * 0.01f);

            tickle->set_position(this->position() + glm::vec3(glm::sphericalRand(m_emission_radius)));
            // tickle->set_position(this->position() + glm::vec3(glm::circularRand(m_emission_radius), 0.0f));
            this->add_child(tickle);
        }
    }
} // namespace nle
