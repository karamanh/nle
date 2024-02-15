#include "object/object_3d.h"
#include "mesh/mesh_3d.h"
#include "mesh/mesh_instance_3d.h"
#include "static/default_shader.h"

#include "nle.h"

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    auto shader = nle::make_ref<nle::shader>(nle::k_default_vertex_shader, nle::k_default_fragment_shader, nle::shader_source::memory);
    auto scene = nle::make_ref<nle::scene_3d>();

    nle::nle app;
    app.renderer_3d()->set_current_scene(scene);

    shader->load();
    app.run();

    return 0;
}