#ifndef NLE_MODEL_H
#define NLE_MODEL_H

#include "Nle.h"
#include "Mesh.h"
#include "Texture.h"

namespace nle
{
    enum class ModelFormat
    {
        OBJ,
        GLTF
    };
    class Model
    {
    private:
        std::string m_name;
        MultiMesh *m_multimesh;
        Shader *m_shader = nullptr;
        Texture *m_texture = nullptr;
        std::vector<Texture*> m_textures;
        std::vector<Material*> m_materials;

    public:
        Model(const std::string& path, Shader *shader, Texture *texture = nullptr);
        ~Model();
        void set_name(const std::string& name);
        const std::string& name() const;
        MultiMeshInstance *create_instance();
        bool load_obj(const std::string &path);
        bool load_gltf(const std::string &path);
        MultiMesh *multimesh() const;
    };
} // namespace nle

#endif