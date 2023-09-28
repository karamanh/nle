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

#include "Terrain.h"

std::string
	nle_proj_extension = ".nleproj",
	nle_scene_extension = ".nlescn",
	nle_workdir = ".",
	nle_projdir = nle_workdir + "/nle_projdir";

void scene_builder(nle::Object3D **o, const nlohmann::json &j, std::map<std::string, nle::Model *> &models);

int main(int argc, char *argv[])
{
	nle::Nle app;

	std::map<std::string, nle::Scene *> scenes;
	std::map<std::string, nle::Model *> models;
	std::vector<nle::Material *> materials;
	std::vector<nle::Mesh *> aabb_meshes;
	std::vector<std::string> logs;
	std::size_t max_logs = 100;

	auto push_log = [&](const std::string &msg)
	{
		prinf("%s", msg.c_str());
		logs.push_back(msg);
		if (logs.size() > max_logs)
		{
			logs.erase(logs.begin());
		}
	};

	prinf("autoloading assets");
	for (auto const &dir_entry : std::filesystem::recursive_directory_iterator(std::filesystem::current_path()))
	{
		std::string path = dir_entry.path().c_str();
		std::string name = dir_entry.path().filename();
		std::size_t n;

		if (dir_entry.path().filename().extension() == ".obj")
		{
			prdbg("loading %s", dir_entry.path().c_str());

			models[name] = new nle::Model(path, nle::globals::DEFAULT_SHADER);
		}
	}
	prinf("%lu models loaded", models.size());

	app.current_scene()->set_id("root");
	app.current_scene()->camera()->set_rotation({-30.f, 135.f, 0.f});
	app.current_scene()->camera()->set_position({5.f, 5.f, -5.f});
	app.current_scene()->light()->set_rotation({180.f, 0.f, 0.f});
	app.current_scene()->light()->set_enabled(true);

	nle::Terrain terrain(1024, 1024);
	auto * terrain_instance = terrain.create_instance();
	auto terrain_offset = glm::vec3(-512.0f, 0.0f, -512.0f);
	terrain_instance->set_position(terrain_offset);

	for(int i = 0; i < 512; i++)
	{
		auto * tree = models["Tree1.obj"]->create_instance();
		tree->set_position(glm::sphericalRand<float>(512.0f) - terrain_offset);
		tree->set_rotation(glm::vec3(0.0f, glm::circularRand(512.0f).y, 0.0f));
		terrain_instance->add_child(tree);
	}

	for(int i = 0; i < 512; i++)
	{
		auto * tree = models["Grass1.obj"]->create_instance();
		tree->set_position(glm::sphericalRand<float>(512.0f) - terrain_offset);
		terrain_instance->add_child(tree);
	}

	app.current_scene()->add_child(terrain_instance);

	app.renderer()->gui()->set_draw_callback([&]()
											 {
		ImGuiIO& io = ImGui::GetIO();
		
		glm::vec3 v3val;
		float fval;
		bool bval;
		int ival;

		static float bottom_window_h = 300.f;
		static bool show_load_model = false, 
					show_load_scene = false,
					show_about = false,
					show_settings = false;
		static char inbuf[512] = {0};
		static nle::RenderObject3D * selected_obj = nullptr;
		// ImVec2 menubar_size, control_panel_size, assets_size, scene_size, bottom_window_size, object_properties_size = {300.f, 0.f}, display_size = io.DisplaySize;
		ImVec2 menubar_size, left_window_size, right_window_size = {300.f, 0.f}, display_size = io.DisplaySize;

		app.renderer()->object_intersects().bind_callback([&](nle::MultiMeshInstance*mmi){
			if(!io.WantCaptureMouse)
			{
				selected_obj = mmi;
			}
		});

        if(ImGui::BeginMainMenuBar())
        {
            if(ImGui::BeginMenu("file"))
            {
				show_load_model = ImGui::MenuItem("load model");
				ImGui::Separator();

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

				show_load_scene = ImGui::MenuItem("load scene");

				ImGui::Separator();

                if(ImGui::MenuItem("quit"))
                {
					app.window()->close();
                }
                ImGui::EndMenu();
            }
            if(ImGui::BeginMenu("settings"))
            {
				if(ImGui::BeginMenu("window"))
				{
					if(ImGui::MenuItem(app.window()->fullscreen() ? "windowed" : "fullscreen"))
					{
						app.window()->set_fullscreen(!app.window()->fullscreen());
					}
					ImGui::EndMenu();
				}
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
							auto * m = new nle::Model(path, nle::globals::DEFAULT_SHADER);
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
								scene_builder(reinterpret_cast<nle::Object3D**>(&new_scene), j, models);

								if(new_scene)
								{
									new_scene->set_id(std::filesystem::path(path).filename().filename());
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
				ImGui::TextWrapped("nice little engine");
				ImGui::Separator();
				ImGui::TextWrapped("https://github.com/grizzlei/nle");
				ImGui::TextWrapped("experimental opengl renderer");
				ImGui::TextWrapped("hasan karaman - hk@hasankaraman.dev - 2023");
				show_about = !ImGui::Button("close");
				ImGui::End();
			}
		}

		if(show_settings)
		{
			ImGui::SetNextWindowPos(ImVec2(display_size.x * 0.5f, display_size.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f,0.5f));
			if (ImGui::Begin("settings & preferences"))
			{
				if(ImGui::Button(app.window()->fullscreen() ? "windowed" : "fullscreen"))
				{
					app.window()->set_fullscreen(!app.window()->fullscreen());
				}

				ImGui::End();
			}
		}

		ImGui::SetNextWindowPos({0, menubar_size.y});
		ImGui::SetNextWindowSize({left_window_size.x, display_size.y - menubar_size.y});
		if (ImGui::Begin("left_window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove))
		{
			if(ImGui::BeginTabBar("general controls", ImGuiTabBarFlags_Reorderable))
			{
				if (ImGui::BeginTabItem(("scene [" + app.current_scene()->id() + "]").c_str()))
				{
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

					if(ImGui::CollapsingHeader("camera transform"))
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

					// light settings
					ImGui::TextWrapped("light settings:");
					ImGui::Text("ambient");
					ImGui::PushID("ambient_intensity");
					v3val = app.current_scene()->light()->ambient();
					ImGui::SetNextItemWidth(60.f);
					ImGui::InputFloat("x", &v3val.x);
					ImGui::SameLine();
					ImGui::SetNextItemWidth(60.f);
					ImGui::InputFloat("y", &v3val.y);
					ImGui::SameLine();
					ImGui::SetNextItemWidth(60.f);
					ImGui::InputFloat("z", &v3val.z);
					app.current_scene()->light()->set_ambient(v3val);
					ImGui::PopID();

					ImGui::Text("diffuse");
					ImGui::PushID("diffuse_intensity");
					v3val = app.current_scene()->light()->diffuse();
					ImGui::SetNextItemWidth(60.f);
					ImGui::InputFloat("x", &v3val.x);
					ImGui::SameLine();
					ImGui::SetNextItemWidth(60.f);
					ImGui::InputFloat("y", &v3val.y);
					ImGui::SameLine();
					ImGui::SetNextItemWidth(60.f);
					ImGui::InputFloat("z", &v3val.z);
					app.current_scene()->light()->set_diffuse(v3val);
					ImGui::PopID();

					ImGui::Text("specular");
					ImGui::PushID("specular_intensity");
					v3val = app.current_scene()->light()->specular();
					ImGui::SetNextItemWidth(60.f);
					ImGui::InputFloat("x", &v3val.x);
					ImGui::SameLine();
					ImGui::SetNextItemWidth(60.f);
					ImGui::InputFloat("y", &v3val.y);
					ImGui::SameLine();
					ImGui::SetNextItemWidth(60.f);
					ImGui::InputFloat("z", &v3val.z);
					app.current_scene()->light()->set_specular(v3val);
					ImGui::PopID();

					if(ImGui::CollapsingHeader("light transform"))
					{
						ImGui::PushID("light_position");
						v3val = app.current_scene()->light()->position();
						ImGui::SetNextItemWidth(60.f);
						ImGui::InputFloat("x", &v3val.x);
						ImGui::SameLine();
						ImGui::SetNextItemWidth(60.f);
						ImGui::InputFloat("y", &v3val.y);
						ImGui::SameLine();
						ImGui::SetNextItemWidth(60.f);
						ImGui::InputFloat("z", &v3val.z);
						app.current_scene()->light()->set_position(v3val);
						ImGui::PopID();
						
						ImGui::PushID("light_rotation");
						v3val = app.current_scene()->light()->rotation();
						ImGui::SetNextItemWidth(60.f);
						ImGui::InputFloat("x", &v3val.x);
						ImGui::SameLine();
						ImGui::SetNextItemWidth(60.f);
						ImGui::InputFloat("y", &v3val.y);
						ImGui::SameLine();
						ImGui::SetNextItemWidth(60.f);
						ImGui::InputFloat("z", &v3val.z);
						app.current_scene()->light()->set_rotation(v3val);
						ImGui::PopID();
						
						ImGui::PushID("light_scale");
						v3val = app.current_scene()->light()->scale();
						ImGui::SetNextItemWidth(60.f);
						ImGui::InputFloat("x", &v3val.x);
						ImGui::SameLine();
						ImGui::SetNextItemWidth(60.f);
						ImGui::InputFloat("y", &v3val.y);
						ImGui::SameLine();
						ImGui::SetNextItemWidth(60.f);
						ImGui::InputFloat("z", &v3val.z);
						app.current_scene()->light()->set_scale(v3val);
						ImGui::PopID();
					}

					ImGui::Separator();

					std::function<void(nle::Object3D*)> generate_tree = [&](nle::Object3D* o){

						if(ImGui::TreeNodeEx(o->id().c_str()))
						{
							for(auto *i : o->children())
							{
								generate_tree(i);
							}

							nle::MultiMeshInstance *mi = dynamic_cast<nle::MultiMeshInstance*>(o);
							if(mi)
							{
								if(selected_obj != mi)
								{
									if(ImGui::Button("select"))
									{
										selected_obj = mi;
									}
								}
								else
								{
									if(ImGui::Button("deselect"))
									{
										selected_obj = nullptr;
									}
								}
								ImGui::SameLine();
								if(ImGui::Button("delete"))
								{
									o->parent()->delete_child(o);
									selected_obj = nullptr;
								}
							}

							ImGui::TreePop();
						}
					};

					generate_tree(app.current_scene());
					ImGui::EndTabItem();
				}

				if(ImGui::BeginTabItem("loaded models"))
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
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("renderer settings"))
				{
					auto rla = app.renderer()->render_layer_attributes();
					ImGui::SliderInt("render distance", &rla.render_distance, 0, 100000);
					ImGui::Checkbox("render layer [0] visible", &rla.visible);
					app.renderer()->set_render_layer_attributes(nle::RenderLayer::_0, rla);
					
					// ImGui::Separator();
					
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}

			ImGui::End();
		}

		// object manager window
		ImGui::SetNextWindowPos(ImVec2(display_size.x -right_window_size.x, menubar_size.y));
		ImGui::SetNextWindowSize(ImVec2(right_window_size.x, (display_size.y - menubar_size.y)*0.66));
		if(ImGui::Begin("object properties", nullptr, ImGuiWindowFlags_NoMove))
		{
			right_window_size = ImGui::GetWindowSize();

			if(selected_obj)
			{
				char idbuf[64] = {0};
				strncpy(idbuf, selected_obj->id().c_str(), sizeof(idbuf) - 1);
				if(ImGui::InputText("id", idbuf, sizeof(idbuf) -1))
				{
					if(strlen(idbuf) > 0)
						selected_obj->set_id(idbuf);
				}

				bval = selected_obj->mouse_pickable();
				ImGui::Checkbox("mouse pickable", &bval);
				selected_obj->set_mouse_pickable(bval);
				
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
					nle::MultiMeshInstance *mmi;
					if((mmi = dynamic_cast<nle::MultiMeshInstance*>(selected_obj)))
					{
						nle::Material *material;
						if((material = mmi->material()))
						{
							ImGui::Text("ambient");
							ImGui::PushID("ambient_intensity");
							v3val = material->ambient();
							ImGui::SetNextItemWidth(60.f);
							ImGui::InputFloat("x", &v3val.x);
							ImGui::SameLine();
							ImGui::SetNextItemWidth(60.f);
							ImGui::InputFloat("y", &v3val.y);
							ImGui::SameLine();
							ImGui::SetNextItemWidth(60.f);
							ImGui::InputFloat("z", &v3val.z);
							material->set_ambient(v3val);
							ImGui::PopID();

							ImGui::Text("diffuse");
							ImGui::PushID("diffuse_intensity");
							v3val = material->diffuse();
							ImGui::SetNextItemWidth(60.f);
							ImGui::InputFloat("x", &v3val.x);
							ImGui::SameLine();
							ImGui::SetNextItemWidth(60.f);
							ImGui::InputFloat("y", &v3val.y);
							ImGui::SameLine();
							ImGui::SetNextItemWidth(60.f);
							ImGui::InputFloat("z", &v3val.z);
							material->set_diffuse(v3val);
							ImGui::PopID();

							ImGui::Text("specular");
							ImGui::PushID("specular_intensity");
							v3val = material->specular();
							ImGui::SetNextItemWidth(60.f);
							ImGui::InputFloat("x", &v3val.x);
							ImGui::SameLine();
							ImGui::SetNextItemWidth(60.f);
							ImGui::InputFloat("y", &v3val.y);
							ImGui::SameLine();
							ImGui::SetNextItemWidth(60.f);
							ImGui::InputFloat("z", &v3val.z);
							material->set_specular(v3val);
							ImGui::PopID();

							ImGui::PushID("shininess");
							fval = material->shininess();
							ImGui::InputFloat("shininess", &fval);
							material->set_shininess(fval);
							ImGui::PopID();
							ImGui::PushID("accept_light");
							bval = material->accept_light();
							ImGui::Checkbox("accept light", &bval);
							material->set_accept_light(bval);
							ImGui::PopID();

							if(ImGui::Button("clear material"))
							{
								mmi->set_material(nullptr);
							}
						}
						else
						{
							if(ImGui::Button("new material"))
							{
								materials.push_back(new nle::Material());
								mmi->set_material(materials.back());
							}
						}
					}
				}
				
				ImGui::Separator();

				auto * po = dynamic_cast<nle::PhysicsObject3D*>(selected_obj->parent());
				if(po)
				{
					if(ImGui::CollapsingHeader("physics settings", ImGuiTreeNodeFlags_DefaultOpen))
					{
						if(ImGui::Button(po->physics_enabled() ? "disable physics" : "enable physics"))
						{
							if(po->physics_enabled())
							{
								push_log("disabled physics for " + selected_obj->id());
								app.physics_engine()->detach_physics_body(po);
							}
							else
							{
								push_log("enabled physics for " + selected_obj->id());
								app.physics_engine()->attach_physics_body(po);
							}
						}

						bval = po->rigid();
						ImGui::Checkbox("rigid", &bval);
						po->set_rigid(bval);
					}

					po->set_position(selected_obj->position());
					po->set_rotation(selected_obj->rotation());
					po->set_scale(selected_obj->scale());
				}
				else
				{
					if(ImGui::Button("wrap by 3d physics object"))
					{
						app.current_scene()->delete_child(selected_obj, false);
						auto * po = new nle::PhysicsObject3D();

						po->add_child(selected_obj);
						app.current_scene()->add_child(po);
						app.current_scene()->register_render_object(selected_obj);
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

		for(auto * i : app.current_scene()->render_objects())
		{
			// i->set_render_mode(selected_obj == i ? nle::Line : nle::Fill);
			auto * mmi = dynamic_cast<nle::MultiMeshInstance*>(i);
			if(mmi)
			{
				mmi->set_render_mode(selected_obj == mmi ? nle::Line : nle::Fill);
			}
		}

		// current scene window
		ImGui::SetNextWindowPos({display_size.x - right_window_size.x, menubar_size.y + right_window_size.y});
		ImGui::SetNextWindowSize({right_window_size.x, display_size.y - right_window_size.y - menubar_size.y});
		if(ImGui::Begin("logs", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{
			for(auto & it: logs)
			{
				ImGui::TextWrapped(it.c_str());
			}
			ImGui::End();
		} });

	app.run();

	for (auto &model : models)
	{
		delete model.second;
	}

	for (auto &material : materials)
	{
		delete material;
	}

	for (auto &aabb_mesh : aabb_meshes)
	{
		delete aabb_mesh;
	}

	return (0);
}

void scene_builder(nle::Object3D **o, const nlohmann::json &j, std::map<std::string, nle::Model *> &models)
{
	int type = j["type"];
	switch (type)
	{
	case static_cast<int>(nle::ObjectType::Scene):
	{
		nle::Scene *h = new nle::Scene();
		h->from_json(j);
		(*o) = h;
		break;
	}
	case static_cast<int>(nle::ObjectType::Camera):
	{
		nle::Camera *h = new nle::Camera();
		h->from_json(j);
		(*o) = h;
		break;
	}
	case static_cast<int>(nle::ObjectType::Light):
	{
		nle::Light *h = new nle::Light();
		h->from_json(j);
		(*o) = h;
		break;
	}
	case static_cast<int>(nle::ObjectType::MultiMeshInstance):
	{
		std::string source = j["source"];
		if (models.find(source) != models.end())
		{
			nle::MultiMeshInstance *h = models[source]->create_instance();
			h->from_json(j);
			(*o) = h;
		}
		else
		{
			prerr("model %s not found", source.c_str());
		}
		break;
	}
	case static_cast<int>(nle::ObjectType::PhysicsObject3D):
	{
		nle::PhysicsObject3D *h = new nle::PhysicsObject3D();
		h->from_json(j);
		(*o) = h;
		break;
	}
	}

	if (*o) // can be null if it's a multimesh instance, and model not found.
	{
		if (j.find("children") != j.end())
		{
			for (const auto &it : j["children"])
			{
				nle::Object3D *c = nullptr;
				scene_builder(&c, it, models);
				if (c)
				{
					(*o)->add_child(c);
				}
			}
		}
	}
}