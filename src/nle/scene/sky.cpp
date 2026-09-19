#include "sky.h"
#include "../static/default_sky.hpp"

namespace nle
{

sky::sky(ref<mesh_3d> mesh)
    : mesh_instance_3d(mesh)
{
    this->mesh()->set_texture(mesh->texture() ? mesh->texture() : make_ref<texture>(default_sky_png, default_sky_png_len));
    this->mesh()->material()->set_accept_light(false);
}

sky::~sky()
{
}

void sky::render(render_command_buffer& command_buffer)
{
    command_buffer.set_depth_mask(false);
    mesh_instance_3d::render(command_buffer);
    command_buffer.set_depth_mask(true);
}

void sky::set_distance_fog_far(float far)
{
    m_distance_fog_far = far;
}

float sky::distance_fog_far()
{
    return m_distance_fog_far;
}

void sky::set_distance_fog_near(float near)
{
    m_distance_fog_near = near;
}

float sky::distance_fog_near()
{
    return m_distance_fog_near;
}

} // namespace nle
