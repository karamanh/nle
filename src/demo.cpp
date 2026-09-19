#include "model/model_obj.h"
#include "model/model_gltf.h"
#include "model/gltf_instance_3d.h"
#include "effects/emitter_3d.h"
#include "core/clock.h"

#include <glm/gtc/random.hpp>

#include <string>

#include "nle.h"

#ifndef NLE_SHADER_DIR
#define NLE_SHADER_DIR "shader"
#endif

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    nle::nle app;

    // auto rockfieldmodel = nle::make_ref<nle::model_obj>("nle_projdir/models/RedRocks-DroneMapper-OBJ/DroneMapper3D-Mesh_textured.obj");
    // auto executionermodel = nle::make_ref<nle::model_obj>("nle_projdir/models/Executioner.obj");
    // auto executionermodel = nle::make_ref<nle::model_obj>("nle_projdir/models/airplane_v2_L2.123c71795678-4b63-46c4-b2c6-549c45f4c806/11805_airplane_v2_L2.obj");
    auto executionermodel = nle::make_ref<nle::model_obj>("nle_projdir/models/Airplane.obj");
    auto spaceshipmodel = nle::make_ref<nle::model_obj>("nle_projdir/models/SkyNebula.obj");
    auto asteroidmodel = nle::make_ref<nle::model_obj>("nle_projdir/models/Asteroid1.obj");
    auto shader = nle::make_ref<nle::shader>(std::string(NLE_SHADER_DIR) + "/default_vert.glsl",
                                             std::string(NLE_SHADER_DIR) + "/default_frag.glsl",
                                             nle::shader_source::file);
    shader->load();

    auto scene = nle::make_ref<nle::scene_3d>();
    scene->set_shader(shader);
    scene->light()->set_rotation({-45.0f, 45.0f, 0.0f});
    scene->set_sky(nle::make_ref<nle::sky>());
    scene->camera()->set_field_of_view(45.0f);
    scene->camera()->set_position(glm::vec3(0.0f, 4.0f, 12.0f));
    // tilt down a little, so the demo opens looking at the scene and not at sky
    scene->camera()->set_rotation({-10.0f, 0.0f, 0.0f});
    scene->camera()->set_turn_speed(0.2f);
    scene->camera()->set_speed(0.2f);
    scene->camera()->set_far(200000);

    auto spaceship = spaceshipmodel->create_instance();
    auto material = nle::make_ref<nle::material>();
    material->set_shininess(4.0f);

    // auto map_bounds_mi = nle::make_ref<nle::mesh_instance_3d>(nle::make_ref<nle::boxmesh>());
    // scene->add_child(map_bounds_mi);
    // map_bounds_mi->set_scale({200, 200, 200});
    // map_bounds_mi->set_primitive_type(nle::primitive_type::lines);

    auto emitter = nle::make_ref<nle::emitter_3d>(asteroidmodel->multimesh(), 100);
    emitter->set_rotation({0.0f, 180.0f, 0.0f});
    emitter->set_position({-5.0f, 0.0f, 5.0f});
    emitter->set_scale(glm::vec3(0.1f));
    emitter->set_emission_radius(0.5f);
    emitter->set_maximum_distance(10.0f);
    // scene->add_child(emitter);

    auto executioner = executionermodel->create_instance();
    executioner->add_child(emitter);
    scene->add_child(executioner);

    // ---- point lights ---------------------------------------------------
    // Three coloured lamps around the origin. They are ordinary objects, so
    // they can be moved, parented and toggled like anything else.
    std::vector<nle::ref<nle::point_light>> lamps;

    for(const auto& [color, offset] : std::vector<std::pair<glm::vec3, glm::vec3>>{
            {{1.0f, 0.2f, 0.2f}, {6.0f, 2.0f, 0.0f}},
            {{0.2f, 1.0f, 0.3f}, {-3.0f, 2.0f, 5.0f}},
            {{0.3f, 0.4f, 1.0f}, {-3.0f, 2.0f, -5.0f}}})
    {
        auto lamp = nle::make_ref<nle::point_light>(color, 25.0f);
        lamp->set_position(offset);
        scene->add_child(lamp);
        lamps.push_back(lamp);
    }

    // ---- an animated glTF model ----------------------------------------
    nle::ref<nle::gltf_instance_3d> bar;
    try
    {
        auto barmodel = nle::make_ref<nle::model_gltf>(std::string(NLE_PROJECT_DIR) + "/tests/assets/skinned_bar.gltf");
        bar = barmodel->create_gltf_instance();
        bar->set_position({0.0f, 0.0f, 4.0f});
        scene->add_child(bar);

        if(!bar->animator()->play("bend"))
        {
            nle::utils::prerror("demo: the bar model has no 'bend' animation");
        }

        nle::utils::print("demo: glTF animations available:", barmodel->animation_names().size());
    }
    catch(const std::exception& e)
    {
        nle::utils::prerror("demo: could not load the glTF model:", e.what());
    }

    auto thread = std::thread([&](){
        nle::clock clk;
        while (app.window()->closed() == false)
        {
            // go up and down slowly
            float time = static_cast<float>(clk.elapsed_time_ms()) / 1000.0f;
            float height = std::sin(time) * 0.5f;
            executioner->set_position({0.0f, height, 0.0f});

            // orbit the lamps so the attenuation is easy to see
            for(size_t i = 0; i < lamps.size(); ++i)
            {
                const float phase = time * 0.7f + static_cast<float>(i) * 2.0944f;
                lamps[i]->set_position({std::cos(phase) * 6.0f, 2.0f + std::sin(time + i) * 1.5f,
                                        std::sin(phase) * 6.0f});
            }
            // scene->camera()->set_position({0.0f, 5.0f + height, 10.0f});
        }
    });

    app.renderer_3d()->set_current_scene(scene);
    app.window()->input_handler()->sig_key_pressed.bind_callback([&](const int& key){

        switch(static_cast<nle::input_handler_glfw::key>(key))
        {
            case nle::input_handler_glfw::key::w:
                scene->camera()->move_forward();
                break;
            case nle::input_handler_glfw::key::s:
                scene->camera()->move_backwards();
                break;
            case nle::input_handler_glfw::key::d:
                scene->camera()->move_right();
                break;
            case nle::input_handler_glfw::key::a:
                scene->camera()->move_left();
                break;
            case nle::input_handler_glfw::key::e:
                scene->camera()->move_up();
                break;
            case nle::input_handler_glfw::key::q:
                scene->camera()->move_down();
                break;
            case nle::input_handler_glfw::key::space:
                // Example: Add a new child object to the scene
                {
                    auto new_object = asteroidmodel->create_instance();
                    new_object->set_position(glm::linearRand(glm::vec3(-50.0f, -50.0f, -50.0f), glm::vec3(50.0f, 50.0f, 50.0f)));
                    new_object->set_rotation(glm::linearRand(glm::vec3(0.0f), glm::vec3(360.0f)));
                    new_object->set_scale(glm::vec3(0.5f));
                    scene->add_child(new_object);
		    nle::utils::print("number of asteroids", scene->children().size());
                }
                break;
            default:
                break;
        }
    });

    app.window()->input_handler()->sig_key_just_pressed.bind_callback([&](const int& key){
        switch (static_cast<nle::input_handler_glfw::key>(key))
        {
        case nle::input_handler_glfw::key::f    : // Toggle fullscreen
            app.window()->set_fullscreen(!app.window()->fullscreen());
            break;
        case nle::input_handler_glfw::key::left_control: // Toggle free roam
            app.renderer_3d()->current_scene()->camera()->set_free_roam(!app.renderer_3d()->current_scene()->camera()->free_roam());
            app.window()->set_cursor_visibility(!app.renderer_3d()->current_scene()->camera()->free_roam());
            break;
        case nle::input_handler_glfw::key::l: // Toggle the point lights
            for(auto& lamp : lamps)
            {
                lamp->set_enabled(!lamp->enabled());
            }
            break;
        case nle::input_handler_glfw::key::p: // Pause/resume the glTF animation
            if(bar)
            {
                bar->animator()->playing() ? bar->animator()->pause()
                                           : bar->animator()->resume();
            }
            break;
        case nle::input_handler_glfw::key::escape: // Close the window
            app.window()->close();
            break;
        default:
            break;
        }
    });

    app.window()->input_handler()->sig_mouse_moved.bind_callback([&](int dx, int dy){
        auto camera = app.renderer_3d()->current_scene()->camera();

        if(!camera->free_roam())
        {
            return;
        }

        float dxf = dx * camera->turn_speed();
        float dyf = dy * camera->turn_speed();

        glm::vec3 camrot = camera->rotation();

        float pitch = camrot.x + dyf;
        // float yaw = camrot.y + dx;
        float yaw = camrot.y - dxf;

        if (pitch > 89.f)
        {
            pitch = 89.f;
        }
        if (pitch < -89.f)
        {
            pitch = -89.f;
        }

        camera->set_rotation({pitch, yaw, 0.0f});

        // nle::utils::prdebug(camera->rotation().x, camera->rotation().y, camera->rotation().z);
    });

    app.run();
    thread.join();

    return 0;
}
