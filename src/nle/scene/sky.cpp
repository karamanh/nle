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

void sky::render(render_command_buffer& command_buffer, const render_context& context)
{
    command_buffer.set_depth_mask(false);
    mesh_instance_3d::render(command_buffer, context);
    command_buffer.set_depth_mask(true);
}

} // namespace nle
