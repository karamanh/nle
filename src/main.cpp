#include "nle/Window.h"
#include "nle/Scene.h"
#include "nle/Renderer3D.h"
#include "nle/Texture.h"
#include "nle/Mesh.h"
#include "nle/MultiMesh.h"
#include "nle/Model.h"
#include "nle/Nle.h"

#include <glm/gtc/random.hpp>

#include "FastNoiseLite.h"

int main(int argc, char *argv[])
{
	nle::Nle app;

	app.window()->input_handler()->key_pressed().bind_callback([&](const int& key){
		switch(key)
		{
			case GLFW_KEY_ESCAPE:
			app.window()->close();
			break;
			case GLFW_KEY_F:
			app.window()->set_fullscreen(!app.window()->fullscreen());
			break;
			case GLFW_KEY_LEFT_CONTROL:
			app.current_scene()->camera()->set_free_roam(!app.current_scene()->camera()->free_roam());
			app.window()->set_cursor_visibility(!app.current_scene()->camera()->free_roam());
			break;
			case GLFW_KEY_G:
			app.renderer()->gui()->set_visible(!app.renderer()->gui()->visible());
			break;
		}
	});

	app.current_scene()->set_id("root");
	app.current_scene()->camera()->set_rotation({-15.f, 0.f, 0.f});
	app.current_scene()->camera()->set_position({-50.f, 50.f, -50.f});
	app.current_scene()->light()->set_position(glm::vec3(0.f, 100.f, 0.f));
	app.current_scene()->light()->set_rotation({180.f, 0.f, 0.f});
	app.current_scene()->light()->set_ambient_intensity(0.5f);
	app.current_scene()->light()->set_diffuse_intensity(1.0f);
	app.current_scene()->light()->set_enabled(true);

	app.renderer()->gui()->set_draw_callback([&](){
		ImGuiIO& io = ImGui::GetIO();
		
		glm::vec3 v3val;
		float fval;
		bool bval;
		int ival;

		static float right_window_w = 300.f;
		static float bottom_window_h = 300.f;
		static bool file_dialog_open = false;
		static bool about_open = false;
		static char inbuf[512] = {0};
		static nle::RenderObject3D * selected_obj = nullptr;
		ImVec2 menubar_size;
		ImVec2 display_size = io.DisplaySize;

        if(ImGui::BeginMainMenuBar())
        {
            if(ImGui::BeginMenu("file"))
            {
				file_dialog_open = ImGui::MenuItem("load model");

                if(ImGui::MenuItem("quit"))
                {
					app.window()->close();
                }
                ImGui::EndMenu();
            }
            if(ImGui::BeginMenu("edit"))
            {
				ImGui::MenuItem("preferences");
                ImGui::EndMenu();
            }
            if(ImGui::BeginMenu("help"))
            {
				about_open = ImGui::MenuItem("about");

                ImGui::EndMenu();
            }

			menubar_size = ImGui::GetWindowSize();

            ImGui::EndMainMenuBar();
        }

		if(file_dialog_open)
		{
			ImGui::SetNextWindowPos(ImVec2(display_size.x * 0.5f, display_size.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f,0.5f));
			if(ImGui::Begin("enter path to *.obj file", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_Modal | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::InputText("model path", inbuf, sizeof(inbuf) -1);
				if(ImGui::Button("load"))
				{
					std::string path(inbuf, strlen(inbuf)); // = std::string(buf, sizeof(buf));
					nle::Model mod(path, nle::DEFAULT_SHADER);
					auto * instance = mod.create_instance();
					app.current_scene()->add_child(instance);
					file_dialog_open = false;
				}
				ImGui::SameLine();
				if(ImGui::Button("cancel"))
				{
					file_dialog_open = false;
				}
				ImGui::End();
			}
		}

		if(about_open)
		{
			ImGui::SetNextWindowPos(ImVec2(display_size.x * 0.5f, display_size.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f,0.5f));
			if(ImGui::Begin("about", nullptr, ImGuiWindowFlags_NoCollapse))
			{
				ImGui::Text("https://github.com/grizzlei/nle");
				ImGui::Text("hasan karaman - 2023");
				ImGui::Text("experimental opengl renderer");
				about_open = !ImGui::Button("close");
				ImGui::End();
			}
		}

		ImGui::SetNextWindowPos(ImVec2(0.f, menubar_size.y), ImGuiCond_Always, ImVec2(0.f,0.f));
		ImGui::SetNextWindowSize({0.f, io.DisplaySize.y - menubar_size.y});
        if(ImGui::Begin("control panel", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
        {
			// renderer settings
            ImGui::TextWrapped("renderer settings");
			bval = app.window()->fullscreen();
			ImGui::Checkbox("fullscreen", &bval);
			app.window()->set_fullscreen(bval);
			auto rla = app.renderer()->render_layer_attributes();
			ImGui::SliderInt("render distance", &rla.render_distance, 0, 100000);
			ImGui::Checkbox("render layer [0] visible", &rla.visible);
			app.renderer()->set_render_layer_attributes(nle::RenderLayer::_0, rla);

			// light settings
			ImGui::TextWrapped("light settings:");
			fval = app.current_scene()->light()->ambient_intensity();
			ImGui::SliderFloat("ambient intensity", &fval, 0.f, 1.f);
			app.current_scene()->light()->set_ambient_intensity(fval);

			fval = app.current_scene()->light()->diffuse_intensity();
			ImGui::SliderFloat("diffuse intensity", &fval, 0.f, 1.f);
			app.current_scene()->light()->set_diffuse_intensity(fval);
			
			ImGui::Separator();
			
			// camera settings
			ImGui::TextWrapped("camera settings:");
			bval = app.current_scene()->camera()->free_roam();
			ImGui::Checkbox("free roam", &bval);
			app.current_scene()->camera()->set_free_roam(bval);
			fval = app.current_scene()->camera()->speed();
			ImGui::SliderFloat("speed", &fval, 0.f, 50.f);
			app.current_scene()->camera()->set_speed(fval);

			fval = app.current_scene()->camera()->turn_speed();
			ImGui::SliderFloat("turn speed", &fval, 0.f, 50.f);
			app.current_scene()->camera()->set_turn_speed(fval);

			if(ImGui::CollapsingHeader("camera transform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::PushID("cam_position");
				v3val = app.current_scene()->camera()->position();
				ImGui::SetNextItemWidth(60.f);
				ImGui::InputFloat("x", &v3val.x);
				ImGui::SameLine();
				ImGui::SetNextItemWidth(60.f);
				ImGui::InputFloat("y", &v3val.y);
				ImGui::SameLine();
				ImGui::SetNextItemWidth(60.f);
				ImGui::InputFloat("z", &v3val.z);
				app.current_scene()->camera()->set_position(v3val);
				ImGui::PopID();
				
				ImGui::PushID("cam_rotation");
				v3val = app.current_scene()->camera()->rotation();
				ImGui::SetNextItemWidth(60.f);
				ImGui::InputFloat("x", &v3val.x);
				ImGui::SameLine();
				ImGui::SetNextItemWidth(60.f);
				ImGui::InputFloat("y", &v3val.y);
				ImGui::SameLine();
				ImGui::SetNextItemWidth(60.f);
				ImGui::InputFloat("z", &v3val.z);
				app.current_scene()->camera()->set_rotation(v3val);
				ImGui::PopID();
				
				ImGui::PushID("cam_scale");
				v3val = app.current_scene()->camera()->scale();
				ImGui::SetNextItemWidth(60.f);
				ImGui::InputFloat("x", &v3val.x);
				ImGui::SameLine();
				ImGui::SetNextItemWidth(60.f);
				ImGui::InputFloat("y", &v3val.y);
				ImGui::SameLine();
				ImGui::SetNextItemWidth(60.f);
				ImGui::InputFloat("z", &v3val.z);
				app.current_scene()->camera()->set_scale(v3val);
				ImGui::PopID();
			}
			
			ImGui::Separator();

			ImGui::TextWrapped("scene");
			for(auto * i : app.current_scene()->children())
			{
				nle::MultiMeshInstance *mi = dynamic_cast<nle::MultiMeshInstance*>(i);
				if(mi)
				{
					if(ImGui::Button(i->id().c_str()))
					{
						mi->set_render_mode(mi->render_mode() == nle::Fill ? nle::Line : nle::Fill);
						if(selected_obj)
						{
							selected_obj = nullptr;
						}
						else
							selected_obj = mi;
					}
				}
			}

			// right window
			ImGui::SetNextWindowPos(ImVec2(display_size.x -right_window_w, menubar_size.y));
			ImGui::SetNextWindowSize(ImVec2(right_window_w, display_size.y));
			if(ImGui::Begin("object properties", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize))
			{
				right_window_w = ImGui::GetWindowSize().x;

				if(selected_obj)
				{
					char idbuf[64] = {0};
					strncpy(idbuf, selected_obj->id().c_str(), sizeof(idbuf) - 1);
					if(ImGui::InputText("id", idbuf, sizeof(idbuf) -1))
					{
						if(strlen(idbuf) > 0)
							selected_obj->set_id(idbuf);
					}

					if(ImGui::CollapsingHeader("transform", ImGuiTreeNodeFlags_DefaultOpen))
					{
						ImGui::Text("position");
						ImGui::PushID("position");
						v3val =selected_obj->position();
						ImGui::SetNextItemWidth(60.f);
						ImGui::InputFloat("x", &v3val.x);
						ImGui::SameLine();
						ImGui::SetNextItemWidth(60.f);
						ImGui::InputFloat("y", &v3val.y);
						ImGui::SameLine();
						ImGui::SetNextItemWidth(60.f);
						ImGui::InputFloat("z", &v3val.z);
						selected_obj->set_position(v3val);
						ImGui::PopID();

						ImGui::Text("rotation");
						ImGui::PushID("rotation");
						v3val = selected_obj->rotation();
						ImGui::SetNextItemWidth(60.f);
						ImGui::InputFloat("x", &v3val.x);
						ImGui::SameLine();
						ImGui::SetNextItemWidth(60.f);
						ImGui::InputFloat("y", &v3val.y);
						ImGui::SameLine();
						ImGui::SetNextItemWidth(60.f);
						ImGui::InputFloat("z", &v3val.z);
						selected_obj->set_rotation(v3val);
						ImGui::PopID();

						ImGui::Text("scale");
						ImGui::PushID("scale");
						v3val = selected_obj->scale();
						ImGui::SetNextItemWidth(60.f);
						ImGui::InputFloat("x", &v3val.x);
						ImGui::SameLine();
						ImGui::SetNextItemWidth(60.f);
						ImGui::InputFloat("y", &v3val.y);
						ImGui::SameLine();
						ImGui::SetNextItemWidth(60.f);
						ImGui::InputFloat("z", &v3val.z);
						selected_obj->set_scale(v3val);
						ImGui::PopID();
					}
				}
				ImGui::End();
			}

			// if(selected_obj != nullptr)
			// {
			// }

            ImGui::End();
        }
	});

	nle::Material material(4.0f, 32);

	auto skybox = nle::Model("models/skybox/skybox.obj", nle::DEFAULT_SHADER, app.texture_factory()->load_and_get("models/skybox/skybox.png", "skybox0"));
	auto skybox_ins = skybox.create_instance();
	skybox_ins->set_scale(glm::vec3(200.f));
	app.current_scene()->add_child(skybox_ins);

	// auto rla = app.renderer()->render_layer_attributes(nle::RenderLayer::_0);
	// rla.render_distance = 1000;
	// app.renderer()->set_render_layer_attributes(nle::RenderLayer::_0, rla);

	// nle::Model mod_minecraft_block("models/Minecraft_Grass_Block_OBJ/Grass_Block.obj", nle::DEFAULT_SHADER, app.texture_factory()->load_and_get("models/Minecraft_Grass_Block_OBJ/Grass_Block_TEX.png", "grass_block"));
	// nle::Model mod_hasan("models/hasan/hasan.obj", nle::DEFAULT_SHADER, app.texture_factory()->load_and_get("models/hasan/hasan.jpg", "hasan"));
	// nle::Model mod_camera("models/obj-camera/source/Obj_Camera.obj", nle::DEFAULT_SHADER, app.texture_factory()->load_and_get("models/obj-camera/textures/Texture_OldCamera_copy.png", "camera"));
	// nle::Model mod_car("models/hugecity/hugecity.obj", nle::DEFAULT_SHADER);
	// // nle::Model mod_car("models/85-cottage_obj/cottage_obj.obj", nle::DEFAULT_SHADER, app.texture_factory()->load_and_get("models/85-cottage_obj/cottage_textures/cottage_diffuse.png", "cottage"));

	// prdbg("%d blocks created", app.current_scene()->render_objects().size());

	// hasan = mod_hasan.create_instance();
	// hasan->set_scale(glm::vec3(10.f));
	// app.current_scene()->add_child(hasan);
	// hasan->set_position(app.current_scene()->light()->position());

	// auto *house = mod_car.create_instance();
	// house->set_material_for_meshes(&material);
	// app.current_scene()->add_child(house);

	app.run();
	// prdbg("%s", app.current_scene()->to_json().dump().c_str());

	return (0);
}