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

#include <filesystem>

int main(int argc, char *argv[])
{
	nle::Nle app;

	std::map<std::string, nle::Model*> models;
	std::map<std::string, nle::Material*> materials;


    // for (auto const& dir_entry : std::filesystem::recursive_directory_iterator("."))
    // {
	// 	std::string path = dir_entry.path().c_str();
	// 	std::string name;
	// 	if((name = path.substr(path.find_last_of('.'))).find(".obj") != std::string::npos)
	// 	{
	// 		// prdbg(name.c_str());
	// 		models[name] = new nle::Model(path, nle::DEFAULT_SHADER);
	// 	}
    // }
	// return 0;

	app.window()->input_handler()->key_pressed().bind_callback([&](const int& key){
		switch(key)
		{
			case GLFW_KEY_LEFT_CONTROL:
			app.current_scene()->camera()->set_free_roam(!app.current_scene()->camera()->free_roam());
			app.window()->set_cursor_visibility(!app.current_scene()->camera()->free_roam());
			break;
		}
	});

	app.current_scene()->set_id("root");
	app.current_scene()->camera()->set_rotation({-30.f, 135.f, 0.f});
	app.current_scene()->camera()->set_position({5.f, 5.f, -5.f});
	app.current_scene()->light()->set_position(glm::vec3(0.f, 100.f, 0.f));
	app.current_scene()->light()->set_rotation({180.f, 0.f, 0.f});
	app.current_scene()->light()->set_ambient_intensity(0.5f);
	app.current_scene()->light()->set_diffuse_intensity(1.0f);
	app.current_scene()->light()->set_enabled(true);

	nle::Material material(4.0f, 32);

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
					std::string path(inbuf, strlen(inbuf));
					if(!path.empty())
					{
						if(std::filesystem::exists(path))
						{
							auto * m = new nle::Model(path, nle::DEFAULT_SHADER);
							if(m->name().empty())
							{
								prerr("model name missing");
								delete m;
								file_dialog_open = false;
							}
							else
							{
								models[m->name()] = m;
								file_dialog_open = false;
							}
						}
					}
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
			if(ImGui::Begin("about", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
			{
				ImGui::Text("https://github.com/grizzlei/nle");
				ImGui::Text("experimental opengl renderer");
				ImGui::Text("hasan karaman - hk@hasankaraman.dev - 2023");
				about_open = !ImGui::Button("close");
				ImGui::End();
			}
		}

		// left window
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
			ImGui::Checkbox("free roam [ctrl]", &bval);
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

			if(ImGui::CollapsingHeader("loaded models", ImGuiTreeNodeFlags_DefaultOpen))
			{
				for(const auto& it : models)
				{
					ImGui::Text(it.second->name().c_str());
					ImGui::SameLine();
					ImGui::PushID(it.second->name().c_str());
					if(ImGui::Button("create instance"))
					{
						prdbg("created instance pressed");
						auto * instance = it.second->create_instance();
						if(instance)
						{
							prdbg("created instance with id: %s", instance->id().c_str());
							// instance->set_material(&material);
							app.current_scene()->add_child(instance);
						}
					}
					ImGui::PopID();
				}
			}

			ImGui::Separator();

			ImGui::TextWrapped("current scene [%s]", app.current_scene()->id().c_str());
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

			

            ImGui::End();
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
				
				ImGui::Separator();

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

				ImGui::Separator();

				if(ImGui::CollapsingHeader("material", ImGuiTreeNodeFlags_DefaultOpen))
				{
					nle::MultiMeshInstance *instance;
					if((instance = dynamic_cast<nle::MultiMeshInstance*>(selected_obj)))
					{
						nle::Material *material;
						if((material = instance->material()))
						{
							ImGui::PushID("specular_intensity");
							fval = material->specular_intensity();
							ImGui::InputFloat("specular intensity", &fval);
							material->set_specular_intensity(fval);
							ImGui::PopID();
							ImGui::PushID("shininess");
							fval = material->shininess();
							ImGui::InputFloat("shininess", &fval);
							material->set_shininess(fval);
							ImGui::PopID();

							if(ImGui::Button("clear material"))
							{
								instance->set_material(nullptr);
							}
						}
					}
				}
				
				ImGui::Separator();

				if(ImGui::Button("delete object"))
				{
					app.current_scene()->delete_child(selected_obj);
					selected_obj = nullptr;
				}

				if(ImGui::Button(selected_obj->physics_enabled() ? "disable_physics" : "enable physics"))
				{
					if(selected_obj->physics_enabled())
					{
						app.physics_engine()->detach_physics_body(selected_obj);
					}
					else
					{
						app.physics_engine()->attach_physics_body(selected_obj);
					}
				}
			}
			ImGui::End();
		}
	});

	app.run();

	prdbg("%s", app.current_scene()->to_json().dump().c_str());

	for(auto & model: models)
	{
		delete model.second;
	}

	for(auto & material: materials)
	{
		delete material.second;
	}

	return (0);
}