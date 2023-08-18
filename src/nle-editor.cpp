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

#include <fstream>
#include <typeinfo> 
#include <filesystem>

std::string
nle_proj_extension = ".nleproj",
nle_scene_extension = ".nlescn",
nle_workdir = ".",
nle_projdir = nle_workdir + "/nle_projdir";

// void build_scene(nle::Scene *s, const nlohmann::json& j);

void object_builder(nle::Object3D **o, const nlohmann::json& j, std::map<std::string, nle::Model*>& models);
void build_scene_from_json(nle::Object3D *o, const nlohmann::json &j, std::map<std::string, nle::Model*>& models);

int main(int argc, char *argv[])
{
	nle::Nle app;

	std::map<std::string, nle::Scene*> scenes;
	std::map<std::string, nle::Model*> models;
	std::vector<nle::Material*> materials;
	std::vector<std::string> logs;
	std::size_t max_logs = 100;

	auto push_log = [&](const std::string& msg){
		prinf("%s", msg.c_str());
		logs.push_back(msg);
		if(logs.size() > max_logs)
		{
			logs.erase(logs.begin());
		}
	};

	prinf("autoloading assets");
	for (auto const& dir_entry : std::filesystem::recursive_directory_iterator(std::filesystem::current_path()))
	{
		std::string path = dir_entry.path().c_str();
		std::string name = dir_entry.path().filename();
		std::size_t n;

		if(dir_entry.path().filename().extension() == ".obj")
		{
			prdbg("loading %s", dir_entry.path().c_str());

			models[name] = new nle::Model(path, nle::DEFAULT_SHADER);
		}
	}
	prinf("%lu models loaded", models.size());

	app.current_scene()->set_id("root");
	app.current_scene()->camera()->set_rotation({-30.f, 135.f, 0.f});
	app.current_scene()->camera()->set_position({5.f, 5.f, -5.f});
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

		static float bottom_window_h = 300.f;
		static bool show_load_model = false, 
					show_load_scene = false,
					show_about = false;
		static char inbuf[512] = {0};
		static nle::RenderObject3D * selected_obj = nullptr;
		ImVec2 menubar_size, control_panel_size, assets_size, scene_size, bottom_window_size, object_properties_size = {300.f, 0.f}, display_size = io.DisplaySize;

        if(ImGui::BeginMainMenuBar())
        {
            if(ImGui::BeginMenu("file"))
            {
				show_load_model = ImGui::MenuItem("load model");

				ImGui::PushID("menuitem_save_screen");
					if(ImGui::MenuItem("save scene"))
					{
						push_log("saving scene " + app.current_scene()->to_json().dump());

						if(!std::filesystem::exists(nle_projdir))
						{
							if(!std::filesystem::create_directory(nle_projdir))
							{
								prerr("could not create directory %s", nle_projdir.c_str());
							}
						}

						std::ofstream ofs(nle_projdir + "/" + app.current_scene()->id() + nle_scene_extension);
						if(ofs.is_open())
						{
							ofs << app.current_scene()->to_json().dump(4, ' ');
							ofs.close();
						}
					}
				ImGui::PopID();

				if(show_load_scene = ImGui::MenuItem("load scene"))
				{
					// prerr("not implemented");
				}

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
				show_about = ImGui::MenuItem("about");

                ImGui::EndMenu();
            }

			menubar_size = ImGui::GetWindowSize();

            ImGui::EndMainMenuBar();
        }

		if(show_load_model)
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
							}
							else
							{
								models[m->name()] = m;
							}
							show_load_model = false;
						}
					}
				}
				ImGui::SameLine();
				if(ImGui::Button("cancel"))
				{
					show_load_model = false;
				}
				ImGui::End();
			}
		}

		if(show_load_scene)
		{
			ImGui::SetNextWindowPos(ImVec2(display_size.x * 0.5f, display_size.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f,0.5f));
			if(ImGui::Begin("enter path to scene", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_Modal | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize))
			{
				for (auto const& dir_entry : std::filesystem::recursive_directory_iterator(nle_projdir))
				{
					std::string path = dir_entry.path().c_str();
					std::string name = dir_entry.path().filename();
					std::size_t n;

					if(dir_entry.path().filename().extension() == nle_scene_extension)
					{
						if(ImGui::Button(dir_entry.path().c_str()))
						{
							memset(inbuf, 0x00, sizeof(inbuf));
							memcpy(inbuf, dir_entry.path().c_str(), sizeof(inbuf) - 1);
						}
					}
				}

				ImGui::InputText("scene path", inbuf, sizeof(inbuf) -1);
				if(ImGui::Button("load"))
				{
					std::string path(inbuf, strlen(inbuf));
					if(!path.empty())
					{
						if(std::filesystem::exists(path))
						{

							std::ifstream ifs(path);
							if(ifs.is_open())
							{
								auto j = nlohmann::json::parse(ifs);
								nle::Scene *new_scene;
								object_builder(reinterpret_cast<nle::Object3D**>(&new_scene), j, models);

								if(new_scene)
								{
									scenes[new_scene->id()] = new_scene;
									app.set_current_scene(new_scene);
									prinf("loaded scene %s", new_scene->id().c_str());
								}
								else
								{
									prerr("could not load scene from %s", path.c_str());
								}
								ifs.close();
							}


							show_load_scene = false;
						}
					}
				}
				ImGui::SameLine();
				if(ImGui::Button("cancel"))
				{
					show_load_scene = false;
				}
				ImGui::End();
			}
		}

		if(show_about)
		{
			ImGui::SetNextWindowPos(ImVec2(display_size.x * 0.5f, display_size.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f,0.5f));
			if(ImGui::Begin("about", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
			{
				ImGui::Text("https://github.com/grizzlei/nle");
				ImGui::Text("experimental opengl renderer");
				ImGui::Text("hasan karaman - hk@hasankaraman.dev - 2023");
				show_about = !ImGui::Button("close");
				ImGui::End();
			}
		}

		// left window
		ImGui::SetNextWindowPos(ImVec2(0.f, menubar_size.y));
		// ImGui::SetNextWindowPos(ImVec2(0.f, menubar_size.y), ImGuiCond_Always, ImVec2(0.f,0.f));
		// ImGui::SetNextWindowSize({0.f, io.DisplaySize.y - menubar_size.y});
        if(ImGui::Begin("general controls", nullptr, ImGuiWindowFlags_NoCollapse))
        {
			control_panel_size = ImGui::GetWindowSize();
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
			
            ImGui::End();
        }

		ImGui::SetNextWindowPos({0.f, menubar_size.y + control_panel_size.y});
		ImGui::SetNextWindowSize({control_panel_size.x, (display_size.y - menubar_size.y - control_panel_size.y) / 2});
		if(ImGui::Begin("assets", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{
			scene_size = assets_size = ImGui::GetWindowSize();

			if(ImGui::CollapsingHeader("loaded models", ImGuiTreeNodeFlags_DefaultOpen))
			{
				for(const auto& it : models)
				{
					ImGui::Text(it.second->name().c_str());
					ImGui::SameLine();
					ImGui::PushID(it.second->name().c_str());
					if(ImGui::Button("create instance"))
					{
						auto * instance = it.second->create_instance();
						if(instance)
						{
							app.current_scene()->add_child(instance);
						}
					}
					ImGui::PopID();
				}
			}

			ImGui::End();
		}

		// current scene window
		ImGui::SetNextWindowPos({0.f, menubar_size.y + control_panel_size.y + assets_size.y});
		ImGui::SetNextWindowSize({control_panel_size.x, scene_size.y});
		if(ImGui::Begin("current scene", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{
			bottom_window_size = ImGui::GetWindowSize();


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
						{
							selected_obj = mi;
						}
					}
				}
			}
			ImGui::End();
		}

		// current scene window
		ImGui::SetNextWindowPos({scene_size.x, menubar_size.y + control_panel_size.y + assets_size.y});
		ImGui::SetNextWindowSize({display_size.x - scene_size.x - object_properties_size.x, scene_size.y});
		if(ImGui::Begin("logs", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{
			for(auto & it: logs)
			{
				ImGui::TextWrapped(it.c_str());
			}
			ImGui::End();
		}

		// object manager window
		ImGui::SetNextWindowPos(ImVec2(display_size.x -object_properties_size.x, menubar_size.y));
		ImGui::SetNextWindowSize(ImVec2(object_properties_size.x, display_size.y));
		if(ImGui::Begin("object properties", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize))
		{
			object_properties_size = ImGui::GetWindowSize();

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
						else
						{
							if(ImGui::Button("new material"))
							{
								material = new nle::Material(1.f, 32.f);
								materials.push_back(material);
								instance->set_material(material);
							}
						}
					}
				}
				
				ImGui::Separator();

				if(ImGui::Button(selected_obj->physics_enabled() ? "disable_physics" : "enable physics"))
				{
					if(selected_obj->physics_enabled())
					{
						push_log("enabled physics for " + selected_obj->id());
						app.physics_engine()->detach_physics_body(selected_obj);
					}
					else
					{
						push_log("disabled physics for " + selected_obj->id());
						app.physics_engine()->attach_physics_body(selected_obj);
					}
				}

				if(ImGui::Button("delete object"))
				{
					push_log("deleted " + selected_obj->id());
					app.current_scene()->delete_child(selected_obj);
					selected_obj = nullptr;
				}
			}
			ImGui::End();
		}
	});

	app.run();

	for(auto & model: models)
	{
		delete model.second;
	}

	for(auto & material: materials)
	{
		delete material;
	}

	return (0);
}

void object_builder(nle::Object3D **o, const nlohmann::json& j, std::map<std::string, nle::Model*>& models)
{
	int type = j["type"];
	switch (type)
	{
		case 1:
		{
			nle::Scene * h = new nle::Scene();
			h->from_json(j);
			(*o) = h;
			break;
		}
		case 2:
		{
			nle::Camera * h = new nle::Camera();
			h->from_json(j);
			(*o) = h;
			break;
		}
		case 3:
		{
			nle::Light * h = new nle::Light();
			h->from_json(j);
			(*o) = h;
			break;
		}
		case 4:
		{
			std::string source = j["source"];
			if(models.find(source) != models.end())
			{
				nle::MultiMeshInstance *h = models[source]->create_instance();
				h->from_json(j);
				// nle::MultiMeshInstance * h; (h = dynamic_cast<nle::MultiMeshInstance*>(o))->from_json(j);
				(*o) = h;				
			}
			else
			{
				prerr("model %s not found", source.c_str());
			}
			break;
		}
	}

	if(*o) // can be null if it's a multimesh instance, and model not found.
	{
		if(j.find("children") != j.end())
		{
			for(const auto& it : j["children"])
			{
				nle::Object3D * c = nullptr;
				object_builder(&c, it, models);
				if(c)
				{
					(*o)->add_child(c);
				}
			}
		}
	}
}