/**
 * @file emitter_3d.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief 
 * @version 0.1
 * @date 2024-09-13
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include "../mesh/multimesh_instance_3d.h"
#include "../renderer/render_object_3d.h"

namespace nle
{

class emitter_3d : public render_object_3d
{
public:
    emitter_3d(nle::ref<multimesh_3d> particle_multimesh, int number_of_particles = 10);

    void set_emission_radius(float radius);
    float emission_radius();

    void set_number_of_particles(int number_of_particles);
    int number_of_particles();

    void set_maximum_distance(float distance);
    float maximum_distance();

    void set_particle_multimesh(nle::ref<multimesh_3d> multimesh);
    ref<multimesh_3d> particle_multimesh();

private:
    std::mutex m_mtx_multimesh;
    ref<multimesh_3d> m_particle_multimesh;
    std::vector<ref<render_object_3d>> m_particles;
    glm::vec3 m_emission_direction;

    int m_number_of_particles = 10;
    float m_emission_radius = 10.0f;
    float m_max_distance = 10.0f;

    void render() override;

    void generate_particles();
};

} // namespace nle
