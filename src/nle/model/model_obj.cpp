#include "model_obj.h"

#include "../../../vendor/OBJ_Loader.h"

#include <algorithm>
#include <filesystem>

namespace nle
{
    model_obj::model_obj(const std::string& path)
    {
        std::string fmt = path.substr(path.find_last_of("."));
        m_name = path.substr(path.find_last_of('/') + 1);

        std::transform(fmt.begin(), fmt.end(), fmt.begin(),
                       [](unsigned char c)
                       { return std::tolower(c); });

        if (fmt == ".obj")
        {
            load(path);
        }
    }

    model_obj::~model_obj()
    {
    }

    ref<multimesh_instance_3d> model_obj::create_instance()
    {
        auto ret = make_ref<multimesh_instance_3d>(m_multimesh_3d);
        
        return ret;
    }

    std::string model_obj::name() const
    {
        return m_name;
    }

    void model_obj::load(const std::string &path)
    {
        objl::Loader loader;

        if (!loader.LoadFile(path))
        {
            return;
        }

        m_multimesh_3d = make_ref<class multimesh_3d>();

        for(const auto& it: loader.LoadedMeshes)
        {
            std::vector<struct vertex> vertices;
            std::vector<uint32_t> indices;

            for (const auto &v : it.Vertices)
            {
                vertex vert;
                
                vert.position.x = v.Position.X;
                vert.position.y = v.Position.Y;
                vert.position.z = v.Position.Z;

                vert.normal.x = v.Normal.X;
                vert.normal.y = v.Normal.Y;
                vert.normal.z = v.Normal.Z;

                vert.color.x = it.MeshMaterial.Kd.X;
                vert.color.y = it.MeshMaterial.Kd.Y;
                vert.color.z = it.MeshMaterial.Kd.Z;

                vert.uv.x = v.TextureCoordinate.X;
                vert.uv.y = v.TextureCoordinate.Y;

                vertices.push_back(vert);
            }

            for (const auto &i : it.Indices)
            {
                indices.push_back(i);
            }

            auto mesh = make_ref<class mesh_3d>(vertices, indices /** textures later */);
            m_multimesh_3d->meshes().push_back(mesh);

            auto material = make_ref<class material>();
            material->set_ambient({it.MeshMaterial.Ka.X, it.MeshMaterial.Ka.Y, it.MeshMaterial.Ka.Z});
            material->set_diffuse({it.MeshMaterial.Kd.X, it.MeshMaterial.Kd.Y, it.MeshMaterial.Kd.Z});
            material->set_specular({it.MeshMaterial.Ks.X, it.MeshMaterial.Ks.Y, it.MeshMaterial.Ks.Z});
            material->set_shininess(it.MeshMaterial.Ns);
            material->set_dissolve(it.MeshMaterial.d);
            material->set_id(it.MeshMaterial.name);

            mesh->set_material(material);
            m_multimesh_3d->meshes().push_back(mesh);
        }
    }
} // namespace nle
