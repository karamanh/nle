#include "sky.h"
#include "../mesh/box_mesh.h"
#include "../static/default_sky.hpp"

namespace nle
{

sky::sky()
    : mesh_instance_3d(make_ref<boxmesh>())
{
    this->mesh()->set_texture(make_ref<texture>(default_sky_png, default_sky_png_len));
    this->mesh()->material()->set_accept_light(false);
}

sky::~sky()
{
}

void sky::render()
{
    glDepthMask(GL_FALSE);
    mesh_instance_3d::render();
    glDepthMask(GL_TRUE);
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
